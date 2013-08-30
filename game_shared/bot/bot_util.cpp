//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

// Author: Michael S. Booth (mike@turtlerockstudios.com), 2003

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"

#include "bot.h"
#include "bot_util.h"
#include "bot_profile.h"
#include "nav.h"

static short s_iBeamSprite = 0;

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if given name is already in use by another player
 */
bool UTIL_IsNameTaken( const char *name, bool ignoreHumans )
{
	for ( int i = 1; i <= gpGlobals->maxClients; ++i )
	{
		CBaseEntity * player = UTIL_PlayerByIndex( i );

		if (player == NULL)
			continue;

		if (FNullEnt( player->pev ))
			continue;

		if (FStrEq( STRING( player->pev->netname ), "" ))
			continue;

		if (player->IsPlayer() && (((CBasePlayer *)player)->IsBot() == TRUE))
		{
			// bots can have prefixes so we need to check the name
			// against the profile name instead.
			CBot *bot = (CBot *)player;
			if (FStrEq(name, bot->GetProfile()->GetName()))
			{
				return true;
			}
		}
		else
		{
			if (!ignoreHumans)
			{
				if (FStrEq( name, STRING( player->pev->netname ) ))
					return true;
			}
		}
	}

	return false;
}


//--------------------------------------------------------------------------------------------------------------
int UTIL_ClientsInGame( void )
{
	int iCount = 0;

	for ( int iIndex = 1; iIndex <= gpGlobals->maxClients; iIndex++ )
	{
		CBaseEntity * pPlayer = UTIL_PlayerByIndex( iIndex );

		if ( pPlayer == NULL )
			continue;

		if ( FNullEnt( pPlayer->pev ) )
			continue;

		if ( FStrEq( STRING( pPlayer->pev->netname ), "" ) )
			continue;

		iCount++;
	}

	return iCount;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return number of active players (not spectators) in the game
 */
int UTIL_ActivePlayersInGame( void )
{
	int iCount = 0;

	for (int iIndex = 1; iIndex <= gpGlobals->maxClients; iIndex++ )
	{
		CBaseEntity *entity = UTIL_PlayerByIndex( iIndex );

		if ( entity == NULL )
			continue;

		if ( FNullEnt( entity->pev ) )
			continue;

		if ( FStrEq( STRING( entity->pev->netname ), "" ) )
			continue;

		CBasePlayer *player = static_cast<CBasePlayer *>( entity );

		// ignore spectators
		if (player->m_iTeam != TERRORIST && player->m_iTeam != CT)
			continue;

		if (player->m_iJoiningState != JOINED)
			continue;

		iCount++;
	}

	return iCount;
}



//--------------------------------------------------------------------------------------------------------------
int UTIL_HumansInGame( bool ignoreSpectators )
{
	int iCount = 0;

	for (int iIndex = 1; iIndex <= gpGlobals->maxClients; iIndex++ )
	{
		CBaseEntity *entity = UTIL_PlayerByIndex( iIndex );

		if ( entity == NULL )
			continue;

		if ( FNullEnt( entity->pev ) )
			continue;

		if ( FStrEq( STRING( entity->pev->netname ), "" ) )
			continue;

		CBasePlayer *player = static_cast<CBasePlayer *>( entity );

		if (player->IsBot())
			continue;

		if (ignoreSpectators && player->m_iTeam != TERRORIST && player->m_iTeam != CT)
			continue;

		if (ignoreSpectators && player->m_iJoiningState != JOINED)
			continue;

		iCount++;
	}

	/*
	if ( IS_DEDICATED_SERVER() && !ignoreSpectators )
	{
		// If we're counting humans, including spectators, don't count the dedicated server
		--iCount;
	}
	*/

	return iCount;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return the number of non-bots on the given team
 */
int UTIL_HumansOnTeam( int teamID, bool isAlive )
{
	int iCount = 0;

	for (int iIndex = 1; iIndex <= gpGlobals->maxClients; iIndex++ )
	{
		CBaseEntity *entity = UTIL_PlayerByIndex( iIndex );

		if ( entity == NULL )
			continue;

		if ( FNullEnt( entity->pev ) )
			continue;

		if ( FStrEq( STRING( entity->pev->netname ), "" ) )
			continue;

		CBasePlayer *player = static_cast<CBasePlayer *>( entity );

		if (player->IsBot())
			continue;

		if (player->m_iTeam != teamID)
			continue;

		if (isAlive && !player->IsAlive())
			continue;

		iCount++;
	}

	return iCount;
}


//--------------------------------------------------------------------------------------------------------------
int UTIL_BotsInGame( void )
{
	int iCount = 0;

	for (int iIndex = 1; iIndex <= gpGlobals->maxClients; iIndex++ )
	{
		CBasePlayer *pPlayer = static_cast<CBasePlayer *>(UTIL_PlayerByIndex( iIndex ));

		if ( pPlayer == NULL )
			continue;

		if ( FNullEnt( pPlayer->pev ) )
			continue;

		if ( FStrEq( STRING( pPlayer->pev->netname ), "" ) )
			continue;

		if ( !pPlayer->IsBot() )
			continue;

		iCount++;
	}

	return iCount;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Kick a bot from the given team. If no bot exists on the team, return false.
 */
bool UTIL_KickBotFromTeam( TeamName kickTeam )
{
	int i;

	// try to kick a dead bot first
	for ( i = 1; i <= gpGlobals->maxClients; ++i )
	{
		CBasePlayer *player = static_cast<CBasePlayer *>( UTIL_PlayerByIndex( i ) );

		if (player == NULL)
			continue;

		if (FNullEnt( player->pev ))
			continue;

		const char *name = STRING( player->pev->netname );
		if (FStrEq( name, "" ))
			continue;

		if (!player->IsBot())
			continue;	

		if (!player->IsAlive() && player->m_iTeam == kickTeam)
		{
			// its a bot on the right team - kick it
			SERVER_COMMAND( UTIL_VarArgs( "kick \"%s\"\n", STRING( player->pev->netname ) ) );

			return true;
		}
	}

	// no dead bots, kick any bot on the given team
	for ( i = 1; i <= gpGlobals->maxClients; ++i )
	{
		CBasePlayer *player = static_cast<CBasePlayer *>( UTIL_PlayerByIndex( i ) );

		if (player == NULL)
			continue;

		if (FNullEnt( player->pev ))
			continue;

		const char *name = STRING( player->pev->netname );
		if (FStrEq( name, "" ))
			continue;

		if (!player->IsBot())
			continue;	

		if (player->m_iTeam == kickTeam)
		{
			// its a bot on the right team - kick it
			SERVER_COMMAND( UTIL_VarArgs( "kick \"%s\"\n", STRING( player->pev->netname ) ) );

			return true;
		}
	}

	return false;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if all of the members of the given team are bots
 */
bool UTIL_IsTeamAllBots( int team )
{
	int botCount = 0;

	for( int i=1; i <= gpGlobals->maxClients; ++i )
	{
		CBasePlayer *player = static_cast<CBasePlayer *>( UTIL_PlayerByIndex( i ) );

		if (player == NULL)
			continue;

		// skip players on other teams
		if (player->m_iTeam != team)
			continue;

		if (FNullEnt( player->pev ))
			continue;

		if (FStrEq( STRING( player->pev->netname ), "" ))
			continue;

		// if not a bot, fail the test
		if (!FBitSet( player->pev->flags, FL_FAKECLIENT ))
			return false;

		// is a bot on given team
		++botCount;
	}

	// if team is empty, there are no bots
	return (botCount) ? true : false;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return the closest active player to the given position.
 * If 'distance' is non-NULL, the distance to the closest player is returned in it.
 */
extern CBasePlayer *UTIL_GetClosestPlayer( const Vector *pos, float *distance )
{
	CBasePlayer *closePlayer = NULL;
	float closeDistSq = 999999999999.9f;

	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CBasePlayer *player = static_cast<CBasePlayer *>( UTIL_PlayerByIndex( i ) );

		if (!IsEntityValid( player ))
			continue;

		if (!player->IsAlive())
			continue;

		float distSq = (player->pev->origin - *pos).LengthSquared();
		if (distSq < closeDistSq)
		{
			closeDistSq = distSq;
			closePlayer = static_cast<CBasePlayer *>( player );
		}
	}
	
	if (distance)
		*distance = sqrt( closeDistSq );

	return closePlayer;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return the closest active player on the given team to the given position.
 * If 'distance' is non-NULL, the distance to the closest player is returned in it.
 */
extern CBasePlayer *UTIL_GetClosestPlayer( const Vector *pos, int team, float *distance )
{
	CBasePlayer *closePlayer = NULL;
	float closeDistSq = 999999999999.9f;

	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CBasePlayer *player = static_cast<CBasePlayer *>( UTIL_PlayerByIndex( i ) );

		if (!IsEntityValid( player ))
			continue;

		if (!player->IsAlive())
			continue;

		if (player->m_iTeam != team)
			continue;

		float distSq = (player->pev->origin - *pos).LengthSquared();
		if (distSq < closeDistSq)
		{
			closeDistSq = distSq;
			closePlayer = static_cast<CBasePlayer *>( player );
		}
	}
	
	if (distance)
		*distance = sqrt( closeDistSq );

	return closePlayer;
}

//--------------------------------------------------------------------------------------------------------------
// returns the string to be used for the bot name prefix.
const char * UTIL_GetBotPrefix()
{
	return cv_bot_prefix.string;
}

//--------------------------------------------------------------------------------------------------------------
// Takes the bot pointer and constructs the net name using the current bot name prefix.
void UTIL_ConstructBotNetName(char *name, int nameLength, const BotProfile *profile)
{
	if (profile == NULL)
	{
		name[0] = 0;
		return;
	}

	// if there is no bot prefix just use the profile name.
	if ((UTIL_GetBotPrefix() == NULL) || (strlen(UTIL_GetBotPrefix()) == 0))
	{
		strncpy(name, profile->GetName(), nameLength);
		return;
	}

	_snprintf(name, nameLength, "%s %s", UTIL_GetBotPrefix(), profile->GetName());
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if anyone on the given team can see the given spot
 */
bool UTIL_IsVisibleToTeam( const Vector &spot, int team, float maxRange )
{
	for( int i = 1; i <= gpGlobals->maxClients; ++i )
	{
		CBasePlayer *player = static_cast<CBasePlayer *>( UTIL_PlayerByIndex( i ) );

		if (player == NULL)
			continue;

		if (FNullEnt( player->pev ))
			continue;

		if (FStrEq( STRING( player->pev->netname ), "" ))
			continue;

		if (!player->IsAlive())
			continue;

		if (player->m_iTeam != team)
			continue;

		if (maxRange > 0.0f && (spot - player->Center()).IsLengthGreaterThan( maxRange ))
			continue;

		TraceResult result;
		UTIL_TraceLine( player->EyePosition(), spot, ignore_monsters, ignore_glass, ENT( player->pev ), &result );

		if (result.flFraction == 1.0f)
			return true;
	}

	return false;
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Return the local player
 */
CBasePlayer *UTIL_GetLocalPlayer( void )
{
	if ( IS_DEDICATED_SERVER() )
	{
		return NULL;
	}
	return static_cast<CBasePlayer *>( UTIL_PlayerByIndex( 1 ) );
}


//------------------------------------------------------------------------------------------------------------
// Some types of entities have no origin set, so we use this instead.
Vector UTIL_ComputeOrigin( entvars_t * pevVars )
{
	if ( ( pevVars->origin.x == 0.0 ) && ( pevVars->origin.y == 0.0 ) && ( pevVars->origin.z == 0.0 ) )
		return ( pevVars->absmax + pevVars->absmin ) * 0.5;
	else
		return pevVars->origin;
}


Vector UTIL_ComputeOrigin( CBaseEntity * pEntity )
{
	return UTIL_ComputeOrigin( pEntity->pev );
}


Vector UTIL_ComputeOrigin( edict_t * pentEdict )
{
	return UTIL_ComputeOrigin( VARS( pentEdict ) );
}


//------------------------------------------------------------------------------------------------------------
void UTIL_DrawBeamFromEnt( int iIndex, Vector vecEnd, int iLifetime, byte bRed, byte bGreen, byte bBlue )
{
	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, vecEnd );   // vecEnd = origin???
									WRITE_BYTE( TE_BEAMENTPOINT );
									WRITE_SHORT( iIndex );
									WRITE_COORD( vecEnd.x );
									WRITE_COORD( vecEnd.y );
									WRITE_COORD( vecEnd.z );
									WRITE_SHORT( s_iBeamSprite );
									WRITE_BYTE( 0 );		 // startframe
									WRITE_BYTE( 0 );		 // framerate
									WRITE_BYTE( iLifetime ); // life
									WRITE_BYTE( 10 );		 // width
									WRITE_BYTE( 0 );		 // noise
									WRITE_BYTE( bRed );		 // r, g, b
									WRITE_BYTE( bGreen );		 // r, g, b
									WRITE_BYTE( bBlue );    // r, g, b
									WRITE_BYTE( 255 );	 // brightness
									WRITE_BYTE( 0 );		 // speed
									MESSAGE_END();
}


//------------------------------------------------------------------------------------------------------------
void UTIL_DrawBeamPoints( Vector vecStart, Vector vecEnd, int iLifetime, byte bRed, byte bGreen, byte bBlue )
{
	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, vecStart );
									WRITE_BYTE( TE_BEAMPOINTS );
									WRITE_COORD( vecStart.x );
									WRITE_COORD( vecStart.y );
									WRITE_COORD( vecStart.z );
									WRITE_COORD( vecEnd.x );
									WRITE_COORD( vecEnd.y );
									WRITE_COORD( vecEnd.z );
									WRITE_SHORT( s_iBeamSprite );
									WRITE_BYTE( 0 );		 // startframe
									WRITE_BYTE( 0 );		 // framerate
									WRITE_BYTE( iLifetime ); // life
									WRITE_BYTE( 10 );		 // width
									WRITE_BYTE( 0 );		 // noise
									WRITE_BYTE( bRed );		 // r, g, b
									WRITE_BYTE( bGreen );		 // r, g, b
									WRITE_BYTE( bBlue );    // r, g, b
									WRITE_BYTE( 255 );	 // brightness
									WRITE_BYTE( 0 );		 // speed
									MESSAGE_END();
}


//------------------------------------------------------------------------------------------------------------
void CONSOLE_ECHO( char * pszMsg, ... )
{
	va_list     argptr;
	static char szStr[1024];

	va_start( argptr, pszMsg );
	vsprintf( szStr, pszMsg, argptr );
	va_end( argptr );

	(*g_engfuncs.pfnServerPrint)( szStr );
}


//------------------------------------------------------------------------------------------------------------
void CONSOLE_ECHO_LOGGED( char * pszMsg, ... )
{
	va_list     argptr;
	static char szStr[1024];

	va_start( argptr, pszMsg );
	vsprintf( szStr, pszMsg, argptr );
	va_end( argptr );

	(*g_engfuncs.pfnServerPrint)( szStr );
	UTIL_LogPrintf( szStr );
}


//------------------------------------------------------------------------------------------------------------
void BotPrecache( void )
{
	s_iBeamSprite = PRECACHE_MODEL( "sprites/smoke.spr" );
	PRECACHE_SOUND( "buttons/bell1.wav" );
	PRECACHE_SOUND( "buttons/blip1.wav" );
	PRECACHE_SOUND( "buttons/blip2.wav" );
	PRECACHE_SOUND( "buttons/button11.wav" );
	PRECACHE_SOUND( "buttons/latchunlocked2.wav" );
	PRECACHE_SOUND( "buttons/lightswitch2.wav" );
	PRECACHE_SOUND( "ambience/quail1.wav" );

	/// @todo This is for the Tutor - move it somewhere sane
	PRECACHE_SOUND( "events/tutor_msg.wav" );
	PRECACHE_SOUND( "events/enemy_died.wav" );
	PRECACHE_SOUND( "events/friend_died.wav" );

	/// @todo This is for the Career mode UI - move it somewhere sane
	PRECACHE_SOUND( "events/task_complete.wav" );

#ifdef TERRORSTRIKE
	/// @todo Zombie mode experiment
	PRECACHE_SOUND( "zombie/attack1.wav" );
	PRECACHE_SOUND( "zombie/attack2.wav" );
	PRECACHE_SOUND( "zombie/attack3.wav" );
	PRECACHE_SOUND( "zombie/attack4.wav" );
	PRECACHE_SOUND( "zombie/attack5.wav" );
	PRECACHE_SOUND( "zombie/bark1.wav" );
	PRECACHE_SOUND( "zombie/bark2.wav" );
	PRECACHE_SOUND( "zombie/bark3.wav" );
	PRECACHE_SOUND( "zombie/bark4.wav" );
	PRECACHE_SOUND( "zombie/bark5.wav" );
	PRECACHE_SOUND( "zombie/bark6.wav" );
	PRECACHE_SOUND( "zombie/bark7.wav" );
	PRECACHE_SOUND( "zombie/breathing1.wav" );
	PRECACHE_SOUND( "zombie/breathing2.wav" );
	PRECACHE_SOUND( "zombie/breathing3.wav" );
	PRECACHE_SOUND( "zombie/breathing4.wav" );
	PRECACHE_SOUND( "zombie/groan1.wav" );
	PRECACHE_SOUND( "zombie/groan2.wav" );
	PRECACHE_SOUND( "zombie/groan3.wav" );
	PRECACHE_SOUND( "zombie/hiss1.wav" );
	PRECACHE_SOUND( "zombie/hiss2.wav" );
	PRECACHE_SOUND( "zombie/hiss3.wav" );
	PRECACHE_SOUND( "ambience/the_horror2.wav" );
	PRECACHE_SOUND( "scientist/scream20.wav" );
	PRECACHE_SOUND( "zombie/human_hurt1.wav" );
	PRECACHE_SOUND( "zombie/human_hurt2.wav" );
	PRECACHE_SOUND( "zombie/human_hurt3.wav" );
	PRECACHE_SOUND( "zombie/human_hurt4.wav" );
	PRECACHE_SOUND( "zombie/shout_reloading1.wav" );
	PRECACHE_SOUND( "zombie/shout_reloading2.wav" );
	PRECACHE_SOUND( "zombie/shout_reloading3.wav" );
	PRECACHE_SOUND( "zombie/deep_heartbeat.wav" );
	PRECACHE_SOUND( "zombie/deep_heartbeat_fast.wav" );
	PRECACHE_SOUND( "zombie/deep_heartbeat_very_fast.wav" );
	PRECACHE_SOUND( "zombie/deep_heartbeat_stopping.wav" );
	PRECACHE_SOUND( "zombie/zombie_step1.wav" );
	PRECACHE_SOUND( "zombie/zombie_step2.wav" );
	PRECACHE_SOUND( "zombie/zombie_step3.wav" );
	PRECACHE_SOUND( "zombie/zombie_step4.wav" );
	PRECACHE_SOUND( "zombie/zombie_step5.wav" );
	PRECACHE_SOUND( "zombie/zombie_step6.wav" );
	PRECACHE_SOUND( "zombie/zombie_step7.wav" );
	PRECACHE_SOUND( "zombie/fear1.wav" );
	PRECACHE_SOUND( "zombie/fear2.wav" );
	PRECACHE_SOUND( "zombie/fear3.wav" );
	PRECACHE_SOUND( "zombie/fear4.wav" );
#endif // TERRORSTRIKE
}

//------------------------------------------------------------------------------------------------------------
#define COS_TABLE_SIZE 256
static float cosTable[ COS_TABLE_SIZE ];

void InitBotTrig( void )
{
	for( int i=0; i<COS_TABLE_SIZE; ++i )
	{
		float angle = 2.0f * M_PI * (float)i / (float)(COS_TABLE_SIZE-1);
		cosTable[i] = cos(angle); 
	}
}

float BotCOS( float angle )
{
	angle = NormalizeAnglePositive( angle );
	int i = angle * (COS_TABLE_SIZE-1) / 360.0f;
	return cosTable[i];
}

float BotSIN( float angle )
{
	angle = NormalizeAnglePositive( angle - 90 );
	int i = angle * (COS_TABLE_SIZE-1) / 360.0f;
	return cosTable[i];
}


//------------------------------------------------------------------------------------------------------------
/**
 * Determine if this event is audible, and if so, return its audible range and priority
 */
bool IsGameEventAudible( GameEventType event, CBaseEntity *entity, CBaseEntity *other, float *range, PriorityType *priority, bool *isHostile )
{
	CBasePlayer *player = static_cast<CBasePlayer *>( entity );
	if (entity == NULL || !player->IsPlayer())
		player = NULL;

	const float ShortRange = 1000.0f;
	const float NormalRange = 2000.0f;
	switch( event )
	{
		/// @todo Check weapon type (knives are pretty quiet)
		/// @todo Use actual volume, account for silencers, etc.
		case EVENT_WEAPON_FIRED:
		{
			if (player->m_pActiveItem == NULL)
				return false;

			switch( player->m_pActiveItem->m_iId )
			{
				// silent "firing"
				case WEAPON_HEGRENADE:
				case WEAPON_SMOKEGRENADE:
				case WEAPON_FLASHBANG:
				case WEAPON_SHIELDGUN:
				case WEAPON_C4:
					return false;

				// quiet
				case WEAPON_KNIFE:
				case WEAPON_TMP:
					*range = ShortRange;
					break;

				// M4A1 - check for silencer
				case WEAPON_M4A1:
					{
						CBasePlayerWeapon *pWeapon = static_cast<CBasePlayerWeapon *>(player->m_pActiveItem);
						if ( pWeapon->m_iWeaponState & WPNSTATE_M4A1_SILENCER_ON )
						{
							*range = ShortRange;
						}
						else
						{
							*range = NormalRange;
						}
					}
					break;

				// USP - check for silencer
				case WEAPON_USP:
					{
						CBasePlayerWeapon *pWeapon = static_cast<CBasePlayerWeapon *>(player->m_pActiveItem);
						if ( pWeapon->m_iWeaponState & WPNSTATE_USP_SILENCER_ON )
						{
							*range = ShortRange;
						}
						else
						{
							*range = NormalRange;
						}
					}
					break;

				// loud
				case WEAPON_AWP:
					*range = 99999.0f;
					break;

				// normal
				default:
					*range = NormalRange;
					break;
			}

			*priority = PRIORITY_HIGH;
			*isHostile = true;
			return true;
		}

		case EVENT_HE_GRENADE_EXPLODED:
			*range = 99999.0f;
			*priority = PRIORITY_HIGH;
			*isHostile = true;
			return true;

		case EVENT_FLASHBANG_GRENADE_EXPLODED:
			*range = 1000.0f;
			*priority = PRIORITY_LOW;
			*isHostile = true;
			return true;

		case EVENT_SMOKE_GRENADE_EXPLODED:
			*range = 1000.0f;
			*priority = PRIORITY_LOW;
			*isHostile = true;
			return true;

		case EVENT_GRENADE_BOUNCED:
			*range = 500.0f;
			*priority = PRIORITY_LOW;
			*isHostile = true;
			return true;

		case EVENT_BREAK_GLASS:
		case EVENT_BREAK_WOOD:
		case EVENT_BREAK_METAL:
		case EVENT_BREAK_FLESH:
		case EVENT_BREAK_CONCRETE:
			*range = 1100.0f;
			*priority = PRIORITY_MEDIUM;
			*isHostile = true;
			return true;

		case EVENT_DOOR:
			*range = 1100.0f;
			*priority = PRIORITY_MEDIUM;
			*isHostile = false;
			return true;

		case EVENT_WEAPON_FIRED_ON_EMPTY:
		case EVENT_PLAYER_FOOTSTEP:
		case EVENT_WEAPON_RELOADED:
		case EVENT_WEAPON_ZOOMED:
		case EVENT_PLAYER_LANDED_FROM_HEIGHT:
			*range = 1100.0f;
			*priority = PRIORITY_LOW;
			*isHostile = false;
			return true;

		case EVENT_HOSTAGE_USED:
		case EVENT_HOSTAGE_CALLED_FOR_HELP:
			*range = 1200.0f;
			*priority = PRIORITY_MEDIUM;
			*isHostile = false;
			return true;
	}

	return false;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Send a "hint" message to all players, dead or alive.
 */
void HintMessageToAllPlayers( const char *message )
{
	hudtextparms_t textParms;

	textParms.x = -1.0f;
	textParms.y = -1.0f;
	textParms.fadeinTime = 1.0f;
	textParms.fadeoutTime = 5.0f;
	textParms.holdTime = 5.0f;
	textParms.fxTime = 0.0f;
	textParms.r1 = 100;
	textParms.g1 = 255;
	textParms.b1 = 100;
	textParms.r2 = 255;
	textParms.g2 = 255;
	textParms.b2 = 255;
	textParms.effect = 0;
	textParms.channel = 0;

	UTIL_HudMessageAll( textParms, message );
}

