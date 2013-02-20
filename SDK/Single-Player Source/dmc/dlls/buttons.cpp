/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
/*

===== buttons.cpp ========================================================

  button-related code

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"
#include "doors.h"


#define SF_BUTTON_DONTMOVE		1
#define SF_ROTBUTTON_NOTSOLID	1
#define	SF_BUTTON_TOGGLE		32	// button stays pushed until reactivated
#define	SF_BUTTON_SPARK_IF_OFF	64	// button sparks in OFF state
#define SF_BUTTON_TOUCH_ONLY	256	// button only fires as a result of USE key.

#define SF_GLOBAL_SET			1	// Set global state to initial state on spawn

class CEnvGlobal : public CPointEntity
{
public:
	void	Spawn( void );
	void	KeyValue( KeyValueData *pkvd );
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );

	static	TYPEDESCRIPTION m_SaveData[];
	
	string_t	m_globalstate;
	int			m_triggermode;
	int			m_initialstate;
};

TYPEDESCRIPTION CEnvGlobal::m_SaveData[] =
{
	DEFINE_FIELD( CEnvGlobal, m_globalstate, FIELD_STRING ),
	DEFINE_FIELD( CEnvGlobal, m_triggermode, FIELD_INTEGER ),
	DEFINE_FIELD( CEnvGlobal, m_initialstate, FIELD_INTEGER ),
};

IMPLEMENT_SAVERESTORE( CEnvGlobal, CBaseEntity );

LINK_ENTITY_TO_CLASS( env_global, CEnvGlobal );

void CEnvGlobal::KeyValue( KeyValueData *pkvd )
{
	pkvd->fHandled = TRUE;

	if ( FStrEq(pkvd->szKeyName, "globalstate") )		// State name
		m_globalstate = ALLOC_STRING( pkvd->szValue );
	else if ( FStrEq(pkvd->szKeyName, "triggermode") )
		m_triggermode = atoi( pkvd->szValue );
	else if ( FStrEq(pkvd->szKeyName, "initialstate") )
		m_initialstate = atoi( pkvd->szValue );
	else 
		CPointEntity::KeyValue( pkvd );
}

void CEnvGlobal::Spawn( void )
{
	if ( !m_globalstate )
	{
		REMOVE_ENTITY( ENT(pev) );
		return;
	}
	if ( FBitSet( pev->spawnflags, SF_GLOBAL_SET ) )
	{
		if ( !gGlobalState.EntityInTable( m_globalstate ) )
			gGlobalState.EntityAdd( m_globalstate, gpGlobals->mapname, (GLOBALESTATE)m_initialstate );
	}
}


void CEnvGlobal::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	GLOBALESTATE oldState = gGlobalState.EntityGetState( m_globalstate );
	GLOBALESTATE newState;

	switch( m_triggermode )
	{
	case 0:
		newState = GLOBAL_OFF;
		break;

	case 1:
		newState = GLOBAL_ON;
		break;

	case 2:
		newState = GLOBAL_DEAD;
		break;

	default:
	case 3:
		if ( oldState == GLOBAL_ON )
			newState = GLOBAL_OFF;
		else if ( oldState == GLOBAL_OFF )
			newState = GLOBAL_ON;
		else
			newState = oldState;
	}

	if ( gGlobalState.EntityInTable( m_globalstate ) )
		gGlobalState.EntitySetState( m_globalstate, newState );
	else
		gGlobalState.EntityAdd( m_globalstate, gpGlobals->mapname, newState );
}



TYPEDESCRIPTION CMultiSource::m_SaveData[] =
{
	//!!!BUGBUG FIX
	DEFINE_ARRAY( CMultiSource, m_rgEntities, FIELD_EHANDLE, MS_MAX_TARGETS ),
	DEFINE_ARRAY( CMultiSource, m_rgTriggered, FIELD_INTEGER, MS_MAX_TARGETS ),
	DEFINE_FIELD( CMultiSource, m_iTotal, FIELD_INTEGER ),
	DEFINE_FIELD( CMultiSource, m_globalstate, FIELD_STRING ),
};

IMPLEMENT_SAVERESTORE( CMultiSource, CBaseEntity );

LINK_ENTITY_TO_CLASS( multisource, CMultiSource );
//
// Cache user-entity-field values until spawn is called.
//

void CMultiSource::KeyValue( KeyValueData *pkvd )
{
	if (	FStrEq(pkvd->szKeyName, "style") ||
				FStrEq(pkvd->szKeyName, "height") ||
				FStrEq(pkvd->szKeyName, "killtarget") ||
				FStrEq(pkvd->szKeyName, "value1") ||
				FStrEq(pkvd->szKeyName, "value2") ||
				FStrEq(pkvd->szKeyName, "value3"))
		pkvd->fHandled = TRUE;
	else if ( FStrEq(pkvd->szKeyName, "globalstate") )
	{
		m_globalstate = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else 
		CPointEntity::KeyValue( pkvd );
}

#define SF_MULTI_INIT		1

void CMultiSource::Spawn()
{ 
	// set up think for later registration

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->nextthink = gpGlobals->time + 0.1;
	pev->spawnflags |= SF_MULTI_INIT;	// Until it's initialized
	SetThink(Register);
}

void CMultiSource::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{ 
	int i = 0;

	// Find the entity in our list
	while (i < m_iTotal)
		if ( m_rgEntities[i++] == pCaller )
			break;

	// if we didn't find it, report error and leave
	if (i > m_iTotal)
	{
		ALERT(at_console, "MultiSrc:Used by non member %s.\n", STRING(pCaller->pev->classname));
		return;	
	}

	// CONSIDER: a Use input to the multisource always toggles.  Could check useType for ON/OFF/TOGGLE

	m_rgTriggered[i-1] ^= 1;

	// 
	if ( IsTriggered( pActivator ) )
	{
		ALERT( at_aiconsole, "Multisource %s enabled (%d inputs)\n", STRING(pev->targetname), m_iTotal );
		USE_TYPE useType = USE_TOGGLE;
		if ( m_globalstate )
			useType = USE_ON;
		SUB_UseTargets( NULL, useType, 0 );
	}
}


BOOL CMultiSource::IsTriggered( CBaseEntity * )
{
	// Is everything triggered?
	int i = 0;

	// Still initializing?
	if ( pev->spawnflags & SF_MULTI_INIT )
		return 0;

	while (i < m_iTotal)
	{
		if (m_rgTriggered[i] == 0)
			break;
		i++;
	}

	if (i == m_iTotal)
	{
		if ( !m_globalstate || gGlobalState.EntityGetState( m_globalstate ) == GLOBAL_ON )
			return 1;
	}
	
	return 0;
}

void CMultiSource::Register(void)
{ 
	edict_t *pentTarget	= NULL;

	m_iTotal = 0;
	memset( m_rgEntities, 0, MS_MAX_TARGETS * sizeof(EHANDLE) );

	SetThink(SUB_DoNothing);

	// search for all entities which target this multisource (pev->targetname)

	pentTarget = FIND_ENTITY_BY_STRING(NULL, "target", STRING(pev->targetname));

	while (!FNullEnt(pentTarget) && (m_iTotal < MS_MAX_TARGETS))
	{
		CBaseEntity *pTarget = CBaseEntity::Instance(pentTarget);
		if ( pTarget )
			m_rgEntities[m_iTotal++] = pTarget;

		pentTarget = FIND_ENTITY_BY_STRING( pentTarget, "target", STRING(pev->targetname));
	}

	pentTarget = FIND_ENTITY_BY_STRING(NULL, "classname", "multi_manager");
	while (!FNullEnt(pentTarget) && (m_iTotal < MS_MAX_TARGETS))
	{
		CBaseEntity *pTarget = CBaseEntity::Instance(pentTarget);
		if ( pTarget && pTarget->HasTarget(pev->targetname) )
			m_rgEntities[m_iTotal++] = pTarget;

		pentTarget = FIND_ENTITY_BY_STRING( pentTarget, "classname", "multi_manager" );
	}

	pev->spawnflags &= ~SF_MULTI_INIT;
}

// CBaseButton
TYPEDESCRIPTION CBaseButton::m_SaveData[] =
{
	DEFINE_FIELD( CBaseButton, m_fStayPushed, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBaseButton, m_fRotating, FIELD_BOOLEAN ),

	DEFINE_FIELD( CBaseButton, m_sounds, FIELD_INTEGER ),
	DEFINE_FIELD( CBaseButton, m_bLockedSound, FIELD_CHARACTER ),
	DEFINE_FIELD( CBaseButton, m_bLockedSentence, FIELD_CHARACTER ),
	DEFINE_FIELD( CBaseButton, m_bUnlockedSound, FIELD_CHARACTER ),	
	DEFINE_FIELD( CBaseButton, m_bUnlockedSentence, FIELD_CHARACTER ),
	DEFINE_FIELD( CBaseButton, m_strChangeTarget, FIELD_STRING ),
//	DEFINE_FIELD( CBaseButton, m_ls, FIELD_??? ),   // This is restored in Precache()
};
	

IMPLEMENT_SAVERESTORE( CBaseButton, CBaseToggle );

void CBaseButton::Precache( void )
{
	char *pszSound;

	if ( FBitSet ( pev->spawnflags, SF_BUTTON_SPARK_IF_OFF ) )// this button should spark in OFF state
	{
		PRECACHE_SOUND ("buttons/spark1.wav");
		PRECACHE_SOUND ("buttons/spark2.wav");
		PRECACHE_SOUND ("buttons/spark3.wav");
		PRECACHE_SOUND ("buttons/spark4.wav");
		PRECACHE_SOUND ("buttons/spark5.wav");
		PRECACHE_SOUND ("buttons/spark6.wav");
	}

	// get door button sounds, for doors which require buttons to open

	if (m_bLockedSound)
	{
		pszSound = ButtonSound( (int)m_bLockedSound );
		PRECACHE_SOUND(pszSound);
		m_ls.sLockedSound = ALLOC_STRING(pszSound);
	}

	if (m_bUnlockedSound)
	{
		pszSound = ButtonSound( (int)m_bUnlockedSound );
		PRECACHE_SOUND(pszSound);
		m_ls.sUnlockedSound = ALLOC_STRING(pszSound);
	}

	// get sentence group names, for doors which are directly 'touched' to open

	switch (m_bLockedSentence)
	{
		case 1: m_ls.sLockedSentence = MAKE_STRING("NA"); break; // access denied
		case 2: m_ls.sLockedSentence = MAKE_STRING("ND"); break; // security lockout
		case 3: m_ls.sLockedSentence = MAKE_STRING("NF"); break; // blast door
		case 4: m_ls.sLockedSentence = MAKE_STRING("NFIRE"); break; // fire door
		case 5: m_ls.sLockedSentence = MAKE_STRING("NCHEM"); break; // chemical door
		case 6: m_ls.sLockedSentence = MAKE_STRING("NRAD"); break; // radiation door
		case 7: m_ls.sLockedSentence = MAKE_STRING("NCON"); break; // gen containment
		case 8: m_ls.sLockedSentence = MAKE_STRING("NH"); break; // maintenance door
		case 9: m_ls.sLockedSentence = MAKE_STRING("NG"); break; // broken door
		
		default: m_ls.sLockedSentence = 0; break;
	}

	switch (m_bUnlockedSentence)
	{
		case 1: m_ls.sUnlockedSentence = MAKE_STRING("EA"); break; // access granted
		case 2: m_ls.sUnlockedSentence = MAKE_STRING("ED"); break; // security door
		case 3: m_ls.sUnlockedSentence = MAKE_STRING("EF"); break; // blast door
		case 4: m_ls.sUnlockedSentence = MAKE_STRING("EFIRE"); break; // fire door
		case 5: m_ls.sUnlockedSentence = MAKE_STRING("ECHEM"); break; // chemical door
		case 6: m_ls.sUnlockedSentence = MAKE_STRING("ERAD"); break; // radiation door
		case 7: m_ls.sUnlockedSentence = MAKE_STRING("ECON"); break; // gen containment
		case 8: m_ls.sUnlockedSentence = MAKE_STRING("EH"); break; // maintenance door
	
		default: m_ls.sUnlockedSentence = 0; break;
	}
}

//
// Cache user-entity-field values until spawn is called.
//

void CBaseButton::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "changetarget"))
	{
		m_strChangeTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}	
	else if (FStrEq(pkvd->szKeyName, "locked_sound"))
	{
		m_bLockedSound = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "locked_sentence"))
	{
		m_bLockedSentence = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "unlocked_sound"))
	{
		m_bUnlockedSound = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "unlocked_sentence"))
	{
		m_bUnlockedSentence = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "sounds"))
	{
		m_sounds = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else 
		CBaseToggle::KeyValue( pkvd );
}

//
// ButtonShot
//
int CBaseButton::TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType )
{
	BUTTON_CODE code = ButtonResponseToTouch();
	
	if ( code == BUTTON_NOTHING )
		return 0;
	// Temporarily disable the touch function, until movement is finished.
	SetTouch( NULL );

	m_hActivator = CBaseEntity::Instance( pevAttacker );
	if ( m_hActivator == NULL )
		return 0;

	if ( code == BUTTON_RETURN )
	{
		EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING(pev->noise), 1, ATTN_NORM);

		// Toggle buttons fire when they get back to their "home" position
		if ( !(pev->spawnflags & SF_BUTTON_TOGGLE) )
			SUB_UseTargets( m_hActivator, USE_TOGGLE, 0 );
		ButtonReturn();
	}
	else // code == BUTTON_ACTIVATE
		ButtonActivate( );

	return 0;
}

/*QUAKED func_button (0 .5 .8) ?
When a button is touched, it moves some distance in the direction of it's angle,
triggers all of it's targets, waits some time, then returns to it's original position
where it can be triggered again.

"angle"		determines the opening direction
"target"	all entities with a matching targetname will be used
"speed"		override the default 40 speed
"wait"		override the default 1 second wait (-1 = never return)
"lip"		override the default 4 pixel lip remaining at end of move
"health"	if set, the button must be killed instead of touched
"sounds"
0) steam metal
1) wooden clunk
2) metallic click
3) in-out
*/
LINK_ENTITY_TO_CLASS( func_button, CBaseButton );


void CBaseButton::Spawn( )
{ 
	char  *pszSound;

	//----------------------------------------------------
	//determine sounds for buttons
	//a sound of 0 should not make a sound
	//----------------------------------------------------
	pszSound = ButtonSound( m_sounds );
	PRECACHE_SOUND(pszSound);
	pev->noise = ALLOC_STRING(pszSound);

	Precache();

	if ( FBitSet ( pev->spawnflags, SF_BUTTON_SPARK_IF_OFF ) )// this button should spark in OFF state
	{
		SetThink ( ButtonSpark );
		pev->nextthink = gpGlobals->time + 0.5;// no hurry, make sure everything else spawns
	}

	SetMovedir(pev);

	pev->movetype	= MOVETYPE_PUSH;
	pev->solid		= SOLID_BSP;
	SET_MODEL(ENT(pev), STRING(pev->model));
	
	if (pev->speed == 0)
		pev->speed = 40;

	if (pev->health > 0)
	{
		pev->takedamage = DAMAGE_YES;
	}

	if (m_flWait == 0)
		m_flWait = 1;
	if (m_flLip == 0)
		m_flLip = 4;

	m_toggle_state = TS_AT_BOTTOM;
	m_vecPosition1 = pev->origin;
	// Subtract 2 from size because the engine expands bboxes by 1 in all directions making the size too big
	m_vecPosition2	= m_vecPosition1 + (pev->movedir * (fabs( pev->movedir.x * (pev->size.x-2) ) + fabs( pev->movedir.y * (pev->size.y-2) ) + fabs( pev->movedir.z * (pev->size.z-2) ) - m_flLip));


	// Is this a non-moving button?
	if ( ((m_vecPosition2 - m_vecPosition1).Length() < 1) || (pev->spawnflags & SF_BUTTON_DONTMOVE) )
		m_vecPosition2 = m_vecPosition1;

	m_fStayPushed = (m_flWait == -1 ? TRUE : FALSE);
	m_fRotating = FALSE;

	// if the button is flagged for USE button activation only, take away it's touch function and add a use function

	if ( FBitSet ( pev->spawnflags, SF_BUTTON_TOUCH_ONLY ) ) // touchable button
	{
		SetTouch( ButtonTouch );
	}
	else 
	{
		SetTouch ( NULL );
		SetUse	 ( ButtonUse );
	}
}


// Button sound table. 
// Also used by CBaseDoor to get 'touched' door lock/unlock sounds

char *ButtonSound( int sound )
{ 
	char *pszSound;

	switch ( sound )
	{
		case 0: pszSound = "common/null.wav";        break;
		case 1: pszSound = "buttons/button1.wav";	break;
		case 2: pszSound = "buttons/button2.wav";	break;
		case 3: pszSound = "buttons/button3.wav";	break;
		case 4: pszSound = "buttons/button4.wav";	break;
		case 5: pszSound = "buttons/button5.wav";	break;
		case 6: pszSound = "buttons/button6.wav";	break;
		case 7: pszSound = "buttons/button7.wav";	break;
		case 8: pszSound = "buttons/button8.wav";	break;
		case 9: pszSound = "buttons/button9.wav";	break;
		case 10: pszSound = "buttons/button10.wav";	break;
		case 11: pszSound = "buttons/button11.wav";	break;
		case 12: pszSound = "buttons/latchlocked1.wav";	break;
		case 13: pszSound = "buttons/latchunlocked1.wav";	break;
		case 14: pszSound = "buttons/lightswitch2.wav";break;

// next 6 slots reserved for any additional sliding button sounds we may add
		
		case 21: pszSound = "buttons/lever1.wav";	break;
		case 22: pszSound = "buttons/lever2.wav";	break;
		case 23: pszSound = "buttons/lever3.wav";	break;
		case 24: pszSound = "buttons/lever4.wav";	break;
		case 25: pszSound = "buttons/lever5.wav";	break;

		default:pszSound = "buttons/button9.wav";	break;
	}

	return pszSound;
}

//
// Makes flagged buttons spark when turned off
//

void DoSpark(entvars_t *pev, const Vector &location )
{
	Vector tmp = location + pev->size * 0.5;
	UTIL_Sparks( tmp );

	float flVolume = RANDOM_FLOAT ( 0.25 , 0.75 ) * 0.4;//random volume range
	switch ( (int)(RANDOM_FLOAT(0,1) * 6) )
	{
		case 0: EMIT_SOUND(ENT(pev), CHAN_VOICE, "buttons/spark1.wav", flVolume, ATTN_NORM);	break;
		case 1: EMIT_SOUND(ENT(pev), CHAN_VOICE, "buttons/spark2.wav", flVolume, ATTN_NORM);	break;
		case 2: EMIT_SOUND(ENT(pev), CHAN_VOICE, "buttons/spark3.wav", flVolume, ATTN_NORM);	break;
		case 3: EMIT_SOUND(ENT(pev), CHAN_VOICE, "buttons/spark4.wav", flVolume, ATTN_NORM);	break;
		case 4: EMIT_SOUND(ENT(pev), CHAN_VOICE, "buttons/spark5.wav", flVolume, ATTN_NORM);	break;
		case 5: EMIT_SOUND(ENT(pev), CHAN_VOICE, "buttons/spark6.wav", flVolume, ATTN_NORM);	break;
	}
}

void CBaseButton::ButtonSpark ( void )
{
	SetThink ( ButtonSpark );
	pev->nextthink = gpGlobals->time + ( 0.1 + RANDOM_FLOAT ( 0, 1.5 ) );// spark again at random interval

	DoSpark( pev, pev->mins );
}


//
// Button's Use function
//
void CBaseButton::ButtonUse ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	// Ignore touches if button is moving, or pushed-in and waiting to auto-come-out.
	// UNDONE: Should this use ButtonResponseToTouch() too?
	if (m_toggle_state == TS_GOING_UP || m_toggle_state == TS_GOING_DOWN )
		return;		

	m_hActivator = pActivator;
	if ( m_toggle_state == TS_AT_TOP)
	{
		if (!m_fStayPushed && FBitSet(pev->spawnflags, SF_BUTTON_TOGGLE))
		{
			EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING(pev->noise), 1, ATTN_NORM);
			
			//SUB_UseTargets( m_eoActivator );
			ButtonReturn();
		}
	}
	else
		ButtonActivate( );
}


CBaseButton::BUTTON_CODE CBaseButton::ButtonResponseToTouch( void )
{
	// Ignore touches if button is moving, or pushed-in and waiting to auto-come-out.
	if (m_toggle_state == TS_GOING_UP ||
		m_toggle_state == TS_GOING_DOWN ||
		(m_toggle_state == TS_AT_TOP && !m_fStayPushed && !FBitSet(pev->spawnflags, SF_BUTTON_TOGGLE) ) )
		return BUTTON_NOTHING;

	if (m_toggle_state == TS_AT_TOP)
	{
		if((FBitSet(pev->spawnflags, SF_BUTTON_TOGGLE) ) && !m_fStayPushed)
		{
			return BUTTON_RETURN;
		}
	}
	else
		return BUTTON_ACTIVATE;

	return BUTTON_NOTHING;
}


//
// Touching a button simply "activates" it.
//
void CBaseButton:: ButtonTouch( CBaseEntity *pOther )
{
	// Ignore touches by anything but players
	if (!FClassnameIs(pOther->pev, "player"))
		return;

	m_hActivator = pOther;

	BUTTON_CODE code = ButtonResponseToTouch();

	if ( code == BUTTON_NOTHING )
		return;

	if (!UTIL_IsMasterTriggered(m_sMaster, pOther))
	{
		// play button locked sound
		PlayLockSounds(pev, &m_ls, TRUE, TRUE);
		return;
	}

	// Temporarily disable the touch function, until movement is finished.
	SetTouch( NULL );

	if ( code == BUTTON_RETURN )
	{
		EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING(pev->noise), 1, ATTN_NORM);
		SUB_UseTargets( m_hActivator, USE_TOGGLE, 0 );
		ButtonReturn();
	}
	else	// code == BUTTON_ACTIVATE
		ButtonActivate( );
}

//
// Starts the button moving "in/up".
//
void CBaseButton::ButtonActivate( )
{
	EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING(pev->noise), 1, ATTN_NORM);
	
	if (!UTIL_IsMasterTriggered(m_sMaster, m_hActivator))
	{
		// button is locked, play locked sound
		PlayLockSounds(pev, &m_ls, TRUE, TRUE);
		return;
	}
	else
	{
		// button is unlocked, play unlocked sound
		PlayLockSounds(pev, &m_ls, FALSE, TRUE);
	}

	ASSERT(m_toggle_state == TS_AT_BOTTOM);
	m_toggle_state = TS_GOING_UP;
	
	SetMoveDone( TriggerAndWait );
	if (!m_fRotating)
		LinearMove( m_vecPosition2, pev->speed);
	else
		AngularMove( m_vecAngle2, pev->speed);
}

//
// Button has reached the "in/up" position.  Activate its "targets", and pause before "popping out".
//
void CBaseButton::TriggerAndWait( void )
{
	ASSERT(m_toggle_state == TS_GOING_UP);

	if (!UTIL_IsMasterTriggered(m_sMaster, m_hActivator))
		return;

	m_toggle_state = TS_AT_TOP;
	
	// If button automatically comes back out, start it moving out.
	// Else re-instate touch method
	if (m_fStayPushed || FBitSet ( pev->spawnflags, SF_BUTTON_TOGGLE ) )
	{
		if ( !FBitSet ( pev->spawnflags, SF_BUTTON_TOUCH_ONLY ) ) // this button only works if USED, not touched!
		{
		// ALL buttons are now use only
		SetTouch ( NULL );
		}
		else
			SetTouch( ButtonTouch );
	}
	else
	{
		pev->nextthink = pev->ltime + m_flWait;
		SetThink( ButtonReturn );
	}
	
	pev->frame = 1;			// use alternate textures


	SUB_UseTargets( m_hActivator, USE_TOGGLE, 0 );
}


//
// Starts the button moving "out/down".
//
void CBaseButton::ButtonReturn( void )
{
	ASSERT(m_toggle_state == TS_AT_TOP);
	m_toggle_state = TS_GOING_DOWN;
	
	SetMoveDone( ButtonBackHome );
	if (!m_fRotating)
		LinearMove( m_vecPosition1, pev->speed);
	else
		AngularMove( m_vecAngle1, pev->speed);

	pev->frame = 0;			// use normal textures
}


//
// Button has returned to start state.  Quiesce it.
//
void CBaseButton::ButtonBackHome( void )
{
	ASSERT(m_toggle_state == TS_GOING_DOWN);
	m_toggle_state = TS_AT_BOTTOM;

	if ( FBitSet(pev->spawnflags, SF_BUTTON_TOGGLE) )
	{
		//EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING(pev->noise), 1, ATTN_NORM);
		
		SUB_UseTargets( m_hActivator, USE_TOGGLE, 0 );
	}


	if (!FStringNull(pev->target))
	{
		edict_t* pentTarget	= NULL;
		for (;;)
		{
			pentTarget = FIND_ENTITY_BY_TARGETNAME(pentTarget, STRING(pev->target));

			if (FNullEnt(pentTarget))
				break;

			if (!FClassnameIs(pentTarget, "multisource"))
				continue;
			CBaseEntity *pTarget = CBaseEntity::Instance( pentTarget );

			if ( pTarget )
				pTarget->Use( m_hActivator, this, USE_TOGGLE, 0 );
		}
	}

// Re-instate touch method, movement cycle is complete.
	if ( !FBitSet ( pev->spawnflags, SF_BUTTON_TOUCH_ONLY ) ) // this button only works if USED, not touched!
	{
	// All buttons are now use only	
		SetTouch ( NULL );
	}
	else
		SetTouch( ButtonTouch );

// reset think for a sparking button
	if ( FBitSet ( pev->spawnflags, SF_BUTTON_SPARK_IF_OFF ) )
	{
		SetThink ( ButtonSpark );
		pev->nextthink = gpGlobals->time + 0.5;// no hurry.
	}
}



//
// Rotating button (aka "lever")
//
class CRotButton : public CBaseButton
{
public:
	void Spawn( void );
};

LINK_ENTITY_TO_CLASS( func_rot_button, CRotButton );

void CRotButton::Spawn( void )
{
	char *pszSound;
	//----------------------------------------------------
	//determine sounds for buttons
	//a sound of 0 should not make a sound
	//----------------------------------------------------
	pszSound = ButtonSound( m_sounds );
	PRECACHE_SOUND(pszSound);
	pev->noise = ALLOC_STRING(pszSound);

	// set the axis of rotation
	CBaseToggle::AxisDir( pev );

	// check for clockwise rotation
	if ( FBitSet (pev->spawnflags, SF_DOOR_ROTATE_BACKWARDS) )
		pev->movedir = pev->movedir * -1;

	pev->movetype	= MOVETYPE_PUSH;
	
	if ( pev->spawnflags & SF_ROTBUTTON_NOTSOLID )
		pev->solid		= SOLID_NOT;
	else
		pev->solid		= SOLID_BSP;

	SET_MODEL(ENT(pev), STRING(pev->model));
	
	if (pev->speed == 0)
		pev->speed = 40;

	if (m_flWait == 0)
		m_flWait = 1;

	if (pev->health > 0)
	{
		pev->takedamage = DAMAGE_YES;
	}

	m_toggle_state = TS_AT_BOTTOM;
	m_vecAngle1	= pev->angles;
	m_vecAngle2	= pev->angles + pev->movedir * m_flMoveDistance;
	ASSERTSZ(m_vecAngle1 != m_vecAngle2, "rotating button start/end positions are equal");

	m_fStayPushed = (m_flWait == -1 ? TRUE : FALSE);
	m_fRotating = TRUE;

	// if the button is flagged for USE button activation only, take away it's touch function and add a use function
	if ( !FBitSet ( pev->spawnflags, SF_BUTTON_TOUCH_ONLY ) )
	{
		SetTouch ( NULL );
		SetUse	 ( ButtonUse );
	}
	else // touchable button
		SetTouch( ButtonTouch );

	//SetTouch( ButtonTouch );
}


// Make this button behave like a door (HACKHACK)
// This will disable use and make the button solid
// rotating buttons were made SOLID_NOT by default since their were some
// collision problems with them...
#define SF_MOMENTARY_DOOR		0x0001

class CMomentaryRotButton : public CBaseToggle
{
public:
	void	Spawn ( void );
	void	KeyValue( KeyValueData *pkvd );
	virtual int	ObjectCaps( void ) 
	{ 
		int flags = CBaseToggle :: ObjectCaps() & (~FCAP_ACROSS_TRANSITION); 
		if ( pev->spawnflags & SF_MOMENTARY_DOOR )
			return flags;
		return flags | FCAP_CONTINUOUS_USE;
	}
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void	EXPORT Off( void );
	void	EXPORT Return( void );
	void	UpdateSelf( float value );
	void	UpdateSelfReturn( float value );
	void	UpdateAllButtons( float value, int start );

	void	PlaySound( void );
	void	UpdateTarget( float value );

	static CMomentaryRotButton *Instance( edict_t *pent ) { return (CMomentaryRotButton *)GET_PRIVATE(pent);};
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );

	static	TYPEDESCRIPTION m_SaveData[];

	int		m_lastUsed;
	int		m_direction;
	float	m_returnSpeed;
	vec3_t	m_start;
	vec3_t	m_end;
	int		m_sounds;
};
TYPEDESCRIPTION CMomentaryRotButton::m_SaveData[] =
{
	DEFINE_FIELD( CMomentaryRotButton, m_lastUsed, FIELD_INTEGER ),
	DEFINE_FIELD( CMomentaryRotButton, m_direction, FIELD_INTEGER ),
	DEFINE_FIELD( CMomentaryRotButton, m_returnSpeed, FIELD_FLOAT ),
	DEFINE_FIELD( CMomentaryRotButton, m_start, FIELD_VECTOR ),
	DEFINE_FIELD( CMomentaryRotButton, m_end, FIELD_VECTOR ),
	DEFINE_FIELD( CMomentaryRotButton, m_sounds, FIELD_INTEGER ),
};

IMPLEMENT_SAVERESTORE( CMomentaryRotButton, CBaseToggle );

LINK_ENTITY_TO_CLASS( momentary_rot_button, CMomentaryRotButton );

void CMomentaryRotButton::Spawn( void )
{
	CBaseToggle::AxisDir( pev );

	if ( pev->speed == 0 )
		pev->speed = 100;

	if ( m_flMoveDistance < 0 ) 
	{
		m_start = pev->angles + pev->movedir * m_flMoveDistance;
		m_end = pev->angles;
		m_direction = 1;		// This will toggle to -1 on the first use()
		m_flMoveDistance = -m_flMoveDistance;
	}
	else
	{
		m_start = pev->angles;
		m_end = pev->angles + pev->movedir * m_flMoveDistance;
		m_direction = -1;		// This will toggle to +1 on the first use()
	}

	if ( pev->spawnflags & SF_MOMENTARY_DOOR )
		pev->solid		= SOLID_BSP;
	else
		pev->solid		= SOLID_NOT;

	pev->movetype	= MOVETYPE_PUSH;
	UTIL_SetOrigin(pev, pev->origin);
	SET_MODEL(ENT(pev), STRING(pev->model) );

	char *pszSound = ButtonSound( m_sounds );
	PRECACHE_SOUND(pszSound);
	pev->noise = ALLOC_STRING(pszSound);
	m_lastUsed = 0;
}

void CMomentaryRotButton::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "returnspeed"))
	{
		m_returnSpeed = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "sounds"))
	{
		m_sounds = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseToggle::KeyValue( pkvd );
}

void CMomentaryRotButton::PlaySound( void )
{
	EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING(pev->noise), 1, ATTN_NORM);
}

// BUGBUG: This design causes a latentcy.  When the button is retriggered, the first impulse
// will send the target in the wrong direction because the parameter is calculated based on the
// current, not future position.
void CMomentaryRotButton::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	pev->ideal_yaw = CBaseToggle::AxisDelta( pev->spawnflags, pev->angles, m_start ) / m_flMoveDistance;

	UpdateAllButtons( pev->ideal_yaw, 1 );
	UpdateTarget( pev->ideal_yaw );
}

void CMomentaryRotButton::UpdateAllButtons( float value, int start )
{
	// Update all rot buttons attached to the same target
	edict_t *pentTarget = NULL;
	for (;;)
	{

		pentTarget = FIND_ENTITY_BY_STRING(pentTarget, "target", STRING(pev->target));
		if (FNullEnt(pentTarget))
			break;

		if ( FClassnameIs( VARS(pentTarget), "momentary_rot_button" ) )
		{
			CMomentaryRotButton *pEntity = CMomentaryRotButton::Instance(pentTarget);
			if ( pEntity )
			{
				if ( start )
					pEntity->UpdateSelf( value );
				else
					pEntity->UpdateSelfReturn( value );
			}
		}
	}
}

void CMomentaryRotButton::UpdateSelf( float value )
{
	BOOL fplaysound = FALSE;

	if ( !m_lastUsed )
	{
		fplaysound = TRUE;
		m_direction = -m_direction;
	}
	m_lastUsed = 1;

	pev->nextthink = pev->ltime + 0.1;
	if ( m_direction > 0 && value >= 1.0 )
	{
		pev->avelocity = g_vecZero;
		pev->angles = m_end;
		return;
	}
	else if ( m_direction < 0 && value <= 0 )
	{
		pev->avelocity = g_vecZero;
		pev->angles = m_start;
		return;
	}
	
	if (fplaysound)
		PlaySound();

	// HACKHACK -- If we're going slow, we'll get multiple player packets per frame, bump nexthink on each one to avoid stalling
	if ( pev->nextthink < pev->ltime )
		pev->nextthink = pev->ltime + 0.1;
	else
		pev->nextthink += 0.1;
	
	pev->avelocity = (m_direction * pev->speed) * pev->movedir;
	SetThink( Off );
}

void CMomentaryRotButton::UpdateTarget( float value )
{
	if (!FStringNull(pev->target))
	{
		edict_t* pentTarget	= NULL;
		for (;;)
		{
			pentTarget = FIND_ENTITY_BY_TARGETNAME(pentTarget, STRING(pev->target));
			if (FNullEnt(pentTarget))
				break;
			CBaseEntity *pEntity = CBaseEntity::Instance(pentTarget);
			if ( pEntity )
			{
				pEntity->Use( this, this, USE_SET, value );
			}
		}
	}
}

void CMomentaryRotButton::Off( void )
{
	pev->avelocity = g_vecZero;
	m_lastUsed = 0;
	if ( FBitSet( pev->spawnflags, SF_PENDULUM_AUTO_RETURN ) && m_returnSpeed > 0 )
	{
		SetThink( Return );
		pev->nextthink = pev->ltime + 0.1;
		m_direction = -1;
	}
	else
		SetThink( NULL );
}

void CMomentaryRotButton::Return( void )
{
	float value = CBaseToggle::AxisDelta( pev->spawnflags, pev->angles, m_start ) / m_flMoveDistance;

	UpdateAllButtons( value, 0 );	// This will end up calling UpdateSelfReturn() n times, but it still works right
	if ( value > 0 )
		UpdateTarget( value );
}


void CMomentaryRotButton::UpdateSelfReturn( float value )
{
	if ( value <= 0 )
	{
		pev->avelocity = g_vecZero;
		pev->angles = m_start;
		pev->nextthink = -1;
		SetThink( NULL );
	}
	else
	{
		pev->avelocity = -m_returnSpeed * pev->movedir;
		pev->nextthink = pev->ltime + 0.1;
	}
}


//----------------------------------------------------------------
// Spark
//----------------------------------------------------------------

class CEnvSpark : public CBaseEntity
{
public:
	void	Spawn(void);
	void	Precache(void);
	void	EXPORT SparkThink(void);
	void	EXPORT SparkStart(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void	EXPORT SparkStop(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void	KeyValue(KeyValueData *pkvd);
	
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );

	static	TYPEDESCRIPTION m_SaveData[];

	float	m_flDelay;
};


TYPEDESCRIPTION CEnvSpark::m_SaveData[] =
{
	DEFINE_FIELD( CEnvSpark, m_flDelay, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE( CEnvSpark, CBaseEntity );

LINK_ENTITY_TO_CLASS(env_spark, CEnvSpark);
LINK_ENTITY_TO_CLASS(env_debris, CEnvSpark);

void CEnvSpark::Spawn(void)
{
	SetThink( NULL );
	SetUse( NULL );

	if (FBitSet(pev->spawnflags, 32)) // Use for on/off
	{
		if (FBitSet(pev->spawnflags, 64)) // Start on
		{
			SetThink(SparkThink);	// start sparking
			SetUse(SparkStop);		// set up +USE to stop sparking
		}
		else
			SetUse(SparkStart);
	}
	else
		SetThink(SparkThink);
		
	pev->nextthink = gpGlobals->time + ( 0.1 + RANDOM_FLOAT ( 0, 1.5 ) );

	if (m_flDelay <= 0)
		m_flDelay = 1.5;

	Precache( );
}


void CEnvSpark::Precache(void)
{
	PRECACHE_SOUND( "buttons/spark1.wav" );
	PRECACHE_SOUND( "buttons/spark2.wav" );
	PRECACHE_SOUND( "buttons/spark3.wav" );
	PRECACHE_SOUND( "buttons/spark4.wav" );
	PRECACHE_SOUND( "buttons/spark5.wav" );
	PRECACHE_SOUND( "buttons/spark6.wav" );
}

void CEnvSpark::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "MaxDelay"))
	{
		m_flDelay = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;	
	}
	else if (	FStrEq(pkvd->szKeyName, "style") ||
				FStrEq(pkvd->szKeyName, "height") ||
				FStrEq(pkvd->szKeyName, "killtarget") ||
				FStrEq(pkvd->szKeyName, "value1") ||
				FStrEq(pkvd->szKeyName, "value2") ||
				FStrEq(pkvd->szKeyName, "value3"))
		pkvd->fHandled = TRUE;
	else
		CBaseEntity::KeyValue( pkvd );
}

void EXPORT CEnvSpark::SparkThink(void)
{
	pev->nextthink = gpGlobals->time + 0.1 + RANDOM_FLOAT (0, m_flDelay);
	DoSpark( pev, pev->origin );
}

void EXPORT CEnvSpark::SparkStart(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	SetUse(SparkStop);
	SetThink(SparkThink);
	pev->nextthink = gpGlobals->time + (0.1 + RANDOM_FLOAT ( 0, m_flDelay));
}

void EXPORT CEnvSpark::SparkStop(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	SetUse(SparkStart);
	SetThink(NULL);
}

#define SF_BTARGET_USE		0x0001
#define SF_BTARGET_ON		0x0002

class CButtonTarget : public CBaseEntity
{
public:
	void Spawn( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType );
	int	ObjectCaps( void );
	
};

LINK_ENTITY_TO_CLASS( button_target, CButtonTarget );

void CButtonTarget::Spawn( void )
{
	pev->movetype	= MOVETYPE_PUSH;
	pev->solid		= SOLID_BSP;
	SET_MODEL(ENT(pev), STRING(pev->model));
	pev->takedamage = DAMAGE_YES;

	if ( FBitSet( pev->spawnflags, SF_BTARGET_ON ) )
		pev->frame = 1;
}

void CButtonTarget::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( !ShouldToggle( useType, (int)pev->frame ) )
		return;
	pev->frame = 1-pev->frame;
	if ( pev->frame )
		SUB_UseTargets( pActivator, USE_ON, 0 );
	else
		SUB_UseTargets( pActivator, USE_OFF, 0 );
}


int	CButtonTarget :: ObjectCaps( void )
{
	int caps = CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION;

	if ( FBitSet(pev->spawnflags, SF_BTARGET_USE) )
		return caps | FCAP_IMPULSE_USE;
	else
		return caps;
}


int CButtonTarget::TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType )
{
	Use( Instance(pevAttacker), this, USE_TOGGLE, 0 );

	return 1;
}
