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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "effects.h"

#define N_SCALE		15
#define N_SPHERES	20

class CNihilanth : public CBaseMonster
{
public:
	int		Save( CSave &save ); 
	int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	void Spawn( void );
	void Precache( void );
	int  Classify( void ) { return CLASS_ALIEN_MILITARY; };
	int  BloodColor( void ) { return BLOOD_COLOR_YELLOW; }
	void Killed( entvars_t *pevAttacker, int iGib );
	void GibMonster( void );

	void SetObjectCollisionBox( void )
	{
		pev->absmin = pev->origin + Vector( -16 * N_SCALE, -16 * N_SCALE, -48 * N_SCALE );
		pev->absmax = pev->origin + Vector( 16 * N_SCALE, 16 * N_SCALE, 28 * N_SCALE );
	}

	void HandleAnimEvent( MonsterEvent_t *pEvent );

	void EXPORT StartupThink( void );
	void EXPORT HuntThink( void );
	void EXPORT CrashTouch( CBaseEntity *pOther );
	void EXPORT DyingThink( void );
	void EXPORT StartupUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT NullThink( void );
	void EXPORT CommandUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	void FloatSequence( void );
	void NextActivity( void );

	void Flight( void );

	BOOL AbsorbSphere( void );
	BOOL EmitSphere( void );
	void TargetSphere( USE_TYPE useType, float value );
	CBaseEntity *RandomTargetname( const char *szName );
	void ShootBalls( void );
	void MakeFriend( Vector vecPos );
	
	int  TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType );
	void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);

	void PainSound( void );
	void DeathSound( void );

	static const char *pAttackSounds[];	// vocalization: play sometimes when he launches an attack
	static const char *pBallSounds[];	// the sound of the lightening ball launch
	static const char *pShootSounds[];	// grunting vocalization: play sometimes when he launches an attack
	static const char *pRechargeSounds[];	// vocalization: play when he recharges
	static const char *pLaughSounds[];	// vocalization: play sometimes when hit and still has lots of health
	static const char *pPainSounds[];	// vocalization: play sometimes when hit and has much less health and no more chargers
	static const char *pDeathSounds[];	// vocalization: play as he dies
	
	// x_teleattack1.wav	the looping sound of the teleport attack ball.

	float m_flForce;

	float m_flNextPainSound;

	Vector m_velocity;
	Vector m_avelocity;

	Vector m_vecTarget;
	Vector m_posTarget;

	Vector m_vecDesired;
	Vector m_posDesired;

	float  m_flMinZ;
	float  m_flMaxZ;

	Vector m_vecGoal;

	float m_flLastSeen;
	float m_flPrevSeen;

	int m_irritation;

	int m_iLevel;
	int m_iTeleport;

	EHANDLE m_hRecharger;

	EHANDLE m_hSphere[N_SPHERES];
	int	m_iActiveSpheres;

	float m_flAdj;

	CSprite *m_pBall;

	char m_szRechargerTarget[64];
	char m_szDrawUse[64];
	char m_szTeleportUse[64];
	char m_szTeleportTouch[64];
	char m_szDeadUse[64];
	char m_szDeadTouch[64];

	float m_flShootEnd;
	float m_flShootTime;

	EHANDLE m_hFriend[3];
};

LINK_ENTITY_TO_CLASS( monster_nihilanth, CNihilanth );

TYPEDESCRIPTION	CNihilanth::m_SaveData[] = 
{
	DEFINE_FIELD( CNihilanth, m_flForce, FIELD_FLOAT ),
	DEFINE_FIELD( CNihilanth, m_flNextPainSound, FIELD_TIME ),
	DEFINE_FIELD( CNihilanth, m_velocity, FIELD_VECTOR ),
	DEFINE_FIELD( CNihilanth, m_avelocity, FIELD_VECTOR ),
	DEFINE_FIELD( CNihilanth, m_vecTarget, FIELD_VECTOR ),
	DEFINE_FIELD( CNihilanth, m_posTarget, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( CNihilanth, m_vecDesired, FIELD_VECTOR ),
	DEFINE_FIELD( CNihilanth, m_posDesired, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( CNihilanth, m_flMinZ, FIELD_FLOAT ),
	DEFINE_FIELD( CNihilanth, m_flMaxZ, FIELD_FLOAT ),
	DEFINE_FIELD( CNihilanth, m_vecGoal, FIELD_VECTOR ),
	DEFINE_FIELD( CNihilanth, m_flLastSeen, FIELD_TIME ),
	DEFINE_FIELD( CNihilanth, m_flPrevSeen, FIELD_TIME ),
	DEFINE_FIELD( CNihilanth, m_irritation, FIELD_INTEGER ),
	DEFINE_FIELD( CNihilanth, m_iLevel, FIELD_INTEGER ),
	DEFINE_FIELD( CNihilanth, m_iTeleport, FIELD_INTEGER ),
	DEFINE_FIELD( CNihilanth, m_hRecharger, FIELD_EHANDLE ),
	DEFINE_ARRAY( CNihilanth, m_hSphere, FIELD_EHANDLE, N_SPHERES ),
	DEFINE_FIELD( CNihilanth, m_iActiveSpheres, FIELD_INTEGER ),
	DEFINE_FIELD( CNihilanth, m_flAdj, FIELD_FLOAT ),
	DEFINE_FIELD( CNihilanth, m_pBall, FIELD_CLASSPTR ),
	DEFINE_ARRAY( CNihilanth, m_szRechargerTarget, FIELD_CHARACTER, 64 ),
	DEFINE_ARRAY( CNihilanth, m_szDrawUse, FIELD_CHARACTER, 64 ),
	DEFINE_ARRAY( CNihilanth, m_szTeleportUse, FIELD_CHARACTER, 64 ),
	DEFINE_ARRAY( CNihilanth, m_szTeleportTouch, FIELD_CHARACTER, 64 ),
	DEFINE_ARRAY( CNihilanth, m_szDeadUse, FIELD_CHARACTER, 64 ),
	DEFINE_ARRAY( CNihilanth, m_szDeadTouch, FIELD_CHARACTER, 64 ),
	DEFINE_FIELD( CNihilanth, m_flShootEnd, FIELD_TIME ),
	DEFINE_FIELD( CNihilanth, m_flShootTime, FIELD_TIME ),
	DEFINE_ARRAY( CNihilanth, m_hFriend, FIELD_EHANDLE, 3 ),
};

IMPLEMENT_SAVERESTORE( CNihilanth, CBaseMonster );

class CNihilanthHVR : public CBaseMonster
{
public:
	int		Save( CSave &save );
	int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	void Spawn( void );
	void Precache( void );

	void CircleInit( CBaseEntity *pTarget );
	void AbsorbInit( void );
	void TeleportInit( CNihilanth *pOwner, CBaseEntity *pEnemy, CBaseEntity *pTarget, CBaseEntity *pTouch );
	void GreenBallInit( void );
	void ZapInit( CBaseEntity *pEnemy );

	void EXPORT HoverThink( void );
	BOOL CircleTarget( Vector vecTarget );
	void EXPORT DissipateThink( void );

	void EXPORT ZapThink( void );
	void EXPORT TeleportThink( void );
	void EXPORT TeleportTouch( CBaseEntity *pOther );
	
	void EXPORT RemoveTouch( CBaseEntity *pOther );
	void EXPORT BounceTouch( CBaseEntity *pOther );
	void EXPORT ZapTouch( CBaseEntity *pOther );

	CBaseEntity *RandomClassname( const char *szName );

	// void EXPORT SphereUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	void MovetoTarget( Vector vecTarget );
	virtual void Crawl( void );

	void Zap( void );
	void Teleport( void );

	float m_flIdealVel;
	Vector m_vecIdeal;
	CNihilanth *m_pNihilanth;
	EHANDLE m_hTouch;
	int m_nFrames;
};

LINK_ENTITY_TO_CLASS( nihilanth_energy_ball, CNihilanthHVR );


TYPEDESCRIPTION	CNihilanthHVR::m_SaveData[] = 
{
	DEFINE_FIELD( CNihilanthHVR, m_flIdealVel, FIELD_FLOAT ),
	DEFINE_FIELD( CNihilanthHVR, m_vecIdeal, FIELD_VECTOR ),
	DEFINE_FIELD( CNihilanthHVR, m_pNihilanth, FIELD_CLASSPTR ),
	DEFINE_FIELD( CNihilanthHVR, m_hTouch, FIELD_EHANDLE ),
	DEFINE_FIELD( CNihilanthHVR, m_nFrames, FIELD_INTEGER ),
};


IMPLEMENT_SAVERESTORE( CNihilanthHVR, CBaseMonster );


//=========================================================
// Nihilanth, final Boss monster
//=========================================================

const char *CNihilanth::pAttackSounds[] = 
{
	"X/x_attack1.wav",
	"X/x_attack2.wav",
	"X/x_attack3.wav",
};

const char *CNihilanth::pBallSounds[] = 
{
	"X/x_ballattack1.wav",
};

const char *CNihilanth::pShootSounds[] = 
{
	"X/x_shoot1.wav",
};

const char *CNihilanth::pRechargeSounds[] = 
{
	"X/x_recharge1.wav",
	"X/x_recharge2.wav",
	"X/x_recharge3.wav",
};

const char *CNihilanth::pLaughSounds[] = 
{
	"X/x_laugh1.wav",
	"X/x_laugh2.wav",
};

const char *CNihilanth::pPainSounds[] = 
{
	"X/x_pain1.wav",
	"X/x_pain2.wav",
};

const char *CNihilanth::pDeathSounds[] = 
{
	"X/x_die1.wav",
};


void CNihilanth :: Spawn( void )
{
	Precache( );
	// motor
	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_BBOX;

	SET_MODEL(edict(), "models/nihilanth.mdl");
	// UTIL_SetSize(pev, Vector( -300, -300, 0), Vector(300, 300, 512));
	UTIL_SetSize(pev, Vector( -32, -32, 0), Vector(32, 32, 64));
	UTIL_SetOrigin( pev, pev->origin );

	pev->flags			|= FL_MONSTER;
	pev->takedamage		= DAMAGE_AIM;
	pev->health			= gSkillData.nihilanthHealth;
	pev->view_ofs		= Vector( 0, 0, 300 );

	m_flFieldOfView = -1; // 360 degrees

	pev->sequence = 0;
	ResetSequenceInfo( );

	InitBoneControllers();

	SetThink( &CNihilanth::StartupThink );
	pev->nextthink = gpGlobals->time + 0.1;

	m_vecDesired = Vector( 1, 0, 0 );
	m_posDesired = Vector( pev->origin.x, pev->origin.y, 512 );

	m_iLevel = 1; 
	m_iTeleport = 1;

	if (m_szRechargerTarget[0] == '\0')	strcpy( m_szRechargerTarget, "n_recharger" );
	if (m_szDrawUse[0] == '\0')			strcpy( m_szDrawUse, "n_draw" );
	if (m_szTeleportUse[0] == '\0')		strcpy( m_szTeleportUse, "n_leaving" );
	if (m_szTeleportTouch[0] == '\0')	strcpy( m_szTeleportTouch, "n_teleport" );
	if (m_szDeadUse[0] == '\0')			strcpy( m_szDeadUse, "n_dead" );
	if (m_szDeadTouch[0] == '\0')		strcpy( m_szDeadTouch, "n_ending" );

	// near death
	/*
	m_iTeleport = 10;
	m_iLevel = 10;
	m_irritation = 2;
	pev->health = 100;
	*/
}


void CNihilanth::Precache( void )
{
	PRECACHE_MODEL("models/nihilanth.mdl");
	PRECACHE_MODEL("sprites/lgtning.spr");
	UTIL_PrecacheOther( "nihilanth_energy_ball" );
	UTIL_PrecacheOther( "monster_alien_controller" );
	UTIL_PrecacheOther( "monster_alien_slave" );

	PRECACHE_SOUND_ARRAY( pAttackSounds );
	PRECACHE_SOUND_ARRAY( pBallSounds );
	PRECACHE_SOUND_ARRAY( pShootSounds );
	PRECACHE_SOUND_ARRAY( pRechargeSounds );
	PRECACHE_SOUND_ARRAY( pLaughSounds );
	PRECACHE_SOUND_ARRAY( pPainSounds );
	PRECACHE_SOUND_ARRAY( pDeathSounds );
	PRECACHE_SOUND("debris/beamstart7.wav");
}



void CNihilanth :: PainSound( void )
{
	if (m_flNextPainSound > gpGlobals->time)
		return;
	
	m_flNextPainSound = gpGlobals->time + RANDOM_FLOAT( 2, 5 );

	if (pev->health > gSkillData.nihilanthHealth / 2)
	{
		EMIT_SOUND( edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY( pLaughSounds ), 1.0, 0.2 ); 
	}
	else if (m_irritation >= 2)
	{
		EMIT_SOUND( edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY( pPainSounds ), 1.0, 0.2 ); 
	}
}	

void CNihilanth :: DeathSound( void )
{
	EMIT_SOUND( edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY( pDeathSounds ), 1.0, 0.1 ); 
}


void CNihilanth::NullThink( void )
{
	StudioFrameAdvance( );
	pev->nextthink = gpGlobals->time + 0.5;
}


void CNihilanth::StartupUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	SetThink( &CNihilanth::HuntThink );
	pev->nextthink = gpGlobals->time + 0.1;
	SetUse( &CNihilanth::CommandUse );
}


void CNihilanth::StartupThink( void )
{
	m_irritation = 0;
	m_flAdj = 512;

	CBaseEntity *pEntity;

	pEntity = UTIL_FindEntityByTargetname( NULL, "n_min");
	if (pEntity)
		m_flMinZ = pEntity->pev->origin.z;
	else
		m_flMinZ = -4096;

	pEntity = UTIL_FindEntityByTargetname( NULL, "n_max");
	if (pEntity)
		m_flMaxZ = pEntity->pev->origin.z;
	else
		m_flMaxZ = 4096;

	m_hRecharger = this;
	for (int i = 0; i < N_SPHERES; i++)
	{
		EmitSphere( );
	}
	m_hRecharger = NULL;

	SetThink( &CNihilanth::HuntThink);
	SetUse( &CNihilanth::CommandUse );
	pev->nextthink = gpGlobals->time + 0.1;
}


void CNihilanth :: Killed( entvars_t *pevAttacker, int iGib )
{
	CBaseMonster::Killed( pevAttacker, iGib );
}

void CNihilanth :: DyingThink( void )
{
	pev->nextthink = gpGlobals->time + 0.1;
	DispatchAnimEvents( );
	StudioFrameAdvance( );

	if (pev->deadflag == DEAD_NO)
	{
		DeathSound( );
		pev->deadflag = DEAD_DYING;

		m_posDesired.z = m_flMaxZ;
	}

	if (pev->deadflag == DEAD_DYING)
	{
		Flight( );

		if (fabs( pev->origin.z - m_flMaxZ ) < 16)
		{
			pev->velocity = Vector( 0, 0, 0 );
			FireTargets( m_szDeadUse, this, this, USE_ON, 1.0 );
			pev->deadflag = DEAD_DEAD;
		}
	}

	if (m_fSequenceFinished)
	{
		pev->avelocity.y += RANDOM_FLOAT( -100, 100 );
		if (pev->avelocity.y < -100)
			pev->avelocity.y = -100;
		if (pev->avelocity.y > 100)
			pev->avelocity.y = 100;

		pev->sequence = LookupSequence( "die1" );
	}

	if (m_pBall)
	{
		if (m_pBall->pev->renderamt > 0)
		{
			m_pBall->pev->renderamt = max<float>( 0, m_pBall->pev->renderamt - 2);
		}
		else
		{
			UTIL_Remove( m_pBall );
			m_pBall = NULL;
		}
	}

	Vector vecDir, vecSrc, vecAngles;

	UTIL_MakeAimVectors( pev->angles );
	int iAttachment = RANDOM_LONG( 1, 4 );

	do {
		vecDir = Vector( RANDOM_FLOAT( -1, 1 ), RANDOM_FLOAT( -1, 1 ), RANDOM_FLOAT( -1, 1 ));
	} while (DotProduct( vecDir, vecDir) > 1.0);

	switch( RANDOM_LONG( 1, 4 ))
	{
	case 1: // head
		vecDir.z = fabs( vecDir.z ) * 0.5;
		vecDir = vecDir + 2 * gpGlobals->v_up;
		break;
	case 2: // eyes
		if (DotProduct( vecDir, gpGlobals->v_forward ) < 0)
			vecDir = vecDir * -1;

		vecDir = vecDir + 2 * gpGlobals->v_forward;
		break;
	case 3: // left hand
		if (DotProduct( vecDir, gpGlobals->v_right ) > 0)
			vecDir = vecDir * -1;
		vecDir = vecDir - 2 * gpGlobals->v_right;
		break;
	case 4: // right hand
		if (DotProduct( vecDir, gpGlobals->v_right ) < 0)
			vecDir = vecDir * -1;
		vecDir = vecDir + 2 * gpGlobals->v_right;
		break;
	}

	GetAttachment( iAttachment - 1, vecSrc, vecAngles );

	TraceResult tr;

	UTIL_TraceLine( vecSrc, vecSrc + vecDir * 4096, ignore_monsters, ENT(pev), &tr );
	
	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_BEAMENTPOINT );
		WRITE_SHORT( entindex() + 0x1000 * iAttachment );
		WRITE_COORD( tr.vecEndPos.x);
		WRITE_COORD( tr.vecEndPos.y);
		WRITE_COORD( tr.vecEndPos.z);
		WRITE_SHORT( g_sModelIndexLaser );
		WRITE_BYTE( 0 ); // frame start
		WRITE_BYTE( 10 ); // framerate
		WRITE_BYTE( 5 ); // life
		WRITE_BYTE( 100 );  // width
		WRITE_BYTE( 120 );   // noise
		WRITE_BYTE( 64 );   // r, g, b
		WRITE_BYTE( 128 );   // r, g, b
		WRITE_BYTE( 255);   // r, g, b
		WRITE_BYTE( 255 );	// brightness
		WRITE_BYTE( 10 );		// speed
	MESSAGE_END();

	GetAttachment( 0, vecSrc, vecAngles ); 
	CNihilanthHVR *pEntity = (CNihilanthHVR *)Create( "nihilanth_energy_ball", vecSrc, pev->angles, edict() );
	pEntity->pev->velocity = Vector ( RANDOM_FLOAT( -0.7, 0.7 ), RANDOM_FLOAT( -0.7, 0.7 ), 1.0 ) * 600.0;
	pEntity->GreenBallInit( );

	return;
}



void CNihilanth::CrashTouch( CBaseEntity *pOther )
{
	// only crash if we hit something solid
	if ( pOther->pev->solid == SOLID_BSP) 
	{
		SetTouch( NULL );
		pev->nextthink = gpGlobals->time;
	}
}



void CNihilanth :: GibMonster( void )
{
	// EMIT_SOUND_DYN(edict(), CHAN_VOICE, "common/bodysplat.wav", 0.75, ATTN_NORM, 0, 200);		
}



void CNihilanth :: FloatSequence( void )
{
	if (m_irritation >= 2)
	{
		pev->sequence = LookupSequence( "float_open" );
	}
	else if (m_avelocity.y > 30)
	{
		pev->sequence = LookupSequence( "walk_r" );
	}
	else if (m_avelocity.y < -30)
	{
		pev->sequence = LookupSequence( "walk_l" );
	}
	else if (m_velocity.z > 30)
	{
		pev->sequence = LookupSequence( "walk_u" );
	} 
	else if (m_velocity.z < -30)
	{
		pev->sequence = LookupSequence( "walk_d" );
	}
	else
	{
		pev->sequence = LookupSequence( "float" );
	}
}


void CNihilanth :: ShootBalls( void )
{
	if (m_flShootEnd > gpGlobals->time)
	{
		Vector vecHand, vecAngle;
		
		while (m_flShootTime < m_flShootEnd && m_flShootTime < gpGlobals->time)
		{
			if (m_hEnemy != NULL)
			{
				Vector vecSrc, vecDir;
				CNihilanthHVR *pEntity;

				GetAttachment( 2, vecHand, vecAngle );
				vecSrc = vecHand + pev->velocity * (m_flShootTime - gpGlobals->time);
				// vecDir = (m_posTarget - vecSrc).Normalize( );
				vecDir = (m_posTarget - pev->origin).Normalize( );
				vecSrc = vecSrc + vecDir * (gpGlobals->time - m_flShootTime);
				pEntity = (CNihilanthHVR *)Create( "nihilanth_energy_ball", vecSrc, pev->angles, edict() );
				pEntity->pev->velocity = vecDir * 200.0; 
				pEntity->ZapInit( m_hEnemy );

				GetAttachment( 3, vecHand, vecAngle );
				vecSrc = vecHand + pev->velocity * (m_flShootTime - gpGlobals->time);
				// vecDir = (m_posTarget - vecSrc).Normalize( );
				vecDir = (m_posTarget - pev->origin).Normalize( );
				vecSrc = vecSrc + vecDir * (gpGlobals->time - m_flShootTime);
				pEntity = (CNihilanthHVR *)Create( "nihilanth_energy_ball", vecSrc, pev->angles, edict() );
				pEntity->pev->velocity = vecDir * 200.0; 
				pEntity->ZapInit( m_hEnemy );
			}
			m_flShootTime += 0.2;
		}
	}
}


void CNihilanth :: MakeFriend( Vector vecStart )
{
	int i;

	for (i = 0; i < 3; i++)
	{
		if (m_hFriend[i] != NULL && !m_hFriend[i]->IsAlive())
		{
			if (pev->rendermode == kRenderNormal) // don't do it if they are already fading
				m_hFriend[i]->MyMonsterPointer()->FadeMonster( );
			m_hFriend[i] = NULL;
		}

		if (m_hFriend[i] == NULL)
		{
			if (RANDOM_LONG(0, 1) == 0)
			{
				int iNode = WorldGraph.FindNearestNode ( vecStart, bits_NODE_AIR );
				if (iNode != NO_NODE)
				{
					CNode &node = WorldGraph.Node( iNode );
					TraceResult tr;
					UTIL_TraceHull( node.m_vecOrigin + Vector( 0, 0, 32 ), node.m_vecOrigin + Vector( 0, 0, 32 ), dont_ignore_monsters, large_hull, NULL, &tr );
					if (tr.fStartSolid == 0)
						m_hFriend[i] = Create("monster_alien_controller", node.m_vecOrigin, pev->angles );
				}
			}
			else
			{
				int iNode = WorldGraph.FindNearestNode ( vecStart, bits_NODE_LAND | bits_NODE_WATER );
				if (iNode != NO_NODE)
				{
					CNode &node = WorldGraph.Node( iNode );
					TraceResult tr;
					UTIL_TraceHull( node.m_vecOrigin + Vector( 0, 0, 36 ), node.m_vecOrigin + Vector( 0, 0, 36 ), dont_ignore_monsters, human_hull, NULL, &tr );
					if (tr.fStartSolid == 0)
						m_hFriend[i] = Create("monster_alien_slave", node.m_vecOrigin, pev->angles );
				}
			}
			if (m_hFriend[i] != NULL)
			{
				EMIT_SOUND( m_hFriend[i]->edict(), CHAN_WEAPON, "debris/beamstart7.wav", 1.0, ATTN_NORM );
			}

			return;
		}
	}
}


void CNihilanth :: NextActivity( )
{
	UTIL_MakeAimVectors( pev->angles );

	if (m_irritation >= 2)
	{
		if (m_pBall == NULL)
		{
			m_pBall = CSprite::SpriteCreate( "sprites/tele1.spr", pev->origin, TRUE );
			if (m_pBall)
			{
				m_pBall->SetTransparency( kRenderTransAdd, 255, 255, 255, 255, kRenderFxNoDissipation );
				m_pBall->SetAttachment( edict(), 1 );
				m_pBall->SetScale( 4.0 );
				m_pBall->pev->framerate = 10.0;
				m_pBall->TurnOn( );
			}
		}

		if (m_pBall)
		{
			MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
				WRITE_BYTE( TE_ELIGHT );
				WRITE_SHORT( entindex( ) + 0x1000 );		// entity, attachment
				WRITE_COORD( pev->origin.x );		// origin
				WRITE_COORD( pev->origin.y );
				WRITE_COORD( pev->origin.z );
				WRITE_COORD( 256 );	// radius
				WRITE_BYTE( 255 );	// R
				WRITE_BYTE( 192 );	// G
				WRITE_BYTE( 64 );	// B
				WRITE_BYTE( 200 );	// life * 10
				WRITE_COORD( 0 ); // decay
			MESSAGE_END();
		}
	}

	if ((pev->health < gSkillData.nihilanthHealth / 2 || m_iActiveSpheres < N_SPHERES / 2) && m_hRecharger == NULL && m_iLevel <= 9)
	{
		char szName[64];

		CBaseEntity *pEnt = NULL;
		CBaseEntity *pRecharger = NULL;
		float flDist = 8192;

		sprintf(szName, "%s%d", m_szRechargerTarget, m_iLevel );

		while ((pEnt = UTIL_FindEntityByTargetname( pEnt, szName )) != NULL)
		{
			float flLocal = (pEnt->pev->origin - pev->origin).Length();
			if (flLocal < flDist)
			{
				flDist = flLocal;
				pRecharger = pEnt;
			}	
		}
		
		if (pRecharger)
		{
			m_hRecharger = pRecharger;
			m_posDesired = Vector( pev->origin.x, pev->origin.y, pRecharger->pev->origin.z );
			m_vecDesired = (pRecharger->pev->origin - m_posDesired).Normalize( );
			m_vecDesired.z = 0;
			m_vecDesired = m_vecDesired.Normalize();
		}
		else
		{
			m_hRecharger = NULL;
			ALERT( at_aiconsole, "nihilanth can't find %s\n", szName );
			m_iLevel++;
			if (m_iLevel > 9)
				m_irritation = 2;
		}
	}

	float flDist = (m_posDesired - pev->origin).Length();
	float flDot = DotProduct( m_vecDesired, gpGlobals->v_forward );

	if (m_hRecharger != NULL)
	{
		// at we at power up yet?
		if (flDist < 128.0)
		{
			int iseq = LookupSequence( "recharge" );

			if (iseq != pev->sequence)
			{
				char szText[64];

				sprintf( szText, "%s%d", m_szDrawUse, m_iLevel );
				FireTargets( szText, this, this, USE_ON, 1.0 );

				ALERT( at_console, "fireing %s\n", szText );
			}
			pev->sequence = LookupSequence( "recharge" );
		}
		else
		{
			FloatSequence( );
		}
		return;
	}

	if (m_hEnemy != NULL && !m_hEnemy->IsAlive())
	{
		m_hEnemy = NULL;
	}

	if (m_flLastSeen + 15 < gpGlobals->time)
	{
		m_hEnemy = NULL;
	}

	if (m_hEnemy == NULL)
	{
		Look( 4096 );
		m_hEnemy = BestVisibleEnemy( );
	}

	if (m_hEnemy != NULL && m_irritation != 0)
	{
		if (m_flLastSeen + 5 > gpGlobals->time && flDist < 256 && flDot > 0)
		{
			if (m_irritation >= 2 && pev->health < gSkillData.nihilanthHealth / 2.0)
			{
				pev->sequence = LookupSequence( "attack1_open" );
			}
			else 
			{
				if (RANDOM_LONG(0, 1 ) == 0)
				{
					pev->sequence = LookupSequence( "attack1" ); // zap
				}
				else
				{
					char szText[64];

					sprintf( szText, "%s%d", m_szTeleportTouch, m_iTeleport );
					CBaseEntity *pTouch = UTIL_FindEntityByTargetname( NULL, szText );

					sprintf( szText, "%s%d", m_szTeleportUse, m_iTeleport );
					CBaseEntity *pTrigger = UTIL_FindEntityByTargetname( NULL, szText );

					if (pTrigger != NULL || pTouch != NULL)
					{
						pev->sequence = LookupSequence( "attack2" ); // teleport
					}
					else
					{
						m_iTeleport++;
						pev->sequence = LookupSequence( "attack1" ); // zap
					}
				}
			}
			return;
		}
	}

	FloatSequence( );		
}

void CNihilanth :: HuntThink( void )
{
	pev->nextthink = gpGlobals->time + 0.1;
	DispatchAnimEvents( );
	StudioFrameAdvance( );

	ShootBalls( );

	// if dead, force cancelation of current animation
	if (pev->health <= 0)
	{
		SetThink( &CNihilanth::DyingThink );
		m_fSequenceFinished = TRUE;
		return;
	}

	// ALERT( at_console, "health %.0f\n", pev->health );

	// if damaged, try to abosorb some spheres
	if (pev->health < gSkillData.nihilanthHealth && AbsorbSphere( ))
	{
		pev->health += gSkillData.nihilanthHealth / N_SPHERES;
	}

	// get new sequence
	if (m_fSequenceFinished)
	{
		// if (!m_fSequenceLoops)
		pev->frame = 0;
		NextActivity( );
		ResetSequenceInfo( );
		pev->framerate = 2.0 - 1.0 * (pev->health / gSkillData.nihilanthHealth);
	}

	// look for current enemy	
	if (m_hEnemy != NULL && m_hRecharger == NULL)
	{
		if (FVisible( m_hEnemy ))
		{
			if (m_flLastSeen < gpGlobals->time - 5)
				m_flPrevSeen = gpGlobals->time;
			m_flLastSeen = gpGlobals->time;
			m_posTarget = m_hEnemy->pev->origin;
			m_vecTarget = (m_posTarget - pev->origin).Normalize();
			m_vecDesired = m_vecTarget;
			m_posDesired = Vector( pev->origin.x, pev->origin.y, m_posTarget.z + m_flAdj );
		}
		else
		{
			m_flAdj = min( m_flAdj + 10.0f, 1000.0f );
		}
	}

	// don't go too high
	if (m_posDesired.z > m_flMaxZ)
		m_posDesired.z = m_flMaxZ;

	// don't go too low
	if (m_posDesired.z < m_flMinZ)
		m_posDesired.z = m_flMinZ;

	Flight( );
}



void CNihilanth :: Flight( void )
{
	// estimate where I'll be facing in one seconds
	UTIL_MakeAimVectors( pev->angles + m_avelocity );
	// Vector vecEst1 = pev->origin + m_velocity + gpGlobals->v_up * m_flForce - Vector( 0, 0, 384 );
	// float flSide = DotProduct( m_posDesired - vecEst1, gpGlobals->v_right );
	
	float flSide = DotProduct( m_vecDesired, gpGlobals->v_right );

	if (flSide < 0)
	{
		if (m_avelocity.y < 180)
		{
			m_avelocity.y += 6; // 9 * (3.0/2.0);
		}
	}
	else
	{
		if (m_avelocity.y > -180)
		{
			m_avelocity.y -= 6; // 9 * (3.0/2.0);
		}
	}
	m_avelocity.y *= 0.98;

	// estimate where I'll be in two seconds
	Vector vecEst = pev->origin + m_velocity * 2.0 + gpGlobals->v_up * m_flForce * 20;

	// add immediate force
	UTIL_MakeAimVectors( pev->angles );
	m_velocity.x += gpGlobals->v_up.x * m_flForce;
	m_velocity.y += gpGlobals->v_up.y * m_flForce;
	m_velocity.z += gpGlobals->v_up.z * m_flForce;


	float flSpeed = m_velocity.Length();
	float flDir = DotProduct( Vector( gpGlobals->v_forward.x, gpGlobals->v_forward.y, 0 ), Vector( m_velocity.x, m_velocity.y, 0 ) );
	if (flDir < 0)
		flSpeed = -flSpeed;

	float flDist = DotProduct( m_posDesired - vecEst, gpGlobals->v_forward );

	// sideways drag
	m_velocity.x = m_velocity.x * (1.0 - fabs( gpGlobals->v_right.x ) * 0.05);
	m_velocity.y = m_velocity.y * (1.0 - fabs( gpGlobals->v_right.y ) * 0.05);
	m_velocity.z = m_velocity.z * (1.0 - fabs( gpGlobals->v_right.z ) * 0.05);

	// general drag
	m_velocity = m_velocity * 0.995;
	
	// apply power to stay correct height
	if (m_flForce < 100 && vecEst.z < m_posDesired.z) 
	{
		m_flForce += 10;
	}
	else if (m_flForce > -100 && vecEst.z > m_posDesired.z)
	{
		if (vecEst.z > m_posDesired.z) 
			m_flForce -= 10;
	}

	UTIL_SetOrigin( pev, pev->origin + m_velocity * 0.1 );
	pev->angles = pev->angles + m_avelocity * 0.1;

	// ALERT( at_console, "%5.0f %5.0f : %4.0f : %3.0f : %2.0f\n", m_posDesired.z, pev->origin.z, m_velocity.z, m_avelocity.y, m_flForce ); 
}


BOOL CNihilanth :: AbsorbSphere( void )
{
	for (int i = 0; i < N_SPHERES; i++)
	{
		if (m_hSphere[i] != NULL)
		{
			CNihilanthHVR *pSphere = (CNihilanthHVR *)((CBaseEntity *)m_hSphere[i]);
			pSphere->AbsorbInit( );
			m_hSphere[i] = NULL;
			m_iActiveSpheres--;
			return TRUE;
		}
	}
	return FALSE;
}


BOOL CNihilanth :: EmitSphere( void )
{
	m_iActiveSpheres = 0;
	int empty = 0;

	for (int i = 0; i < N_SPHERES; i++)
	{
		if (m_hSphere[i] != NULL)
		{
			m_iActiveSpheres++;
		}
		else
		{
			empty = i;
		}
	}

	if (m_iActiveSpheres >= N_SPHERES)
		return FALSE;

	Vector vecSrc = m_hRecharger->pev->origin;
	CNihilanthHVR *pEntity = (CNihilanthHVR *)Create( "nihilanth_energy_ball", vecSrc, pev->angles, edict() );
	pEntity->pev->velocity = pev->origin - vecSrc;
	pEntity->CircleInit( this );

	m_hSphere[empty] = pEntity;
	return TRUE;
}


void CNihilanth :: 	TargetSphere( USE_TYPE useType, float value )
{
	CBaseMonster *pSphere;
	int i;
	for (i = 0; i < N_SPHERES; i++)
	{
		if (m_hSphere[i] != NULL)
		{
			pSphere = m_hSphere[i]->MyMonsterPointer();
			if (pSphere->m_hEnemy == NULL)
				break;
		}
	}
	if (i == N_SPHERES)
	{
		return;
	}

	Vector vecSrc, vecAngles;
	GetAttachment( 2, vecSrc, vecAngles ); 
	UTIL_SetOrigin( pSphere->pev, vecSrc );
	pSphere->Use( this, this, useType, value );
	pSphere->pev->velocity = m_vecDesired * RANDOM_FLOAT( 50, 100 ) + Vector( RANDOM_FLOAT( -50, 50 ), RANDOM_FLOAT( -50, 50 ), RANDOM_FLOAT( -50, 50 ) );
}



void CNihilanth :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
	case 1:	// shoot 
		break;
	case 2:	// zen
		if (m_hEnemy != NULL)
		{
			if (RANDOM_LONG(0,4) == 0)
				EMIT_SOUND( edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY( pAttackSounds ), 1.0, 0.2 ); 

			EMIT_SOUND( edict(), CHAN_WEAPON, RANDOM_SOUND_ARRAY( pBallSounds ), 1.0, 0.2 ); 

			MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
				WRITE_BYTE( TE_ELIGHT );
				WRITE_SHORT( entindex( ) + 0x3000 );		// entity, attachment
				WRITE_COORD( pev->origin.x );		// origin
				WRITE_COORD( pev->origin.y );
				WRITE_COORD( pev->origin.z );
				WRITE_COORD( 256 );	// radius
				WRITE_BYTE( 128 );	// R
				WRITE_BYTE( 128 );	// G
				WRITE_BYTE( 255 );	// B
				WRITE_BYTE( 10 );	// life * 10
				WRITE_COORD( 128 ); // decay
			MESSAGE_END();

			MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
				WRITE_BYTE( TE_ELIGHT );
				WRITE_SHORT( entindex( ) + 0x4000 );		// entity, attachment
				WRITE_COORD( pev->origin.x );		// origin
				WRITE_COORD( pev->origin.y );
				WRITE_COORD( pev->origin.z );
				WRITE_COORD( 256 );	// radius
				WRITE_BYTE( 128 );	// R
				WRITE_BYTE( 128 );	// G
				WRITE_BYTE( 255 );	// B
				WRITE_BYTE( 10 );	// life * 10
				WRITE_COORD( 128 ); // decay
			MESSAGE_END();
			
			m_flShootTime = gpGlobals->time;
			m_flShootEnd = gpGlobals->time + 1.0;
		}
		break;
	case 3:	// prayer
		if (m_hEnemy != NULL)
		{
			char szText[32];

			sprintf( szText, "%s%d", m_szTeleportTouch, m_iTeleport );
			CBaseEntity *pTouch = UTIL_FindEntityByTargetname( NULL, szText );

			sprintf( szText, "%s%d", m_szTeleportUse, m_iTeleport );
			CBaseEntity *pTrigger = UTIL_FindEntityByTargetname( NULL, szText );

			if (pTrigger != NULL || pTouch != NULL)
			{
				EMIT_SOUND( edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY( pAttackSounds ), 1.0, 0.2 ); 

				Vector vecSrc, vecAngles;
				GetAttachment( 2, vecSrc, vecAngles ); 
				CNihilanthHVR *pEntity = (CNihilanthHVR *)Create( "nihilanth_energy_ball", vecSrc, pev->angles, edict() );
				pEntity->pev->velocity = pev->origin - vecSrc;
				pEntity->TeleportInit( this, m_hEnemy, pTrigger, pTouch );
			}
			else
			{
				m_iTeleport++; // unexpected failure

				EMIT_SOUND( edict(), CHAN_WEAPON, RANDOM_SOUND_ARRAY( pBallSounds ), 1.0, 0.2 ); 

				ALERT( at_aiconsole, "nihilanth can't target %s\n", szText );

				MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
					WRITE_BYTE( TE_ELIGHT );
					WRITE_SHORT( entindex( ) + 0x3000 );		// entity, attachment
					WRITE_COORD( pev->origin.x );		// origin
					WRITE_COORD( pev->origin.y );
					WRITE_COORD( pev->origin.z );
					WRITE_COORD( 256 );	// radius
					WRITE_BYTE( 128 );	// R
					WRITE_BYTE( 128 );	// G
					WRITE_BYTE( 255 );	// B
					WRITE_BYTE( 10 );	// life * 10
					WRITE_COORD( 128 ); // decay
				MESSAGE_END();

				MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
					WRITE_BYTE( TE_ELIGHT );
					WRITE_SHORT( entindex( ) + 0x4000 );		// entity, attachment
					WRITE_COORD( pev->origin.x );		// origin
					WRITE_COORD( pev->origin.y );
					WRITE_COORD( pev->origin.z );
					WRITE_COORD( 256 );	// radius
					WRITE_BYTE( 128 );	// R
					WRITE_BYTE( 128 );	// G
					WRITE_BYTE( 255 );	// B
					WRITE_BYTE( 10 );	// life * 10
					WRITE_COORD( 128 ); // decay
				MESSAGE_END();

				m_flShootTime = gpGlobals->time;
				m_flShootEnd = gpGlobals->time + 1.0;
			}
		}
		break;
	case 4:	// get a sphere
		{
			if (m_hRecharger != NULL)
			{
				if (!EmitSphere( ))
				{
					m_hRecharger = NULL;
				}
			}
		}
		break;
	case 5:	// start up sphere machine
		{
			EMIT_SOUND( edict(), CHAN_VOICE , RANDOM_SOUND_ARRAY( pRechargeSounds ), 1.0, 0.2 ); 
		}
		break;
	case 6:
		if (m_hEnemy != NULL)
		{
			Vector vecSrc, vecAngles;
			GetAttachment( 2, vecSrc, vecAngles ); 
			CNihilanthHVR *pEntity = (CNihilanthHVR *)Create( "nihilanth_energy_ball", vecSrc, pev->angles, edict() );
			pEntity->pev->velocity = pev->origin - vecSrc;
			pEntity->ZapInit( m_hEnemy );
		}
		break;
	case 7:
		/*
		Vector vecSrc, vecAngles;
		GetAttachment( 0, vecSrc, vecAngles ); 
		CNihilanthHVR *pEntity = (CNihilanthHVR *)Create( "nihilanth_energy_ball", vecSrc, pev->angles, edict() );
		pEntity->pev->velocity = Vector ( RANDOM_FLOAT( -0.7, 0.7 ), RANDOM_FLOAT( -0.7, 0.7 ), 1.0 ) * 600.0;
		pEntity->GreenBallInit( );
		*/
		break;
	}
}



void CNihilanth::CommandUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	switch (useType)
	{
	case USE_OFF:
		{
			CBaseEntity *pTouch = UTIL_FindEntityByTargetname( NULL, m_szDeadTouch );

			if ( pTouch )
			{
				if ( m_hEnemy != NULL )
				{
					pTouch->Touch( m_hEnemy );
				}
				// if the player is using "notarget", the ending sequence won't fire unless we catch it here
				else
				{
					CBaseEntity *pEntity = UTIL_FindEntityByClassname( NULL, "player" );				
					if ( pEntity != NULL && pEntity->IsAlive() )
					{
						pTouch->Touch( pEntity );
					}
				}
			}
		}
		break;
	case USE_ON:
		if (m_irritation == 0)
		{
			m_irritation = 1;
		}
		break;
	case USE_SET:
		break;
	case USE_TOGGLE:
		break;
	}
}


int CNihilanth :: TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType )
{
	if (pevInflictor->owner == edict())
		return 0;

	if (flDamage >= pev->health)
	{
		pev->health = 1;
		if (m_irritation != 3)
			return 0;
	}
	
	PainSound( );

	pev->health -= flDamage;
	return 0;
}



void CNihilanth::TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
	if (m_irritation == 3)
		m_irritation = 2;

	if (m_irritation == 2 && ptr->iHitgroup == 2 && flDamage > 2)
		m_irritation = 3;

	if (m_irritation != 3)
	{
		Vector vecBlood = (ptr->vecEndPos - pev->origin).Normalize( );

		UTIL_BloodStream( ptr->vecEndPos, vecBlood, BloodColor(), flDamage + (100 - 100 * (pev->health / gSkillData.nihilanthHealth)));
	}

	// SpawnBlood(ptr->vecEndPos, BloodColor(), flDamage * 5.0);// a little surface blood.
	AddMultiDamage( pevAttacker, this, flDamage, bitsDamageType );
}



CBaseEntity *CNihilanth::RandomTargetname( const char *szName )
{
	int total = 0;

	CBaseEntity *pEntity = NULL;
	CBaseEntity *pNewEntity = NULL;
	while ((pNewEntity = UTIL_FindEntityByTargetname( pNewEntity, szName )) != NULL)
	{
		total++;
		if (RANDOM_LONG(0,total-1) < 1)
			pEntity = pNewEntity;
	}
	return pEntity;
}









//=========================================================
// Controller bouncy ball attack
//=========================================================



void CNihilanthHVR :: Spawn( void )
{
	Precache( );

	pev->rendermode = kRenderTransAdd;
	pev->renderamt = 255;
	pev->scale = 3.0;
}


void CNihilanthHVR :: Precache( void )
{
	PRECACHE_MODEL("sprites/flare6.spr");
	PRECACHE_MODEL("sprites/nhth1.spr");
	PRECACHE_MODEL("sprites/exit1.spr");
	PRECACHE_MODEL("sprites/tele1.spr");
	PRECACHE_MODEL("sprites/animglow01.spr");
	PRECACHE_MODEL("sprites/xspark4.spr");
	PRECACHE_MODEL("sprites/muzzleflash3.spr");
	PRECACHE_SOUND("debris/zap4.wav");
	PRECACHE_SOUND("weapons/electro4.wav");
	PRECACHE_SOUND("x/x_teleattack1.wav");
}



void CNihilanthHVR :: CircleInit( CBaseEntity *pTarget )
{
	pev->movetype = MOVETYPE_NOCLIP;
	pev->solid = SOLID_NOT;

	// SET_MODEL(edict(), "sprites/flare6.spr");
	// pev->scale = 3.0;
	// SET_MODEL(edict(), "sprites/xspark4.spr");
	SET_MODEL(edict(), "sprites/muzzleflash3.spr");
	pev->rendercolor.x = 255;
	pev->rendercolor.y = 224;
	pev->rendercolor.z = 192;
	pev->scale = 2.0;
	m_nFrames = 1;
	pev->renderamt = 255;

	UTIL_SetSize(pev, Vector( 0, 0, 0), Vector(0, 0, 0));
	UTIL_SetOrigin( pev, pev->origin );

	SetThink( &CNihilanthHVR::HoverThink );
	SetTouch( &CNihilanthHVR::BounceTouch );
	pev->nextthink = gpGlobals->time + 0.1;
	
	m_hTargetEnt = pTarget;
}


CBaseEntity *CNihilanthHVR::RandomClassname( const char *szName )
{
	int total = 0;

	CBaseEntity *pEntity = NULL;
	CBaseEntity *pNewEntity = NULL;
	while ((pNewEntity = UTIL_FindEntityByClassname( pNewEntity, szName )) != NULL)
	{
		total++;
		if (RANDOM_LONG(0,total-1) < 1)
			pEntity = pNewEntity;
	}
	return pEntity;
}

void CNihilanthHVR :: HoverThink( void  )
{
	pev->nextthink = gpGlobals->time + 0.1;

	if (m_hTargetEnt != NULL)
	{
		CircleTarget( m_hTargetEnt->pev->origin + Vector( 0, 0, 16 * N_SCALE ) );
	}
	else
	{
		UTIL_Remove( this );
	}


	if (RANDOM_LONG( 0, 99 ) < 5)
	{
/*
		CBaseEntity *pOther = RandomClassname( STRING(pev->classname) );

		if (pOther && pOther != this)
		{
			MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
				WRITE_BYTE( TE_BEAMENTS );
				WRITE_SHORT( this->entindex() );
				WRITE_SHORT( pOther->entindex() );
				WRITE_SHORT( g_sModelIndexLaser );
				WRITE_BYTE( 0 ); // framestart
				WRITE_BYTE( 0 ); // framerate
				WRITE_BYTE( 10 ); // life
				WRITE_BYTE( 80 );  // width
				WRITE_BYTE( 80 );   // noise
				WRITE_BYTE( 255 );   // r, g, b
				WRITE_BYTE( 128 );   // r, g, b
				WRITE_BYTE( 64 );   // r, g, b
				WRITE_BYTE( 255 );	// brightness
				WRITE_BYTE( 30 );		// speed
			MESSAGE_END();
		}
*/
/*
		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE( TE_BEAMENTS );
			WRITE_SHORT( this->entindex() );
			WRITE_SHORT( m_hTargetEnt->entindex() + 0x1000 );
			WRITE_SHORT( g_sModelIndexLaser );
			WRITE_BYTE( 0 ); // framestart
			WRITE_BYTE( 0 ); // framerate
			WRITE_BYTE( 10 ); // life
			WRITE_BYTE( 80 );  // width
			WRITE_BYTE( 80 );   // noise
			WRITE_BYTE( 255 );   // r, g, b
			WRITE_BYTE( 128 );   // r, g, b
			WRITE_BYTE( 64 );   // r, g, b
			WRITE_BYTE( 255 );	// brightness
			WRITE_BYTE( 30 );		// speed
		MESSAGE_END();
*/
	}

	pev->frame = ((int)pev->frame + 1) % m_nFrames;
}




void CNihilanthHVR :: ZapInit( CBaseEntity *pEnemy )
{
	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_BBOX;

	SET_MODEL(edict(), "sprites/nhth1.spr");

	pev->rendercolor.x = 255;
	pev->rendercolor.y = 255;
	pev->rendercolor.z = 255;
	pev->scale = 2.0;

	pev->velocity = (pEnemy->pev->origin - pev->origin).Normalize() * 200;

	m_hEnemy = pEnemy;
	SetThink( &CNihilanthHVR::ZapThink );
	SetTouch( &CNihilanthHVR::ZapTouch );
	pev->nextthink = gpGlobals->time + 0.1;

	EMIT_SOUND_DYN( edict(), CHAN_WEAPON, "debris/zap4.wav", 1, ATTN_NORM, 0, 100 );
}

void CNihilanthHVR :: ZapThink( void  )
{
	pev->nextthink = gpGlobals->time + 0.05;

	// check world boundaries
	if (m_hEnemy == NULL ||  pev->origin.x < -4096 || pev->origin.x > 4096 || pev->origin.y < -4096 || pev->origin.y > 4096 || pev->origin.z < -4096 || pev->origin.z > 4096)
	{
		SetTouch( NULL );
		UTIL_Remove( this );
		return;
	}

	if (pev->velocity.Length() < 2000)
	{
		pev->velocity = pev->velocity * 1.2;
	}


	// MovetoTarget( m_hEnemy->Center( ) );

	if ((m_hEnemy->Center() - pev->origin).Length() < 256)
	{
		TraceResult tr;

		UTIL_TraceLine( pev->origin, m_hEnemy->Center(), dont_ignore_monsters, edict(), &tr );

		CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);
		if (pEntity != NULL && pEntity->pev->takedamage)
		{
			ClearMultiDamage( );
			pEntity->TraceAttack( pev, gSkillData.nihilanthZap, pev->velocity, &tr, DMG_SHOCK );
			ApplyMultiDamage( pev, pev );
		}

		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE( TE_BEAMENTPOINT );
			WRITE_SHORT( entindex() );
			WRITE_COORD( tr.vecEndPos.x );
			WRITE_COORD( tr.vecEndPos.y );
			WRITE_COORD( tr.vecEndPos.z );
			WRITE_SHORT( g_sModelIndexLaser );
			WRITE_BYTE( 0 ); // frame start
			WRITE_BYTE( 10 ); // framerate
			WRITE_BYTE( 3 ); // life
			WRITE_BYTE( 20 );  // width
			WRITE_BYTE( 20 );   // noise
			WRITE_BYTE( 64 );   // r, g, b
			WRITE_BYTE( 196 );   // r, g, b
			WRITE_BYTE( 255);   // r, g, b
			WRITE_BYTE( 255 );	// brightness
			WRITE_BYTE( 10 );		// speed
		MESSAGE_END();

		UTIL_EmitAmbientSound( edict(), tr.vecEndPos, "weapons/electro4.wav", 0.5, ATTN_NORM, 0, RANDOM_LONG( 140, 160 ) );

		SetTouch( NULL );
		UTIL_Remove( this );
		pev->nextthink = gpGlobals->time + 0.2;
		return;
	}

	pev->frame = (int)(pev->frame + 1) % 11;

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_ELIGHT );
		WRITE_SHORT( entindex( ) );		// entity, attachment
		WRITE_COORD( pev->origin.x );		// origin
		WRITE_COORD( pev->origin.y );
		WRITE_COORD( pev->origin.z );
		WRITE_COORD( 128 );	// radius
		WRITE_BYTE( 128 );	// R
		WRITE_BYTE( 128 );	// G
		WRITE_BYTE( 255 );	// B
		WRITE_BYTE( 10 );	// life * 10
		WRITE_COORD( 128 ); // decay
	MESSAGE_END();

	// Crawl( );
}


void CNihilanthHVR::ZapTouch( CBaseEntity *pOther )
{
	UTIL_EmitAmbientSound( edict(), pev->origin, "weapons/electro4.wav", 1.0, ATTN_NORM, 0, RANDOM_LONG( 90, 95 ) );

	RadiusDamage( pev, pev, 50, CLASS_NONE, DMG_SHOCK );
	pev->velocity = pev->velocity * 0;

	/*
	for (int i = 0; i < 10; i++)
	{
		Crawl( );
	}
	*/

	SetTouch( NULL );
	UTIL_Remove( this );
	pev->nextthink = gpGlobals->time + 0.2;
}



void CNihilanthHVR :: TeleportInit( CNihilanth *pOwner, CBaseEntity *pEnemy, CBaseEntity *pTarget, CBaseEntity *pTouch )
{
	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_BBOX;

	pev->rendercolor.x = 255;
	pev->rendercolor.y = 255;
	pev->rendercolor.z = 255;
	pev->velocity.z *= 0.2;

	SET_MODEL(edict(), "sprites/exit1.spr");

	m_pNihilanth = pOwner;
	m_hEnemy = pEnemy;
	m_hTargetEnt = pTarget;
	m_hTouch = pTouch;

	SetThink( &CNihilanthHVR::TeleportThink );
	SetTouch( &CNihilanthHVR::TeleportTouch );
	pev->nextthink = gpGlobals->time + 0.1;

	EMIT_SOUND_DYN( edict(), CHAN_WEAPON, "x/x_teleattack1.wav", 1, 0.2, 0, 100 );
}


void CNihilanthHVR :: GreenBallInit( )
{
	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_BBOX;

	pev->rendercolor.x = 255;
	pev->rendercolor.y = 255;
	pev->rendercolor.z = 255;
	pev->scale = 1.0;

	SET_MODEL(edict(), "sprites/exit1.spr");

	SetTouch( &CNihilanthHVR::RemoveTouch );
}


void CNihilanthHVR :: TeleportThink( void  )
{
	pev->nextthink = gpGlobals->time + 0.1;

	// check world boundaries
	if (m_hEnemy == NULL || !m_hEnemy->IsAlive() || pev->origin.x < -4096 || pev->origin.x > 4096 || pev->origin.y < -4096 || pev->origin.y > 4096 || pev->origin.z < -4096 || pev->origin.z > 4096)
	{
		STOP_SOUND(edict(), CHAN_WEAPON, "x/x_teleattack1.wav" );
		UTIL_Remove( this );
		return;
	}

	if ((m_hEnemy->Center() - pev->origin).Length() < 128)
	{
		STOP_SOUND(edict(), CHAN_WEAPON, "x/x_teleattack1.wav" );
		UTIL_Remove( this );

		if (m_hTargetEnt != NULL)
			m_hTargetEnt->Use( m_hEnemy, m_hEnemy, USE_ON, 1.0 );

		if ( m_hTouch != NULL && m_hEnemy != NULL )
			m_hTouch->Touch( m_hEnemy );
	}
	else 
	{
		MovetoTarget( m_hEnemy->Center( ) );
	}

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_ELIGHT );
		WRITE_SHORT( entindex( ) );		// entity, attachment
		WRITE_COORD( pev->origin.x );		// origin
		WRITE_COORD( pev->origin.y );
		WRITE_COORD( pev->origin.z );
		WRITE_COORD( 256 );	// radius
		WRITE_BYTE( 0 );	// R
		WRITE_BYTE( 255 );	// G
		WRITE_BYTE( 0 );	// B
		WRITE_BYTE( 10 );	// life * 10
		WRITE_COORD( 256 ); // decay
	MESSAGE_END();

	pev->frame = (int)(pev->frame + 1) % 20;
}


void CNihilanthHVR :: AbsorbInit( void  )
{
	SetThink( &CNihilanthHVR::DissipateThink );
	pev->renderamt = 255;

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_BEAMENTS );
		WRITE_SHORT( this->entindex() );
		WRITE_SHORT( m_hTargetEnt->entindex() + 0x1000 );
		WRITE_SHORT( g_sModelIndexLaser );
		WRITE_BYTE( 0 ); // framestart
		WRITE_BYTE( 0 ); // framerate
		WRITE_BYTE( 50 ); // life
		WRITE_BYTE( 80 );  // width
		WRITE_BYTE( 80 );   // noise
		WRITE_BYTE( 255 );   // r, g, b
		WRITE_BYTE( 128 );   // r, g, b
		WRITE_BYTE( 64 );   // r, g, b
		WRITE_BYTE( 255 );	// brightness
		WRITE_BYTE( 30 );		// speed
	MESSAGE_END();
}

void CNihilanthHVR::TeleportTouch( CBaseEntity *pOther )
{
	CBaseEntity *pEnemy = m_hEnemy;

	if (pOther == pEnemy)
	{
		if (m_hTargetEnt != NULL)
			m_hTargetEnt->Use( pEnemy, pEnemy, USE_ON, 1.0 );

		if (m_hTouch != NULL && pEnemy != NULL )
			m_hTouch->Touch( pEnemy );
	}
	else
	{
		m_pNihilanth->MakeFriend( pev->origin );
	}

	SetTouch( NULL );
	STOP_SOUND(edict(), CHAN_WEAPON, "x/x_teleattack1.wav" );
	UTIL_Remove( this );
}


void CNihilanthHVR :: DissipateThink( void  )
{
	pev->nextthink = gpGlobals->time + 0.1;

	if (pev->scale > 5.0)
		UTIL_Remove( this );

	pev->renderamt -= 2;
	pev->scale += 0.1;

	if (m_hTargetEnt != NULL)
	{
		CircleTarget( m_hTargetEnt->pev->origin + Vector( 0, 0, 4096 ) );
	}
	else
	{
		UTIL_Remove( this );
	}

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_ELIGHT );
		WRITE_SHORT( entindex( ) );		// entity, attachment
		WRITE_COORD( pev->origin.x );		// origin
		WRITE_COORD( pev->origin.y );
		WRITE_COORD( pev->origin.z );
		WRITE_COORD( pev->renderamt );	// radius
		WRITE_BYTE( 255 );	// R
		WRITE_BYTE( 192 );	// G
		WRITE_BYTE( 64 );	// B
		WRITE_BYTE( 2 );	// life * 10
		WRITE_COORD( 0 ); // decay
	MESSAGE_END();
}


BOOL CNihilanthHVR :: CircleTarget( Vector vecTarget )
{
	BOOL fClose = FALSE;

	Vector vecDest = vecTarget;
	Vector vecEst = pev->origin + pev->velocity * 0.5;
	Vector vecSrc = pev->origin;
	vecDest.z = 0;
	vecEst.z = 0;
	vecSrc.z = 0;
	float d1 = (vecDest - vecSrc).Length() - 24 * N_SCALE;
	float d2 = (vecDest - vecEst).Length() - 24 * N_SCALE;

	if (m_vecIdeal == Vector( 0, 0, 0 ))
	{
		m_vecIdeal = pev->velocity;
	}

	if (d1 < 0 && d2 <= d1)
	{
		// ALERT( at_console, "too close\n");
		m_vecIdeal = m_vecIdeal - (vecDest - vecSrc).Normalize() * 50;
	}
	else if (d1 > 0 && d2 >= d1)
	{
		// ALERT( at_console, "too far\n");
		m_vecIdeal = m_vecIdeal + (vecDest - vecSrc).Normalize() * 50;
	}
	pev->avelocity.z = d1 * 20;

	if (d1 < 32)
	{
		fClose = TRUE;
	}

	m_vecIdeal = m_vecIdeal + Vector( RANDOM_FLOAT( -2, 2 ), RANDOM_FLOAT( -2, 2 ), RANDOM_FLOAT( -2, 2 ));
	m_vecIdeal = Vector( m_vecIdeal.x, m_vecIdeal.y, 0 ).Normalize( ) * 200
		/* + Vector( -m_vecIdeal.y, m_vecIdeal.x, 0 ).Normalize( ) * 32 */
		+ Vector( 0, 0, m_vecIdeal.z );
	// m_vecIdeal = m_vecIdeal + Vector( -m_vecIdeal.y, m_vecIdeal.x, 0 ).Normalize( ) * 2;

	// move up/down
	d1 = vecTarget.z - pev->origin.z;
	if (d1 > 0 && m_vecIdeal.z < 200)
		m_vecIdeal.z += 20;
	else if (d1 < 0 && m_vecIdeal.z > -200)
		m_vecIdeal.z -= 20;

	pev->velocity = m_vecIdeal;

	// ALERT( at_console, "%.0f %.0f %.0f\n", m_vecIdeal.x, m_vecIdeal.y, m_vecIdeal.z );
	return fClose;
}


void CNihilanthHVR :: MovetoTarget( Vector vecTarget )
{
	if (m_vecIdeal == Vector( 0, 0, 0 ))
	{
		m_vecIdeal = pev->velocity;
	}

	// accelerate
	float flSpeed = m_vecIdeal.Length();
	if (flSpeed > 300)
	{
		m_vecIdeal = m_vecIdeal.Normalize( ) * 300;
	}
	m_vecIdeal = m_vecIdeal + (vecTarget - pev->origin).Normalize() * 300;
	pev->velocity = m_vecIdeal;
}




void CNihilanthHVR :: Crawl( void  )
{

	Vector vecAim = Vector( RANDOM_FLOAT( -1, 1 ), RANDOM_FLOAT( -1, 1 ), RANDOM_FLOAT( -1, 1 ) ).Normalize( );
	Vector vecPnt = pev->origin + pev->velocity * 0.2 + vecAim * 128;

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_BEAMENTPOINT );
		WRITE_SHORT( entindex() );
		WRITE_COORD( vecPnt.x);
		WRITE_COORD( vecPnt.y);
		WRITE_COORD( vecPnt.z);
		WRITE_SHORT( g_sModelIndexLaser );
		WRITE_BYTE( 0 ); // frame start
		WRITE_BYTE( 10 ); // framerate
		WRITE_BYTE( 3 ); // life
		WRITE_BYTE( 20 );  // width
		WRITE_BYTE( 80 );   // noise
		WRITE_BYTE( 64 );   // r, g, b
		WRITE_BYTE( 128 );   // r, g, b
		WRITE_BYTE( 255);   // r, g, b
		WRITE_BYTE( 255 );	// brightness
		WRITE_BYTE( 10 );		// speed
	MESSAGE_END();
}


void CNihilanthHVR::RemoveTouch( CBaseEntity *pOther )
{
	STOP_SOUND(edict(), CHAN_WEAPON, "x/x_teleattack1.wav" );
	UTIL_Remove( this );
}

void CNihilanthHVR::BounceTouch( CBaseEntity *pOther )
{
	Vector vecDir = m_vecIdeal.Normalize( );

	TraceResult tr = UTIL_GetGlobalTrace( );

	float n = -DotProduct(tr.vecPlaneNormal, vecDir);

	vecDir = 2.0 * tr.vecPlaneNormal * n + vecDir;

	m_vecIdeal = vecDir * m_vecIdeal.Length();
}



#endif
