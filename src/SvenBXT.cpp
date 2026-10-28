//
// Created by scriptedsnark on 7/18/24.
//

#include "SvenBXT.h"
#include <chrono>

#ifdef PLATFORM_WINDOWS
Utils utils = Utils::Utils(NULL, NULL, NULL);
Utils sv_utils = Utils::Utils(NULL, NULL, NULL);
#endif

funchook_t* g_lpFuncHook_Server;
funchook_t* g_lpFuncHook_Engine;
funchook_t* g_lpFuncHook_Client;

dllhandle_t g_lpEngine;
dllhandle_t g_lpServer;
dllhandle_t g_lpClient;

cl_enginefunc_t* g_lpEngfuncs;
enginefuncs_t* g_engfuncs;
globalvars_t* gpGlobals;
server_t* sv;

bool g_bHasLoaded = false;

void SvenBXT_HookClient();

void SvenBXT_FindEngineStuff()
{
	TRACE("Finding engine stuff...\n");

	g_lpFuncHook_Engine = funchook_create();

	int status;
#ifdef PLATFORM_LINUX
	g_lpEngfuncs = (cl_enginefunc_t*)Sys_GetProcAddress(g_lpEngine, "cl_enginefuncs");

	if (!g_lpEngfuncs)
	{
		Sys_Printf("[hw so] Failed to get \"cl_enginefuncs\".\n");
		return;
	}
	else
	{
		Sys_Printf("[hw so] Found cl_enginefuncs at %p.\n", g_lpEngfuncs);
	}

	// TODO: use GET_VARIABLE define
	enginefuncs_t** p_engfuncs = (enginefuncs_t**)Sys_GetProcAddress(g_lpEngine, "g_pengfuncsExportedToDlls");
	g_engfuncs = p_engfuncs ? *p_engfuncs : nullptr;
	gpGlobals = (globalvars_t*)Sys_GetProcAddress(g_lpEngine, "gGlobalVariables");
	sv = (server_t*)Sys_GetProcAddress(g_lpEngine, "sv");

	if (!gpGlobals)
	{
		Sys_Printf("[hw so] Failed to get \"gGlobalVariables\".\n");
	}

	if (!g_engfuncs)
	{
		Sys_Printf("[hw so] Failed to get \"g_engfuncsExportedToDlls\".\n[Engine] Sharing time to clients is not available.\n");
	}

	if (!sv)
	{
		Sys_Printf("[hw so] Failed to get \"sv\".\n");
	}

	GET_VARIABLE(Engine, ORIG_GL_Begin2D, _Z10GL_Begin2Dv);
	GET_VARIABLE(Engine, ORIG_GL_Finish2D, _Z11GL_Finish2Dv);

	GET_VARIABLE(Engine, ORIG_LoadThisDll, _Z11LoadThisDllPc);
	GET_VARIABLE(Engine, ORIG_SCR_BeginLoadingPlaque, _Z22SCR_BeginLoadingPlaquei);
	GET_VARIABLE(Engine, ORIG_SCR_EndLoadingPlaque, _Z20SCR_EndLoadingPlaquev);
	GET_VARIABLE(Engine, ORIG_GL_EndRendering, _Z15GL_EndRenderingv);
#else
	void* handle = nullptr;
	void* base = nullptr;
	size_t size = 0;

	if (!MemUtils::GetModuleInfo(L"hw.dll", &handle, &base, &size))
	{
		Sys_Printf("[Engine] Could not inspect hw.dll, leaving hooks disabled.\n");
		return;
	}

	utils = Utils(handle, base, size);

	// Resolve all engine addresses synchronously before any hooks can run.
	// Hardcoded offsets are valid only for the matching recognized signatures.
	void* clientInit = nullptr;
	auto clientPattern = utils.FindSync(clientInit, patterns::engine::ClientDLL_Init);
	if (clientInit && clientPattern != patterns::engine::ClientDLL_Init.cend())
	{
		const auto index = clientPattern - patterns::engine::ClientDLL_Init.cbegin();
		uintptr_t offset = 0;
		switch (index) {
		case 0: case 1: offset = 1; break; // 5.11 / 5.23
		case 2: offset = 332; break;       // 5.25
		case 3: offset = 354; break;       // 5.26-rc1 signature
		default: break;
		}
		if (offset && reinterpret_cast<uintptr_t>(clientInit) + offset + sizeof(void*) <=
		              reinterpret_cast<uintptr_t>(base) + size)
		{
			g_lpEngfuncs = *reinterpret_cast<cl_enginefunc_t**>(reinterpret_cast<uintptr_t>(clientInit) + offset);
		}
	}
	if (!g_lpEngfuncs)
		Sys_Printf("[Engine] No known client engine signature; client hooks disabled.\n");

	void* loadThisDll = nullptr;
	auto loadPattern = utils.FindSync(loadThisDll, patterns::engine::LoadThisDll);
	if (loadThisDll && loadPattern != patterns::engine::LoadThisDll.cend())
	{
		const auto index = loadPattern - patterns::engine::LoadThisDll.cbegin();
		if (index == 0) {
			g_engfuncs = *reinterpret_cast<enginefuncs_t**>(reinterpret_cast<uintptr_t>(loadThisDll) + 91);
			gpGlobals = *reinterpret_cast<globalvars_t**>(reinterpret_cast<uintptr_t>(loadThisDll) + 86);
		} else if (index == 1) {
			auto indirect = *reinterpret_cast<enginefuncs_t***>(reinterpret_cast<uintptr_t>(loadThisDll) + 109);
			g_engfuncs = indirect ? *indirect : nullptr;
			gpGlobals = *reinterpret_cast<globalvars_t**>(reinterpret_cast<uintptr_t>(loadThisDll) + 67);
		}
	}
	if (!g_engfuncs)
		Sys_Printf("[Engine] No matching server engine-functions signature; server features disabled.\n");

	void* hostClearMemory = nullptr;
	auto clearPattern = utils.FindSync(hostClearMemory, patterns::engine::Host_ClearMemory);
	if (hostClearMemory && clearPattern != patterns::engine::Host_ClearMemory.cend())
	{
		const auto index = clearPattern - patterns::engine::Host_ClearMemory.cbegin();
		if (index == 0)
			sv = *reinterpret_cast<server_t**>(reinterpret_cast<uintptr_t>(hostClearMemory) + 0x5B);
		else if (index == 1)
			sv = *reinterpret_cast<server_t**>(reinterpret_cast<uintptr_t>(hostClearMemory) + 0x98);
	}
	if (!sv)
		Sys_Printf("[Engine] Server state not found; host-only features disabled.\n");

	SPTEngineFind(LoadThisDll);
	SPTEngineFind(SCR_BeginLoadingPlaque);
	SPTEngineFind(SCR_EndLoadingPlaque);
	SPTEngineFind(GL_EndRendering);
	SPTEngineFind(GL_Finish2D);
	SPTEngineFind(GL_Begin2D);
#endif

	CreateHook(Engine, LoadThisDll);
	CreateHook(Engine, SCR_BeginLoadingPlaque);
	CreateHook(Engine, SCR_EndLoadingPlaque);
	// The rendering callback calls both GL helpers and the client engine table.
	if (ORIG_GL_Begin2D && ORIG_GL_Finish2D && g_lpEngfuncs)
		CreateHook(Engine, GL_EndRendering);
	else
		Sys_Printf("[Engine] Skipping GL_EndRendering: rendering dependencies unavailable.\n");

	if (g_lpFuncHook_Engine && funchook_install(g_lpFuncHook_Engine, 0) != FUNCHOOK_ERROR_SUCCESS)
		Sys_Printf("[Engine] Failed to install hooks.\n");

	if (g_lpEngfuncs)
		SvenBXT_HookClient();
}

void SvenBXT_HookClient()
{
	if (!g_lpEngfuncs || g_lpFuncHook_Client)
		return;
	TRACE("Hooking client...\n");
#ifdef PLATFORM_WINDOWS
	g_lpClient = Sys_GetModuleHandle("client");
#else
	g_lpClient = Sys_GetModuleHandle("svencoop/cl_dlls/client.so");
#endif

	if (!g_lpClient)
	{
		Sys_Printf("Failed to get client module handle.\n");
		return;
	}

	g_lpFuncHook_Client = funchook_create();
	if (!g_lpFuncHook_Client) {
		Sys_Printf("[Client] Could not allocate hook manager.\n");
		return;
	}
	CL_Initialize();
	if (funchook_install(g_lpFuncHook_Client, 0) != FUNCHOOK_ERROR_SUCCESS)
		Sys_Printf("[Client] Failed to install hooks.\n");
}

void SvenBXT_HookEngine()
{
	g_lpEngine = Sys_GetModuleHandle("hw" DLL_FORMAT);

	if (!g_lpEngine)
	{
		Sys_Printf("Failed to get engine module handle.\n");
		return;
	}

	SvenBXT_FindEngineStuff();
}

void SvenBXT_UnhookClient()
{
	if (g_lpFuncHook_Client)
	{
		funchook_uninstall(g_lpFuncHook_Client, 0);
		funchook_destroy(g_lpFuncHook_Client);
		g_lpFuncHook_Client = nullptr;
	}
}

void SvenBXT_UnhookEngine()
{
	if (g_lpFuncHook_Engine)
	{
		funchook_uninstall(g_lpFuncHook_Engine, 0);
		funchook_destroy(g_lpFuncHook_Engine);
		g_lpFuncHook_Engine = nullptr;
	}
}

//-----------------------------------------------------------------------------
// Purpose: hook in right time a.k.a I don't wanna hook LoadLibraryA/dlopen
//-----------------------------------------------------------------------------
void WaitUntilClientLoads()
{
lbl_waitFor:
#ifdef PLATFORM_WINDOWS
	g_lpClient = Sys_GetModuleHandle("client");
#else
	g_lpClient = Sys_GetModuleHandle("svencoop/cl_dlls/client" DLL_FORMAT);
#endif

	if (g_lpClient)
	{
		SvenBXT_HookEngine();
		g_bHasLoaded = true;
	}
	else {
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		goto lbl_waitFor;
	}
}

void SvenBXT_Main()
{
	if (g_bHasLoaded)
	{
		Sys_Printf("Loader/injector or whatever else tried to initialize SvenBXT again!");
		return;
	}

	// too lazy to do this using fopen
	std::ofstream ofs;
	ofs.open("svenbxt.log", std::ofstream::out | std::ofstream::trunc);
	ofs.close();

	TRACE("Initializing SvenBXT...\n");

	std::thread t(WaitUntilClientLoads);
	t.detach();
}

void SvenBXT_Shutdown()
{
	TRACE("Shutting down...\n");

	if (g_lpFuncHook_Server) {
		funchook_uninstall(g_lpFuncHook_Server, 0);
		funchook_destroy(g_lpFuncHook_Server);
		g_lpFuncHook_Server = nullptr;
	}
	SvenBXT_UnhookEngine();
	SvenBXT_UnhookClient();

	g_bHasLoaded = false;
}

#ifdef WIN32
static DWORD WINAPI SvenBXT_Bootstrap(LPVOID)
{
	SvenBXT_Main();
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
#ifdef _DEBUG
		AllocConsole();
		FILE *in, *out;
		freopen_s(&in, "conin$", "r", stdin);
		freopen_s(&out, "conout$", "w+", stdout);
#endif
		if (HANDLE worker = CreateThread(NULL, 0, SvenBXT_Bootstrap, NULL, 0, NULL))
			CloseHandle(worker);
		break;

	case DLL_PROCESS_DETACH:
		SvenBXT_Shutdown();
		break;
	}

	return TRUE;
}
#else
static __attribute__((constructor)) void Construct()
{
	SvenBXT_Main();
}

static __attribute__((destructor)) void Destruct()
{
	SvenBXT_Shutdown();
}
#endif
