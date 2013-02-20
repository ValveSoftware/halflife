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
// Default behaviors.
//=========================================================
#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"monsters.h"
#include	"schedule.h"
#include	"defaultai.h"
#include	"nodes.h"
#include	"scripted.h"

Schedule_t *CBaseMonster::m_scheduleList[] = 
{
};

Schedule_t *CBaseMonster::ScheduleFromName( const char *pName )
{
	return ScheduleInList( pName, m_scheduleList, ARRAYSIZE(m_scheduleList) );
}


Schedule_t *CBaseMonster :: ScheduleInList( const char *pName, Schedule_t **pList, int listCount )
{
	int i;
	
	if ( !pName )
	{
		ALERT( at_console, "%s set to unnamed schedule!\n", STRING(pev->classname) );
		return NULL;
	}


	for ( i = 0; i < listCount; i++ )
	{
		if ( !pList[i]->pName )
		{
			ALERT( at_console, "Unnamed schedule!\n" );
			continue;
		}
		if ( stricmp( pName, pList[i]->pName ) == 0 )
			return pList[i];
	}
	return NULL;
}

//=========================================================
// GetScheduleOfType - returns a pointer to one of the 
// monster's available schedules of the indicated type.
//=========================================================
Schedule_t* CBaseMonster :: GetScheduleOfType ( int Type ) 
{
//	ALERT ( at_console, "Sched Type:%d\n", Type );
	switch	( Type )
	{
	// This is the schedule for scripted sequences AND scripted AI
	case SCHED_AISCRIPT:
		{
			ASSERT( m_pCine != NULL );
			if ( !m_pCine )
			{
				ALERT( at_aiconsole, "Script failed for %s\n", STRING(pev->classname) );
				CineCleanup();
				return GetScheduleOfType( SCHED_IDLE_STAND );
			}
//			else
//				ALERT( at_aiconsole, "Starting script %s for %s\n", STRING( m_pCine->m_iszPlay ), STRING(pev->classname) );

			switch ( m_pCine->m_fMoveTo )
			{
				case 0: 
				case 4: 
					return slWaitScript;
				case 1: 
					return slWalkToScript;
				case 2: 
					return slRunToScript;
				case 5:
					return slFaceScript;
			}
			break;
		}
	case SCHED_IDLE_STAND:
		{
			if ( RANDOM_LONG(0,14) == 0 && FCanActiveIdle() )
			{
				return &slActiveIdle[ 0 ];
			}

			return &slIdleStand[ 0 ];
		}
	case SCHED_IDLE_WALK:
		{
			return &slIdleWalk[ 0 ];
		}
	case SCHED_WAIT_TRIGGER:
		{
			return &slIdleTrigger[ 0 ];
		}
	case SCHED_WAKE_ANGRY:
		{
			return &slWakeAngry[ 0 ];
		}
	case SCHED_ALERT_FACE:
		{
			return &slAlertFace[ 0 ];
		}
	case SCHED_ALERT_STAND:
		{
			return &slAlertStand[ 0 ];
		}
	case SCHED_COMBAT_STAND:
		{
			return &slCombatStand[ 0 ];
		}
	case SCHED_COMBAT_FACE:
		{
			return &slCombatFace[ 0 ];
		}
	case SCHED_CHASE_ENEMY:
		{
			return &slChaseEnemy[ 0 ];
		}
	case SCHED_CHASE_ENEMY_FAILED:
		{
			return &slFail[ 0 ];
		}
	case SCHED_SMALL_FLINCH:
		{
			return &slSmallFlinch[ 0 ];
		}
	case SCHED_ALERT_SMALL_FLINCH:
		{
			return &slAlertSmallFlinch[ 0 ];
		}
	case SCHED_RELOAD:
		{
			return &slReload[ 0 ];
		}
	case SCHED_ARM_WEAPON:
		{
			return &slArmWeapon[ 0 ];
		}
	case SCHED_STANDOFF:
		{
			return &slStandoff[ 0 ];
		}
	case SCHED_RANGE_ATTACK1:
		{
			return &slRangeAttack1[ 0 ];
		}
	case SCHED_RANGE_ATTACK2:
		{
			return &slRangeAttack2[ 0 ];
		}
	case SCHED_MELEE_ATTACK1:
		{
			return &slPrimaryMeleeAttack[ 0 ];
		}
	case SCHED_MELEE_ATTACK2:
		{
			return &slSecondaryMeleeAttack[ 0 ];
		}
	case SCHED_SPECIAL_ATTACK1:
		{
			return &slSpecialAttack1[ 0 ];
		}
	case SCHED_SPECIAL_ATTACK2:
		{
			return &slSpecialAttack2[ 0 ];
		}
	case SCHED_TAKE_COVER_FROM_BEST_SOUND:
		{
			return &slTakeCoverFromBestSound[ 0 ];
		}
	case SCHED_TAKE_COVER_FROM_ENEMY:
		{
			return &slTakeCoverFromEnemy[ 0 ];
		}
	case SCHED_COWER:
		{
			return &slCower[ 0 ];
		}
	case SCHED_AMBUSH:
		{
			return &slAmbush[ 0 ];
		}
	case SCHED_BARNACLE_VICTIM_GRAB:
		{
			return &slBarnacleVictimGrab[ 0 ];
		}
	case SCHED_BARNACLE_VICTIM_CHOMP:
		{
			return &slBarnacleVictimChomp[ 0 ];
		}
	case SCHED_INVESTIGATE_SOUND:
		{
			return &slInvestigateSound[ 0 ];
		}
	case SCHED_DIE:
		{
			return &slDie[ 0 ];
		}
	case SCHED_TAKE_COVER_FROM_ORIGIN:
		{
			return &slTakeCoverFromOrigin[ 0 ];
		}
	case SCHED_VICTORY_DANCE:
		{
			return &slVictoryDance[ 0 ];
		}
	case SCHED_FAIL:
		{
			return slFail;
		}
	default:
		{
			ALERT ( at_console, "GetScheduleOfType()\nNo CASE for Schedule Type %d!\n", Type );

			return &slIdleStand[ 0 ];
			break;
		}
	}

	return NULL;
}
