/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
//=========================================================
// Scheduling 
//=========================================================

#ifndef	SCHEDULE_H
#define	SCHEDULE_H

#define	TASKSTATUS_NEW				0			// Just started
#define TASKSTATUS_RUNNING			1			// Running task & movement
#define TASKSTATUS_RUNNING_MOVEMENT	2			// Just running movement
#define TASKSTATUS_RUNNING_TASK		3			// Just running task
#define TASKSTATUS_COMPLETE			4			// Completed, get next task


//=========================================================
// These are the schedule types
//=========================================================
typedef enum 
{
		SCHED_NONE = 0,
		SCHED_IDLE_STAND,
		SCHED_IDLE_WALK,
		SCHED_WAKE_ANGRY,
		SCHED_WAKE_CALLED,
		SCHED_ALERT_FACE,
		SCHED_ALERT_SMALL_FLINCH,
		SCHED_ALERT_BIG_FLINCH,
		SCHED_ALERT_STAND,
		SCHED_INVESTIGATE_SOUND,
		SCHED_COMBAT_FACE,
		SCHED_COMBAT_STAND,
		SCHED_CHASE_ENEMY,
		SCHED_CHASE_ENEMY_FAILED,
		SCHED_VICTORY_DANCE,
		SCHED_TARGET_FACE,
		SCHED_TARGET_CHASE,
		SCHED_SMALL_FLINCH,
		SCHED_TAKE_COVER_FROM_ENEMY,
		SCHED_TAKE_COVER_FROM_BEST_SOUND,
		SCHED_TAKE_COVER_FROM_ORIGIN,
		SCHED_COWER, // usually a last resort!
		SCHED_MELEE_ATTACK1,
		SCHED_MELEE_ATTACK2,
		SCHED_RANGE_ATTACK1,
		SCHED_RANGE_ATTACK2,
		SCHED_SPECIAL_ATTACK1,
		SCHED_SPECIAL_ATTACK2,
		SCHED_STANDOFF,
		SCHED_ARM_WEAPON,
		SCHED_RELOAD,
		SCHED_GUARD,
		SCHED_AMBUSH,
		SCHED_DIE,
		SCHED_WAIT_TRIGGER,
		SCHED_FOLLOW,
		SCHED_SLEEP,
		SCHED_WAKE,
		SCHED_BARNACLE_VICTIM_GRAB,
		SCHED_BARNACLE_VICTIM_CHOMP,
		SCHED_AISCRIPT,
		SCHED_FAIL,

		LAST_COMMON_SCHEDULE			// Leave this at the bottom
} SCHEDULE_TYPE;

//=========================================================
// These are the shared tasks
//=========================================================
typedef enum 
{
		TASK_INVALID = 0,
		TASK_WAIT,
		TASK_WAIT_FACE_ENEMY,
		TASK_WAIT_PVS,
		TASK_SUGGEST_STATE,
		TASK_WALK_TO_TARGET,
		TASK_RUN_TO_TARGET,
		TASK_MOVE_TO_TARGET_RANGE,
		TASK_GET_PATH_TO_ENEMY,
		TASK_GET_PATH_TO_ENEMY_LKP,
		TASK_GET_PATH_TO_ENEMY_CORPSE,
		TASK_GET_PATH_TO_LEADER,
		TASK_GET_PATH_TO_SPOT,
		TASK_GET_PATH_TO_TARGET,
		TASK_GET_PATH_TO_HINTNODE,
		TASK_GET_PATH_TO_LASTPOSITION,
		TASK_GET_PATH_TO_BESTSOUND,
		TASK_GET_PATH_TO_BESTSCENT,
		TASK_RUN_PATH,	
		TASK_WALK_PATH,	
		TASK_STRAFE_PATH,
		TASK_CLEAR_MOVE_WAIT,
		TASK_STORE_LASTPOSITION,
		TASK_CLEAR_LASTPOSITION,
		TASK_PLAY_ACTIVE_IDLE,
		TASK_FIND_HINTNODE,
		TASK_CLEAR_HINTNODE,
		TASK_SMALL_FLINCH,
		TASK_FACE_IDEAL,
		TASK_FACE_ROUTE,
		TASK_FACE_ENEMY,
		TASK_FACE_HINTNODE,
		TASK_FACE_TARGET,
		TASK_FACE_LASTPOSITION,
		TASK_RANGE_ATTACK1,
		TASK_RANGE_ATTACK2,		
		TASK_MELEE_ATTACK1,		
		TASK_MELEE_ATTACK2,		
		TASK_RELOAD,
		TASK_RANGE_ATTACK1_NOTURN,
		TASK_RANGE_ATTACK2_NOTURN,		
		TASK_MELEE_ATTACK1_NOTURN,		
		TASK_MELEE_ATTACK2_NOTURN,		
		TASK_RELOAD_NOTURN,
		TASK_SPECIAL_ATTACK1,
		TASK_SPECIAL_ATTACK2,
		TASK_CROUCH,
		TASK_STAND,
		TASK_GUARD,
		TASK_STEP_LEFT,
		TASK_STEP_RIGHT,
		TASK_STEP_FORWARD,
		TASK_STEP_BACK,
		TASK_DODGE_LEFT,
		TASK_DODGE_RIGHT,
		TASK_SOUND_ANGRY,
		TASK_SOUND_DEATH,
		TASK_SET_ACTIVITY,
		TASK_SET_SCHEDULE,
		TASK_SET_FAIL_SCHEDULE,
		TASK_CLEAR_FAIL_SCHEDULE,
		TASK_PLAY_SEQUENCE,
		TASK_PLAY_SEQUENCE_FACE_ENEMY,
		TASK_PLAY_SEQUENCE_FACE_TARGET,
		TASK_SOUND_IDLE,
		TASK_SOUND_WAKE,
		TASK_SOUND_PAIN,
		TASK_SOUND_DIE,
		TASK_FIND_COVER_FROM_BEST_SOUND,// tries lateral cover first, then node cover
		TASK_FIND_COVER_FROM_ENEMY,// tries lateral cover first, then node cover
		TASK_FIND_LATERAL_COVER_FROM_ENEMY,
		TASK_FIND_NODE_COVER_FROM_ENEMY,
		TASK_FIND_NEAR_NODE_COVER_FROM_ENEMY,// data for this one is the MAXIMUM acceptable distance to the cover.
		TASK_FIND_FAR_NODE_COVER_FROM_ENEMY,// data for this one is there MINIMUM aceptable distance to the cover.
		TASK_FIND_COVER_FROM_ORIGIN,
		TASK_EAT,
		TASK_DIE,
		TASK_WAIT_FOR_SCRIPT,
		TASK_PLAY_SCRIPT,
		TASK_ENABLE_SCRIPT,
		TASK_PLANT_ON_SCRIPT,
		TASK_FACE_SCRIPT,
		TASK_WAIT_RANDOM,
		TASK_WAIT_INDEFINITE,
		TASK_STOP_MOVING,
		TASK_TURN_LEFT,
		TASK_TURN_RIGHT,
		TASK_REMEMBER,
		TASK_FORGET,
		TASK_WAIT_FOR_MOVEMENT,			// wait until MovementIsComplete()
		LAST_COMMON_TASK, // LEAVE THIS AT THE BOTTOM!! (sjb)
} SHARED_TASKS;


// These go in the flData member of the TASK_WALK_TO_TARGET, TASK_RUN_TO_TARGET
enum 
{
	TARGET_MOVE_NORMAL = 0,
	TARGET_MOVE_SCRIPTED = 1,
};


// A goal should be used for a task that requires several schedules to complete.  
// The goal index should indicate which schedule (ordinally) the monster is running.  
// That way, when tasks fail, the AI can make decisions based on the context of the 
// current goal and sequence rather than just the current schedule.
enum
{
	GOAL_ATTACK_ENEMY,
	GOAL_MOVE,
	GOAL_TAKE_COVER,
	GOAL_MOVE_TARGET,
	GOAL_EAT,
};

// an array of tasks is a task list
// an array of schedules is a schedule list
struct Task_t
{

	int		iTask;
	float	flData;
};

struct Schedule_t
{

	Task_t	*pTasklist;
	int		cTasks;	 
	int		iInterruptMask;// a bit mask of conditions that can interrupt this schedule 
	
	// a more specific mask that indicates which TYPES of sounds will interrupt the schedule in the 
	// event that the schedule is broken by COND_HEAR_SOUND
	int		iSoundMask;
	const	char *pName;
};

// an array of waypoints makes up the monster's route. 
// !!!LATER- this declaration doesn't belong in this file.
struct WayPoint_t
{
	Vector	vecLocation;
	int		iType;
};

// these MoveFlag values are assigned to a WayPoint's TYPE in order to demonstrate the 
// type of movement the monster should use to get there.
#define bits_MF_TO_TARGETENT		( 1 << 0 ) // local move to targetent.
#define bits_MF_TO_ENEMY			( 1 << 1 ) // local move to enemy
#define bits_MF_TO_COVER			( 1 << 2 ) // local move to a hiding place
#define bits_MF_TO_DETOUR			( 1 << 3 ) // local move to detour point.
#define bits_MF_TO_PATHCORNER		( 1 << 4 ) // local move to a path corner
#define bits_MF_TO_NODE				( 1 << 5 ) // local move to a node
#define bits_MF_TO_LOCATION			( 1 << 6 ) // local move to an arbitrary point
#define bits_MF_IS_GOAL				( 1 << 7 ) // this waypoint is the goal of the whole move.
#define bits_MF_DONT_SIMPLIFY		( 1 << 8 ) // Don't let the route code simplify this waypoint

// If you define any flags that aren't _TO_ flags, add them here so we can mask
// them off when doing compares.
#define bits_MF_NOT_TO_MASK (bits_MF_IS_GOAL | bits_MF_DONT_SIMPLIFY)

#define MOVEGOAL_NONE				(0)
#define MOVEGOAL_TARGETENT			(bits_MF_TO_TARGETENT)
#define MOVEGOAL_ENEMY				(bits_MF_TO_ENEMY)
#define MOVEGOAL_PATHCORNER			(bits_MF_TO_PATHCORNER)
#define MOVEGOAL_LOCATION			(bits_MF_TO_LOCATION)
#define MOVEGOAL_NODE				(bits_MF_TO_NODE)

// these bits represent conditions that may befall the monster, of which some are allowed 
// to interrupt certain schedules. 
#define bits_COND_NO_AMMO_LOADED		( 1 << 0 ) // weapon needs to be reloaded!
#define	bits_COND_SEE_HATE				( 1 << 1 ) // see something that you hate
#define bits_COND_SEE_FEAR				( 1 << 2 ) // see something that you are afraid of
#define bits_COND_SEE_DISLIKE			( 1 << 3 ) // see something that you dislike
#define bits_COND_SEE_ENEMY				( 1 << 4 ) // target entity is in full view.
#define bits_COND_ENEMY_OCCLUDED		( 1 << 5 ) // target entity occluded by the world
#define bits_COND_SMELL_FOOD			( 1 << 6 )
#define bits_COND_ENEMY_TOOFAR			( 1 << 7 )
#define bits_COND_LIGHT_DAMAGE			( 1 << 8 ) // hurt a little 
#define bits_COND_HEAVY_DAMAGE			( 1 << 9 ) // hurt a lot
#define bits_COND_CAN_RANGE_ATTACK1		( 1 << 10)
#define bits_COND_CAN_MELEE_ATTACK1		( 1 << 11)
#define bits_COND_CAN_RANGE_ATTACK2		( 1 << 12)
#define bits_COND_CAN_MELEE_ATTACK2		( 1 << 13)
// #define bits_COND_CAN_RANGE_ATTACK3		( 1 << 14)
#define bits_COND_PROVOKED				( 1 << 15)
#define bits_COND_NEW_ENEMY				( 1 << 16)
#define bits_COND_HEAR_SOUND			( 1 << 17) // there is an interesting sound
#define bits_COND_SMELL					( 1 << 18) // there is an interesting scent
#define bits_COND_ENEMY_FACING_ME		( 1 << 19) // enemy is facing me
#define bits_COND_ENEMY_DEAD			( 1 << 20) // enemy was killed. If you get this in combat, try to find another enemy. If you get it in alert, victory dance.
#define bits_COND_SEE_CLIENT			( 1 << 21) // see a client
#define bits_COND_SEE_NEMESIS			( 1 << 22) // see my nemesis

#define bits_COND_SPECIAL1				( 1 << 28) // Defined by individual monster
#define bits_COND_SPECIAL2				( 1 << 29) // Defined by individual monster

#define bits_COND_TASK_FAILED			( 1 << 30)
#define bits_COND_SCHEDULE_DONE			( 1 << 31)


#define bits_COND_ALL_SPECIAL			(bits_COND_SPECIAL1 | bits_COND_SPECIAL2)

#define bits_COND_CAN_ATTACK			(bits_COND_CAN_RANGE_ATTACK1 | bits_COND_CAN_MELEE_ATTACK1 | bits_COND_CAN_RANGE_ATTACK2 | bits_COND_CAN_MELEE_ATTACK2)

#endif	// SCHEDULE_H
