#include "SvenBXT.h"

_SCR_BeginLoadingPlaque ORIG_SCR_BeginLoadingPlaque = NULL;
_SCR_EndLoadingPlaque ORIG_SCR_EndLoadingPlaque = NULL;

_GL_Begin2D ORIG_GL_Begin2D = NULL;
_GL_Finish2D ORIG_GL_Finish2D = NULL;

_GL_EndRendering ORIG_GL_EndRendering = NULL;

void HOOKED_SCR_BeginLoadingPlaque(qboolean reconnect)
{
	ORIG_SCR_BeginLoadingPlaque(reconnect);

	if (g_lpHUDTimer && g_lpHUDTimer->IsInILMode())
		g_lpHUDTimer->TimerStop();
}

void HOOKED_SCR_EndLoadingPlaque()
{
	ORIG_SCR_EndLoadingPlaque();

	if (g_lpHUDTimer && g_lpHUDTimer->IsInILMode() && g_RTATimer.GetMilliseconds() <= 0)
		g_lpHUDTimer->TimerStart();
}

void HOOKED_GL_EndRendering()
{
	// FIXME: GameUI background somehow hides HUD sprites
	// And it happens in the end of rendering (after all shenanigans)...
	if (ORIG_GL_Begin2D && ORIG_GL_Finish2D && g_lpEngfuncs) {
		ORIG_GL_Begin2D();
		gBXTHud.RedrawAlwaysVisibleElements(g_lpEngfuncs->GetClientTime());
		ORIG_GL_Finish2D();
	}

	ORIG_GL_EndRendering();
}