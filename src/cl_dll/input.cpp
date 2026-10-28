#include "SvenBXT.h"

cvar_t* sbxt_autojump;

playermove_t* g_ppmove;

bool bDucktapDown = false;

void IN_BXT_TAS_Ducktap_Down()
{
	bDucktapDown = true;
}

void IN_BXT_TAS_Ducktap_Up()
{
	bDucktapDown = false;
}

static void handle_autojump(usercmd_t* cmd)
{
	static bool s_jump_was_down_last_frame = false;

	if (sbxt_autojump && sbxt_autojump->value)
	{
		bool should_release_jump = (!UTIL_IsPlayerOnGround() && !UTIL_IsPlayerInWater() && UTIL_IsPlayerWalking());

		/*
		 * Spam pressing and releasing jump if we're stuck in a spot where jumping still results in
		 * being onground in the end of the frame. Without this check, +jump would remain held and
		 * when the player exits this spot they would have to release and press the jump button to
		 * start jumping again. This also helps with exiting water or ladder right onto the ground.
		 */
		if (s_jump_was_down_last_frame && UTIL_IsPlayerOnGround() && !UTIL_IsPlayerInWater() && UTIL_IsPlayerWalking())
			should_release_jump = true;

		if (should_release_jump)
			cmd->buttons &= ~IN_JUMP;
	}

	s_jump_was_down_last_frame = ((cmd->buttons & IN_JUMP) != 0);
}

static void handle_ducktap(usercmd_t* cmd)
{
	static bool s_duck_was_down_last_frame = false;
	static bool should_release_duck;

	should_release_duck = (!UTIL_IsPlayerOnGround() && !UTIL_IsPlayerInWater() && UTIL_IsPlayerWalking());

	if (s_duck_was_down_last_frame && UTIL_IsPlayerOnGround() && !UTIL_IsPlayerInWater() && UTIL_IsPlayerWalking())
		should_release_duck = true;

	if (should_release_duck)
	{
		cmd->buttons &= ~IN_DUCK;
	}

	s_duck_was_down_last_frame = ((cmd->buttons & IN_DUCK) != 0);
}

void HOOKED_HUD_PlayerMove(struct playermove_s* ppmove, qboolean server)
{
	ORIG_HUD_PlayerMove(ppmove, server);
	g_ppmove = ppmove;
}

void HOOKED_CL_CreateMove(float frametime, usercmd_s* cmd, int active)
{
	ORIG_CL_CreateMove(frametime, cmd, active);

	if (!g_ppmove || !cmd)
		return;

	if (bDucktapDown)
	{
		cmd->buttons |= IN_DUCK;
		handle_ducktap(cmd); // Ducktap takes priority over autojump
	}
	else
	{
		handle_autojump(cmd);
	}
}

void IN_Init()
{
	g_lpEngfuncs->AddCommand("+sbxt_ducktap", IN_BXT_TAS_Ducktap_Down);
	g_lpEngfuncs->AddCommand("-sbxt_ducktap", IN_BXT_TAS_Ducktap_Up);
	sbxt_autojump = CVAR_CREATE("sbxt_autojump", "0", 0);
}