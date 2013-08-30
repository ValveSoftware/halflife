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
/*


===== scripted.cpp ========================================================

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"

#ifndef ANIMATION_H
#include "animation.h"
#endif

#ifndef SAVERESTORE_H
#include "saverestore.h"
#endif

#include "schedule.h"
#include "scripted.h"
#include "defaultai.h"



/*
classname "scripted_sequence"
targetname "me" - there can be more than one with the same name, and they act in concert
target "the_entity_I_want_to_start_playing" or "class entity_classname" will pick the closest inactive scientist
play "name_of_sequence"
idle "name of idle sequence to play before starting"
donetrigger "whatever" - can be any other triggerable entity such as another sequence, train, door, or a special case like "die" or "remove"
moveto - if set the monster first moves to this nodes position
range # - only search this far to find the target
spawnflags - (stop if blocked, stop if player seen)
*/


//
// Cache user-entity-field values until spawn is called.
//

void CCineMonster :: KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "m_iszIdle"))
	{
		m_iszIdle = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszPlay"))
	{
		m_iszPlay = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszEntity"))
	{
		m_iszEntity = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_fMoveTo"))
	{
		m_fMoveTo = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_flRepeat"))
	{
		m_flRepeat = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_flRadius"))
	{
		m_flRadius = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iFinishSchedule"))
	{
		m_iFinishSchedule = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
	{
		CBaseMonster::KeyValue( pkvd );
	}
}

TYPEDESCRIPTION	CCineMonster::m_SaveData[] = 
{
	DEFINE_FIELD( CCineMonster, m_iszIdle, FIELD_STRING ),
	DEFINE_FIELD( CCineMonster, m_iszPlay, FIELD_STRING ),
	DEFINE_FIELD( CCineMonster, m_iszEntity, FIELD_STRING ),
	DEFINE_FIELD( CCineMonster, m_fMoveTo, FIELD_INTEGER ),
	DEFINE_FIELD( CCineMonster, m_flRepeat, FIELD_FLOAT ),
	DEFINE_FIELD( CCineMonster, m_flRadius, FIELD_FLOAT ),

	DEFINE_FIELD( CCineMonster, m_iDelay, FIELD_INTEGER ),
	DEFINE_FIELD( CCineMonster, m_startTime, FIELD_TIME ),

	DEFINE_FIELD( CCineMonster,	m_saved_movetype, FIELD_INTEGER ),
	DEFINE_FIELD( CCineMonster,	m_saved_solid, FIELD_INTEGER ),
	DEFINE_FIELD( CCineMonster, m_saved_effects, FIELD_INTEGER ),
	DEFINE_FIELD( CCineMonster, m_iFinishSchedule, FIELD_INTEGER ),
	DEFINE_FIELD( CCineMonster, m_interruptable, FIELD_BOOLEAN ),
};


IMPLEMENT_SAVERESTORE( CCineMonster, CBaseMonster );

LINK_ENTITY_TO_CLASS( scripted_sequence, CCineMonster );
#define CLASSNAME "scripted_sequence"

LINK_ENTITY_TO_CLASS( aiscripted_sequence, CCineAI );


void CCineMonster :: Spawn( void )
{
	// pev->solid = SOLID_TRIGGER;
	// UTIL_SetSize(pev, Vector(-8, -8, -8), Vector(8, 8, 8));
	pev->solid = SOLID_NOT;


	// REMOVE: The old side-effect
#if 0
	if ( m_iszIdle )
		m_fMoveTo = 4;
#endif

	// if no targetname, start now
	if ( FStringNull(pev->targetname) || !FStringNull( m_iszIdle ) )
	{
		SetThink( &CCineMonster::CineThink );
		pev->nextthink = gpGlobals->time + 1.0;
		// Wait to be used?
		if ( pev->targetname )
			m_startTime = gpGlobals->time + 1E6;
	}
	if ( pev->spawnflags & SF_SCRIPT_NOINTERRUPT )
		m_interruptable = FALSE;
	else
		m_interruptable = TRUE;
}

//=========================================================
// FCanOverrideState - returns FALSE, scripted sequences 
// cannot possess entities regardless of state.
//=========================================================
BOOL CCineMonster :: FCanOverrideState( void )
{
	if ( pev->spawnflags & SF_SCRIPT_OVERRIDESTATE )
		return TRUE;
	return FALSE;
}

//=========================================================
// FCanOverrideState - returns true because scripted AI can
// possess entities regardless of their state.
//=========================================================
BOOL CCineAI :: FCanOverrideState( void )
{
	return TRUE;
}


//
// CineStart
//
void CCineMonster :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	// do I already know who I should use
	CBaseEntity		*pEntity = m_hTargetEnt;
	CBaseMonster	*pTarget = NULL;

	if ( pEntity )
		pTarget = pEntity->MyMonsterPointer();

	if ( pTarget )
	{
		// am I already playing the script?
		if ( pTarget->m_scriptState == SCRIPT_PLAYING )
			return;

		m_startTime = gpGlobals->time + 0.05;
	}
	else
	{
		// if not, try finding them
		SetThink( &CCineMonster::CineThink );
		pev->nextthink = gpGlobals->time;
	}
}


// This doesn't really make sense since only MOVETYPE_PUSH get 'Blocked' events
void CCineMonster :: Blocked( CBaseEntity *pOther )
{

}

void CCineMonster :: Touch( CBaseEntity *pOther )
{
/*
	ALERT( at_aiconsole, "Cine Touch\n" );
	if (m_pentTarget && OFFSET(pOther->pev) == OFFSET(m_pentTarget))
	{
		CBaseMonster *pTarget = GetClassPtr((CBaseMonster *)VARS(m_pentTarget));
		pTarget->m_monsterState == MONSTERSTATE_SCRIPT;
	}
*/
}


/*
	entvars_t *pevOther = VARS( gpGlobals->other );

	if ( !FBitSet ( pevOther->flags , FL_MONSTER ) ) 
	{// touched by a non-monster.
		return;
	}

	pevOther->origin.z += 1;
	
	if ( FBitSet ( pevOther->flags, FL_ONGROUND ) ) 
	{// clear the onground so physics don't bitch
		pevOther->flags -= FL_ONGROUND;
	}

	// toss the monster!
	pevOther->velocity = pev->movedir * pev->speed;
	pevOther->velocity.z += m_flHeight;


	pev->solid = SOLID_NOT;// kill the trigger for now !!!UNDONE
}
*/


//
// ********** Cinematic DIE **********
//
void CCineMonster :: Die( void )
{
	SetThink( &CCineMonster::SUB_Remove );
}

//
// ********** Cinematic PAIN **********
//
void CCineMonster :: Pain( void )
{

}

//
// ********** Cinematic Think **********
//

// find a viable entity
int CCineMonster :: FindEntity( void )
{
	edict_t *pentTarget;

	pentTarget = FIND_ENTITY_BY_TARGETNAME(NULL, STRING(m_iszEntity));
	m_hTargetEnt = NULL;
	CBaseMonster	*pTarget = NULL;

	while (!FNullEnt(pentTarget))
	{
		if ( FBitSet( VARS(pentTarget)->flags, FL_MONSTER ))
		{
			pTarget = GetMonsterPointer( pentTarget );
			if ( pTarget && pTarget->CanPlaySequence( FCanOverrideState(), SS_INTERRUPT_BY_NAME ) )
			{
				m_hTargetEnt = pTarget;
				return TRUE;
			}
			ALERT( at_console, "Found %s, but can't play!\n", STRING(m_iszEntity) );
		}
		pentTarget = FIND_ENTITY_BY_TARGETNAME(pentTarget, STRING(m_iszEntity));
		pTarget = NULL;
	}
	
	if ( !pTarget )
	{
		CBaseEntity *pEntity = NULL;
		while ((pEntity = UTIL_FindEntityInSphere( pEntity, pev->origin, m_flRadius )) != NULL)
		{
			if (FClassnameIs( pEntity->pev, STRING(m_iszEntity)))
			{
				if ( FBitSet( pEntity->pev->flags, FL_MONSTER ))
				{
					pTarget = pEntity->MyMonsterPointer( );
					if ( pTarget && pTarget->CanPlaySequence( FCanOverrideState(), SS_INTERRUPT_IDLE ) )
					{
						m_hTargetEnt = pTarget;
						return TRUE;
					}
				}
			}
		}
	}
	pTarget = NULL;
	m_hTargetEnt = NULL;
	return FALSE;
}

// make the entity enter a scripted sequence
void CCineMonster :: PossessEntity( void )
{
	CBaseEntity		*pEntity = m_hTargetEnt;
	CBaseMonster	*pTarget = NULL;
	if ( pEntity )
		pTarget = pEntity->MyMonsterPointer();

	if ( pTarget )
	{

	// FindEntity() just checked this!
#if 0
		if ( !pTarget->CanPlaySequence(  FCanOverrideState() ) )
		{
			ALERT( at_aiconsole, "Can't possess entity %s\n", STRING(pTarget->pev->classname) );
			return;
		}
#endif

		pTarget->m_pGoalEnt = this;
		pTarget->m_pCine = this;
		pTarget->m_hTargetEnt = this;

		m_saved_movetype = pTarget->pev->movetype;
		m_saved_solid = pTarget->pev->solid;
		m_saved_effects = pTarget->pev->effects;
		pTarget->pev->effects |= pev->effects;

		switch (m_fMoveTo)
		{
		case 0: 
			pTarget->m_scriptState = SCRIPT_WAIT; 
			break;

		case 1: 
			pTarget->m_scriptState = SCRIPT_WALK_TO_MARK; 
			DelayStart( 1 ); 
			break;

		case 2: 
			pTarget->m_scriptState = SCRIPT_RUN_TO_MARK; 
			DelayStart( 1 ); 
			break;

		case 4: 
			UTIL_SetOrigin( pTarget->pev, pev->origin );
			pTarget->pev->ideal_yaw = pev->angles.y;
			pTarget->pev->avelocity = Vector( 0, 0, 0 );
			pTarget->pev->velocity = Vector( 0, 0, 0 );
			pTarget->pev->effects |= EF_NOINTERP;
			pTarget->pev->angles.y = pev->angles.y;
			pTarget->m_scriptState = SCRIPT_WAIT;
			m_startTime = gpGlobals->time + 1E6;
			// UNDONE: Add a flag to do this so people can fixup physics after teleporting monsters
			//			pTarget->pev->flags &= ~FL_ONGROUND;
			break;
		}
//		ALERT( at_aiconsole, "\"%s\" found and used (INT: %s)\n", STRING( pTarget->pev->targetname ), FBitSet(pev->spawnflags, SF_SCRIPT_NOINTERRUPT)?"No":"Yes" );

		pTarget->m_IdealMonsterState = MONSTERSTATE_SCRIPT;
		if (m_iszIdle)
		{
			StartSequence( pTarget, m_iszIdle, FALSE );
			if (FStrEq( STRING(m_iszIdle), STRING(m_iszPlay)))
			{
				pTarget->pev->framerate = 0;
			}
		}
	}
}

// make the entity carry out the scripted sequence instructions, but without 
// destroying the monster's state.
void CCineAI :: PossessEntity( void )
{
	Schedule_t *pNewSchedule;

	CBaseEntity		*pEntity = m_hTargetEnt;
	CBaseMonster	*pTarget = NULL;
	if ( pEntity )
		pTarget = pEntity->MyMonsterPointer();

	if ( pTarget )
	{
		if ( !pTarget->CanPlaySequence( FCanOverrideState(), SS_INTERRUPT_AI ) )
		{
			ALERT( at_aiconsole, "(AI)Can't possess entity %s\n", STRING(pTarget->pev->classname) );
			return;
		}

		pTarget->m_pGoalEnt = this;
		pTarget->m_pCine = this;
		pTarget->m_hTargetEnt = this;

		m_saved_movetype = pTarget->pev->movetype;
		m_saved_solid = pTarget->pev->solid;
		m_saved_effects = pTarget->pev->effects;
		pTarget->pev->effects |= pev->effects;

		switch (m_fMoveTo)
		{
		case 0: 
		case 5:
			pTarget->m_scriptState = SCRIPT_WAIT; 
			break;

		case 1: 
			pTarget->m_scriptState = SCRIPT_WALK_TO_MARK; 
			break;

		case 2: 
			pTarget->m_scriptState = SCRIPT_RUN_TO_MARK; 
			break;

		case 4: 
			// zap the monster instantly to the site of the script entity.
			UTIL_SetOrigin( pTarget->pev, pev->origin );
			pTarget->pev->ideal_yaw = pev->angles.y;
			pTarget->pev->avelocity = Vector( 0, 0, 0 );
			pTarget->pev->velocity = Vector( 0, 0, 0 );
			pTarget->pev->effects |= EF_NOINTERP;
			pTarget->pev->angles.y = pev->angles.y;
			pTarget->m_scriptState = SCRIPT_WAIT;
			m_startTime = gpGlobals->time + 1E6;
			// UNDONE: Add a flag to do this so people can fixup physics after teleporting monsters
			pTarget->pev->flags &= ~FL_ONGROUND;
			break;
		default:
			ALERT ( at_aiconsole, "aiscript:  invalid Move To Position value!" );
			break;
		}
		
		ALERT( at_aiconsole, "\"%s\" found and used\n", STRING( pTarget->pev->targetname ) );

		pTarget->m_IdealMonsterState = MONSTERSTATE_SCRIPT;

/*
		if (m_iszIdle)
		{
			StartSequence( pTarget, m_iszIdle, FALSE );
			if (FStrEq( STRING(m_iszIdle), STRING(m_iszPlay)))
			{
				pTarget->pev->framerate = 0;
			}
		}
*/
		// Already in a scripted state?
		if ( pTarget->m_MonsterState == MONSTERSTATE_SCRIPT )
		{
			pNewSchedule = pTarget->GetScheduleOfType( SCHED_AISCRIPT );
			pTarget->ChangeSchedule( pNewSchedule );
		}
	}
}

void CCineMonster :: CineThink( void )
{
	if (FindEntity())
	{
		PossessEntity( );
		ALERT( at_aiconsole, "script \"%s\" using monster \"%s\"\n", STRING( pev->targetname ), STRING( m_iszEntity ) );
	}
	else
	{
		CancelScript( );
		ALERT( at_aiconsole, "script \"%s\" can't find monster \"%s\"\n", STRING( pev->targetname ), STRING( m_iszEntity ) );
		pev->nextthink = gpGlobals->time + 1.0;
	}
}


// lookup a sequence name and setup the target monster to play it
BOOL CCineMonster :: StartSequence( CBaseMonster *pTarget, int iszSeq, BOOL completeOnEmpty )
{
	if ( !iszSeq && completeOnEmpty )
	{
		SequenceDone( pTarget );
		return FALSE;
	}

	pTarget->pev->sequence = pTarget->LookupSequence( STRING( iszSeq ) );
	if (pTarget->pev->sequence == -1)
	{
		ALERT( at_error, "%s: unknown scripted sequence \"%s\"\n", STRING( pTarget->pev->targetname ), STRING( iszSeq) );
		pTarget->pev->sequence = 0;
		// return FALSE;
	}

#if 0
	char *s;
	if ( pev->spawnflags & SF_SCRIPT_NOINTERRUPT ) 
		s = "No";
	else
		s = "Yes";

	ALERT( at_console, "%s (%s): started \"%s\":INT:%s\n", STRING( pTarget->pev->targetname ), STRING( pTarget->pev->classname ), STRING( iszSeq), s );
#endif

	pTarget->pev->frame = 0;
	pTarget->ResetSequenceInfo( );
	return TRUE;
}

// lookup a sequence name and setup the target monster to play it
// overridden for CCineAI because it's ok for them to not have an animation sequence
// for the monster to play. For a regular Scripted Sequence, that situation is an error.
BOOL CCineAI :: StartSequence( CBaseMonster *pTarget, int iszSeq, BOOL completeOnEmpty )
{
	if ( iszSeq == 0 && completeOnEmpty )
	{
		// no sequence was provided. Just let the monster proceed, however, we still have to fire any Sequence target
		// and remove any non-repeatable CineAI entities here ( because there is code elsewhere that handles those tasks, but
		// not until the animation sequence is finished. We have to manually take care of these things where there is no sequence.

		SequenceDone ( pTarget );

		return TRUE;
	}

	pTarget->pev->sequence = pTarget->LookupSequence( STRING( iszSeq ) );

	if (pTarget->pev->sequence == -1)
	{
		ALERT( at_error, "%s: unknown aiscripted sequence \"%s\"\n", STRING( pTarget->pev->targetname ), STRING( iszSeq) );
		pTarget->pev->sequence = 0;
		// return FALSE;
	}

	pTarget->pev->frame = 0;
	pTarget->ResetSequenceInfo( );
	return TRUE;
}

//=========================================================
// SequenceDone - called when a scripted sequence animation
// sequence is done playing ( or when an AI Scripted Sequence
// doesn't supply an animation sequence to play ). Expects
// the CBaseMonster pointer to the monster that the sequence
// possesses. 
//=========================================================
void CCineMonster :: SequenceDone ( CBaseMonster *pMonster )
{
	//ALERT( at_aiconsole, "Sequence %s finished\n", STRING( m_pCine->m_iszPlay ) );

	if ( !( pev->spawnflags & SF_SCRIPT_REPEATABLE ) )
	{
		SetThink( &CCineMonster::SUB_Remove );
		pev->nextthink = gpGlobals->time + 0.1;
	}
	
	// This is done so that another sequence can take over the monster when triggered by the first
	
	pMonster->CineCleanup();

	FixScriptMonsterSchedule( pMonster );
	
	// This may cause a sequence to attempt to grab this guy NOW, so we have to clear him out
	// of the existing sequence
	SUB_UseTargets( NULL, USE_TOGGLE, 0 );
}

//=========================================================
// When a monster finishes a scripted sequence, we have to 
// fix up its state and schedule for it to return to a 
// normal AI monster. 
//
// Scripted sequences just dirty the Schedule and drop the
// monster in Idle State.
//=========================================================
void CCineMonster :: FixScriptMonsterSchedule( CBaseMonster *pMonster )
{
	if ( pMonster->m_IdealMonsterState != MONSTERSTATE_DEAD )
		pMonster->m_IdealMonsterState = MONSTERSTATE_IDLE;
	pMonster->ClearSchedule();
}

//=========================================================
// When a monster finishes a scripted sequence, we have to 
// fix up its state and schedule for it to return to a 
// normal AI monster. 
//
// AI Scripted sequences will, depending on what the level
// designer selects:
//
// -Dirty the monster's schedule and drop out of the 
//  sequence in their current state.
//
// -Select a specific AMBUSH schedule, regardless of state.
//=========================================================
void CCineAI :: FixScriptMonsterSchedule( CBaseMonster *pMonster )
{
	switch ( m_iFinishSchedule )
	{
		case SCRIPT_FINISHSCHED_DEFAULT:
			pMonster->ClearSchedule();
			break;
		case SCRIPT_FINISHSCHED_AMBUSH:
			pMonster->ChangeSchedule( pMonster->GetScheduleOfType( SCHED_AMBUSH ) );
			break;
		default:
			ALERT ( at_aiconsole, "FixScriptMonsterSchedule - no case!\n" );
			pMonster->ClearSchedule();
			break;
	}
}

BOOL CBaseMonster :: ExitScriptedSequence( )
{
	if ( pev->deadflag == DEAD_DYING )
	{
		// is this legal?
		// BUGBUG -- This doesn't call Killed()
		m_IdealMonsterState = MONSTERSTATE_DEAD;
		return FALSE;
	}

	if (m_pCine)
	{
		m_pCine->CancelScript( );
	}

	return TRUE;
}


void CCineMonster::AllowInterrupt( BOOL fAllow )
{
	if ( pev->spawnflags & SF_SCRIPT_NOINTERRUPT )
		return;
	m_interruptable = fAllow;
}


BOOL CCineMonster::CanInterrupt( void )
{
	if ( !m_interruptable )
		return FALSE;

	CBaseEntity *pTarget = m_hTargetEnt;

	if ( pTarget != NULL && pTarget->pev->deadflag == DEAD_NO )
		return TRUE;

	return FALSE;
}


int	CCineMonster::IgnoreConditions( void )
{
	if ( CanInterrupt() )
		return 0;
	return SCRIPT_BREAK_CONDITIONS;
}


void ScriptEntityCancel( edict_t *pentCine )
{
	// make sure they are a scripted_sequence
	if (FClassnameIs( pentCine, CLASSNAME ))
	{
		CCineMonster *pCineTarget = GetClassPtr((CCineMonster *)VARS(pentCine));
		// make sure they have a monster in mind for the script
		CBaseEntity		*pEntity = pCineTarget->m_hTargetEnt;
		CBaseMonster	*pTarget = NULL;
		if ( pEntity )
			pTarget = pEntity->MyMonsterPointer();
		
		if (pTarget)
		{
			// make sure their monster is actually playing a script
			if ( pTarget->m_MonsterState == MONSTERSTATE_SCRIPT )
			{
				// tell them do die
				pTarget->m_scriptState = CCineMonster::SCRIPT_CLEANUP;
				// do it now
				pTarget->CineCleanup( );
			}
		}
	}
}


// find all the cinematic entities with my targetname and stop them from playing
void CCineMonster :: CancelScript( void )
{
	ALERT( at_aiconsole, "Cancelling script: %s\n", STRING(m_iszPlay) );
	
	if ( !pev->targetname )
	{
		ScriptEntityCancel( edict() );
		return;
	}

	edict_t *pentCineTarget = FIND_ENTITY_BY_TARGETNAME(NULL, STRING(pev->targetname));

	while (!FNullEnt(pentCineTarget))
	{
		ScriptEntityCancel( pentCineTarget );
		pentCineTarget = FIND_ENTITY_BY_TARGETNAME(pentCineTarget, STRING(pev->targetname));
	}
}


// find all the cinematic entities with my targetname and tell them to wait before starting
void CCineMonster :: DelayStart( int state )
{
	edict_t *pentCine = FIND_ENTITY_BY_TARGETNAME(NULL, STRING(pev->targetname));

	while (!FNullEnt(pentCine))
	{
		if (FClassnameIs( pentCine, "scripted_sequence" ))
		{
			CCineMonster *pTarget = GetClassPtr((CCineMonster *)VARS(pentCine));
			if (state)
			{
				pTarget->m_iDelay++;
			}
			else
			{
				pTarget->m_iDelay--;
				if (pTarget->m_iDelay <= 0)
					pTarget->m_startTime = gpGlobals->time + 0.05;
			}
		}
		pentCine = FIND_ENTITY_BY_TARGETNAME(pentCine, STRING(pev->targetname));
	}
}



// Find an entity that I'm interested in and precache the sounds he'll need in the sequence.
void CCineMonster :: Activate( void )
{
	edict_t			*pentTarget;
	CBaseMonster	*pTarget;

	// The entity name could be a target name or a classname
	// Check the targetname
	pentTarget = FIND_ENTITY_BY_TARGETNAME(NULL, STRING(m_iszEntity));
	pTarget = NULL;

	while (!pTarget && !FNullEnt(pentTarget))
	{
		if ( FBitSet( VARS(pentTarget)->flags, FL_MONSTER ))
		{
			pTarget = GetMonsterPointer( pentTarget );
		}
		pentTarget = FIND_ENTITY_BY_TARGETNAME(pentTarget, STRING(m_iszEntity));
	}
	
	// If no entity with that targetname, check the classname
	if ( !pTarget )
	{
		pentTarget = FIND_ENTITY_BY_CLASSNAME(NULL, STRING(m_iszEntity));
		while (!pTarget && !FNullEnt(pentTarget))
		{
			pTarget = GetMonsterPointer( pentTarget );
			pentTarget = FIND_ENTITY_BY_TARGETNAME(pentTarget, STRING(m_iszEntity));
		}
	}
	// Found a compatible entity
	if ( pTarget )
	{
		void *pmodel;
		pmodel = GET_MODEL_PTR( pTarget->edict() );
		if ( pmodel )
		{
			// Look through the event list for stuff to precache
			SequencePrecache( pmodel, STRING( m_iszIdle ) );
			SequencePrecache( pmodel, STRING( m_iszPlay ) );
		}
	}
}

		
BOOL CBaseMonster :: CineCleanup( )
{
	CCineMonster *pOldCine = m_pCine;

	// am I linked to a cinematic?
	if (m_pCine)
	{
		// okay, reset me to what it thought I was before
		m_pCine->m_hTargetEnt = NULL;
		pev->movetype = m_pCine->m_saved_movetype;
		pev->solid = m_pCine->m_saved_solid;
		pev->effects = m_pCine->m_saved_effects;
	}
	else
	{
		// arg, punt
		pev->movetype = MOVETYPE_STEP;// this is evil
		pev->solid = SOLID_SLIDEBOX;
	}
	m_pCine = NULL;
	m_hTargetEnt = NULL;
	m_pGoalEnt = NULL;
	if (pev->deadflag == DEAD_DYING)
	{
		// last frame of death animation?
		pev->health			= 0;
		pev->framerate		= 0.0;
		pev->solid			= SOLID_NOT;
		SetState( MONSTERSTATE_DEAD );
		pev->deadflag = DEAD_DEAD;
		UTIL_SetSize( pev, pev->mins, Vector(pev->maxs.x, pev->maxs.y, pev->mins.z + 2) );

		if ( pOldCine && FBitSet( pOldCine->pev->spawnflags, SF_SCRIPT_LEAVECORPSE ) )
		{
			SetUse( NULL );		// BUGBUG -- This doesn't call Killed()
			SetThink( NULL );	// This will probably break some stuff
			SetTouch( NULL );
		}
		else
			SUB_StartFadeOut(); // SetThink( SUB_DoNothing );
		// This turns off animation & physics in case their origin ends up stuck in the world or something
		StopAnimation();
		pev->movetype = MOVETYPE_NONE;
		pev->effects |= EF_NOINTERP;	// Don't interpolate either, assume the corpse is positioned in its final resting place
		return FALSE;
	}

	// If we actually played a sequence
	if ( pOldCine && pOldCine->m_iszPlay )
	{
		if ( !(pOldCine->pev->spawnflags & SF_SCRIPT_NOSCRIPTMOVEMENT) )
		{
			// reset position
			Vector new_origin, new_angle;
			GetBonePosition( 0, new_origin, new_angle );

			// Figure out how far they have moved
			// We can't really solve this problem because we can't query the movement of the origin relative
			// to the sequence.  We can get the root bone's position as we do here, but there are
			// cases where the root bone is in a different relative position to the entity's origin
			// before/after the sequence plays.  So we are stuck doing this:

			// !!!HACKHACK: Float the origin up and drop to floor because some sequences have
			// irregular motion that can't be properly accounted for.

			// UNDONE: THIS SHOULD ONLY HAPPEN IF WE ACTUALLY PLAYED THE SEQUENCE.
			Vector oldOrigin = pev->origin;

			// UNDONE: ugly hack.  Don't move monster if they don't "seem" to move
			// this really needs to be done with the AX,AY,etc. flags, but that aren't consistantly
			// being set, so animations that really do move won't be caught.
			if ((oldOrigin - new_origin).Length2D() < 8.0)
				new_origin = oldOrigin;

			pev->origin.x = new_origin.x;
			pev->origin.y = new_origin.y;
			pev->origin.z += 1;

			pev->flags |= FL_ONGROUND;
			int drop = DROP_TO_FLOOR( ENT(pev) );
			
			// Origin in solid?  Set to org at the end of the sequence
			if ( drop < 0 )
				pev->origin = oldOrigin;
			else if ( drop == 0 ) // Hanging in air?
			{
				pev->origin.z = new_origin.z;
				pev->flags &= ~FL_ONGROUND;
			}
			// else entity hit floor, leave there

			// pEntity->pev->origin.z = new_origin.z + 5.0; // damn, got to fix this

			UTIL_SetOrigin( pev, pev->origin );
			pev->effects |= EF_NOINTERP;
		}

		// We should have some animation to put these guys in, but for now it's idle.
		// Due to NOINTERP above, there won't be any blending between this anim & the sequence
		m_Activity = ACT_RESET;
	}
	// set them back into a normal state
	pev->enemy = NULL;
	if ( pev->health > 0 )
		m_IdealMonsterState = MONSTERSTATE_IDLE; // m_previousState;
	else
	{
		// Dropping out because he got killed
		// Can't call killed() no attacker and weirdness (late gibbing) may result
		m_IdealMonsterState = MONSTERSTATE_DEAD;
		SetConditions( bits_COND_LIGHT_DAMAGE );
		pev->deadflag = DEAD_DYING;
		FCheckAITrigger();
		pev->deadflag = DEAD_NO;
	}


	//	SetAnimation( m_MonsterState );
	ClearBits(pev->spawnflags, SF_MONSTER_WAIT_FOR_SCRIPT );

	return TRUE;
}




class CScriptedSentence : public CBaseToggle
{
public:
	void Spawn( void );
	void KeyValue( KeyValueData *pkvd );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT FindThink( void );
	void EXPORT DelayThink( void );
	int	 ObjectCaps( void ) { return (CBaseToggle :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION); }

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	
	static	TYPEDESCRIPTION m_SaveData[];

	CBaseMonster *FindEntity( void );
	BOOL AcceptableSpeaker( CBaseMonster *pMonster );
	BOOL StartSentence( CBaseMonster *pTarget );


private:
	int		m_iszSentence;		// string index for idle animation
	int		m_iszEntity;	// entity that is wanted for this sentence
	float	m_flRadius;		// range to search
	float	m_flDuration;	// How long the sentence lasts
	float	m_flRepeat;	// repeat rate
	float	m_flAttenuation;
	float	m_flVolume;
	BOOL	m_active;
	int		m_iszListener;	// name of entity to look at while talking
};

#define SF_SENTENCE_ONCE		0x0001
#define SF_SENTENCE_FOLLOWERS	0x0002	// only say if following player
#define SF_SENTENCE_INTERRUPT	0x0004	// force talking except when dead
#define SF_SENTENCE_CONCURRENT	0x0008	// allow other people to keep talking

TYPEDESCRIPTION	CScriptedSentence::m_SaveData[] = 
{
	DEFINE_FIELD( CScriptedSentence, m_iszSentence, FIELD_STRING ),
	DEFINE_FIELD( CScriptedSentence, m_iszEntity, FIELD_STRING ),
	DEFINE_FIELD( CScriptedSentence, m_flRadius, FIELD_FLOAT ),
	DEFINE_FIELD( CScriptedSentence, m_flDuration, FIELD_FLOAT ),
	DEFINE_FIELD( CScriptedSentence, m_flRepeat, FIELD_FLOAT ),
	DEFINE_FIELD( CScriptedSentence, m_flAttenuation, FIELD_FLOAT ),
	DEFINE_FIELD( CScriptedSentence, m_flVolume, FIELD_FLOAT ),
	DEFINE_FIELD( CScriptedSentence, m_active, FIELD_BOOLEAN ),
	DEFINE_FIELD( CScriptedSentence, m_iszListener, FIELD_STRING ),
};


IMPLEMENT_SAVERESTORE( CScriptedSentence, CBaseToggle );

LINK_ENTITY_TO_CLASS( scripted_sentence, CScriptedSentence );

void CScriptedSentence :: KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "sentence"))
	{
		m_iszSentence = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "entity"))
	{
		m_iszEntity = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "duration"))
	{
		m_flDuration = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "radius"))
	{
		m_flRadius = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "refire"))
	{
		m_flRepeat = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if(FStrEq(pkvd->szKeyName, "attenuation"))
	{
		pev->impulse = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if(FStrEq(pkvd->szKeyName, "volume"))
	{
		m_flVolume = atof( pkvd->szValue ) * 0.1;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "listener"))
	{
		m_iszListener = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
		CBaseToggle::KeyValue( pkvd );
}


void CScriptedSentence :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( !m_active )
		return;
//	ALERT( at_console, "Firing sentence: %s\n", STRING(m_iszSentence) );
	SetThink( &CScriptedSentence::FindThink );
	pev->nextthink = gpGlobals->time;
}


void CScriptedSentence :: Spawn( void )
{
	pev->solid = SOLID_NOT;
	
	m_active = TRUE;
	// if no targetname, start now
	if ( !pev->targetname )
	{
		SetThink( &CScriptedSentence::FindThink );
		pev->nextthink = gpGlobals->time + 1.0;
	}

	switch( pev->impulse )
	{
	case 1: // Medium radius
		m_flAttenuation = ATTN_STATIC;
		break;
	
	case 2:	// Large radius
		m_flAttenuation = ATTN_NORM;
		break;

	case 3:	//EVERYWHERE
		m_flAttenuation = ATTN_NONE;
		break;
	
	default:
	case 0: // Small radius
		m_flAttenuation = ATTN_IDLE;
		break;
	}
	pev->impulse = 0;

	// No volume, use normal
	if ( m_flVolume <= 0 )
		m_flVolume = 1.0;
}


void CScriptedSentence :: FindThink( void )
{
	CBaseMonster *pMonster = FindEntity();
	if ( pMonster )
	{
		StartSentence( pMonster );
		if ( pev->spawnflags & SF_SENTENCE_ONCE )
			UTIL_Remove( this );
		SetThink( &CScriptedSentence::DelayThink );
		pev->nextthink = gpGlobals->time + m_flDuration + m_flRepeat;
		m_active = FALSE;
//		ALERT( at_console, "%s: found monster %s\n", STRING(m_iszSentence), STRING(m_iszEntity) );
	}
	else
	{
//		ALERT( at_console, "%s: can't find monster %s\n", STRING(m_iszSentence), STRING(m_iszEntity) );
		pev->nextthink = gpGlobals->time + m_flRepeat + 0.5;
	}
}


void CScriptedSentence :: DelayThink( void )
{
	m_active = TRUE;
	if ( !pev->targetname )
		pev->nextthink = gpGlobals->time + 0.1;
	SetThink( &CScriptedSentence::FindThink );
}


BOOL CScriptedSentence :: AcceptableSpeaker( CBaseMonster *pMonster )
{
	if ( pMonster )
	{
		if ( pev->spawnflags & SF_SENTENCE_FOLLOWERS )
		{
			if ( pMonster->m_hTargetEnt == NULL || !FClassnameIs(pMonster->m_hTargetEnt->pev, "player") )
				return FALSE;
		}
		BOOL override;
		if ( pev->spawnflags & SF_SENTENCE_INTERRUPT )
			override = TRUE;
		else
			override = FALSE;
		if ( pMonster->CanPlaySentence( override ) )
			return TRUE;
	}
	return FALSE;
}


CBaseMonster *CScriptedSentence :: FindEntity( void )
{
	edict_t *pentTarget;
	CBaseMonster *pMonster;


	pentTarget = FIND_ENTITY_BY_TARGETNAME(NULL, STRING(m_iszEntity));
	pMonster = NULL;

	while (!FNullEnt(pentTarget))
	{
		pMonster = GetMonsterPointer( pentTarget );
		if ( pMonster != NULL )
		{
			if ( AcceptableSpeaker( pMonster ) )
				return pMonster;
//			ALERT( at_console, "%s (%s), not acceptable\n", STRING(pMonster->pev->classname), STRING(pMonster->pev->targetname) );
		}
		pentTarget = FIND_ENTITY_BY_TARGETNAME(pentTarget, STRING(m_iszEntity));
	}
	
	CBaseEntity *pEntity = NULL;
	while ((pEntity = UTIL_FindEntityInSphere( pEntity, pev->origin, m_flRadius )) != NULL)
	{
		if (FClassnameIs( pEntity->pev, STRING(m_iszEntity)))
		{
			if ( FBitSet( pEntity->pev->flags, FL_MONSTER ))
			{
				pMonster = pEntity->MyMonsterPointer( );
				if ( AcceptableSpeaker( pMonster ) )
					return pMonster;
			}
		}
	}
	
	return NULL;
}


BOOL CScriptedSentence :: StartSentence( CBaseMonster *pTarget )
{
	if ( !pTarget )
	{
		ALERT( at_aiconsole, "Not Playing sentence %s\n", STRING(m_iszSentence) );
		return NULL;
	}

	BOOL bConcurrent = FALSE;
	if ( !(pev->spawnflags & SF_SENTENCE_CONCURRENT) )
		bConcurrent = TRUE;

	CBaseEntity *pListener = NULL;
	if (!FStringNull(m_iszListener))
	{
		float radius = m_flRadius;

		if ( FStrEq( STRING(m_iszListener ), "player" ) )
			radius = 4096;	// Always find the player

		pListener = UTIL_FindEntityGeneric( STRING( m_iszListener ), pTarget->pev->origin, radius );
	}

	pTarget->PlayScriptedSentence( STRING(m_iszSentence), m_flDuration,  m_flVolume, m_flAttenuation, bConcurrent, pListener );
	ALERT( at_aiconsole, "Playing sentence %s (%.1f)\n", STRING(m_iszSentence), m_flDuration );
	SUB_UseTargets( NULL, USE_TOGGLE, 0 );
	return TRUE;
}





/*

*/


//=========================================================
// Furniture - this is the cool comment I cut-and-pasted
//=========================================================
class CFurniture : public CBaseMonster
{
public:
	void Spawn ( void );
	void Die( void );
	int	 Classify ( void );
	virtual int	ObjectCaps( void ) { return (CBaseMonster :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION); }
};


LINK_ENTITY_TO_CLASS( monster_furniture, CFurniture );


//=========================================================
// Furniture is killed
//=========================================================
void CFurniture :: Die ( void )
{
	SetThink ( &CFurniture::SUB_Remove );
	pev->nextthink = gpGlobals->time;
}

//=========================================================
// This used to have something to do with bees flying, but 
// now it only initializes moving furniture in scripted sequences
//=========================================================
void CFurniture :: Spawn( )
{
	PRECACHE_MODEL((char *)STRING(pev->model));
	SET_MODEL(ENT(pev),	STRING(pev->model));

	pev->movetype	= MOVETYPE_NONE;
	pev->solid		= SOLID_BBOX;
	pev->health		= 80000;
	pev->takedamage = DAMAGE_AIM;
	pev->effects		= 0;
	pev->yaw_speed		= 0;
	pev->sequence		= 0;
	pev->frame			= 0;

//	pev->nextthink += 1.0;
//	SetThink (WalkMonsterDelay);

	ResetSequenceInfo( );
	pev->frame = 0;
	MonsterInit();
}

//=========================================================
// ID's Furniture as neutral (noone will attack it)
//=========================================================
int CFurniture::Classify ( void )
{
	return	CLASS_NONE;
}


