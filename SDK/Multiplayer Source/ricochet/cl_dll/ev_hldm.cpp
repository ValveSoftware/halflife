/***
*
*	Copyright (c) 1999, Valve LLC. All rights reserved.
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
#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "entity_types.h"
#include "usercmd.h"
#include "pm_defs.h"
#include "pm_materials.h"

#include "eventscripts.h"
#include "ev_hldm.h"

#include "r_efx.h"
#include "event_api.h"
#include "event_args.h"
#include "in_defs.h"

#include <string.h>

extern "C"
{
// RICOCHET
void EV_FireDisc( struct event_args_s *args );
void EV_TriggerJump( struct event_args_s *args );
void EV_TrainPitchAdjust( struct event_args_s *args );
}

/*
==============================
EV_TriggerJump

Plays the jump pad sound
==============================
*/
void EV_TriggerJump( event_args_t *args )
{
	int idx;
	idx = args->entindex;
	vec3_t origin;

	VectorCopy( args->origin, origin );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_AUTO, "triggerjump.wav", 1.0, ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
}

/*
==============================
EV_FireDisc

Play's disc firing animation and play's appropriate sound effect
==============================
*/
void EV_FireDisc( event_args_t *args )
{
	int idx;

	idx = args->entindex;
	vec3_t origin;
	int decap;

	VectorCopy( args->origin, origin );
	decap = args->bparam1 ? 1 : 0;

	if ( EV_IsLocal( idx ) )
	{
		// Add muzzle flash to current weapon model
		gEngfuncs.pEventAPI->EV_WeaponAnimation( DISC_THROW1, 2 );
	}

	if ( decap )
	{
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/altfire.wav", 0.8, ATTN_NORM, 0, 100 );
	}
	else
	{
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/cbar_miss1.wav", 0.8, ATTN_NORM, 0, 100 );
	}
}

#define SND_CHANGE_PITCH	(1<<7)

/*
==============================
EV_TrainPitchAdjust

Do we support trains in Ricochet?
==============================
*/
void EV_TrainPitchAdjust( event_args_t *args )
{
	int idx;
	vec3_t origin;

	unsigned short us_params;
	int noise;
	float m_flVolume;
	int pitch;
	int stop;
	
	char sz[ 256 ];

	idx = args->entindex;
	
	VectorCopy( args->origin, origin );

	us_params = (unsigned short)args->iparam1;
	stop	  = args->bparam1;

	m_flVolume	= (float)(us_params & 0x003f)/40.0;
	noise		= (int)(((us_params) >> 12 ) & 0x0007);
	pitch		= (int)( 10.0 * (float)( ( us_params >> 6 ) & 0x003f ) );

	switch ( noise )
	{
	case 1: strcpy( sz, "plats/ttrain1.wav"); break;
	case 2: strcpy( sz, "plats/ttrain2.wav"); break;
	case 3: strcpy( sz, "plats/ttrain3.wav"); break; 
	case 4: strcpy( sz, "plats/ttrain4.wav"); break;
	case 5: strcpy( sz, "plats/ttrain6.wav"); break;
	case 6: strcpy( sz, "plats/ttrain7.wav"); break;
	default:
		// no sound
		strcpy( sz, "" );
		return;
	}

	if ( stop )
	{
		gEngfuncs.pEventAPI->EV_StopSound( idx, CHAN_STATIC, sz );
	}
	else
	{
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_STATIC, sz, m_flVolume, ATTN_NORM, SND_CHANGE_PITCH, pitch );
	}
}
