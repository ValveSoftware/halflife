//=========== (C) Copyright 1996-2002, Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: Functionality for the observer chase camera
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================
#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"player.h"
#include	"pm_shared.h"

// Find the next client in the game for this player to spectate
void CBasePlayer::Observer_FindNextPlayer()
{
	// MOD AUTHORS: Modify the logic of this function if you want to restrict the observer to watching
	//				only a subset of the players. e.g. Make it check the target's team.

	CBaseEntity *client = m_hObserverTarget;
	while ( (client = (CBaseEntity*)UTIL_FindEntityByClassname( client, "player" )) != m_hObserverTarget ) 
	{
		if ( !client )
			continue;
		if ( !client->pev )
			continue;
		if ( client == this )
			continue;

		// Add checks on target here.

		m_hObserverTarget = client;
		break;
	}

	// Did we find a target?
	if ( m_hObserverTarget )
	{
		// Store the target in pev so the physics DLL can get to it
		if (pev->iuser1 != OBS_ROAMING)
			pev->iuser2 = ENTINDEX( m_hObserverTarget->edict() );
		// Move to the target
		UTIL_SetOrigin( pev, m_hObserverTarget->pev->origin );

		ALERT( at_console, "Now Tracking %s\n", STRING( m_hObserverTarget->pev->classname ) );
	}
	else
	{
		ALERT( at_console, "No observer targets.\n" );
	}
}

// Handle buttons in observer mode
void CBasePlayer::Observer_HandleButtons()
{
	// Slow down mouse clicks
	if ( m_flNextObserverInput > gpGlobals->time )
		return;

	// Jump changes from modes: Chase to Roaming
	if ( m_afButtonPressed & IN_JUMP )
	{
		if ( pev->iuser1 == OBS_CHASE_LOCKED )
			Observer_SetMode( OBS_CHASE_FREE );

		else if ( pev->iuser1 == OBS_CHASE_FREE )
			Observer_SetMode( OBS_ROAMING );

		else if ( pev->iuser1 == OBS_ROAMING )
			Observer_SetMode( OBS_IN_EYE );

		else if ( pev->iuser1 == OBS_IN_EYE )
			Observer_SetMode( OBS_MAP_FREE );

		else if ( pev->iuser1 == OBS_MAP_FREE )
			Observer_SetMode( OBS_MAP_CHASE );

		else
			Observer_SetMode( OBS_CHASE_FREE );	// don't use OBS_CHASE_LOCKED anymore

		m_flNextObserverInput = gpGlobals->time + 0.2;
	}

	// Attack moves to the next player
	if ( m_afButtonPressed & IN_ATTACK )
	{
		Observer_FindNextPlayer();

		m_flNextObserverInput = gpGlobals->time + 0.2;
	}
 
}

// Attempt to change the observer mode
void CBasePlayer::Observer_SetMode( int iMode )
{
	// Just abort if we're changing to the mode we're already in
	if ( iMode == pev->iuser1 )
		return;

	// is valid mode ?
	if ( iMode < OBS_CHASE_LOCKED || iMode > OBS_MAP_CHASE )
		iMode = OBS_IN_EYE; // now it is

	// if we are not roaming, we need a valid target to track
	if ( (iMode != OBS_ROAMING) && (m_hObserverTarget == NULL) )
	{
		Observer_FindNextPlayer();

		// if we didn't find a valid target switch to roaming
		if (m_hObserverTarget == NULL)
		{
			ClientPrint( pev, HUD_PRINTCENTER, "#Spec_NoTarget"  );
			iMode = OBS_ROAMING;
		}
	}

	// set spectator mode
	pev->iuser1 = iMode;

	// set target if not roaming
	if (iMode == OBS_ROAMING)
		pev->iuser2 = 0;
	else
		pev->iuser2 = ENTINDEX( m_hObserverTarget->edict() );
	
	// print spepctaor mode on client screen

	char modemsg[16];
	sprintf(modemsg,"#Spec_Mode%i", iMode);
	ClientPrint( pev, HUD_PRINTCENTER, modemsg );
}
