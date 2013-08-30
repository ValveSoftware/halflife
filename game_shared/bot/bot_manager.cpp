//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

// Author: Michael S. Booth (mike@turtlerockstudios.com), 2003

#pragma warning( disable : 4530 )					// STL uses exceptions, but we are not compiling with them - ignore warning

#define DEFINE_EVENT_NAMES

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "soundent.h"
#include "gamerules.h"
#include "player.h"
#include "client.h"
#include "perf_counter.h"

#include "bot.h"
#include "bot_manager.h"
#include "nav_area.h"
#include "bot_util.h"
#include "hostage.h"

#include "tutor.h"

const float smokeRadius = 115.0f;		///< for smoke grenades


//#define CHECK_PERFORMANCE
#ifdef CHECK_PERFORMANCE
	// crude performance timing
	static CPerformanceCounter perfCounter;

	struct PerfInfo
	{
		float frameTime;
		float botThinkTime;
	};

	#define MAX_PERF_DATA 50000
	static PerfInfo perfData[ MAX_PERF_DATA ];
	static int perfDataCount = 0;
	static int perfFileIndex = 0;
#endif



/**
 * Convert name to GameEventType
 * @todo Find more appropriate place for this function
 */
GameEventType NameToGameEvent( const char *name )
{
	for( int i=0; GameEventName[i]; ++i )
		if (!stricmp( GameEventName[i], name ))
			return static_cast<GameEventType>( i );

	return EVENT_INVALID;
}


//--------------------------------------------------------------------------------------------------------------
CBotManager::CBotManager()
{
	InitBotTrig();
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Invoked when the round is restarting
 */
void CBotManager::RestartRound( void )
{
#ifdef CHECK_PERFORMANCE
	// dump previous round's performance
	char filename[80];
	sprintf( filename, "perfdata%02X.txt", perfFileIndex++ );
	FILE *fp = fopen( filename, "w" );

	if (fp)
	{
		for( int p=0; p<perfDataCount; ++p )
			fprintf( fp, "%f\t%f\n", perfData[p].frameTime, perfData[p].botThinkTime );

		fclose( fp );
	}
		
	perfDataCount = 0;
#endif

	DestroyAllGrenades();
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Invoked at the start of each frame
 */
void CBotManager::StartFrame( void )
{
	// debug smoke grenade visualization
	if (cv_bot_debug.value == 5)
	{
		Vector edge, lastEdge;

		ActiveGrenadeList::iterator iter = m_activeGrenadeList.begin();
		while( iter != m_activeGrenadeList.end() )
		{
			ActiveGrenade *ag = *iter;

			// lazy validation
			if (!ag->IsValid())
			{
				delete ag;
				iter = m_activeGrenadeList.erase( iter );
				continue;
			}
			else
			{
				++iter;
			}

			const Vector *pos = ag->GetDetonationPosition();

			UTIL_DrawBeamPoints( *pos, *pos + Vector( 0, 0, 50 ), 1, 255, 100, 0 );

			lastEdge = Vector( smokeRadius + pos->x, pos->y, pos->z );
			float angle;
			for( angle=0.0f; angle <= 180.0f; angle += 22.5f )
			{
				edge.x = smokeRadius * BotCOS( angle ) + pos->x;
				edge.y = pos->y;
				edge.z = smokeRadius * BotSIN( angle ) + pos->z;

				UTIL_DrawBeamPoints( edge, lastEdge, 1, 255, 50, 0 );

				lastEdge = edge;
			}

			lastEdge = Vector( pos->x, smokeRadius + pos->y, pos->z );
			for( angle=0.0f; angle <= 180.0f; angle += 22.5f )
			{
				edge.x = pos->x;
				edge.y = smokeRadius * BotCOS( angle ) + pos->y;
				edge.z = smokeRadius * BotSIN( angle ) + pos->z;

				UTIL_DrawBeamPoints( edge, lastEdge, 1, 255, 50, 0 );

				lastEdge = edge;
			}
		}
	}


	//
	// Process each active bot
	//

#ifdef CHECK_PERFORMANCE
	static double lastTime = 0.0f;
	double startTime = perfCounter.GetCurTime();
#endif

	for( int i = 1; i <= gpGlobals->maxClients; ++i )
	{
		CBasePlayer *pPlayer = (CBasePlayer *)UTIL_PlayerByIndex( i );

		if (!pPlayer)
			continue;

		if (pPlayer->IsBot() && IsEntityValid( pPlayer ))
		{
			CBot *pBot = static_cast<CBot *>( pPlayer );

			pBot->BotThink();
		}
	}

#ifdef CHECK_PERFORMANCE
	if (perfDataCount < MAX_PERF_DATA)
	{
		if (lastTime > 0.0f)
		{
			double endTime = perfCounter.GetCurTime();

			perfData[ perfDataCount ].frameTime = (float)(startTime - lastTime);
			perfData[ perfDataCount ].botThinkTime = (float)(endTime - startTime);
			++perfDataCount;
		}

		lastTime = startTime;
	}
#endif
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return the filename for this map's "nav map" file
 */
const char *CBotManager::GetNavMapFilename( void ) const
{
	static char filename[256];
	sprintf( filename, "maps\\%s.nav", STRING( gpGlobals->mapname ) );
	return filename;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Invoked when given player does given event (some events have NULL player).
 * Events are propogated to all bots.
 *
 * @todo This has become the game-wide event dispatcher. We should restructure this.
 */
void CBotManager::OnEvent( GameEventType event, CBaseEntity *entity, CBaseEntity *other )
{
	// propogate event to all bots
	for ( int i=1; i <= gpGlobals->maxClients; ++i )
	{
		CBasePlayer *player = static_cast<CBasePlayer *>( UTIL_PlayerByIndex( i ) );

		if (player == NULL)
			continue;

		if (FNullEnt( player->pev ))
			continue;

		if (FStrEq( STRING( player->pev->netname ), "" ))
			continue;

		if (!player->IsBot())
			continue;

		// do not send self-generated event
		if (entity == player)
			continue;

		CBot *bot = static_cast<CBot *>( player );
		bot->OnEvent( event, entity, other );
	}

	if (TheTutor)
		TheTutor->OnEvent( event, entity, other );

	if (g_pHostages)
		g_pHostages->OnEvent( event, entity, other );
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Add an active grenade to the bot's awareness
 */
void CBotManager::AddGrenade( int type, CGrenade *grenade )
{
	ActiveGrenade *ag = new ActiveGrenade( type, grenade );
	m_activeGrenadeList.push_back( ag );
}

//--------------------------------------------------------------------------------------------------------------
/**
 * The grenade entity in the world is going away
 */
void CBotManager::RemoveGrenade( CGrenade *grenade )
{
	for( ActiveGrenadeList::iterator iter = m_activeGrenadeList.begin(); iter != m_activeGrenadeList.end(); ++iter )
	{
		ActiveGrenade *ag = *iter;

		if (ag->IsEntity( grenade ))
		{
			ag->OnEntityGone();
			return;
		}
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Destroy any invalid active grenades
 */
void CBotManager::ValidateActiveGrenades( void )
{
	ActiveGrenadeList::iterator iter = m_activeGrenadeList.begin();
	while( iter != m_activeGrenadeList.end() )
	{
		ActiveGrenade *ag = *iter;

		if (!ag->IsValid())
		{
			delete ag;
			iter = m_activeGrenadeList.erase( iter );
		}
		else
		{
			++iter;
		}
	}
}

//--------------------------------------------------------------------------------------------------------------
void CBotManager::DestroyAllGrenades( void )
{
	for( ActiveGrenadeList::iterator iter = m_activeGrenadeList.begin(); iter != m_activeGrenadeList.end(); ++iter )
		delete *iter;

	m_activeGrenadeList.clear();
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if position is inside a smoke cloud
 */
bool CBotManager::IsInsideSmokeCloud( const Vector *pos )
{
	ActiveGrenadeList::iterator iter = m_activeGrenadeList.begin();
	while( iter != m_activeGrenadeList.end() )
	{
		ActiveGrenade *ag = *iter;

		// lazy validation
		if (!ag->IsValid())
		{
			delete ag;
			iter = m_activeGrenadeList.erase( iter );
			continue;
		}
		else
		{
			++iter;
		}

		if (ag->GetID() == WEAPON_SMOKEGRENADE)
		{
			const Vector *smokeOrigin = ag->GetDetonationPosition();

			if ((*smokeOrigin - *pos).IsLengthLessThan( smokeRadius ))
				return true;			
		}
	}

	return false;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if line intersects smoke volume
 * Determine the length of the line of sight covered by each smoke cloud, 
 * and sum them (overlap is additive for obstruction).
 * If the overlap exceeds the threshold, the bot can't see through.
 */
bool CBotManager::IsLineBlockedBySmoke( const Vector *from, const Vector *to )
{
	const float smokeRadiusSq = smokeRadius * smokeRadius;
	float totalSmokedLength = 0.0f;	// distance along line of sight covered by smoke

	// compute unit vector and length of line of sight segment
	Vector sightDir = *to - *from;
	float sightLength = sightDir.NormalizeInPlace();


	ActiveGrenadeList::iterator iter = m_activeGrenadeList.begin();
	while( iter != m_activeGrenadeList.end() )
	{
		ActiveGrenade *ag = *iter;

		// lazy validation
		if (!ag->IsValid())
		{
			delete ag;
			iter = m_activeGrenadeList.erase( iter );
			continue;
		}
		else
		{
			++iter;
		}

		if (ag->GetID() == WEAPON_SMOKEGRENADE)
		{
			const Vector *smokeOrigin = ag->GetDetonationPosition();

			Vector toGrenade = *smokeOrigin - *from;

			float alongDist = DotProduct( toGrenade, sightDir );

			// compute closest point to grenade along line of sight ray
			Vector close;

			// constrain closest point to line segment
			if (alongDist < 0.0f)
				close = *from;
			else if (alongDist >= sightLength)
				close = *to;
			else
				close = *from + sightDir * alongDist;

			// if closest point is within smoke radius, the line overlaps the smoke cloud
			Vector toClose = close - *smokeOrigin;
			float lengthSq = toClose.LengthSquared();

			if (lengthSq < smokeRadiusSq)
			{
				// some portion of the ray intersects the cloud

				float fromSq = toGrenade.LengthSquared();
				float toSq = (*smokeOrigin - *to).LengthSquared();

				if (fromSq < smokeRadiusSq)
				{
					if (toSq < smokeRadiusSq)
					{
						// both 'from' and 'to' lie within the cloud
						// entire length is smoked
						totalSmokedLength += (*to - *from).Length();
					}
					else
					{
						// 'from' is inside the cloud, 'to' is outside
						// compute half of total smoked length as if ray crosses entire cloud chord
						float halfSmokedLength = sqrt( smokeRadiusSq - lengthSq );

						if (alongDist > 0.0f)
						{
							// ray goes thru 'close'
							totalSmokedLength += halfSmokedLength + (close - *from).Length();						
						}
						else
						{
							// ray starts after 'close'
							totalSmokedLength += halfSmokedLength - (close - *from).Length();						
						}

					}
				}
				else if (toSq < smokeRadiusSq)
				{
					// 'from' is outside the cloud, 'to' is inside
					// compute half of total smoked length as if ray crosses entire cloud chord
					float halfSmokedLength = sqrt( smokeRadiusSq - lengthSq );

					Vector v = *to - *smokeOrigin;
					if (DotProduct( v, sightDir ) > 0.0f)
					{
						// ray goes thru 'close'
						totalSmokedLength += halfSmokedLength + (close - *to).Length();					
					}
					else
					{
						// ray ends before 'close'
						totalSmokedLength += halfSmokedLength - (close - *to).Length();
					}
				}
				else
				{			
					// 'from' and 'to' lie outside of the cloud - the line of sight completely crosses it
					// determine the length of the chord that crosses the cloud
					float smokedLength = 2.0f * sqrt( smokeRadiusSq - lengthSq );

					totalSmokedLength += smokedLength;
				}
			}
		}
	}

	// define how much smoke a bot can see thru
	const float maxSmokedLength = 0.7f * smokeRadius;

	// return true if the total length of smoke-covered line-of-sight is too much
	return (totalSmokedLength > maxSmokedLength);
}
