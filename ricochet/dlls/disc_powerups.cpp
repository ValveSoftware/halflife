//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: Code for the various Discwar powerups
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "items.h"
#include "gamerules.h"
#include "discwar.h"
#include "disc_objects.h"
#include "disc_arena.h"

extern int gmsgPowerup;

//=========================================================
// POWERUPS
char *szPowerupModels[NUM_POWERUPS] =
{
	"models/pow_triple.mdl",
	"models/pow_fast.mdl",
	"models/pow_hard.mdl",
	"models/pow_freeze.mdl",
	//"models/pow_visual.mdl",
};

LINK_ENTITY_TO_CLASS( item_powerup, CDiscwarPowerup );

//=========================================================
void CDiscwarPowerup::Spawn( void )
{
	Precache( );

	// Don't fall down
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_TRIGGER;
	UTIL_SetOrigin( pev, pev->origin );
	UTIL_SetSize(pev, Vector(-32, -32, -32), Vector(32, 32, 32));

	// Use first model for now
	SET_MODEL(ENT(pev), szPowerupModels[0]);
	pev->effects |= EF_NODRAW;
}

void CDiscwarPowerup::Activate( void )
{
	// If Arena mode is on, spawn another powerup for every arena
	if ( InArenaMode() )
	{
		// If our groupinfo is set, we're not the first powerup
		if ( pev->groupinfo == 0 )
		{
			// Put this powerup in the first arena
			pev->groupinfo = g_pArenaList[0]->pev->groupinfo;

			// Create a powerup for each of the other arenas
			for (int i = 1; i < MAX_ARENAS; i++)
			{
				CBaseEntity * pPowerup;

				pPowerup = CBaseEntity::Create( "item_powerup", pev->origin, pev->angles );
				pPowerup->pev->groupinfo = g_pArenaList[i]->pev->groupinfo;
			}
		}
	}
	else
	{
		// Make the powerup start thinking
		Enable();
	}
}

void CDiscwarPowerup::Precache( void )
{
	for (int i = 0; i < NUM_POWERUPS; i++)
		PRECACHE_MODEL( szPowerupModels[i] );
	PRECACHE_SOUND( "powerup.wav" );
	PRECACHE_SOUND( "pspawn.wav" );
}

void CDiscwarPowerup::SetObjectCollisionBox( void )
{
	pev->absmin = pev->origin + Vector( -64, -64, 0 );
	pev->absmax = pev->origin + Vector( 64, 64, 128 );
}

void CDiscwarPowerup::PowerupTouch( CBaseEntity *pOther )
{
	if ( !pOther->IsPlayer() )
		return;

	CBasePlayer *pPlayer = (CBasePlayer *)pOther;

	// Give the powerup to the player
	pPlayer->GivePowerup( m_iPowerupType );
	m_hPlayerIGaveTo = pPlayer;
	SetTouch( NULL );
	pev->effects |= EF_NODRAW;

	// Choose another powerup soon 
	SetThink( &CDiscwarPowerup::ChoosePowerupThink );
	pev->nextthink = gpGlobals->time + DISC_POWERUP_RESPAWN_TIME;

	// Play the powerup sound
	EMIT_SOUND_DYN( pOther->edict(), CHAN_STATIC, "powerup.wav", 1.0, ATTN_NORM, 0, 98 + RANDOM_LONG(0,3)); 
}

// Disappear and don't appear again until enabled
void CDiscwarPowerup::Disable()
{
	pev->effects |= EF_NODRAW;
	pev->nextthink = 0;
	SetThink( NULL );
	SetTouch( NULL );
}

// Come back and pick a new powerup
void CDiscwarPowerup::Enable()
{
	// Pick a powerup 
	SetThink( &CDiscwarPowerup::ChoosePowerupThink );
	pev->nextthink = gpGlobals->time + (DISC_POWERUP_RESPAWN_TIME / 2);
}

//=========================================================
// Randomly decide what powerup to be
void CDiscwarPowerup::ChoosePowerupThink( void )
{
	int iPowerup = RANDOM_LONG(0, NUM_POWERUPS-1);
	m_iPowerupType = (1 << iPowerup);

	SET_MODEL( ENT(pev), szPowerupModels[iPowerup] );
	pev->effects &= ~EF_NODRAW;

	SetTouch(&CDiscwarPowerup::PowerupTouch);
	
	// Start Animating
	pev->sequence = 0;
	pev->frame = 0;
	ResetSequenceInfo();

	SetThink(&CDiscwarPowerup::AnimateThink);
	pev->nextthink = gpGlobals->time + 0.1;

	pev->rendermode = kRenderTransAdd;
	pev->renderamt = 150;

	// Play the powerup appear sound
	EMIT_SOUND_DYN( edict(), CHAN_STATIC, "pspawn.wav", 1.0, ATTN_NORM, 0, 98 + RANDOM_LONG(0,3)); 
}

void CDiscwarPowerup::AnimateThink( void )
{
	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1;
}

// Remove the powerup from the person we gave it to
void CDiscwarPowerup::RemovePowerupThink( void )
{
	if (m_hPlayerIGaveTo == NULL)
		return;

	((CBasePlayer*)(CBaseEntity*)m_hPlayerIGaveTo)->RemovePowerup( m_iPowerupType );

	// Pick a powerup later
	SetThink( &CDiscwarPowerup::ChoosePowerupThink );
	pev->nextthink = gpGlobals->time + DISC_POWERUP_RESPAWN_TIME;
}

//=================================================================================
// PLAYER HANDLING FOR POWERUPS
//=========================================================
void CBasePlayer::GivePowerup( int iPowerupType )
{
	m_iPowerups |= iPowerupType;

	if ( m_iPowerups & POW_HARD )
		strcpy( m_szAnimExtention, "models/p_disc_hard.mdl" );

	MESSAGE_BEGIN( MSG_ONE, gmsgPowerup, NULL, pev );
		WRITE_BYTE( m_iPowerups );
	MESSAGE_END();

	m_iPowerupDiscs = MAX_DISCS;
}

void CBasePlayer::RemovePowerup( int iPowerupType )
{
	if ( iPowerupType & POW_HARD )
		strcpy( m_szAnimExtention, "models/p_disc.mdl" );

	m_iPowerups &= ~iPowerupType;

	MESSAGE_BEGIN( MSG_ONE, gmsgPowerup, NULL, pev );
		WRITE_BYTE( m_iPowerups );
	MESSAGE_END();

	m_iPowerupDiscs = 0;
}

void CBasePlayer::RemoveAllPowerups( void )
{
	m_iPowerups = 0;
	m_iPowerupDiscs = 0;

	MESSAGE_BEGIN( MSG_ONE, gmsgPowerup, NULL, pev );
		WRITE_BYTE( m_iPowerups );
	MESSAGE_END();
}

bool CBasePlayer::HasPowerup( int iPowerupType )
{
	return (m_iPowerups & iPowerupType) != 0;
}

