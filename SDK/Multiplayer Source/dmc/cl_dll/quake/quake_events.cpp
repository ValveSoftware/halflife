/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
#include "../hud.h"
#include "../cl_util.h"
#include "event_api.h"

extern "C"
{
// Deathmatch Classic
void EV_FireShotGunSingle( struct event_args_s *args  );
void EV_FireShotGunDouble( struct event_args_s *args  );
void EV_FireAxe( struct event_args_s *args  );
void EV_FireAxeSwing( struct event_args_s *args  );
void EV_FireRocket( struct event_args_s *args  );
void EV_FireLightning( struct event_args_s *args  );
void EV_FireSpike( struct event_args_s *args  );
void EV_FireSuperSpike( struct event_args_s *args  );
void EV_FireGrenade( struct event_args_s *args  );
void EV_Gibbed (struct event_args_s *args );
void EV_Teleport (struct event_args_s *args );
void EV_Trail (struct event_args_s *args );
void EV_Explosion (struct event_args_s *args );
void EV_PlayerPowerup ( struct event_args_s *args );

void EV_DMC_DoorGoUp( struct event_args_s *args  );
void EV_DMC_DoorGoDown( struct event_args_s *args  );
void EV_DMC_DoorHitTop( struct event_args_s *args  );
void EV_DMC_DoorHitBottom( struct event_args_s *args  );

// HLDM
void EV_TrainPitchAdjust( struct event_args_s *args );
}

/*
======================
Game_HookEvents

Associate script file name with callback functions.  Callback's must be extern "C" so
 the engine doesn't get confused about name mangling stuff.  Note that the format is
 always the same.  Of course, a clever mod team could actually embed parameters, behavior
 into the actual .sc files and create a .sc file parser and hook their functionality through
 that.. i.e., a scripting system.

That was what we were going to do, but we ran out of time...oh well.
======================
*/
void Game_HookEvents( void )
{
	gEngfuncs.pfnHookEvent( "events/shotgun1.sc",				EV_FireShotGunSingle );
	gEngfuncs.pfnHookEvent( "events/shotgun2.sc",				EV_FireShotGunDouble );
	gEngfuncs.pfnHookEvent( "events/axe.sc",					EV_FireAxe );
	gEngfuncs.pfnHookEvent( "events/axeswing.sc",				EV_FireAxeSwing );
	gEngfuncs.pfnHookEvent( "events/rocket.sc",					EV_FireRocket );
	gEngfuncs.pfnHookEvent( "events/lightning.sc",				EV_FireLightning );
	gEngfuncs.pfnHookEvent( "events/grenade.sc",				EV_FireGrenade );
	gEngfuncs.pfnHookEvent( "events/spike.sc",					EV_FireSpike );
	gEngfuncs.pfnHookEvent( "events/superspike.sc",				EV_FireSuperSpike );
	gEngfuncs.pfnHookEvent( "events/gibs.sc",					EV_Gibbed );
	gEngfuncs.pfnHookEvent( "events/teleport.sc",				EV_Teleport );
	gEngfuncs.pfnHookEvent( "events/trail.sc",					EV_Trail );
	gEngfuncs.pfnHookEvent( "events/explosion.sc",				EV_Explosion );

	gEngfuncs.pfnHookEvent( "events/powerup.sc",				EV_PlayerPowerup );

	gEngfuncs.pfnHookEvent( "events/door/doorgoup.sc",			EV_DMC_DoorGoUp );
	gEngfuncs.pfnHookEvent( "events/door/doorgodown.sc",		EV_DMC_DoorGoDown );
	gEngfuncs.pfnHookEvent( "events/door/doorhittop.sc",		EV_DMC_DoorHitTop );
	gEngfuncs.pfnHookEvent( "events/door/doorhitbottom.sc",		EV_DMC_DoorHitBottom );

	gEngfuncs.pfnHookEvent( "events/train.sc",					EV_TrainPitchAdjust );
}
