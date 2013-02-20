/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
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
// schedule.cpp - functions and data pertaining to the 
// monsters' AI scheduling system.
//=========================================================
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "animation.h"
#include "scripted.h"
#include "nodes.h"
#include "defaultai.h"
#include "soundent.h"

extern CGraph WorldGraph;

//=========================================================
// FHaveSchedule - Returns TRUE if monster's m_pSchedule
// is anything other than NULL.
//=========================================================
BOOL CBaseMonster :: FHaveSchedule( void )
{
	if ( m_pSchedule == NULL )
	{
		return FALSE;
	}

	return TRUE;
}

//=========================================================
// ClearSchedule - blanks out the caller's schedule pointer
// and index.
//=========================================================
void CBaseMonster :: ClearSchedule( void )
{
	m_iTaskStatus = TASKSTATUS_NEW;
	m_pSchedule = NULL;
	m_iScheduleIndex = 0;
}

//=========================================================
// FScheduleDone - Returns TRUE if the caller is on the
// last task in the schedule
//=========================================================
BOOL CBaseMonster :: FScheduleDone ( void )
{
	ASSERT( m_pSchedule != NULL );
	
	if ( m_iScheduleIndex == m_pSchedule->cTasks )
	{
		return TRUE;
	}

	return FALSE;
}

//=========================================================
// ChangeSchedule - replaces the monster's schedule pointer
// with the passed pointer, and sets the ScheduleIndex back
// to 0
//=========================================================
void CBaseMonster :: ChangeSchedule ( Schedule_t *pNewSchedule )
{
	ASSERT( pNewSchedule != NULL );

	m_pSchedule			= pNewSchedule;
	m_iScheduleIndex	= 0;
	m_iTaskStatus		= TASKSTATUS_NEW;
	m_afConditions		= 0;// clear all of the conditions
	m_failSchedule		= SCHED_NONE;

	if ( m_pSchedule->iInterruptMask & bits_COND_HEAR_SOUND && !(m_pSchedule->iSoundMask) )
	{
		ALERT ( at_aiconsole, "COND_HEAR_SOUND with no sound mask!\n" );
	}
	else if ( m_pSchedule->iSoundMask && !(m_pSchedule->iInterruptMask & bits_COND_HEAR_SOUND) )
	{
		ALERT ( at_aiconsole, "Sound mask without COND_HEAR_SOUND!\n" );
	}

#if _DEBUG
	if ( !ScheduleFromName( pNewSchedule->pName ) )
	{
		ALERT( at_console, "Schedule %s not in table!!!\n", pNewSchedule->pName );
	}
#endif
	
// this is very useful code if you can isolate a test case in a level with a single monster. It will notify
// you of every schedule selection the monster makes.
#if 0
	if ( FClassnameIs( pev, "monster_human_grunt" ) )
	{
		Task_t *pTask = GetTask();
		
		if ( pTask )
		{
			const char *pName = NULL;

			if ( m_pSchedule )
			{
				pName = m_pSchedule->pName;
			}
			else
			{
				pName = "No Schedule";
			}
			
			if ( !pName )
			{
				pName = "Unknown";
			}

			ALERT( at_aiconsole, "%s: picked schedule %s\n", STRING( pev->classname ), pName );
		}
	}
#endif// 0

}

//=========================================================
// NextScheduledTask - increments the ScheduleIndex
//=========================================================
void CBaseMonster :: NextScheduledTask ( void )
{
	ASSERT( m_pSchedule != NULL );

	m_iTaskStatus = TASKSTATUS_NEW;
	m_iScheduleIndex++;

	if ( FScheduleDone() )
	{
		// just completed last task in schedule, so make it invalid by clearing it.
		SetConditions( bits_COND_SCHEDULE_DONE );
		//ClearSchedule();	
	}
}

//=========================================================
// IScheduleFlags - returns an integer with all Conditions
// bits that are currently set and also set in the current
// schedule's Interrupt mask.
//=========================================================
int CBaseMonster :: IScheduleFlags ( void )
{
	if( !m_pSchedule )
	{
		return 0;
	}
	
	// strip off all bits excepts the ones capable of breaking this schedule.
	return m_afConditions & m_pSchedule->iInterruptMask;
}

//=========================================================
// FScheduleValid - returns TRUE as long as the current
// schedule is still the proper schedule to be executing,
// taking into account all conditions
//=========================================================
BOOL CBaseMonster :: FScheduleValid ( void )
{
	if ( m_pSchedule == NULL )
	{
		// schedule is empty, and therefore not valid.
		return FALSE;
	}

	if ( HasConditions( m_pSchedule->iInterruptMask | bits_COND_SCHEDULE_DONE | bits_COND_TASK_FAILED ) )
	{
#ifdef DEBUG
		if ( HasConditions ( bits_COND_TASK_FAILED ) && m_failSchedule == SCHED_NONE )
		{
			// fail! Send a visual indicator.
			ALERT ( at_aiconsole, "Schedule: %s Failed\n", m_pSchedule->pName );

			Vector tmp = pev->origin;
			tmp.z = pev->absmax.z + 16;
			UTIL_Sparks( tmp );
		}
#endif // DEBUG

		// some condition has interrupted the schedule, or the schedule is done
		return FALSE;
	}
	
	return TRUE;
}

//=========================================================
// MaintainSchedule - does all the per-think schedule maintenance.
// ensures that the monster leaves this function with a valid
// schedule!
//=========================================================
void CBaseMonster :: MaintainSchedule ( void )
{
	Schedule_t	*pNewSchedule;
	int			i;

	// UNDONE: Tune/fix this 10... This is just here so infinite loops are impossible
	for ( i = 0; i < 10; i++ )
	{
		if ( m_pSchedule != NULL && TaskIsComplete() )
		{
			NextScheduledTask();                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   
		}

	// validate existing schedule 
		if ( !FScheduleValid() || m_MonsterState != m_IdealMonsterState )
		{
			// if we come into this block of code, the schedule is going to have to be changed.
			// if the previous schedule was interrupted by a condition, GetIdealState will be 
			// called. Else, a schedule finished normally.

			// Notify the monster that his schedule is changing
			ScheduleChange();

			// Call GetIdealState if we're not dead and one or more of the following...
			// - in COMBAT state with no enemy (it died?)
			// - conditions bits (excluding SCHEDULE_DONE) indicate interruption,
			// - schedule is done but schedule indicates it wants GetIdealState called
			//   after successful completion (by setting bits_COND_SCHEDULE_DONE in iInterruptMask)
			// DEAD & SCRIPT are not suggestions, they are commands!
			if ( m_IdealMonsterState != MONSTERSTATE_DEAD && 
				 (m_IdealMonsterState != MONSTERSTATE_SCRIPT || m_IdealMonsterState == m_MonsterState) )
			{
				if (	(m_afConditions && !HasConditions(bits_COND_SCHEDULE_DONE)) ||
						(m_pSchedule && (m_pSchedule->iInterruptMask & bits_COND_SCHEDULE_DONE)) ||
						((m_MonsterState == MONSTERSTATE_COMBAT) && (m_hEnemy == NULL))	)
				{
					GetIdealState();
				}
			}
			if ( HasConditions( bits_COND_TASK_FAILED ) && m_MonsterState == m_IdealMonsterState )
			{
				if ( m_failSchedule != SCHED_NONE )
					pNewSchedule = GetScheduleOfType( m_failSchedule );
				else
					pNewSchedule = GetScheduleOfType( SCHED_FAIL );
				// schedule was invalid because the current task failed to start or complete
				ALERT ( at_aiconsole, "Schedule Failed at %d!\n", m_iScheduleIndex );
				ChangeSchedule( pNewSchedule );
			}
			else
			{
				SetState( m_IdealMonsterState );
				if ( m_MonsterState == MONSTERSTATE_SCRIPT || m_MonsterState == MONSTERSTATE_DEAD )
					pNewSchedule = CBaseMonster::GetSchedule();
				else
					pNewSchedule = GetSchedule();
				ChangeSchedule( pNewSchedule );
			}
		}

		if ( m_iTaskStatus == TASKSTATUS_NEW )
		{	
			Task_t *pTask = GetTask();
			ASSERT( pTask != NULL );
			TaskBegin();
			StartTask( pTask );
		}

		// UNDONE: Twice?!!!
		if ( m_Activity != m_IdealActivity )
		{
			SetActivity ( m_IdealActivity );
		}
		
		if ( !TaskIsComplete() && m_iTaskStatus != TASKSTATUS_NEW )
			break;
	}

	if ( TaskIsRunning() )
	{
		Task_t *pTask = GetTask();
		ASSERT( pTask != NULL );
		RunTask( pTask );
	}

	// UNDONE: We have to do this so that we have an animation set to blend to if RunTask changes the animation
	// RunTask() will always change animations at the end of a script!
	// Don't do this twice
	if ( m_Activity != m_IdealActivity )
	{
		SetActivity ( m_IdealActivity );
	}
}

//=========================================================
// RunTask 
//=========================================================
void CBaseMonster :: RunTask ( Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_TURN_RIGHT:
	case TASK_TURN_LEFT:
		{
			ChangeYaw( pev->yaw_speed );

			if ( FacingIdeal() )
			{
				TaskComplete();
			}
			break;
		}

	case TASK_PLAY_SEQUENCE_FACE_ENEMY:
	case TASK_PLAY_SEQUENCE_FACE_TARGET:
		{
			CBaseEntity *pTarget;

			if ( pTask->iTask == TASK_PLAY_SEQUENCE_FACE_TARGET )
				pTarget = m_hTargetEnt;
			else
				pTarget = m_hEnemy;
			if ( pTarget )
			{
				pev->ideal_yaw = UTIL_VecToYaw( pTarget->pev->origin - pev->origin );
				ChangeYaw( pev->yaw_speed );
			}
			if ( m_fSequenceFinished )
				TaskComplete();
		}
		break;

	case TASK_PLAY_SEQUENCE:
	case TASK_PLAY_ACTIVE_IDLE:
		{
			if ( m_fSequenceFinished )
			{
				TaskComplete();
			}
			break;
		}


	case TASK_FACE_ENEMY:
		{
			MakeIdealYaw( m_vecEnemyLKP );

			ChangeYaw( pev->yaw_speed );

			if ( FacingIdeal() )
			{
				TaskComplete();
			}
			break;
		}
	case TASK_FACE_HINTNODE:
	case TASK_FACE_LASTPOSITION:
	case TASK_FACE_TARGET:
	case TASK_FACE_IDEAL:
	case TASK_FACE_ROUTE:
		{
			ChangeYaw( pev->yaw_speed );

			if ( FacingIdeal() )
			{
				TaskComplete();
			}
			break;
		}
	case TASK_WAIT_PVS:
		{
			if ( !FNullEnt(FIND_CLIENT_IN_PVS(edict())) )
			{
				TaskComplete();
			}
			break;
		}
	case TASK_WAIT_INDEFINITE:
		{
			// don't do anything.
			break;
		}
	case TASK_WAIT:
	case TASK_WAIT_RANDOM:
		{
			if ( gpGlobals->time >= m_flWaitFinished )
			{
				TaskComplete();
			}
			break;
		}
	case TASK_WAIT_FACE_ENEMY:
		{
			MakeIdealYaw ( m_vecEnemyLKP );
			ChangeYaw( pev->yaw_speed ); 

			if ( gpGlobals->time >= m_flWaitFinished )
			{
				TaskComplete();
			}
			break;
		}
	case TASK_MOVE_TO_TARGET_RANGE:
		{
			float distance;

			if ( m_hTargetEnt == NULL )
				TaskFail();
			else
			{
				distance = ( m_vecMoveGoal - pev->origin ).Length2D();
				// Re-evaluate when you think your finished, or the target has moved too far
				if ( (distance < pTask->flData) || (m_vecMoveGoal - m_hTargetEnt->pev->origin).Length() > pTask->flData * 0.5 )
				{
					m_vecMoveGoal = m_hTargetEnt->pev->origin;
					distance = ( m_vecMoveGoal - pev->origin ).Length2D();
					FRefreshRoute();
				}

				// Set the appropriate activity based on an overlapping range
				// overlap the range to prevent oscillation
				if ( distance < pTask->flData )
				{
					TaskComplete();
					RouteClear();		// Stop moving
				}
				else if ( distance < 190 && m_movementActivity != ACT_WALK )
					m_movementActivity = ACT_WALK;
				else if ( distance >= 270 && m_movementActivity != ACT_RUN )
					m_movementActivity = ACT_RUN;
			}

			break;
		}
	case TASK_WAIT_FOR_MOVEMENT:
		{
			if (MovementIsComplete())
			{
				TaskComplete();
				RouteClear();		// Stop moving
			}
			break;
		}
	case TASK_DIE:
		{
			if ( m_fSequenceFinished && pev->frame >= 255 )
			{
				pev->deadflag = DEAD_DEAD;
				
				SetThink ( NULL );
				StopAnimation();

				if ( !BBoxFlat() )
				{
					// a bit of a hack. If a corpses' bbox is positioned such that being left solid so that it can be attacked will
					// block the player on a slope or stairs, the corpse is made nonsolid. 
//					pev->solid = SOLID_NOT;
					UTIL_SetSize ( pev, Vector ( -4, -4, 0 ), Vector ( 4, 4, 1 ) );
				}
				else // !!!HACKHACK - put monster in a thin, wide bounding box until we fix the solid type/bounding volume problem
					UTIL_SetSize ( pev, Vector ( pev->mins.x, pev->mins.y, pev->mins.z ), Vector ( pev->maxs.x, pev->maxs.y, pev->mins.z + 1 ) );

				if ( ShouldFadeOnDeath() )
				{
					// this monster was created by a monstermaker... fade the corpse out.
					SUB_StartFadeOut();
				}
				else
				{
					// body is gonna be around for a while, so have it stink for a bit.
					CSoundEnt::InsertSound ( bits_SOUND_CARCASS, pev->origin, 384, 30 );
				}
			}
			break;
		}
	case TASK_RANGE_ATTACK1_NOTURN:
	case TASK_MELEE_ATTACK1_NOTURN:
	case TASK_MELEE_ATTACK2_NOTURN:
	case TASK_RANGE_ATTACK2_NOTURN:
	case TASK_RELOAD_NOTURN:
		{
			if ( m_fSequenceFinished )
			{
				m_Activity = ACT_RESET;
				TaskComplete();
			}
			break;
		}
	case TASK_RANGE_ATTACK1:
	case TASK_MELEE_ATTACK1:
	case TASK_MELEE_ATTACK2:
	case TASK_RANGE_ATTACK2:
	case TASK_SPECIAL_ATTACK1:
	case TASK_SPECIAL_ATTACK2:
	case TASK_RELOAD:
		{
			MakeIdealYaw ( m_vecEnemyLKP );
			ChangeYaw ( pev->yaw_speed );

			if ( m_fSequenceFinished )
			{
				m_Activity = ACT_RESET;
				TaskComplete();
			}
			break;
		}
	case TASK_SMALL_FLINCH:
		{
			if ( m_fSequenceFinished )
			{
				TaskComplete();
			}
		}
		break;
	case TASK_WAIT_FOR_SCRIPT:
		{
			if ( m_pCine->m_iDelay <= 0 && gpGlobals->time >= m_pCine->m_startTime )
			{
				TaskComplete();
				m_pCine->StartSequence( (CBaseMonster *)this, m_pCine->m_iszPlay, TRUE );
				if ( m_fSequenceFinished )
					ClearSchedule();
				pev->framerate = 1.0;
				//ALERT( at_aiconsole, "Script %s has begun for %s\n", STRING( m_pCine->m_iszPlay ), STRING(pev->classname) );
			}
			break;
		}
	case TASK_PLAY_SCRIPT:
		{
			if (m_fSequenceFinished)
			{
				m_pCine->SequenceDone( this );
			}
			break;
		}
	}
}

//=========================================================
// SetTurnActivity - measures the difference between the way
// the monster is facing and determines whether or not to
// select one of the 180 turn animations.
//=========================================================
void CBaseMonster :: SetTurnActivity ( void )
{
	float flYD;
	flYD = FlYawDiff();

	if ( flYD <= -45 && LookupActivity ( ACT_TURN_RIGHT ) != ACTIVITY_NOT_AVAILABLE )
	{// big right turn
		m_IdealActivity = ACT_TURN_RIGHT;
	}
	else if ( flYD > 45 && LookupActivity ( ACT_TURN_LEFT ) != ACTIVITY_NOT_AVAILABLE )
	{// big left turn
		m_IdealActivity = ACT_TURN_LEFT;
	}
}

//=========================================================
// Start task - selects the correct activity and performs
// any necessary calculations to start the next task on the
// schedule. 
//=========================================================
void CBaseMonster :: StartTask ( Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_TURN_RIGHT:
		{
			float flCurrentYaw;
			
			flCurrentYaw = UTIL_AngleMod( pev->angles.y );
			pev->ideal_yaw = UTIL_AngleMod( flCurrentYaw - pTask->flData );
			SetTurnActivity();
			break;
		}
	case TASK_TURN_LEFT:
		{
			float flCurrentYaw;
			
			flCurrentYaw = UTIL_AngleMod( pev->angles.y );
			pev->ideal_yaw = UTIL_AngleMod( flCurrentYaw + pTask->flData );
			SetTurnActivity();
			break;
		}
	case TASK_REMEMBER:
		{
			Remember ( (int)pTask->flData );
			TaskComplete();
			break;
		}
	case TASK_FORGET:
		{
			Forget ( (int)pTask->flData );
			TaskComplete();
			break;
		}
	case TASK_FIND_HINTNODE:
		{
			m_iHintNode = FindHintNode();

			if ( m_iHintNode != NO_NODE )
			{
				TaskComplete();
			}
			else
			{
				TaskFail();
			}
			break;
		}
	case TASK_STORE_LASTPOSITION:
		{
			m_vecLastPosition = pev->origin;
			TaskComplete();
			break;
		}
	case TASK_CLEAR_LASTPOSITION:
		{
			m_vecLastPosition = g_vecZero;
			TaskComplete();
			break;
		}
	case TASK_CLEAR_HINTNODE:
		{
			m_iHintNode = NO_NODE;
			TaskComplete();
			break;
		}
	case TASK_STOP_MOVING:
		{
			if ( m_IdealActivity == m_movementActivity )
			{
				m_IdealActivity = GetStoppedActivity();
			}

			RouteClear();
			TaskComplete();
			break;
		}
	case TASK_PLAY_SEQUENCE_FACE_ENEMY:
	case TASK_PLAY_SEQUENCE_FACE_TARGET:
	case TASK_PLAY_SEQUENCE:
		{
			m_IdealActivity = ( Activity )( int )pTask->flData;
			break;
		}
	case TASK_PLAY_ACTIVE_IDLE:
		{
			// monsters verify that they have a sequence for the node's activity BEFORE
			// moving towards the node, so it's ok to just set the activity without checking here.
			m_IdealActivity = ( Activity )WorldGraph.m_pNodes[ m_iHintNode ].m_sHintActivity;
			break;
		}
	case TASK_SET_SCHEDULE:
		{
			Schedule_t *pNewSchedule;

			pNewSchedule = GetScheduleOfType( (int)pTask->flData );
			
			if ( pNewSchedule )
			{
				ChangeSchedule( pNewSchedule );
			}
			else
			{
				TaskFail();
			}

			break;
		}
	case TASK_FIND_NEAR_NODE_COVER_FROM_ENEMY:
		{
			if ( m_hEnemy == NULL )
			{
				TaskFail();
				return;
			}

			if ( FindCover( m_hEnemy->pev->origin, m_hEnemy->pev->view_ofs, 0, pTask->flData ) )
			{
				// try for cover farther than the FLData from the schedule.
				TaskComplete();
			}
			else
			{
				// no coverwhatsoever.
				TaskFail();
			}
			break;
		}
	case TASK_FIND_FAR_NODE_COVER_FROM_ENEMY:
		{
			if ( m_hEnemy == NULL )
			{
				TaskFail();
				return;
			}

			if ( FindCover( m_hEnemy->pev->origin, m_hEnemy->pev->view_ofs, pTask->flData, CoverRadius() ) )
			{
				// try for cover farther than the FLData from the schedule.
				TaskComplete();
			}
			else
			{
				// no coverwhatsoever.
				TaskFail();
			}
			break;
		}
	case TASK_FIND_NODE_COVER_FROM_ENEMY:
		{
			if ( m_hEnemy == NULL )
			{
				TaskFail();
				return;
			}

			if ( FindCover( m_hEnemy->pev->origin, m_hEnemy->pev->view_ofs, 0, CoverRadius() ) )
			{
				// try for cover farther than the FLData from the schedule.
				TaskComplete();
			}
			else
			{
				// no coverwhatsoever.
				TaskFail();
			}
			break;
		}
	case TASK_FIND_COVER_FROM_ENEMY:
		{
			entvars_t *pevCover;

			if ( m_hEnemy == NULL )
			{
				// Find cover from self if no enemy available
				pevCover = pev;
//				TaskFail();
//				return;
			}
			else
				pevCover = m_hEnemy->pev;

			if ( FindLateralCover( pevCover->origin, pevCover->view_ofs ) )
			{
				// try lateral first
				m_flMoveWaitFinished = gpGlobals->time + pTask->flData;
				TaskComplete();
			}
			else if ( FindCover( pevCover->origin, pevCover->view_ofs, 0, CoverRadius() ) )
			{
				// then try for plain ole cover
				m_flMoveWaitFinished = gpGlobals->time + pTask->flData;
				TaskComplete();
			}
			else
			{
				// no coverwhatsoever.
				TaskFail();
			}
			break;
		}
	case TASK_FIND_COVER_FROM_ORIGIN:
		{
			if ( FindCover( pev->origin, pev->view_ofs, 0, CoverRadius() ) )
			{
				// then try for plain ole cover
				m_flMoveWaitFinished = gpGlobals->time + pTask->flData;
				TaskComplete();
			}
			else
			{
				// no cover!
				TaskFail();
			}
		}
		break;
	case TASK_FIND_COVER_FROM_BEST_SOUND:
		{
			CSound *pBestSound;

			pBestSound = PBestSound();

			ASSERT( pBestSound != NULL );
			/*
			if ( pBestSound && FindLateralCover( pBestSound->m_vecOrigin, g_vecZero ) )
			{
				// try lateral first
				m_flMoveWaitFinished = gpGlobals->time + pTask->flData;
				TaskComplete();
			}
			*/

			if ( pBestSound && FindCover( pBestSound->m_vecOrigin, g_vecZero, pBestSound->m_iVolume, CoverRadius() ) )
			{
				// then try for plain ole cover
				m_flMoveWaitFinished = gpGlobals->time + pTask->flData;
				TaskComplete();
			}
			else
			{
				// no coverwhatsoever. or no sound in list
				TaskFail();
			}
			break;
		}
	case TASK_FACE_HINTNODE:
		{
			pev->ideal_yaw = WorldGraph.m_pNodes[ m_iHintNode ].m_flHintYaw;
			SetTurnActivity();
			break;
		}
	
	case TASK_FACE_LASTPOSITION:
		MakeIdealYaw ( m_vecLastPosition );
		SetTurnActivity(); 
		break;

	case TASK_FACE_TARGET:
		if ( m_hTargetEnt != NULL )
		{
			MakeIdealYaw ( m_hTargetEnt->pev->origin );
			SetTurnActivity(); 
		}
		else
			TaskFail();
		break;
	case TASK_FACE_ENEMY:
		{
			MakeIdealYaw ( m_vecEnemyLKP );
			SetTurnActivity(); 
			break;
		}
	case TASK_FACE_IDEAL:
		{
			SetTurnActivity();
			break;
		}
	case TASK_FACE_ROUTE:
		{
			if (FRouteClear())
			{
				ALERT(at_aiconsole, "No route to face!\n");
				TaskFail();
			}
			else
			{
				MakeIdealYaw(m_Route[m_iRouteIndex].vecLocation);
				SetTurnActivity();
			}
			break;
		}
	case TASK_WAIT_PVS:
	case TASK_WAIT_INDEFINITE:
		{
			// don't do anything.
			break;
		}
	case TASK_WAIT:
	case TASK_WAIT_FACE_ENEMY:
		{// set a future time that tells us when the wait is over.
			m_flWaitFinished = gpGlobals->time + pTask->flData;	
			break;
		}
	case TASK_WAIT_RANDOM:
		{// set a future time that tells us when the wait is over.
			m_flWaitFinished = gpGlobals->time + RANDOM_FLOAT( 0.1, pTask->flData );
			break;
		}
	case TASK_MOVE_TO_TARGET_RANGE:
		{
			if ( (m_hTargetEnt->pev->origin - pev->origin).Length() < 1 )
				TaskComplete();
			else
			{
				m_vecMoveGoal = m_hTargetEnt->pev->origin;
				if ( !MoveToTarget( ACT_WALK, 2 ) )
					TaskFail();
			}
			break;
		}
	case TASK_RUN_TO_TARGET:
	case TASK_WALK_TO_TARGET:
		{
			Activity newActivity;

			if ( (m_hTargetEnt->pev->origin - pev->origin).Length() < 1 )
				TaskComplete();
			else
			{
				if ( pTask->iTask == TASK_WALK_TO_TARGET )
					newActivity = ACT_WALK;
				else
					newActivity = ACT_RUN;
				// This monster can't do this!
				if ( LookupActivity( newActivity ) == ACTIVITY_NOT_AVAILABLE )
					TaskComplete();
				else 
				{
					if ( m_hTargetEnt == NULL || !MoveToTarget( newActivity, 2 ) )
					{
						TaskFail();
						ALERT( at_aiconsole, "%s Failed to reach target!!!\n", STRING(pev->classname) );
						RouteClear();
					}
				}
			}
			TaskComplete();
			break;
		}
	case TASK_CLEAR_MOVE_WAIT:
		{
			m_flMoveWaitFinished = gpGlobals->time;
			TaskComplete();
			break;
		}
	case TASK_MELEE_ATTACK1_NOTURN:
	case TASK_MELEE_ATTACK1:
		{
			m_IdealActivity = ACT_MELEE_ATTACK1;
			break;
		}
	case TASK_MELEE_ATTACK2_NOTURN:
	case TASK_MELEE_ATTACK2:
		{
			m_IdealActivity = ACT_MELEE_ATTACK2;
			break;
		}
	case TASK_RANGE_ATTACK1_NOTURN:
	case TASK_RANGE_ATTACK1:
		{
			m_IdealActivity = ACT_RANGE_ATTACK1;
			break;
		}
	case TASK_RANGE_ATTACK2_NOTURN:
	case TASK_RANGE_ATTACK2:
		{
			m_IdealActivity = ACT_RANGE_ATTACK2;
			break;
		}
	case TASK_RELOAD_NOTURN:
	case TASK_RELOAD:
		{
			m_IdealActivity = ACT_RELOAD;
			break;
		}
	case TASK_SPECIAL_ATTACK1:
		{
			m_IdealActivity = ACT_SPECIAL_ATTACK1;
			break;
		}
	case TASK_SPECIAL_ATTACK2:
		{
			m_IdealActivity = ACT_SPECIAL_ATTACK2;
			break;
		}
	case TASK_SET_ACTIVITY:
		{
			m_IdealActivity = (Activity)(int)pTask->flData;
			TaskComplete();
			break;
		}
	case TASK_GET_PATH_TO_ENEMY_LKP:
		{
			if ( BuildRoute ( m_vecEnemyLKP, bits_MF_TO_LOCATION, NULL ) )
			{
				TaskComplete();
			}
			else if (BuildNearestRoute( m_vecEnemyLKP, pev->view_ofs, 0, (m_vecEnemyLKP - pev->origin).Length() ))
			{
				TaskComplete();
			}
			else
			{
				// no way to get there =(
				ALERT ( at_aiconsole, "GetPathToEnemyLKP failed!!\n" );
				TaskFail();
			}
			break;
		}
	case TASK_GET_PATH_TO_ENEMY:
		{
			CBaseEntity *pEnemy = m_hEnemy;

			if ( pEnemy == NULL )
			{
				TaskFail();
				return;
			}

			if ( BuildRoute ( pEnemy->pev->origin, bits_MF_TO_ENEMY, pEnemy ) )
			{
				TaskComplete();
			}
			else if (BuildNearestRoute( pEnemy->pev->origin, pEnemy->pev->view_ofs, 0, (pEnemy->pev->origin - pev->origin).Length() ))
			{
				TaskComplete();
			}
			else
			{
				// no way to get there =(
				ALERT ( at_aiconsole, "GetPathToEnemy failed!!\n" );
				TaskFail();
			}
			break;
		}
	case TASK_GET_PATH_TO_ENEMY_CORPSE:
		{
			UTIL_MakeVectors( pev->angles );
			if ( BuildRoute ( m_vecEnemyLKP - gpGlobals->v_forward * 64, bits_MF_TO_LOCATION, NULL ) )
			{
				TaskComplete();
			}
			else
			{
				ALERT ( at_aiconsole, "GetPathToEnemyCorpse failed!!\n" );
				TaskFail();
			}
		}
		break;
	case TASK_GET_PATH_TO_SPOT:
		{
			CBaseEntity *pPlayer = CBaseEntity::Instance( FIND_ENTITY_BY_CLASSNAME( NULL, "player" ) );
			if ( BuildRoute ( m_vecMoveGoal, bits_MF_TO_LOCATION, pPlayer ) )
			{
				TaskComplete();
			}
			else
			{
				// no way to get there =(
				ALERT ( at_aiconsole, "GetPathToSpot failed!!\n" );
				TaskFail();
			}
			break;
		}

	case TASK_GET_PATH_TO_TARGET:
		{
			RouteClear();
			if ( m_hTargetEnt != NULL && MoveToTarget( m_movementActivity, 1 ) )
			{
				TaskComplete();
			}
			else
			{
				// no way to get there =(
				ALERT ( at_aiconsole, "GetPathToSpot failed!!\n" );
				TaskFail();
			}
			break;
		}
	case TASK_GET_PATH_TO_HINTNODE:// for active idles!
		{
			if ( MoveToLocation( m_movementActivity, 2, WorldGraph.m_pNodes[ m_iHintNode ].m_vecOrigin ) )
			{
				TaskComplete();
			}
			else
			{
				// no way to get there =(
				ALERT ( at_aiconsole, "GetPathToHintNode failed!!\n" );
				TaskFail();
			}
			break;
		}
	case TASK_GET_PATH_TO_LASTPOSITION:
		{
			m_vecMoveGoal = m_vecLastPosition;

			if ( MoveToLocation( m_movementActivity, 2, m_vecMoveGoal ) )
			{
				TaskComplete();
			}
			else
			{
				// no way to get there =(
				ALERT ( at_aiconsole, "GetPathToLastPosition failed!!\n" );
				TaskFail();
			}
			break;
		}
	case TASK_GET_PATH_TO_BESTSOUND:
		{
			CSound *pSound;

			pSound = PBestSound();

			if ( pSound && MoveToLocation( m_movementActivity, 2, pSound->m_vecOrigin ) )
			{
				TaskComplete();
			}
			else
			{
				// no way to get there =(
				ALERT ( at_aiconsole, "GetPathToBestSound failed!!\n" );
				TaskFail();
			}
			break;
		}
case TASK_GET_PATH_TO_BESTSCENT:
		{
			CSound *pScent;

			pScent = PBestScent();

			if ( pScent && MoveToLocation( m_movementActivity, 2, pScent->m_vecOrigin ) )
			{
				TaskComplete();
			}
			else
			{
				// no way to get there =(
				ALERT ( at_aiconsole, "GetPathToBestScent failed!!\n" );
				
				TaskFail();
			}
			break;
		}
	case TASK_RUN_PATH:
		{
			// UNDONE: This is in some default AI and some monsters can't run? -- walk instead?
			if ( LookupActivity( ACT_RUN ) != ACTIVITY_NOT_AVAILABLE )
			{
				m_movementActivity = ACT_RUN;
			}
			else
			{
				m_movementActivity = ACT_WALK;
			}
			TaskComplete();
			break;
		}
	case TASK_WALK_PATH:
		{
			if ( pev->movetype == MOVETYPE_FLY )
			{
				m_movementActivity = ACT_FLY;
			}
			if ( LookupActivity( ACT_WALK ) != ACTIVITY_NOT_AVAILABLE )
			{
				m_movementActivity = ACT_WALK;
			}
			else
			{
				m_movementActivity = ACT_RUN;
			}
			TaskComplete();
			break;
		}
	case TASK_STRAFE_PATH:
		{
			Vector2D	vec2DirToPoint; 
			Vector2D	vec2RightSide;

			// to start strafing, we have to first figure out if the target is on the left side or right side
			UTIL_MakeVectors ( pev->angles );

			vec2DirToPoint = ( m_Route[ 0 ].vecLocation - pev->origin ).Make2D().Normalize();
			vec2RightSide = gpGlobals->v_right.Make2D().Normalize();

			if ( DotProduct ( vec2DirToPoint, vec2RightSide ) > 0 )
			{
				// strafe right
				m_movementActivity = ACT_STRAFE_RIGHT;
			}
			else
			{
				// strafe left
				m_movementActivity = ACT_STRAFE_LEFT;
			}
			TaskComplete();
			break;
		}


	case TASK_WAIT_FOR_MOVEMENT:
		{
			if (FRouteClear())
			{
				TaskComplete();
			}
			break;
		}

	case TASK_EAT:
		{
			Eat( pTask->flData );
			TaskComplete();
			break;
		}
	case TASK_SMALL_FLINCH:
		{
			m_IdealActivity = GetSmallFlinchActivity();
			break;
		}
	case TASK_DIE:
		{
			RouteClear();	
			
			m_IdealActivity = GetDeathActivity();

			pev->deadflag = DEAD_DYING;
			break;
		}
	case TASK_SOUND_WAKE:
		{
			AlertSound();
			TaskComplete();
			break;
		}
	case TASK_SOUND_DIE:
		{
			DeathSound();
			TaskComplete();
			break;
		}
	case TASK_SOUND_IDLE:
		{
			IdleSound();
			TaskComplete();
			break;
		}
	case TASK_SOUND_PAIN:
		{
			PainSound();
			TaskComplete();
			break;
		}
	case TASK_SOUND_DEATH:
		{
			DeathSound();
			TaskComplete();
			break;
		}
	case TASK_SOUND_ANGRY:
		{
			// sounds are complete as soon as we get here, cause we've already played them.
			ALERT ( at_aiconsole, "SOUND\n" );			
			TaskComplete();
			break;
		}
	case TASK_WAIT_FOR_SCRIPT:
		{
			if (m_pCine->m_iszIdle)
			{
				m_pCine->StartSequence( (CBaseMonster *)this, m_pCine->m_iszIdle, FALSE );
				if (FStrEq( STRING(m_pCine->m_iszIdle), STRING(m_pCine->m_iszPlay)))
				{
					pev->framerate = 0;
				}
			}
			else
				m_IdealActivity = ACT_IDLE;

			break;
		}
	case TASK_PLAY_SCRIPT:
		{
			pev->movetype = MOVETYPE_FLY;
			ClearBits(pev->flags, FL_ONGROUND);
			m_scriptState = SCRIPT_PLAYING;
			break;
		}
	case TASK_ENABLE_SCRIPT:
		{
			m_pCine->DelayStart( 0 );
			TaskComplete();
			break;
		}
	case TASK_PLANT_ON_SCRIPT:
		{
			if ( m_hTargetEnt != NULL )
			{
				pev->origin = m_hTargetEnt->pev->origin;	// Plant on target
			}

			TaskComplete();
			break;
		}
	case TASK_FACE_SCRIPT:
		{
			if ( m_hTargetEnt != NULL )
			{
				pev->ideal_yaw = UTIL_AngleMod( m_hTargetEnt->pev->angles.y );
			}

			TaskComplete();
			m_IdealActivity = ACT_IDLE;
			RouteClear();
			break;
		}

	case TASK_SUGGEST_STATE:
		{
			m_IdealMonsterState = (MONSTERSTATE)(int)pTask->flData;
			TaskComplete();
			break;
		}

	case TASK_SET_FAIL_SCHEDULE:
		m_failSchedule = (int)pTask->flData;
		TaskComplete();
		break;

	case TASK_CLEAR_FAIL_SCHEDULE:
		m_failSchedule = SCHED_NONE;
		TaskComplete();
		break;

	default:
		{
			ALERT ( at_aiconsole, "No StartTask entry for %d\n", (SHARED_TASKS)pTask->iTask );
			break;
		}
	}
}

//=========================================================
// GetTask - returns a pointer to the current 
// scheduled task. NULL if there's a problem.
//=========================================================
Task_t	*CBaseMonster :: GetTask ( void ) 
{
	if ( m_iScheduleIndex < 0 || m_iScheduleIndex >= m_pSchedule->cTasks )
	{
		// m_iScheduleIndex is not within valid range for the monster's current schedule.
		return NULL;
	}
	else
	{
		return &m_pSchedule->pTasklist[ m_iScheduleIndex ];
	}
}

//=========================================================
// GetSchedule - Decides which type of schedule best suits
// the monster's current state and conditions. Then calls
// monster's member function to get a pointer to a schedule
// of the proper type.
//=========================================================
Schedule_t *CBaseMonster :: GetSchedule ( void )
{
	switch	( m_MonsterState )
	{
	case MONSTERSTATE_PRONE:
		{
			return GetScheduleOfType( SCHED_BARNACLE_VICTIM_GRAB );
			break;
		}
	case MONSTERSTATE_NONE:
		{
			ALERT ( at_aiconsole, "MONSTERSTATE IS NONE!\n" );
			break;
		}
	case MONSTERSTATE_IDLE:
		{
			if ( HasConditions ( bits_COND_HEAR_SOUND ) )
			{
				return GetScheduleOfType( SCHED_ALERT_FACE );
			}
			else if ( FRouteClear() )
			{
				// no valid route!
				return GetScheduleOfType( SCHED_IDLE_STAND );
			}
			else
			{
				// valid route. Get moving
				return GetScheduleOfType( SCHED_IDLE_WALK );
			}
			break;
		}
	case MONSTERSTATE_ALERT:
		{
			if ( HasConditions( bits_COND_ENEMY_DEAD ) && LookupActivity( ACT_VICTORY_DANCE ) != ACTIVITY_NOT_AVAILABLE )
			{
				return GetScheduleOfType ( SCHED_VICTORY_DANCE );
			}

			if ( HasConditions(bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE) )
			{
				if ( fabs( FlYawDiff() ) < (1.0 - m_flFieldOfView) * 60 ) // roughly in the correct direction
				{
					return GetScheduleOfType( SCHED_TAKE_COVER_FROM_ORIGIN );
				}
				else
				{
					return GetScheduleOfType( SCHED_ALERT_SMALL_FLINCH );
				}
			}

			else if ( HasConditions ( bits_COND_HEAR_SOUND ) )
			{
				return GetScheduleOfType( SCHED_ALERT_FACE );
			}
			else
			{
				return GetScheduleOfType( SCHED_ALERT_STAND );
			}
			break;
		}
	case MONSTERSTATE_COMBAT:
		{
			if ( HasConditions( bits_COND_ENEMY_DEAD ) )
			{
				// clear the current (dead) enemy and try to find another.
				m_hEnemy = NULL;

				if ( GetEnemy() )
				{
					ClearConditions( bits_COND_ENEMY_DEAD );
					return GetSchedule();
				}
				else
				{
					SetState( MONSTERSTATE_ALERT );
					return GetSchedule();
				}
			}

			if ( HasConditions(bits_COND_NEW_ENEMY) )
			{
				return GetScheduleOfType ( SCHED_WAKE_ANGRY );
			}
			else if (HasConditions(bits_COND_LIGHT_DAMAGE) && !HasMemory( bits_MEMORY_FLINCHED) )
			{
				return GetScheduleOfType( SCHED_SMALL_FLINCH );
			}
			else if ( !HasConditions(bits_COND_SEE_ENEMY) )
			{
				// we can't see the enemy
				if ( !HasConditions(bits_COND_ENEMY_OCCLUDED) )
				{
					// enemy is unseen, but not occluded!
					// turn to face enemy
					return GetScheduleOfType( SCHED_COMBAT_FACE );
				}
				else
				{
					// chase!
					return GetScheduleOfType( SCHED_CHASE_ENEMY );
				}
			}
			else  
			{
				// we can see the enemy
				if ( HasConditions(bits_COND_CAN_RANGE_ATTACK1) )
				{
					return GetScheduleOfType( SCHED_RANGE_ATTACK1 );
				}
				if ( HasConditions(bits_COND_CAN_RANGE_ATTACK2) )
				{
					return GetScheduleOfType( SCHED_RANGE_ATTACK2 );
				}
				if ( HasConditions(bits_COND_CAN_MELEE_ATTACK1) )
				{
					return GetScheduleOfType( SCHED_MELEE_ATTACK1 );
				}
				if ( HasConditions(bits_COND_CAN_MELEE_ATTACK2) )
				{
					return GetScheduleOfType( SCHED_MELEE_ATTACK2 );
				}
				if ( !HasConditions(bits_COND_CAN_RANGE_ATTACK1 | bits_COND_CAN_MELEE_ATTACK1) )
				{
					// if we can see enemy but can't use either attack type, we must need to get closer to enemy
					return GetScheduleOfType( SCHED_CHASE_ENEMY );
				}
				else if ( !FacingIdeal() )
				{
					//turn
					return GetScheduleOfType( SCHED_COMBAT_FACE );
				}
				else
				{
					ALERT ( at_aiconsole, "No suitable combat schedule!\n" );
				}
			}
			break;
		}
	case MONSTERSTATE_DEAD:
		{
			return GetScheduleOfType( SCHED_DIE );
			break;
		}
	case MONSTERSTATE_SCRIPT:
		{
			ASSERT( m_pCine != NULL );
			if ( !m_pCine )
			{
				ALERT( at_aiconsole, "Script failed for %s\n", STRING(pev->classname) );
				CineCleanup();
				return GetScheduleOfType( SCHED_IDLE_STAND );
			}

			return GetScheduleOfType( SCHED_AISCRIPT );
		}
	default:
		{
			ALERT ( at_aiconsole, "Invalid State for GetSchedule!\n" );
			break;
		}
	}

	return &slError[ 0 ];
}
