//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

// Author: Michael S. Booth (mike@turtlerockstudios.com), Leon Hartwig, 2003

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "soundent.h"
#include "gamerules.h"
#include "player.h"
#include "client.h"
#include "pm_shared.h"

#include "bot.h"
#include "bot_util.h"

DLL_GLOBAL float g_flBotCommandInterval		= 1.0 / 30.0;	// 30 times per second, just like human clients
DLL_GLOBAL float g_flBotFullThinkInterval	= 1.0 / 10.0;	// full AI only 10 times per second


//--------------------------------------------------------------------------------------------------------------
CBot::CBot( void )
{
	// the profile will be attached after this instance is constructed
	m_profile = NULL;

	// assign this bot a unique ID
	static unsigned int nextID = 1;

	// wraparound (highly unlikely)
	if (nextID == 0)
		++nextID;

	m_id = nextID;
	++nextID;

	m_postureStackIndex = 0;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Prepare bot for action
 */
bool CBot::Initialize( const BotProfile *profile )
{
	m_profile = profile;
	return true;
}

//--------------------------------------------------------------------------------------------------------------
void CBot::Spawn( void )
{
	// Let CBasePlayer set some things up
	CBasePlayer::Spawn();

	// Make sure everyone knows we are a bot
	pev->flags |= ( FL_CLIENT | FL_FAKECLIENT );

	// Bots use their own thinking mechanism
	SetThink( NULL );
	pev->nextthink = -1;

	m_flNextBotThink		= gpGlobals->time + g_flBotCommandInterval;
	m_flNextFullBotThink	= gpGlobals->time + g_flBotFullThinkInterval;
	m_flPreviousCommandTime	= gpGlobals->time;

	m_isRunning = true;
	m_isCrouching = false;
	m_postureStackIndex = 0;

	m_jumpTimestamp = 0.0f;

	// Command interface variable initialization
	ResetCommand();

	// Allow derived classes to setup at spawn time
	SpawnBot();
}


//--------------------------------------------------------------------------------------------------------------
Vector CBot::GetAutoaimVector( float flDelta )
{
	UTIL_MakeVectors( pev->v_angle + pev->punchangle );

	return gpGlobals->v_forward;
}


//--------------------------------------------------------------------------------------------------------------
void CBot::BotThink( void )
{
	if ( gpGlobals->time >= m_flNextBotThink )
	{
		m_flNextBotThink = gpGlobals->time + g_flBotCommandInterval;

		Upkeep();

		if ( gpGlobals->time >= m_flNextFullBotThink )
		{
			m_flNextFullBotThink = gpGlobals->time + g_flBotFullThinkInterval;

			ResetCommand();
			Update();
		}

		ExecuteCommand();
	}
}


//--------------------------------------------------------------------------------------------------------------
void CBot::MoveForward( void )
{
	m_forwardSpeed = GetMoveSpeed();
	SetBits( m_buttonFlags, IN_FORWARD );

	// make mutually exclusive
	ClearBits( m_buttonFlags, IN_BACK );
}


//--------------------------------------------------------------------------------------------------------------
void CBot::MoveBackward( void )
{
	m_forwardSpeed = -GetMoveSpeed();
	SetBits( m_buttonFlags, IN_BACK );

	// make mutually exclusive
	ClearBits( m_buttonFlags, IN_FORWARD );
}

//--------------------------------------------------------------------------------------------------------------
void CBot::StrafeLeft( void )
{
	m_strafeSpeed = -GetMoveSpeed();
	SetBits( m_buttonFlags, IN_MOVELEFT );

	// make mutually exclusive
	ClearBits( m_buttonFlags, IN_MOVERIGHT );
}

//--------------------------------------------------------------------------------------------------------------
void CBot::StrafeRight( void )
{
	m_strafeSpeed = GetMoveSpeed();
	SetBits( m_buttonFlags, IN_MOVERIGHT );

	// make mutually exclusive
	ClearBits( m_buttonFlags, IN_MOVELEFT );
}

//--------------------------------------------------------------------------------------------------------------
bool CBot::Jump( bool mustJump )
{
	if (IsJumping() || IsCrouching())
		return false;

	if (!mustJump)
	{
		const float minJumpInterval = 0.9f; // 1.5f;
		if (gpGlobals->time - m_jumpTimestamp < minJumpInterval)
			return false;
	}

	// still need sanity check for jumping frequency
	const float sanityInterval = 0.3f;
	if (gpGlobals->time - m_jumpTimestamp < sanityInterval)
		return false;

	// jump
	SetBits( m_buttonFlags, IN_JUMP );
	m_jumpTimestamp = gpGlobals->time;
	return true;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Zero any MoveForward(), Jump(), etc
 */
void CBot::ClearMovement( void )
{
	ResetCommand();
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Returns true if we are in the midst of a jump
 */
bool CBot::IsJumping( void )
{
	// if long time after last jump, we can't be jumping
	if (gpGlobals->time - m_jumpTimestamp > 3.0f)
		return false;

	// if we just jumped, we're still jumping
	if (gpGlobals->time - m_jumpTimestamp < 1.0f)
		return true;

	// a little after our jump, we're jumping until we hit the ground
	if (FBitSet( pev->flags, FL_ONGROUND ))
		return false;

	return true;
}

//--------------------------------------------------------------------------------------------------------------
void CBot::Crouch( void )
{
	m_isCrouching = true;
}

//--------------------------------------------------------------------------------------------------------------
void CBot::StandUp( void )
{
	m_isCrouching = false;
}


//--------------------------------------------------------------------------------------------------------------
void CBot::UseEnvironment( void )
{
	SetBits( m_buttonFlags, IN_USE );
}


//--------------------------------------------------------------------------------------------------------------
void CBot::PrimaryAttack( void )
{
	SetBits( m_buttonFlags, IN_ATTACK );
}

//--------------------------------------------------------------------------------------------------------------
void CBot::ClearPrimaryAttack( void )
{
	ClearBits( m_buttonFlags, IN_ATTACK );
}

//--------------------------------------------------------------------------------------------------------------
void CBot::TogglePrimaryAttack( void )
{
	if (FBitSet( m_buttonFlags, IN_ATTACK ))
	{
		ClearBits( m_buttonFlags, IN_ATTACK );
	}
	else
	{
		SetBits( m_buttonFlags, IN_ATTACK );
	}
}


//--------------------------------------------------------------------------------------------------------------
void CBot::SecondaryAttack( void )
{
	SetBits( m_buttonFlags, IN_ATTACK2 );
}

//--------------------------------------------------------------------------------------------------------------
void CBot::Reload( void )
{
	SetBits( m_buttonFlags, IN_RELOAD );
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Returns ratio of ammo left to max ammo (1 = full clip, 0 = empty)
 */
float CBot::GetActiveWeaponAmmoRatio( void ) const
{
	CBasePlayerWeapon *gun = GetActiveWeapon();

	if ( !gun )
		return 0.0f;

	// weapons with no ammo are always full
	if (gun->m_iClip < 0)
		return 1.0f;

	return (float)gun->m_iClip / (float)gun->iMaxClip();
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if active weapon has an empty clip
 */
bool CBot::IsActiveWeaponClipEmpty( void ) const
{
	CBasePlayerWeapon *gun = GetActiveWeapon();

	if (gun && gun->m_iClip == 0)
		return true;

	return false;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if active weapon has no ammo at all
 */
bool CBot::IsActiveWeaponOutOfAmmo( void ) const
{
	CBasePlayerWeapon *gun = GetActiveWeapon();

	if (gun == NULL)
		return true;

	if (gun->m_iClip < 0)
		return false;

	if (gun->m_iClip == 0 && m_rgAmmo[ gun->m_iPrimaryAmmoType ] <= 0)
		return true;

	return false;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if looking thru weapon's scope
 */
bool CBot::IsUsingScope( void ) const
{
	// if our field of view is less than 90, we're looking thru a scope (maybe only true for CS...)
	if (m_iFOV < 90.0f)
		return true;

	return false;
}


//--------------------------------------------------------------------------------------------------------------
void CBot::ExecuteCommand( void )
{
	byte adjustedMSec;

	// Adjust msec to command time interval
	adjustedMSec = ThrottledMsec();

	// player model is "munged"
	pev->angles = pev->v_angle;
	pev->angles.x /= -3.0;

	// save the command time
	m_flPreviousCommandTime = gpGlobals->time;

	if (m_isCrouching)
		SetBits( m_buttonFlags, IN_DUCK );

	// Run the command
	(*g_engfuncs.pfnRunPlayerMove)( edict(), pev->v_angle, m_forwardSpeed, m_strafeSpeed, m_verticalSpeed, 
																	m_buttonFlags, 0, adjustedMSec );
}


//--------------------------------------------------------------------------------------------------------------
void CBot::ResetCommand( void )
{
	m_forwardSpeed = 0.0;
	m_strafeSpeed = 0.0;
	m_verticalSpeed	= 0.0;
	m_buttonFlags = 0;
}


//--------------------------------------------------------------------------------------------------------------
byte CBot::ThrottledMsec( void ) const
{
	int iNewMsec;

	// Estimate Msec to use for this command based on time passed from the previous command
	iNewMsec = (int)( (gpGlobals->time - m_flPreviousCommandTime) * 1000 );
	if (iNewMsec > 255)  // Doh, bots are going to be slower than they should if this happens.
		iNewMsec = 255;		 // Upgrade that CPU or use less bots!

	return (byte)iNewMsec;
}

//--------------------------------------------------------------------------------------------------------------

// Nasty Hack.  See client.cpp/ClientCommand()
const char *BotArgs[4] = { NULL };
bool UseBotArgs = false;

/**
 * Do a "client command" - useful for invoking menu choices, etc.
 */
void CBot::ClientCommand( const char *cmd, const char *arg1, const char *arg2, const char *arg3 )
{
	BotArgs[0] = cmd;
	BotArgs[1] = arg1;
	BotArgs[2] = arg2;
	BotArgs[3] = arg3;

	UseBotArgs = true;
	::ClientCommand( ENT( pev ) );
	UseBotArgs = false;
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Returns TRUE if given entity is our enemy
 */
bool CBot::IsEnemy( CBaseEntity *ent ) const
{
	// only Players (real and AI) can be enemies
	if (!ent->IsPlayer())
		return false;

	// corpses are no threat
	if (!ent->IsAlive())
		return false;	

	CBasePlayer *player = static_cast<CBasePlayer *>( ent );

	// if they are on our team, they are our friends
	if (player->m_iTeam == m_iTeam)
		return false;

	// yep, we hate 'em
	return true;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return number of enemies left alive
 */
int CBot::GetEnemiesRemaining( void ) const
{
	int count = 0;

	for ( int i = 1; i <= gpGlobals->maxClients; ++i )
	{
		CBaseEntity *player = UTIL_PlayerByIndex( i );

		if (player == NULL)
			continue;

		if (FNullEnt( player->pev ))
			continue;

		if (FStrEq( STRING( player->pev->netname ), "" ))
			continue;

		if (!IsEnemy( player ))
			continue;

		if (!player->IsAlive())
			continue;

		count++;
	}

	return count;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return number of friends left alive
 */
int CBot::GetFriendsRemaining( void ) const
{
	int count = 0;

	for ( int i = 1; i <= gpGlobals->maxClients; ++i )
	{
		CBaseEntity *player = UTIL_PlayerByIndex( i );

		if (player == NULL)
			continue;

		if (FNullEnt( player->pev ))
			continue;

		if (FStrEq( STRING( player->pev->netname ), "" ))
			continue;

		if (IsEnemy( player ))
			continue;

		if (!player->IsAlive())
			continue;

		if (player == static_cast<CBaseEntity *>( const_cast<CBot *>( this ) ))
			continue;

		count++;
	}

	return count;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if the local player is currently in observer mode watching this bot.
 */
bool CBot::IsLocalPlayerWatchingMe( void ) const
{
	// avoid crash during spawn
	if (pev == NULL)
		return false;

	int myIndex = const_cast<CBot *>(this)->entindex();

	CBasePlayer *player = UTIL_GetLocalPlayer();
	if (player == NULL)
		return false;

	if (player->pev->flags & FL_SPECTATOR || player->m_iTeam == SPECTATOR)
	{
		if (player->pev->iuser2 == myIndex)
		{
			switch( player->pev->iuser1 )
			{
				case OBS_IN_EYE:
				case OBS_CHASE_LOCKED:
				case OBS_CHASE_FREE:
					return true;
			}
		}
	}

	return false;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Output message to console
 */
void CBot::Print( char *format, ... ) const
{
	va_list varg;
	char buffer[1024];

	// prefix the message with the bot's name
	sprintf( buffer, "%s: ", STRING(pev->netname) );
	(*g_engfuncs.pfnServerPrint)( buffer );

	va_start( varg, format );
	vsprintf( buffer, format, varg );
	va_end( varg );

	(*g_engfuncs.pfnServerPrint)( buffer );
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Output message to console if we are being watched by the local player
 */
void CBot::PrintIfWatched( char *format, ... ) const
{
	if (cv_bot_debug.value == 0)
		return;

	if ((IsLocalPlayerWatchingMe() && (cv_bot_debug.value == 1 || cv_bot_debug.value == 3)) ||
			(cv_bot_debug.value == 2 || cv_bot_debug.value == 4))
	{
		va_list varg;
		char buffer[1024];

		// prefix the message with the bot's name (this can be NULL if bot was just added)
		const char *name;
		if (pev == NULL)
			name = "(NULL pev)";
		else
			name = STRING(pev->netname);
		sprintf( buffer, "%s: ", (name) ? name : "(NULL netname)" );
		(*g_engfuncs.pfnServerPrint)( buffer );

		va_start( varg, format );
		vsprintf( buffer, format, varg );
		va_end( varg );

		(*g_engfuncs.pfnServerPrint)( buffer );
	}
}

//--------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------

ActiveGrenade::ActiveGrenade( int weaponID, CGrenade *grenadeEntity )
{
	m_id = weaponID;
	m_entity = grenadeEntity;
	m_detonationPosition = grenadeEntity->pev->origin;
	m_dieTimestamp = 0.0f;
}

//--------------------------------------------------------------------------------------------------------------
void ActiveGrenade::OnEntityGone( void )									///< called when the grenade in the world goes away
{
	if (m_id == WEAPON_SMOKEGRENADE)
	{
		// smoke lingers after grenade is gone
		const float smokeLingerTime = 4.0f;
		m_dieTimestamp = gpGlobals->time + smokeLingerTime;
	}

	m_entity = NULL;
}

//--------------------------------------------------------------------------------------------------------------
bool ActiveGrenade::IsValid( void ) const							///< return true if this grenade is valid
{
	if (m_entity)
		return true;

	if (gpGlobals->time > m_dieTimestamp)
		return false;

	return true;
}

//--------------------------------------------------------------------------------------------------------------
const Vector *ActiveGrenade::GetPosition( void ) const
{ 
	return &m_entity->pev->origin; 
}

