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
#if !defined( OEM_BUILD ) && !defined( HLDEMO_BUILD )

/*

	h_tentacle.cpp - silo of death tentacle monster (half life)

*/

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"monsters.h"
#include	"weapons.h"
#include	"soundent.h"


#define ACT_T_IDLE		1010
#define ACT_T_TAP			1020
#define ACT_T_STRIKE		1030
#define ACT_T_REARIDLE	1040

class CTentacle : public CBaseMonster 
{
public:
	CTentacle( void );

	void Spawn( );
	void Precache( );
	void KeyValue( KeyValueData *pkvd );

	int		Save( CSave &save );
	int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	// Don't allow the tentacle to go across transitions!!!
	virtual int	ObjectCaps( void ) { return CBaseMonster :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	void SetObjectCollisionBox( void )
	{
		pev->absmin = pev->origin + Vector(-400, -400, 0);
		pev->absmax = pev->origin + Vector(400, 400, 850);
	}

	void EXPORT Cycle( void );
	void EXPORT CommandUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT Start( void );
	void EXPORT DieThink( void );

	void EXPORT Test( void );

	void EXPORT HitTouch( CBaseEntity *pOther );

	float HearingSensitivity( void ) { return 2.0; };

	int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType );
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	void Killed( entvars_t *pevAttacker, int iGib );

	MONSTERSTATE GetIdealState ( void ) { return MONSTERSTATE_IDLE; };
	int CanPlaySequence( BOOL fDisregardState ) { return TRUE; };

	int Classify( void );

	int Level( float dz );
	int MyLevel( void );
	float MyHeight( void );

	float m_flInitialYaw;
	int m_iGoalAnim;
	int m_iLevel;
	int m_iDir;
	float m_flFramerateAdj;
	float m_flSoundYaw;
	int m_iSoundLevel;
	float m_flSoundTime;
	float m_flSoundRadius;
	int m_iHitDmg;
	float m_flHitTime;

	float m_flTapRadius;

	float m_flNextSong;
	static int g_fFlySound;
	static int g_fSquirmSound;

	float m_flMaxYaw;
	int m_iTapSound;

	Vector m_vecPrevSound;
	float m_flPrevSoundTime;

	static const char *pHitSilo[];
	static const char *pHitDirt[];
	static const char *pHitWater[];
};



int CTentacle :: g_fFlySound;
int CTentacle :: g_fSquirmSound;

LINK_ENTITY_TO_CLASS( monster_tentacle, CTentacle );

// stike sounds
#define TE_NONE -1
#define TE_SILO 0
#define TE_DIRT 1
#define TE_WATER 2

const char *CTentacle::pHitSilo[] = 
{
	"tentacle/te_strike1.wav",
	"tentacle/te_strike2.wav",
};

const char *CTentacle::pHitDirt[] = 
{
	"player/pl_dirt1.wav",
	"player/pl_dirt2.wav",
	"player/pl_dirt3.wav",
	"player/pl_dirt4.wav",
};

const char *CTentacle::pHitWater[] = 
{
	"player/pl_slosh1.wav",
	"player/pl_slosh2.wav",
	"player/pl_slosh3.wav",
	"player/pl_slosh4.wav",
};


TYPEDESCRIPTION	CTentacle::m_SaveData[] = 
{
	DEFINE_FIELD( CTentacle, m_flInitialYaw, FIELD_FLOAT ),
	DEFINE_FIELD( CTentacle, m_iGoalAnim, FIELD_INTEGER ),
	DEFINE_FIELD( CTentacle, m_iLevel, FIELD_INTEGER ),
	DEFINE_FIELD( CTentacle, m_iDir, FIELD_INTEGER ),
	DEFINE_FIELD( CTentacle, m_flFramerateAdj, FIELD_FLOAT ),
	DEFINE_FIELD( CTentacle, m_flSoundYaw, FIELD_FLOAT ),
	DEFINE_FIELD( CTentacle, m_iSoundLevel, FIELD_INTEGER ),
	DEFINE_FIELD( CTentacle, m_flSoundTime, FIELD_TIME ),
	DEFINE_FIELD( CTentacle, m_flSoundRadius, FIELD_FLOAT ),
	DEFINE_FIELD( CTentacle, m_iHitDmg, FIELD_INTEGER ),
	DEFINE_FIELD( CTentacle, m_flHitTime, FIELD_TIME ),
	DEFINE_FIELD( CTentacle, m_flTapRadius, FIELD_FLOAT ),
	DEFINE_FIELD( CTentacle, m_flNextSong, FIELD_TIME ),
	DEFINE_FIELD( CTentacle, m_iTapSound, FIELD_INTEGER ),
	DEFINE_FIELD( CTentacle, m_flMaxYaw, FIELD_FLOAT ),
	DEFINE_FIELD( CTentacle, m_vecPrevSound, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( CTentacle, m_flPrevSoundTime, FIELD_TIME ),
};
IMPLEMENT_SAVERESTORE( CTentacle, CBaseMonster );


// animation sequence aliases 
typedef enum
{
	TENTACLE_ANIM_Pit_Idle,

	TENTACLE_ANIM_rise_to_Temp1,
	TENTACLE_ANIM_Temp1_to_Floor,
	TENTACLE_ANIM_Floor_Idle,
	TENTACLE_ANIM_Floor_Fidget_Pissed,
	TENTACLE_ANIM_Floor_Fidget_SmallRise,
	TENTACLE_ANIM_Floor_Fidget_Wave,
	TENTACLE_ANIM_Floor_Strike,
	TENTACLE_ANIM_Floor_Tap,
	TENTACLE_ANIM_Floor_Rotate,
	TENTACLE_ANIM_Floor_Rear,
	TENTACLE_ANIM_Floor_Rear_Idle,
	TENTACLE_ANIM_Floor_to_Lev1,

	TENTACLE_ANIM_Lev1_Idle,
	TENTACLE_ANIM_Lev1_Fidget_Claw,
	TENTACLE_ANIM_Lev1_Fidget_Shake,
	TENTACLE_ANIM_Lev1_Fidget_Snap,
	TENTACLE_ANIM_Lev1_Strike,
	TENTACLE_ANIM_Lev1_Tap,
	TENTACLE_ANIM_Lev1_Rotate,
	TENTACLE_ANIM_Lev1_Rear,
	TENTACLE_ANIM_Lev1_Rear_Idle,
	TENTACLE_ANIM_Lev1_to_Lev2,

	TENTACLE_ANIM_Lev2_Idle,
	TENTACLE_ANIM_Lev2_Fidget_Shake,
	TENTACLE_ANIM_Lev2_Fidget_Swing,
	TENTACLE_ANIM_Lev2_Fidget_Tut,
	TENTACLE_ANIM_Lev2_Strike,
	TENTACLE_ANIM_Lev2_Tap,
	TENTACLE_ANIM_Lev2_Rotate,
	TENTACLE_ANIM_Lev2_Rear,
	TENTACLE_ANIM_Lev2_Rear_Idle,
	TENTACLE_ANIM_Lev2_to_Lev3,

	TENTACLE_ANIM_Lev3_Idle,
	TENTACLE_ANIM_Lev3_Fidget_Shake,
	TENTACLE_ANIM_Lev3_Fidget_Side,
	TENTACLE_ANIM_Lev3_Fidget_Swipe,
	TENTACLE_ANIM_Lev3_Strike,
	TENTACLE_ANIM_Lev3_Tap,
	TENTACLE_ANIM_Lev3_Rotate,
	TENTACLE_ANIM_Lev3_Rear,
	TENTACLE_ANIM_Lev3_Rear_Idle,

	TENTACLE_ANIM_Lev1_Door_reach,

	TENTACLE_ANIM_Lev3_to_Engine,
	TENTACLE_ANIM_Engine_Idle,
	TENTACLE_ANIM_Engine_Sway,
	TENTACLE_ANIM_Engine_Swat,
	TENTACLE_ANIM_Engine_Bob,
	TENTACLE_ANIM_Engine_Death1,
	TENTACLE_ANIM_Engine_Death2,
	TENTACLE_ANIM_Engine_Death3,

	TENTACLE_ANIM_none
} TENTACLE_ANIM;





//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int	CTentacle :: Classify ( void )
{
	return	CLASS_ALIEN_MONSTER;
}

//
// Tentacle Spawn
//
void CTentacle :: Spawn( )
{
	Precache( );

	pev->solid			= SOLID_BBOX;
	pev->movetype		= MOVETYPE_FLY;
	pev->effects		= 0;
	pev->health			= 75;
	pev->sequence		= 0;

	SET_MODEL(ENT(pev), "models/tentacle2.mdl");
	UTIL_SetSize( pev, Vector( -32, -32, 0 ), Vector( 32, 32, 64 ) );

	pev->takedamage		= DAMAGE_AIM;
	pev->flags			|= FL_MONSTER;
	
	m_bloodColor		= BLOOD_COLOR_GREEN;

	SetThink( &CTentacle::Start );
	SetTouch( &CTentacle::HitTouch );
	SetUse( &CTentacle::CommandUse );

	pev->nextthink = gpGlobals->time + 0.2;

	ResetSequenceInfo( );
	m_iDir = 1;

	pev->yaw_speed = 18;
	m_flInitialYaw = pev->angles.y;
	pev->ideal_yaw = m_flInitialYaw;

	g_fFlySound = FALSE;
	g_fSquirmSound = FALSE;

	m_iHitDmg = 20;

	if (m_flMaxYaw <= 0)
		m_flMaxYaw = 65;

	m_MonsterState = MONSTERSTATE_IDLE;

	// SetThink( Test );
	UTIL_SetOrigin( pev, pev->origin );
}

void CTentacle :: Precache( )
{
	PRECACHE_MODEL("models/tentacle2.mdl");

	PRECACHE_SOUND("ambience/flies.wav");
	PRECACHE_SOUND("ambience/squirm2.wav");

	PRECACHE_SOUND("tentacle/te_alert1.wav");
	PRECACHE_SOUND("tentacle/te_alert2.wav");
	PRECACHE_SOUND("tentacle/te_flies1.wav");
	PRECACHE_SOUND("tentacle/te_move1.wav");
	PRECACHE_SOUND("tentacle/te_move2.wav");
	PRECACHE_SOUND("tentacle/te_roar1.wav");
	PRECACHE_SOUND("tentacle/te_roar2.wav");
	PRECACHE_SOUND("tentacle/te_search1.wav");
	PRECACHE_SOUND("tentacle/te_search2.wav");
	PRECACHE_SOUND("tentacle/te_sing1.wav");
	PRECACHE_SOUND("tentacle/te_sing2.wav");
	PRECACHE_SOUND("tentacle/te_squirm2.wav");
	PRECACHE_SOUND("tentacle/te_strike1.wav");
	PRECACHE_SOUND("tentacle/te_strike2.wav");
	PRECACHE_SOUND("tentacle/te_swing1.wav");
	PRECACHE_SOUND("tentacle/te_swing2.wav");

	PRECACHE_SOUND_ARRAY( pHitSilo );
	PRECACHE_SOUND_ARRAY( pHitDirt );
	PRECACHE_SOUND_ARRAY( pHitWater );
}


CTentacle::CTentacle( )
{
	m_flMaxYaw = 65;
	m_iTapSound = 0;
}

void CTentacle::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "sweeparc"))
	{
		m_flMaxYaw = atof(pkvd->szValue) / 2.0;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "sound"))
	{
		m_iTapSound = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;

	}
	else
		CBaseMonster::KeyValue( pkvd );
}



int CTentacle :: Level( float dz )
{
	if (dz < 216)
		return 0;
	if (dz < 408)
		return 1;
	if (dz < 600)
		return 2;
	return 3;
}


float CTentacle :: MyHeight( )
{
	switch ( MyLevel( ) )
	{
	case 1:
		return 256;
	case 2:
		return 448;
	case 3:
		return 640;
	}
	return 0;
}


int CTentacle :: MyLevel( )
{
	switch( pev->sequence )
	{
	case TENTACLE_ANIM_Pit_Idle: 
		return -1;

	case TENTACLE_ANIM_rise_to_Temp1:
	case TENTACLE_ANIM_Temp1_to_Floor:
	case TENTACLE_ANIM_Floor_to_Lev1:
		return 0;

	case TENTACLE_ANIM_Floor_Idle:
	case TENTACLE_ANIM_Floor_Fidget_Pissed:
	case TENTACLE_ANIM_Floor_Fidget_SmallRise:
	case TENTACLE_ANIM_Floor_Fidget_Wave:
	case TENTACLE_ANIM_Floor_Strike:
	case TENTACLE_ANIM_Floor_Tap:
	case TENTACLE_ANIM_Floor_Rotate:
	case TENTACLE_ANIM_Floor_Rear:
	case TENTACLE_ANIM_Floor_Rear_Idle:
		return 0;

	case TENTACLE_ANIM_Lev1_Idle:
	case TENTACLE_ANIM_Lev1_Fidget_Claw:
	case TENTACLE_ANIM_Lev1_Fidget_Shake:
	case TENTACLE_ANIM_Lev1_Fidget_Snap:
	case TENTACLE_ANIM_Lev1_Strike:
	case TENTACLE_ANIM_Lev1_Tap:
	case TENTACLE_ANIM_Lev1_Rotate:
	case TENTACLE_ANIM_Lev1_Rear:
	case TENTACLE_ANIM_Lev1_Rear_Idle:
		return 1;

	case TENTACLE_ANIM_Lev1_to_Lev2:
		return 1;

	case TENTACLE_ANIM_Lev2_Idle:
	case TENTACLE_ANIM_Lev2_Fidget_Shake:
	case TENTACLE_ANIM_Lev2_Fidget_Swing:
	case TENTACLE_ANIM_Lev2_Fidget_Tut:
	case TENTACLE_ANIM_Lev2_Strike:
	case TENTACLE_ANIM_Lev2_Tap:
	case TENTACLE_ANIM_Lev2_Rotate:
	case TENTACLE_ANIM_Lev2_Rear:
	case TENTACLE_ANIM_Lev2_Rear_Idle:
		return 2;

	case TENTACLE_ANIM_Lev2_to_Lev3:
		return 2;

	case TENTACLE_ANIM_Lev3_Idle:
	case TENTACLE_ANIM_Lev3_Fidget_Shake:
	case TENTACLE_ANIM_Lev3_Fidget_Side:
	case TENTACLE_ANIM_Lev3_Fidget_Swipe:
	case TENTACLE_ANIM_Lev3_Strike:
	case TENTACLE_ANIM_Lev3_Tap:
	case TENTACLE_ANIM_Lev3_Rotate:
	case TENTACLE_ANIM_Lev3_Rear:
	case TENTACLE_ANIM_Lev3_Rear_Idle:
		return 3;

	case TENTACLE_ANIM_Lev1_Door_reach:
		return -1;
	}
	return -1;
}


void CTentacle :: Test( void )
{
	pev->sequence = TENTACLE_ANIM_Floor_Strike;
	pev->framerate = 0;
	StudioFrameAdvance( );
	pev->nextthink = gpGlobals->time + 0.1;
}



//
// TentacleThink
//
void CTentacle :: Cycle( void )
{
	// ALERT( at_console, "%s %.2f %d %d\n", STRING( pev->targetname ), pev->origin.z, m_MonsterState, m_IdealMonsterState );
	pev->nextthink = gpGlobals-> time + 0.1;

	// ALERT( at_console, "%s %d %d %d %f %f\n", STRING( pev->targetname ), pev->sequence, m_iGoalAnim, m_iDir, pev->framerate, pev->health );

	if (m_MonsterState == MONSTERSTATE_SCRIPT || m_IdealMonsterState == MONSTERSTATE_SCRIPT)
	{
		pev->angles.y = m_flInitialYaw;
		pev->ideal_yaw = m_flInitialYaw;	
		ClearConditions( IgnoreConditions() );
		MonsterThink( );
		m_iGoalAnim = TENTACLE_ANIM_Pit_Idle;
		return;
	}

	DispatchAnimEvents( );
	StudioFrameAdvance( );

	ChangeYaw( pev->yaw_speed );

	CSound *pSound;

	Listen( );

	// Listen will set this if there's something in my sound list
	if ( HasConditions( bits_COND_HEAR_SOUND ) )
		pSound = PBestSound();
	else
		pSound = NULL;

	if ( pSound )
	{
		Vector vecDir;
		if (gpGlobals->time - m_flPrevSoundTime < 0.5)
		{
			float dt = gpGlobals->time - m_flPrevSoundTime;
			vecDir = pSound->m_vecOrigin + (pSound->m_vecOrigin - m_vecPrevSound) / dt - pev->origin;
		}
		else
		{
			vecDir = pSound->m_vecOrigin - pev->origin;
		}
		m_flPrevSoundTime = gpGlobals->time;
		m_vecPrevSound = pSound->m_vecOrigin;

		m_flSoundYaw = UTIL_VecToYaw ( vecDir ) - m_flInitialYaw;
		m_iSoundLevel = Level( vecDir.z );

		if (m_flSoundYaw < -180)
			m_flSoundYaw += 360;
		if (m_flSoundYaw > 180)
			m_flSoundYaw -= 360;

		// ALERT( at_console, "sound %d %.0f\n", m_iSoundLevel, m_flSoundYaw );
		if (m_flSoundTime < gpGlobals->time)
		{
			// play "I hear new something" sound
			char *sound;	

			switch( RANDOM_LONG(0,1) )
			{
			case 0: sound = "tentacle/te_alert1.wav"; break;
			case 1: sound = "tentacle/te_alert2.wav"; break;
			}

			// UTIL_EmitAmbientSound(ENT(pev), pev->origin + Vector( 0, 0, MyHeight()), sound, 1.0, ATTN_NORM, 0, 100);
		}
		m_flSoundTime = gpGlobals->time + RANDOM_FLOAT( 5.0, 10.0 );
	}

	// clip ideal_yaw
	float dy = m_flSoundYaw;
	switch( pev->sequence )
	{
	case TENTACLE_ANIM_Floor_Rear:
	case TENTACLE_ANIM_Floor_Rear_Idle:
	case TENTACLE_ANIM_Lev1_Rear:
	case TENTACLE_ANIM_Lev1_Rear_Idle:
	case TENTACLE_ANIM_Lev2_Rear:
	case TENTACLE_ANIM_Lev2_Rear_Idle:
	case TENTACLE_ANIM_Lev3_Rear:
	case TENTACLE_ANIM_Lev3_Rear_Idle:
		if (dy < 0 && dy > -m_flMaxYaw)
			dy = -m_flMaxYaw;
		if (dy > 0 && dy < m_flMaxYaw)
			dy = m_flMaxYaw;
		break;
	default:
		if (dy < -m_flMaxYaw)
			dy = -m_flMaxYaw;
		if (dy > m_flMaxYaw)
			dy = m_flMaxYaw;
	}
	pev->ideal_yaw = m_flInitialYaw + dy;

	if (m_fSequenceFinished)
	{
		// ALERT( at_console, "%s done %d %d\n", STRING( pev->targetname ), pev->sequence, m_iGoalAnim );
		if (pev->health <= 1)
		{
			m_iGoalAnim = TENTACLE_ANIM_Pit_Idle;
			if (pev->sequence == TENTACLE_ANIM_Pit_Idle)
			{
				pev->health = 75;
			}
		}
		else if ( m_flSoundTime > gpGlobals->time )
		{
			if (m_flSoundYaw >= -(m_flMaxYaw + 30) && m_flSoundYaw <= (m_flMaxYaw + 30))
			{
				// strike
				m_iGoalAnim = LookupActivity( ACT_T_STRIKE + m_iSoundLevel );
			}
			else if (m_flSoundYaw >= -m_flMaxYaw * 2 && m_flSoundYaw <= m_flMaxYaw * 2) 
			{
				// tap
				m_iGoalAnim = LookupActivity( ACT_T_TAP + m_iSoundLevel );
			}
			else
			{
				// go into rear idle
				m_iGoalAnim = LookupActivity( ACT_T_REARIDLE + m_iSoundLevel );
			}
		}
		else if (pev->sequence == TENTACLE_ANIM_Pit_Idle)
		{
			// stay in pit until hear noise
			m_iGoalAnim = TENTACLE_ANIM_Pit_Idle;
		}
		else if (pev->sequence == m_iGoalAnim)
		{
			if (MyLevel() >= 0 && gpGlobals->time < m_flSoundTime)
			{
				if (RANDOM_LONG(0,9) < m_flSoundTime - gpGlobals->time)
				{
					// continue stike
					m_iGoalAnim = LookupActivity( ACT_T_STRIKE + m_iSoundLevel );
				}
				else
				{
					// tap
					m_iGoalAnim = LookupActivity( ACT_T_TAP + m_iSoundLevel );
				}
			}
			else if (MyLevel( ) < 0)
			{
				m_iGoalAnim = LookupActivity( ACT_T_IDLE + 0 );
			}
			else
			{
				if (m_flNextSong < gpGlobals->time)
				{
					// play "I hear new something" sound
					char *sound;	

					switch( RANDOM_LONG(0,1) )
					{
					case 0: sound = "tentacle/te_sing1.wav"; break;
					case 1: sound = "tentacle/te_sing2.wav"; break;
					}

					EMIT_SOUND(ENT(pev), CHAN_VOICE, sound, 1.0, ATTN_NORM);

					m_flNextSong = gpGlobals->time + RANDOM_FLOAT( 10, 20 );
				}

				if (RANDOM_LONG(0,15) == 0)
				{
					// idle on new level
					m_iGoalAnim = LookupActivity( ACT_T_IDLE + RANDOM_LONG(0,3) );
				}
				else if (RANDOM_LONG(0,3)  == 0)
				{
					// tap
					m_iGoalAnim = LookupActivity( ACT_T_TAP + MyLevel( ) );
				}
				else
				{
					// idle
					m_iGoalAnim = LookupActivity( ACT_T_IDLE + MyLevel( ) );
				}
			}
			if (m_flSoundYaw < 0)
				m_flSoundYaw += RANDOM_FLOAT( 2, 8 );
			else
				m_flSoundYaw -= RANDOM_FLOAT( 2, 8 );
		}

		pev->sequence = FindTransition( pev->sequence, m_iGoalAnim, &m_iDir );

		if (m_iDir > 0)
		{
			pev->frame = 0;
		}
		else
		{
			m_iDir = -1; // just to safe
			pev->frame = 255;
		}
		ResetSequenceInfo( );

		m_flFramerateAdj = RANDOM_FLOAT( -0.2, 0.2 );
		pev->framerate = m_iDir * 1.0 + m_flFramerateAdj;

		switch( pev->sequence)
		{
		case TENTACLE_ANIM_Floor_Tap:
		case TENTACLE_ANIM_Lev1_Tap:
		case TENTACLE_ANIM_Lev2_Tap:
		case TENTACLE_ANIM_Lev3_Tap:
			{
				Vector vecSrc;
				UTIL_MakeVectors( pev->angles );

				TraceResult tr1, tr2;

				vecSrc = pev->origin + Vector( 0, 0, MyHeight() - 4);
				UTIL_TraceLine( vecSrc, vecSrc + gpGlobals->v_forward * 512, ignore_monsters, ENT( pev ), &tr1 );

				vecSrc = pev->origin + Vector( 0, 0, MyHeight() + 8);
				UTIL_TraceLine( vecSrc, vecSrc + gpGlobals->v_forward * 512, ignore_monsters, ENT( pev ), &tr2 );

				// ALERT( at_console, "%f %f\n", tr1.flFraction * 512, tr2.flFraction * 512 );

				m_flTapRadius = SetBlending( 0, RANDOM_FLOAT( tr1.flFraction * 512, tr2.flFraction * 512 ) );
			}
			break;
		default:
			m_flTapRadius = 336; // 400 - 64
			break;
		}
		pev->view_ofs.z = MyHeight( );
		// ALERT( at_console, "seq %d\n", pev->sequence );
	}

	if (m_flPrevSoundTime + 2.0 > gpGlobals->time)
	{
		// 1.5 normal speed if hears sounds
		pev->framerate = m_iDir * 1.5 + m_flFramerateAdj;
	}
	else if (m_flPrevSoundTime + 5.0 > gpGlobals->time)
	{
		// slowdown to normal
		pev->framerate = m_iDir + m_iDir * (5 - (gpGlobals->time - m_flPrevSoundTime)) / 2 + m_flFramerateAdj;
	}
}



void CTentacle::CommandUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	// ALERT( at_console, "%s triggered %d\n", STRING( pev->targetname ), useType ); 
	switch( useType )
	{
	case USE_OFF:
		pev->takedamage = DAMAGE_NO;
		SetThink( &CTentacle::DieThink );
		m_iGoalAnim = TENTACLE_ANIM_Engine_Death1;
		break;
	case USE_ON:
		if (pActivator)
		{
			// ALERT( at_console, "insert sound\n");
			CSoundEnt::InsertSound ( bits_SOUND_WORLD, pActivator->pev->origin, 1024, 1.0 );
		}
		break;
	case USE_SET:
		break;
	case USE_TOGGLE:
		pev->takedamage = DAMAGE_NO;
		SetThink( &CTentacle::DieThink );
		m_iGoalAnim = TENTACLE_ANIM_Engine_Idle;
		break;
	}

}



void CTentacle :: DieThink( void )
{
	pev->nextthink = gpGlobals-> time + 0.1;

	DispatchAnimEvents( );
	StudioFrameAdvance( );

	ChangeYaw( 24 );

	if (m_fSequenceFinished)
	{
		if (pev->sequence == m_iGoalAnim)
		{
			switch( m_iGoalAnim )
			{
			case TENTACLE_ANIM_Engine_Idle:
			case TENTACLE_ANIM_Engine_Sway:
			case TENTACLE_ANIM_Engine_Swat:
			case TENTACLE_ANIM_Engine_Bob:
				m_iGoalAnim = TENTACLE_ANIM_Engine_Sway + RANDOM_LONG( 0, 2 );
				break;
			case TENTACLE_ANIM_Engine_Death1:
			case TENTACLE_ANIM_Engine_Death2:
			case TENTACLE_ANIM_Engine_Death3:
				UTIL_Remove( this );
				return;
			}
		}

		// ALERT( at_console, "%d : %d => ", pev->sequence, m_iGoalAnim );
		pev->sequence = FindTransition( pev->sequence, m_iGoalAnim, &m_iDir );
		// ALERT( at_console, "%d\n", pev->sequence );

		if (m_iDir > 0)
		{
			pev->frame = 0;
		}
		else
		{
			pev->frame = 255;
		}
		ResetSequenceInfo( );

		float dy;
		switch( pev->sequence )
		{
		case TENTACLE_ANIM_Floor_Rear:
		case TENTACLE_ANIM_Floor_Rear_Idle:
		case TENTACLE_ANIM_Lev1_Rear:
		case TENTACLE_ANIM_Lev1_Rear_Idle:
		case TENTACLE_ANIM_Lev2_Rear:
		case TENTACLE_ANIM_Lev2_Rear_Idle:
		case TENTACLE_ANIM_Lev3_Rear:
		case TENTACLE_ANIM_Lev3_Rear_Idle:
		case TENTACLE_ANIM_Engine_Idle:
		case TENTACLE_ANIM_Engine_Sway:
		case TENTACLE_ANIM_Engine_Swat:
		case TENTACLE_ANIM_Engine_Bob:
		case TENTACLE_ANIM_Engine_Death1:
		case TENTACLE_ANIM_Engine_Death2:
		case TENTACLE_ANIM_Engine_Death3:
			pev->framerate = RANDOM_FLOAT( m_iDir - 0.2, m_iDir + 0.2 );
			dy = 180;
			break;
		default:
			pev->framerate = 1.5;
			dy = 0;
			break;
		}
		pev->ideal_yaw = m_flInitialYaw + dy;
	}
}


void CTentacle :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	char *sound;

	switch( pEvent->event )
	{
	case 1:	// bang 
		{
			Vector vecSrc, vecAngles;
			GetAttachment( 0, vecSrc, vecAngles );

			// Vector vecSrc = pev->origin + m_flTapRadius * Vector( cos( pev->angles.y * (3.14192653 / 180.0) ), sin( pev->angles.y * (M_PI / 180.0) ), 0.0 );

			// vecSrc.z += MyHeight( );

			switch( m_iTapSound )
			{
			case TE_SILO:
				UTIL_EmitAmbientSound(ENT(pev), vecSrc, RANDOM_SOUND_ARRAY( pHitSilo ), 1.0, ATTN_NORM, 0, 100);
				break;
			case TE_NONE:
				break;
			case TE_DIRT:
				UTIL_EmitAmbientSound(ENT(pev), vecSrc, RANDOM_SOUND_ARRAY( pHitDirt ), 1.0, ATTN_NORM, 0, 100);
				break;
			case TE_WATER:
				UTIL_EmitAmbientSound(ENT(pev), vecSrc, RANDOM_SOUND_ARRAY( pHitWater ), 1.0, ATTN_NORM, 0, 100);
				break;
			}
          gpGlobals->force_retouch++;
		}
		break;

	case 3: // start killing swing
		m_iHitDmg = 200;
		// UTIL_EmitAmbientSound(ENT(pev), pev->origin + Vector( 0, 0, MyHeight()), "tentacle/te_swing1.wav", 1.0, ATTN_NORM, 0, 100);
		break;

	case 4: // end killing swing
		m_iHitDmg = 25;
		break;

	case 5: // just "whoosh" sound
		// UTIL_EmitAmbientSound(ENT(pev), pev->origin + Vector( 0, 0, MyHeight()), "tentacle/te_swing2.wav", 1.0, ATTN_NORM, 0, 100);
		break;

	case 2:	// tap scrape
	case 6: // light tap
		{
			Vector vecSrc = pev->origin + m_flTapRadius * Vector( cos( pev->angles.y * (M_PI / 180.0) ), sin( pev->angles.y * (M_PI / 180.0) ), 0.0 );

			vecSrc.z += MyHeight( );

			float flVol = RANDOM_FLOAT( 0.3, 0.5 );

			switch( m_iTapSound )
			{
			case TE_SILO:
				UTIL_EmitAmbientSound(ENT(pev), vecSrc, RANDOM_SOUND_ARRAY( pHitSilo ), flVol, ATTN_NORM, 0, 100);
				break;
			case TE_NONE:
				break;
			case TE_DIRT:
				UTIL_EmitAmbientSound(ENT(pev), vecSrc, RANDOM_SOUND_ARRAY( pHitDirt ), flVol, ATTN_NORM, 0, 100);
				break;
			case TE_WATER:
				UTIL_EmitAmbientSound(ENT(pev), vecSrc, RANDOM_SOUND_ARRAY( pHitWater ), flVol, ATTN_NORM, 0, 100);
				break;
			}
		}
		break;


	case 7: // roar
		switch( RANDOM_LONG(0,1) )
		{
		case 0: sound = "tentacle/te_roar1.wav"; break;
		case 1: sound = "tentacle/te_roar2.wav"; break;
		}

		UTIL_EmitAmbientSound(ENT(pev), pev->origin + Vector( 0, 0, MyHeight()), sound, 1.0, ATTN_NORM, 0, 100);
		break;

	case 8: // search
		switch( RANDOM_LONG(0,1) )
		{
		case 0: sound = "tentacle/te_search1.wav"; break;
		case 1: sound = "tentacle/te_search2.wav"; break;
		}

		UTIL_EmitAmbientSound(ENT(pev), pev->origin + Vector( 0, 0, MyHeight()), sound, 1.0, ATTN_NORM, 0, 100);
		break;

	case 9: // swing
		switch( RANDOM_LONG(0,1) )
		{
		case 0: sound = "tentacle/te_move1.wav"; break;
		case 1: sound = "tentacle/te_move2.wav"; break;
		}

		UTIL_EmitAmbientSound(ENT(pev), pev->origin + Vector( 0, 0, MyHeight()), sound, 1.0, ATTN_NORM, 0, 100);
		break;

	default:
		CBaseMonster::HandleAnimEvent( pEvent );
	}
}


//
// TentacleStart
//
// void CTentacle :: Start( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
void CTentacle :: Start( void )
{
	SetThink( &CTentacle::Cycle );

	if ( !g_fFlySound )
	{
		EMIT_SOUND (ENT(pev), CHAN_BODY, "ambience/flies.wav", 1, ATTN_NORM );
		g_fFlySound = TRUE;
//		pev->nextthink = gpGlobals-> time + 0.1;
	}
	else if ( !g_fSquirmSound )
	{
		EMIT_SOUND (ENT(pev), CHAN_BODY, "ambience/squirm2.wav", 1, ATTN_NORM );
		g_fSquirmSound = TRUE;
	}
	
	pev->nextthink = gpGlobals->time + 0.1;
}




void CTentacle :: HitTouch( CBaseEntity *pOther )
{
	TraceResult tr = UTIL_GetGlobalTrace( );

	if (pOther->pev->modelindex == pev->modelindex)
		return;

	if (m_flHitTime > gpGlobals->time)
		return;

	// only look at the ones where the player hit me
	if (tr.pHit == NULL || tr.pHit->v.modelindex != pev->modelindex)
		return;

	if (tr.iHitgroup >= 3)
	{
		pOther->TakeDamage( pev, pev, m_iHitDmg, DMG_CRUSH );
		// ALERT( at_console, "wack %3d : ", m_iHitDmg );
	}
	else if (tr.iHitgroup != 0)
	{
		pOther->TakeDamage( pev, pev, 20, DMG_CRUSH );
		// ALERT( at_console, "tap  %3d : ", 20 );
	}
	else
	{
		return; // Huh?
	}

	m_flHitTime = gpGlobals->time + 0.5;

	// ALERT( at_console, "%s : ", STRING( tr.pHit->v.classname ) );

	// ALERT( at_console, "%.0f : %s : %d\n", pev->angles.y, STRING( pOther->pev->classname ), tr.iHitgroup );
}


int CTentacle::TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType )
{
	if (flDamage > pev->health)
	{
		pev->health = 1;
	}
	else
	{
		pev->health -= flDamage;
	}
	return 1;
}




void CTentacle :: Killed( entvars_t *pevAttacker, int iGib )
{
	m_iGoalAnim = TENTACLE_ANIM_Pit_Idle;
	return;
}



class CTentacleMaw : public CBaseMonster
{
public:
	void Spawn( );
	void Precache( );
};

LINK_ENTITY_TO_CLASS( monster_tentaclemaw, CTentacleMaw );

//
// Tentacle Spawn
//
void CTentacleMaw :: Spawn( )
{
	Precache( );
	SET_MODEL(ENT(pev), "models/maw.mdl");
	UTIL_SetSize(pev, Vector(-32, -32, 0), Vector(32, 32, 64));

	pev->solid			= SOLID_NOT;
	pev->movetype		= MOVETYPE_STEP;
	pev->effects		= 0;
	pev->health			= 75;
	pev->yaw_speed		= 8;
	pev->sequence		= 0;
	
	pev->angles.x		= 90;
	// ResetSequenceInfo( );
}

void CTentacleMaw :: Precache( )
{
	PRECACHE_MODEL("models/maw.mdl");
}

#endif
