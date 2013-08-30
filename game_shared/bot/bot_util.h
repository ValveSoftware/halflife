//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef BOT_UTIL_H
#define BOT_UTIL_H


#include "eiface.h"
#include "player.h"
#include "shared_util.h"
#include "GameEvent.h"

//--------------------------------------------------------------------------------------------------------------
enum PriorityType
{
	PRIORITY_LOW, PRIORITY_MEDIUM, PRIORITY_HIGH, PRIORITY_UNINTERRUPTABLE
};


extern cvar_t cv_bot_traceview;
extern cvar_t cv_bot_stop;
extern cvar_t cv_bot_show_nav;
extern cvar_t cv_bot_show_danger;
extern cvar_t cv_bot_nav_edit;
extern cvar_t cv_bot_nav_zdraw;
extern cvar_t cv_bot_walk;
extern cvar_t cv_bot_difficulty;
extern cvar_t cv_bot_debug;
extern cvar_t cv_bot_quicksave;
extern cvar_t cv_bot_quota;
extern cvar_t cv_bot_quota_match;
extern cvar_t cv_bot_prefix;
extern cvar_t cv_bot_allow_rogues;
extern cvar_t cv_bot_allow_pistols;
extern cvar_t cv_bot_allow_shotguns;
extern cvar_t cv_bot_allow_sub_machine_guns;
extern cvar_t cv_bot_allow_rifles;
extern cvar_t cv_bot_allow_machine_guns;
extern cvar_t cv_bot_allow_grenades;
extern cvar_t cv_bot_allow_snipers;
extern cvar_t cv_bot_allow_shield;
extern cvar_t cv_bot_join_team;
extern cvar_t cv_bot_join_after_player;
extern cvar_t cv_bot_auto_vacate;
extern cvar_t cv_bot_zombie;
extern cvar_t cv_bot_defer_to_human;
extern cvar_t cv_bot_chatter;
extern cvar_t cv_bot_profile_db;

#ifdef TERRORSTRIKE
extern cvar_t cv_zombie_near_spawn;
extern cvar_t cv_zombie_far_spawn;
extern cvar_t cv_zombie_relocate;
extern cvar_t cv_zombie_min_spawn_time;
extern cvar_t cv_zombie_max_spawn_time;
#endif

#define RAD_TO_DEG( deg ) ((deg) * 180.0 / M_PI)
#define DEG_TO_RAD( rad ) ((rad) * M_PI / 180.0)

#define SIGN( num )	      (((num) < 0) ? -1 : 1)
#define ABS( num )        (SIGN(num) * (num))


#define CREATE_FAKE_CLIENT		( *g_engfuncs.pfnCreateFakeClient )
#define GET_USERINFO			( *g_engfuncs.pfnGetInfoKeyBuffer )
#define SET_KEY_VALUE			( *g_engfuncs.pfnSetKeyValue )
#define SET_CLIENT_KEY_VALUE	( *g_engfuncs.pfnSetClientKeyValue )

class BotProfile;

extern void   BotPrecache( void );
extern int		UTIL_ClientsInGame( void );

extern bool UTIL_IsNameTaken( const char *name, bool ignoreHumans = false );		///< return true if given name is already in use by another player

// return number of active players (not spectators) in the game
extern int UTIL_ActivePlayersInGame( void );

#define IGNORE_SPECTATORS true
extern int UTIL_HumansInGame( bool ignoreSpectators = false );

#define IS_ALIVE true
extern int UTIL_HumansOnTeam( int teamID, bool isAlive = false );

extern int		UTIL_BotsInGame( void );
extern bool		UTIL_IsTeamAllBots( int team );
extern Vector	UTIL_ComputeOrigin( entvars_t * pevVars );
extern Vector	UTIL_ComputeOrigin( CBaseEntity * pEntity );
extern Vector	UTIL_ComputeOrigin( edict_t * pentEdict );
extern void		UTIL_DrawBeamFromEnt( int iIndex, Vector vecEnd, int iLifetime, byte bRed, byte bGreen, byte bBlue );
extern void		UTIL_DrawBeamPoints( Vector vecStart, Vector vecEnd, int iLifetime, byte bRed, byte bGreen, byte bBlue );
extern CBasePlayer *UTIL_GetClosestPlayer( const Vector *pos, float *distance = NULL );
extern CBasePlayer *UTIL_GetClosestPlayer( const Vector *pos, int team, float *distance = NULL );
extern CBasePlayer *UTIL_GetLocalPlayer( void );
extern bool UTIL_KickBotFromTeam( TeamName kickTeam ); ///< kick a bot from the given team. If no bot exists on the team, return false.

extern bool UTIL_IsVisibleToTeam( const Vector &spot, int team, float maxRange = -1.0f ); ///< return true if anyone on the given team can see the given spot

extern const char * UTIL_GetBotPrefix(); ///< returns the bot prefix string.
extern void UTIL_ConstructBotNetName(char *name, int nameLength, const BotProfile *bot);

/**
 * Echos text to the console, and prints it on the client's screen.  This is NOT tied to the developer cvar.
 * If you are adding debugging output in cstrike, use UTIL_DPrintf() (debug.h) instead.
 */
extern void			CONSOLE_ECHO( char * pszMsg, ... );
extern void			CONSOLE_ECHO_LOGGED( char * pszMsg, ... );

extern void InitBotTrig( void );
extern float BotCOS( float angle );
extern float BotSIN( float angle );

/// determine if this event is audible, and if so, return its audible range and priority
bool IsGameEventAudible( GameEventType event, CBaseEntity *entity, CBaseEntity *other, float *range, PriorityType *priority, bool *isHostile );

extern void HintMessageToAllPlayers( const char *message );


//--------------------------------------------------------------------------------------------------------------
/**
 * Simple class for tracking intervals of game time
 */
class IntervalTimer
{
public:
	IntervalTimer( void )
	{
		m_timestamp = -1.0f;
	}

	void Reset( void )
	{
		m_timestamp = gpGlobals->time;
	}		

	void Start( void )
	{
		m_timestamp = gpGlobals->time;
	}

	void Invalidate( void )
	{
		m_timestamp = -1.0f;
	}		

	bool HasStarted( void ) const
	{
		return (m_timestamp > 0.0f);
	}

	/// if not started, elapsed time is very large
	float GetElapsedTime( void ) const
	{
		return (HasStarted()) ? (gpGlobals->time - m_timestamp) : 99999.9f;
	}

	bool IsLessThen( float duration ) const
	{
		return (gpGlobals->time - m_timestamp < duration) ? true : false;
	}

	bool IsGreaterThen( float duration ) const
	{
		return (gpGlobals->time - m_timestamp > duration) ? true : false;
	}

private:
	float m_timestamp;
};

//--------------------------------------------------------------------------------------------------------------
/**
 * Simple class for counting down a short interval of time
 */
class CountdownTimer
{
public:
	CountdownTimer( void )
	{
		m_timestamp = -1.0f;
		m_duration = 0.0f;
	}

	void Reset( void )
	{
		m_timestamp = gpGlobals->time + m_duration;
	}		

	void Start( float duration )
	{
		m_timestamp = gpGlobals->time + duration;
		m_duration = duration;
	}

	void Invalidate( void )
	{
		m_timestamp = -1.0f;
	}		

	bool HasStarted( void ) const
	{
		return (m_timestamp > 0.0f);
	}

	bool IsElapsed( void ) const
	{
		return (gpGlobals->time > m_timestamp);
	}

private:
	float m_duration;
	float m_timestamp;
};

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if the given entity is valid
 */
inline bool IsEntityValid( CBaseEntity *entity )
{
	if (entity == NULL)
		return false;

	if (FNullEnt( entity->pev ))
		return false;

	if (FStrEq( STRING( entity->pev->netname ), "" ))
		return false;

	if (entity->pev->flags & FL_DORMANT)
		return false;

	return true;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Given two line segments: startA to endA, and startB to endB, return true if they intesect
 * and put the intersection point in "result".
 * Note that this computes the intersection of the 2D (x,y) projection of the line segments.
 */
inline bool IsIntersecting2D( const Vector &startA, const Vector &endA, 
															const Vector &startB, const Vector &endB, 
															Vector *result = NULL )
{
	float denom = (endA.x - startA.x) * (endB.y - startB.y) - (endA.y - startA.y) * (endB.x - startB.x);
	if (denom == 0.0f)
	{
		// parallel
		return false;
	}

	float numS = (startA.y - startB.y) * (endB.x - startB.x) - (startA.x - startB.x) * (endB.y - startB.y);
	if (numS == 0.0f)
	{
		// coincident
		return true;
	}

	float numT = (startA.y - startB.y) * (endA.x - startA.x) - (startA.x - startB.x) * (endA.y - startA.y);

	float s = numS / denom;
	if (s < 0.0f || s > 1.0f)
	{
		// intersection is not within line segment of startA to endA
		return false;
	}

	float t = numT / denom;
	if (t < 0.0f || t > 1.0f)
	{
		// intersection is not within line segment of startB to endB
		return false;
	}

	// compute intesection point
	if (result)
		*result = startA + s * (endA - startA);

	return true;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Iterate over all active players in the game, invoking functor on each.
 * If functor returns false, stop iteration and return false.
 */
template < typename Functor >
bool ForEachPlayer( Functor &func )
{
	for( int i=1; i<=gpGlobals->maxClients; ++i )
	{
		CBasePlayer *player = static_cast<CBasePlayer *>( UTIL_PlayerByIndex( i ) );

		if (!IsEntityValid( player ))
			continue;

		if (!player->IsPlayer())
			continue;

		if (func( player ) == false)
			return false;
	}

	return true;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * For zombie game
 */
inline bool IsZombieGame( void )
{
#ifdef TERRORSTRIKE
	return true;
#else
	return false;
#endif
}

#endif
