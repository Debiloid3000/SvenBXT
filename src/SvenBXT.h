#ifdef SVENBXT_H_RECURSE_GUARD
#error Recursive header files inclusion detected in SvenBXT.h
#else // SVENBXT_H_RECURSE_GUARD

#define SVENBXT_H_RECURSE_GUARD

#ifndef SVENBXT_H_GUARD
#define SVENBXT_H_GUARD
#pragma once

#ifdef __cplusplus

// GLOBAL DEFINE
#define SVENBXT_VERSION __DATE__ // maybe something else? :thinking:
#define SVENBXT_GITHUB_URL "https://github.com/ScriptedSnark/SvenBXT"

// WINDOWS
#ifdef PLATFORM_WINDOWS
#include <windows.h>
#include <DbgHelp.h>
#else
#include <dlfcn.h>
#include <stdarg.h>
#include <cstring>
#endif

// STL
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <functional>
#include <array>
#include <random>
#include <fstream>
#include <thread>

// GL
#include <GL/gl.h>

#include "external/funchook/include/funchook.h"

#ifdef PLATFORM_WINDOWS
#include "iface.hpp"

#include "external/SPTLib/Utils.hpp"
#include "engine_patterns.hpp"
extern Utils utils;
extern Utils sv_utils;
#endif

#include <stdio.h>
#include "hlsdk_mini.hpp"
#include "engine/sound.h"
#include "engine/server.h"

// OWN STUFF
#include "Utils.h"
#include "CTimer.h"

extern funchook_t* g_lpFuncHook_Server;
extern funchook_t* g_lpFuncHook_Client;
extern dllhandle_t g_lpEngine;
extern dllhandle_t g_lpServer;
extern dllhandle_t g_lpClient;
extern cl_enginefunc_t* g_lpEngfuncs;
extern enginefuncs_t* g_engfuncs;
extern globalvars_t* gpGlobals;
extern server_t* sv;

#include "cl_dll/cdll_int.h"
#include "cl_dll/hud.h"
#include "cl_dll/parsemsg.h"
#include "cl_dll/view.h"
#include "cl_dll/tri.h"
#include "cl_dll/input.h"
#include "cl_dll/opengl_utils.hpp"

#include "engine/gl_screen.h"

#include "dlls/enginecallback.h"
#include "dlls/server.h"

// HUD
#include "cl_dll/hud_speedometer.h"
#include "cl_dll/hud_viewangles.h"
#include "cl_dll/hud_origin.h"
#include "cl_dll/hud_timer.h"
#include "cl_dll/hud_jumpspeed.h"
#include "cl_dll/hud_crosshair.h"

#ifdef PLATFORM_WINDOWS
#define FASTCALL __fastcall
#elif defined(PLATFORM_LINUX)
#define FASTCALL
#else
#error Unsupported platform
#endif

#define GET_VARIABLE(tag, resultVar, procName)								   \
	resultVar = (decltype(resultVar))Sys_GetProcAddress(g_lp##tag, #procName); \
	if (!resultVar)                                                            \
		Sys_Printf("[" #tag "] Failed to get \"" #procName "\".\n");            \
	else                                                                       \
		Sys_Printf("[" #tag "] Found " #procName " at %p.\n", resultVar)      \

#define Find(lib, func_name)                                                                                  \
	if ((ORIG_##func_name = reinterpret_cast<_##func_name>(Sys_GetProcAddress(g_lp##lib, "" #func_name "")))) \
		Sys_Printf("[" #lib "] Found " #func_name " at %p.\n", ORIG_##func_name);                             \
	else                                                                                                      \
		Sys_Printf("[" #lib "] Could not find " #func_name ".\n")


#ifdef PLATFORM_WINDOWS
// A missing signature is normal after an engine update. Do not dereference end().
#define SPTEngineFind(func_name) do { \
	auto match = utils.FindSync(ORIG_##func_name, patterns::engine::func_name); \
	if (match != patterns::engine::func_name.cend() && ORIG_##func_name) \
		Sys_Printf("[Engine] Found " #func_name " at %p (%s).\\n", ORIG_##func_name, match->name()); \
	else { ORIG_##func_name = nullptr; Sys_Printf("[Engine] " #func_name " unavailable, skipping hook.\\n"); } \
} while (0)

#define SPTServerFind(func_name) do { \
	auto match = sv_utils.FindSync(ORIG_##func_name, patterns::server::func_name); \
	if (match != patterns::server::func_name.cend() && ORIG_##func_name) \
		Sys_Printf("[Server] Found " #func_name " at %p (%s).\\n", ORIG_##func_name, match->name()); \
	else { ORIG_##func_name = nullptr; Sys_Printf("[Server] " #func_name " unavailable, skipping hook.\\n"); } \
} while (0)
#endif

// Never pass a null hook handle or a failed signature scan to funchook.
#define CreateHook(lib, func_name) do { \
	if (!g_lpFuncHook_##lib || !ORIG_##func_name) { \
		Sys_Printf("[" #lib "] Skipping " #func_name " (no hook handle or target).\\n"); \
	} else { \
		status = funchook_prepare(g_lpFuncHook_##lib, (void**)&ORIG_##func_name, (void*)HOOKED_##func_name); \
		if (status == FUNCHOOK_ERROR_SUCCESS) \
			Sys_Printf("[" #lib "] Prepared " #func_name ".\\n"); \
		else \
			Sys_Printf("[" #lib "] Failed to prepare " #func_name ": %i\\n", status); \
	} \
} while (0)

#else //!__cplusplus
#error C++ compiler required to compile SvenBXT.h
#endif //__cplusplus

#endif // SVENBXT_H_GUARD

#undef SVENBXT_H_RECURSE_GUARD
#endif // SVENBXT_H_RECURSE_GUARD
