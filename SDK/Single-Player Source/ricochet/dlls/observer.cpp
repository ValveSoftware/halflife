//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================
// observer.cpp
//
#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"player.h"
#include	"weapons.h"

extern int gmsgCurWeapon;
extern int gmsgSetFOV;
extern int gmsgTeamInfo;
extern int gmsgSpectator;

// Player has become a spectator. Set it up.
// This was moved from player.cpp.
void CBasePlayer::StartObserver( Vector vecPosition, Vector vecViewAngle )
{
	// clear any clientside entities attached to this player
	MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, pev->origin );
		WRITE_BYTE( TE_KILLPLAYERATTACHMENTS );
		WRITE_BYTE( (BYTE)entindex() );
	MESSAGE_END();

	// Holster weapon immediately, to allow it to cleanup
	if (m_pActiveItem)
		m_pActiveItem->Holster( );

	if ( m_pTank != NULL )
	{
		m_pTank->Use( this, this, USE_OFF, 0 );
		m_pTank = NULL;
	}

	// clear out the suit message cache so we don't keep chattering
	SetSuitUpdate(NULL, FALSE, 0);

	// Tell Ammo Hud that the player is dead
	MESSAGE_BEGIN( MSG_ONE, gmsgCurWeapon, NULL, pev );
		WRITE_BYTE(0);
		WRITE_BYTE(0XFF);
		WRITE_BYTE(0xFF);
	MESSAGE_END();

	// reset FOV
	m_iFOV = m_iClientFOV = 0;
	pev->fov = m_iFOV;
	MESSAGE_BEGIN( MSG_ONE, gmsgSetFOV, NULL, pev );
		WRITE_BYTE(0);
	MESSAGE_END();

	// Setup flags
	m_iHideHUD = (HIDEHUD_HEALTH | HIDEHUD_WEAPONS);
	m_afPhysicsFlags |= PFLAG_OBSERVER;
	pev->effects = EF_NODRAW;
	pev->view_ofs = g_vecZero;
	pev->solid = SOLID_NOT;
	pev->takedamage = DAMAGE_NO;
	pev->movetype = MOVETYPE_NONE;
	ClearBits( m_afPhysicsFlags, PFLAG_DUCKING );
	ClearBits( pev->flags, FL_DUCKING );
	pev->deadflag = DEAD_RESPAWNABLE;
	pev->health = 1;
	
	// Clear out the status bar
	m_fInitHUD = TRUE;

	// Update Team Status
	MESSAGE_BEGIN( MSG_ALL, gmsgTeamInfo );
		WRITE_BYTE( ENTINDEX(edict()) );
		WRITE_STRING( "" );
	MESSAGE_END();

	// Remove all the player's stuff
	RemoveAllItems( FALSE );

	// Move them to the new position
	UTIL_SetOrigin( pev, vecPosition );

	// Find a player to watch
	m_flNextObserverInput = 0;
	Observer_SetMode(OBS_ROAMING);

	// Tell all clients this player is now a spectator
	MESSAGE_BEGIN( MSG_ALL, gmsgSpectator );  
		WRITE_BYTE( ENTINDEX( edict() ) );
		WRITE_BYTE( 1 );
	MESSAGE_END();

	pev->angles = pev->v_angle = vecViewAngle;
	pev->fixangle = TRUE;
}

// Leave observer mode
void CBasePlayer::StopObserver( void )
{
	// Turn off spectator
	if ( pev->iuser1 || pev->iuser2 )
	{
		// Tell all clients this player is not a spectator anymore
		MESSAGE_BEGIN( MSG_ALL, gmsgSpectator );  
			WRITE_BYTE( ENTINDEX( edict() ) );
			WRITE_BYTE( 0 );
		MESSAGE_END();

		pev->iuser1 = pev->iuser2 = 0; 
		m_hObserverTarget = NULL;
	}

	m_fWeapon = FALSE; // force weapon send
	m_iHideHUD = 0;
}

// Find the next client in the game for this player to spectate
void CBasePlayer::Observer_FindNextPlayer( bool bReverse )
{
	// MOD AUTHORS: Modify the logic of this function if you want to restrict the observer to watching
	//				only a subset of the players. e.g. Make it check the target's team.

	int		iStart;
	if ( m_hObserverTarget )
		iStart = ENTINDEX( m_hObserverTarget->edict() );
	else
		iStart = ENTINDEX( edict() );
	int	    iCurrent = iStart;
	m_hObserverTarget = NULL;
	int iDir = bReverse ? -1 : 1; 

	do
	{
		iCurrent += iDir;

		// Loop through the clients
		if (iCurrent > gpGlobals->maxClients)
			iCurrent = 1;
		if (iCurrent < 1)
			iCurrent = gpGlobals->maxClients;

		CBaseEntity *pEnt = UTIL_PlayerByIndex( iCurrent );
		if ( !pEnt )
			continue;
		if ( pEnt == this )
			continue;
		// Don't spec observers or invisible players
		if ( ((CBasePlayer*)pEnt)->IsObserver() || (pEnt->pev->effects & EF_NODRAW) )
			continue;

		// MOD AUTHORS: Add checks on target here.

		m_hObserverTarget = pEnt;
		break;

	} while ( iCurrent != iStart );

	// Did we find a target?
	if ( m_hObserverTarget )
	{
		// Store the target in pev so the physics DLL can get to it
		pev->iuser2 = ENTINDEX( m_hObserverTarget->edict() );
		pev->groupinfo = m_hObserverTarget->pev->groupinfo;

		// Move to the target
		if ( pev->iuser1 != OBS_LOCKEDVIEW )
			 UTIL_SetOrigin( pev, m_hObserverTarget->pev->origin );
		
		ALERT( at_console, "Now Tracking %s\n", STRING( m_hObserverTarget->pev->netname ) );
	}
	else
	{
		ALERT( at_console, "No observer targets.\n" );
	}
}

void CBasePlayer::ObserverInput_ChangeMode()
{
	if ( pev->iuser1 == OBS_ROAMING )
		Observer_SetMode( OBS_CHASE_FREE );
	else if ( pev->iuser1 == OBS_CHASE_FREE )
		Observer_SetMode( OBS_LOCKEDVIEW );
	else
		Observer_SetMode( OBS_ROAMING );
}

void CBasePlayer::ObserverInput_NextPlayer()
{
	if ( pev->iuser1 != OBS_ROAMING )
		 Observer_FindNextPlayer( false );
}

void CBasePlayer::ObserverInput_PrevPlayer()
{
	if ( pev->iuser1 != OBS_ROAMING )
		 Observer_FindNextPlayer( true );
}

// Handle buttons in observer mode
void CBasePlayer::Observer_HandleButtons()
{

	if ( m_flChangeAngles != -1 && m_flChangeAngles <= gpGlobals->time )
	{
		if ( pev->iuser1 == OBS_LOCKEDVIEW )
		{
			pev->angles = m_vecHitVelocity;
			pev->fixangle = TRUE;
		}

		m_flChangeAngles = -1;
	}

	return;

	// Slow down mouse clicks
	if ( m_flNextObserverInput > gpGlobals->time )
		return;

	// Jump changes from modes: Chase to Roaming
	if ( m_afButtonPressed & IN_JUMP )
	{
		if ( pev->iuser1 == OBS_ROAMING )
			Observer_SetMode( OBS_CHASE_FREE );
		else if ( pev->iuser1 == OBS_CHASE_FREE )
			Observer_SetMode( OBS_LOCKEDVIEW );
		else
			Observer_SetMode( OBS_ROAMING );

		m_flNextObserverInput = gpGlobals->time + 0.2;
	}

	// Attack moves to the next player
	if ( m_afButtonPressed & IN_ATTACK && pev->iuser1 != OBS_ROAMING )
	{
		Observer_FindNextPlayer( false );

		m_flNextObserverInput = gpGlobals->time + 0.2;
	}

	// Attack2 moves to the prev player
	if ( m_afButtonPressed & IN_ATTACK2 && pev->iuser1 != OBS_ROAMING )
	{
		Observer_FindNextPlayer( true );

		m_flNextObserverInput = gpGlobals->time + 0.2;
	}
}

// Attempt to change the observer mode
void CBasePlayer::Observer_SetMode( int iMode )
{
	// Just abort if we're changing to the mode we're already in
	if ( iMode == pev->iuser1 )
		return;

	// Changing to Roaming?
	if ( iMode == OBS_ROAMING )
	{
		// MOD AUTHORS: If you don't want to allow roaming observers at all in your mod, just abort here.
		pev->iuser1 = OBS_ROAMING;
		pev->iuser2 = 0;
		pev->maxspeed = 320;

		ClientPrint( pev, HUD_PRINTCENTER, "#Spec_Mode3" );
		return;
	}

	if ( iMode == OBS_LOCKEDVIEW )
	{
		// Find the spectator spawn position
		CBaseEntity *pSpot = UTIL_FindEntityByClassname( NULL, "info_player_spectator");
		
		if ( pSpot )
		{
			// Move them to the new position
			UTIL_SetOrigin( pev, pSpot->pev->origin );

			pev->iuser1 = OBS_LOCKEDVIEW;

			m_flChangeAngles = gpGlobals->time + 0.1;
			m_vecHitVelocity = pSpot->pev->v_angle;
			pev->iuser2 = 0;
			pev->maxspeed = 1;
		}

		return;
	}

	// Changing to Chase Freelook?
	if ( iMode == OBS_CHASE_FREE )
	{
		// If changing from Roaming, or starting observing, make sure there is a target
		if ( m_hObserverTarget == NULL )
			Observer_FindNextPlayer( false );

		if (m_hObserverTarget)
		{
			pev->iuser1 = OBS_CHASE_FREE;
			pev->iuser2 = ENTINDEX( m_hObserverTarget->edict() );
			ClientPrint( pev, HUD_PRINTCENTER, "#Spec_Mode2" );
			pev->maxspeed = 1;
		}
		else
		{
			ClientPrint( pev, HUD_PRINTCENTER, "#Spec_NoTarget"  );
			Observer_SetMode(OBS_ROAMING);
		}

		return;
	}
}
