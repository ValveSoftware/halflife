// improv.h
// Improv interface
// Author: Michael S. Booth (mike@turtlerockstudios.com), November 2003

#ifndef _IMPROV_H_
#define _IMPROV_H_

#include "nav_path.h"

class CBaseEntity;


//--------------------------------------------------------------------------------------------------------
/**
 * Improv-specific events
 */
class IImprovEvent
{
public:
	virtual void OnMoveToSuccess( const Vector &goal ) { }	///< invoked when an improv reaches its MoveTo goal

	enum MoveToFailureType
	{
		FAIL_INVALID_PATH,
		FAIL_STUCK,
		FAIL_FELL_OFF,
	};
	virtual void OnMoveToFailure( const Vector &goal, MoveToFailureType reason ) { }	///< invoked when an improv fails to reach a MoveTo goal

	virtual void OnInjury( float amount ) { }				///< invoked when the improv is injured
};

//--------------------------------------------------------------------------------------------------------
/**
 * The Improv interface definition
 *
 * An "Improv" is an improvisational actor that simulates the 
 * behavor of a human in an unscripted, "make it up as you go" manner.
 */
class CImprov : public IImprovEvent
{
public:
	virtual ~CImprov() { }

	virtual bool IsAlive( void ) const = 0;								///< return true if this improv is alive

	virtual void MoveTo( const Vector &goal ) = 0;						///< move improv towards far-away goal (pathfind)
	virtual void LookAt( const Vector &target ) = 0;					///< define desired view target
	virtual void ClearLookAt( void ) = 0;								///< remove view goal
	virtual void FaceTo( const Vector &goal ) = 0;						///< orient body towards goal
	virtual void ClearFaceTo( void ) = 0;								///< remove body orientation goal

	virtual bool IsAtMoveGoal( float error = 20.0f ) const = 0;			///< return true if improv is standing on its movement goal
	virtual bool HasLookAt( void ) const = 0;						///< return true if improv has a look at goal
	virtual bool HasFaceTo( void ) const = 0;						///< return true if improv has a face to goal
	virtual bool IsAtFaceGoal( void ) const = 0;						///< return true if improv is facing towards its face goal
	virtual bool IsFriendInTheWay( const Vector &goalPos ) const = 0;	///< return true if a friend is blocking our line to the given goal position
	virtual bool IsFriendInTheWay( CBaseEntity *myFriend, const Vector &goalPos ) const = 0;	///< return true if the given friend is blocking our line to the given goal position

	virtual void MoveForward( void ) = 0;
	virtual void MoveBackward( void ) = 0;
	virtual void StrafeLeft( void ) = 0;
	virtual void StrafeRight( void ) = 0;
	virtual bool Jump( void ) = 0;
	virtual void Crouch( void ) = 0;
	virtual void StandUp( void ) = 0;									///< "un-crouch"

	virtual void TrackPath( const Vector &pathGoal, float deltaT ) = 0;	///< move along path by following "pathGoal"
	
	virtual void StartLadder( const CNavLadder *ladder, NavTraverseType how, const Vector *approachPos, const Vector *departPos ) = 0;	///< invoked when a ladder is encountered while following a path
	virtual bool TraverseLadder( const CNavLadder *ladder, NavTraverseType how, const Vector *approachPos, const Vector *departPos, float deltaT ) = 0;	///< traverse given ladder

	virtual bool GetSimpleGroundHeightWithFloor( const Vector *pos, float *height, Vector *normal = NULL ) = 0;	///< find "simple" ground height, treating current nav area as part of the floor

	virtual void Run( void ) = 0;
	virtual void Walk( void ) = 0;
	virtual void Stop( void ) = 0;

	virtual float GetMoveAngle( void ) const = 0;						///< return direction of movement
	virtual float GetFaceAngle( void ) const = 0;						///< return direction of view

	virtual const Vector &GetFeet( void ) const = 0;					///< return position of "feet" - point below centroid of improv at feet level
	virtual const Vector &GetCentroid( void ) const = 0;
	virtual const Vector &GetEyes( void ) const = 0;

	virtual bool IsRunning( void ) const = 0;
	virtual bool IsWalking( void ) const = 0;
	virtual bool IsStopped( void ) const = 0;

	virtual bool IsCrouching( void ) const = 0;
	virtual bool IsJumping( void ) const = 0;
	virtual bool IsUsingLadder( void ) const = 0;
	virtual bool IsOnGround( void ) const = 0;
	virtual bool IsMoving( void ) const = 0;							///< if true, improv is walking, crawling, running somewhere

	virtual bool CanRun( void ) const = 0;
	virtual bool CanCrouch( void ) const = 0;
	virtual bool CanJump( void ) const = 0;

	#define CHECK_FOV true
	virtual bool IsVisible( const Vector &pos, bool testFOV = false ) const = 0;	///< return true if improv can see position

	virtual bool IsPlayerLookingAtMe( CBasePlayer *other, float cosTolerance = 0.95f ) const = 0;	///< return true if 'other' is looking right at me
	virtual CBasePlayer *IsAnyPlayerLookingAtMe( int team = 0, float cosTolerance = 0.95f ) const = 0;	///< return player on given team that is looking right at me (team == 0 means any team), NULL otherwise

	virtual CBasePlayer *GetClosestPlayerByTravelDistance( int team = 0, float *range = NULL ) const = 0;	///< return actual travel distance to closest player on given team (team == 0 means any team)

	virtual CNavArea *GetLastKnownArea( void ) const = 0;

	virtual void OnUpdate( float deltaT ) = 0;							///< a less frequent, full update 'tick'
	virtual void OnUpkeep( float deltaT ) = 0;							///< a frequent, lightweight update 'tick'
	virtual void OnReset( void ) = 0;									///< reset improv to initial state
	virtual void OnGameEvent( GameEventType event, CBaseEntity *entity, CBaseEntity *other ) = 0;	///< invoked when an event occurs in the game
	virtual void OnTouch( CBaseEntity *other ) = 0;						///< "other" has touched us
};


#endif // _IMPROV_H_
