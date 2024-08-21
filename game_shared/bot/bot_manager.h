//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

// Author: Michael S. Booth (mike@turtlerockstudios.com), 2003

#ifndef BASE_CONTROL_H
#define BASE_CONTROL_H

#pragma warning( disable : 4530 )					// STL uses exceptions, but we are not compiling with them - ignore warning

#include "extdll.h"
#include "util.h"
#undef min
#undef max
#include <list>
#include "GameEvent.h" // Game event enum used by career mode, tutor system, and bots

#include "minmax.h"

class CNavArea;


//--------------------------------------------------------------------------------------------------------------
class CGrenade;

/**
 * An ActiveGrenade is a representation of a grenade in the world
 * NOTE: Currently only used for smoke grenade line-of-sight testing
 * @todo Use system allow bots to avoid HE and Flashbangs
 */
class ActiveGrenade
{
public:
	ActiveGrenade( int weaponID, CGrenade *grenadeEntity );

	void OnEntityGone( void );								///< called when the grenade in the world goes away
	bool IsValid( void ) const	;							///< return true if this grenade is valid
	bool IsEntity( CGrenade *grenade ) const	{ return (grenade == m_entity) ? true : false; }
	int GetID( void ) const										{ return m_id; }
	const Vector *GetDetonationPosition( void ) const	{ return &m_detonationPosition; }
	const Vector *GetPosition( void ) const;

private:
	int m_id;																	///< weapon id
	CGrenade *m_entity;												///< the entity
	Vector m_detonationPosition;							///< the location where the grenade detonated (smoke)
	float m_dieTimestamp;											///< time this should go away after m_entity is NULL
};

typedef std::list<ActiveGrenade *> ActiveGrenadeList;


//--------------------------------------------------------------------------------------------------------------
/**
 * This class manages all active bots, propagating events to them and updating them.
 */
class CBotManager 
{
public:
	CBotManager();

	virtual void ClientDisconnect( CBasePlayer * pPlayer ) = 0;
	virtual BOOL ClientCommand( CBasePlayer * pPlayer, const char * pcmd ) = 0;

	virtual void ServerActivate( void ) = 0;
	virtual void ServerDeactivate( void ) = 0;
	virtual void ServerCommand( const char * pcmd ) = 0;
	virtual void AddServerCommand( const char *cmd ) = 0;
	virtual void AddServerCommands( void ) = 0;

	virtual void RestartRound( void );							///< (EXTEND) invoked when a new round begins
	virtual void StartFrame( void );							///< (EXTEND) called each frame

	const char *GetNavMapFilename( void ) const;				///< return the filename for this map's "nav" file

	/**
	 * Invoked when event occurs in the game (some events have NULL entity).
	 * Events are propogated to all bots.
	 */
	virtual void OnEvent( GameEventType event, CBaseEntity *entity = NULL, CBaseEntity *other = NULL );

	virtual unsigned int GetPlayerPriority( CBasePlayer *player ) const = 0;	///< return priority of player (0 = max pri)
	

	void AddGrenade( int type, CGrenade *grenade );				///< add an active grenade to the bot's awareness
	void RemoveGrenade( CGrenade *grenade );					///< the grenade entity in the world is going away
	void ValidateActiveGrenades( void );						///< destroy any invalid active grenades
	void DestroyAllGrenades( void );
	bool IsLineBlockedBySmoke( const Vector *from, const Vector *to );	///< return true if line intersects smoke volume
	bool IsInsideSmokeCloud( const Vector *pos );				///< return true if position is inside a smoke cloud

private:
	ActiveGrenadeList m_activeGrenadeList;///< the list of active grenades the bots are aware of
};

#endif
