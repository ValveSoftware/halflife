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
#if !defined( OEM_BUILD ) && !defined( HLDEMO_BUILD )

//=========================================================
// hassassin - Human assassin, fast and stealthy
//=========================================================

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"monsters.h"
#include	"schedule.h"
#include	"squadmonster.h"
#include	"weapons.h"
#include	"soundent.h"
#include	"game.h"

extern DLL_GLOBAL int  g_iSkillLevel;

//=========================================================
// monster-specific schedule types
//=========================================================
enum
{
	SCHED_ASSASSIN_EXPOSED = LAST_COMMON_SCHEDULE + 1,// cover was blown.
	SCHED_ASSASSIN_JUMP,	// fly through the air
	SCHED_ASSASSIN_JUMP_ATTACK,	// fly through the air and shoot
	SCHED_ASSASSIN_JUMP_LAND, // hit and run away
};

//=========================================================
// monster-specific tasks
//=========================================================

enum
{
	TASK_ASSASSIN_FALL_TO_GROUND = LAST_COMMON_TASK + 1, // falling and waiting to hit ground
};


//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define		ASSASSIN_AE_SHOOT1	1
#define		ASSASSIN_AE_TOSS1	2
#define		ASSASSIN_AE_JUMP	3


#define bits_MEMORY_BADJUMP		(bits_MEMORY_CUSTOM1)

class CHAssassin : public CBaseMonster
{
public:
	void Spawn( void );
	void Precache( void );
	void SetYawSpeed ( void );
	int  Classify ( void );
	int  ISoundMask ( void);
	void Shoot( void );
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	Schedule_t* GetSchedule ( void );
	Schedule_t* GetScheduleOfType ( int Type );
	BOOL CheckMeleeAttack1 ( float flDot, float flDist );	// jump
	// BOOL CheckMeleeAttack2 ( float flDot, float flDist );
	BOOL CheckRangeAttack1 ( float flDot, float flDist );	// shoot
	BOOL CheckRangeAttack2 ( float flDot, float flDist );	// throw grenade
	void StartTask ( Task_t *pTask );
	void RunAI( void );
	void RunTask ( Task_t *pTask );
	void DeathSound ( void );
	void IdleSound ( void );
	CUSTOM_SCHEDULES;

	int	Save( CSave &save ); 
	int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	float m_flLastShot;
	float m_flDiviation;

	float m_flNextJump;
	Vector m_vecJumpVelocity;

	float m_flNextGrenadeCheck;
	Vector	m_vecTossVelocity;
	BOOL	m_fThrowGrenade;

	int		m_iTargetRanderamt;

	int		m_iFrustration;

	int		m_iShell;
};
LINK_ENTITY_TO_CLASS( monster_human_assassin, CHAssassin );


TYPEDESCRIPTION	CHAssassin::m_SaveData[] = 
{
	DEFINE_FIELD( CHAssassin, m_flLastShot, FIELD_TIME ),
	DEFINE_FIELD( CHAssassin, m_flDiviation, FIELD_FLOAT ),

	DEFINE_FIELD( CHAssassin, m_flNextJump, FIELD_TIME ),
	DEFINE_FIELD( CHAssassin, m_vecJumpVelocity, FIELD_VECTOR ),

	DEFINE_FIELD( CHAssassin, m_flNextGrenadeCheck, FIELD_TIME ),
	DEFINE_FIELD( CHAssassin, m_vecTossVelocity, FIELD_VECTOR ),
	DEFINE_FIELD( CHAssassin, m_fThrowGrenade, FIELD_BOOLEAN ),

	DEFINE_FIELD( CHAssassin, m_iTargetRanderamt, FIELD_INTEGER ),
	DEFINE_FIELD( CHAssassin, m_iFrustration, FIELD_INTEGER ),
};

IMPLEMENT_SAVERESTORE( CHAssassin, CBaseMonster );


//=========================================================
// DieSound
//=========================================================
void CHAssassin :: DeathSound ( void )
{
}

//=========================================================
// IdleSound
//=========================================================
void CHAssassin :: IdleSound ( void )
{
}

//=========================================================
// ISoundMask - returns a bit mask indicating which types
// of sounds this monster regards. 
//=========================================================
int CHAssassin :: ISoundMask ( void) 
{
	return	bits_SOUND_WORLD	|
			bits_SOUND_COMBAT	|
			bits_SOUND_DANGER	|
			bits_SOUND_PLAYER;
}


//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int	CHAssassin :: Classify ( void )
{
	return	CLASS_HUMAN_MILITARY;
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CHAssassin :: SetYawSpeed ( void )
{
	int ys;

	switch ( m_Activity )
	{
	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:
		ys = 360;
		break;
	default:			
		ys = 360;
		break;
	}

	pev->yaw_speed = ys;
}


//=========================================================
// Shoot
//=========================================================
void CHAssassin :: Shoot ( void )
{
	if (m_hEnemy == NULL)
	{
		return;
	}

	Vector vecShootOrigin = GetGunPosition();
	Vector vecShootDir = ShootAtEnemy( vecShootOrigin );

	if (m_flLastShot + 2 < gpGlobals->time)
	{
		m_flDiviation = 0.10;
	}
	else
	{
		m_flDiviation -= 0.01;
		if (m_flDiviation < 0.02)
			m_flDiviation = 0.02;
	}
	m_flLastShot = gpGlobals->time;

	UTIL_MakeVectors ( pev->angles );

	Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
	EjectBrass ( pev->origin + gpGlobals->v_up * 32 + gpGlobals->v_forward * 12, vecShellVelocity, pev->angles.y, m_iShell, TE_BOUNCE_SHELL); 
	FireBullets(1, vecShootOrigin, vecShootDir, Vector( m_flDiviation, m_flDiviation, m_flDiviation ), 2048, BULLET_MONSTER_9MM ); // shoot +-8 degrees

	switch(RANDOM_LONG(0,1))
	{
	case 0:
		EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/pl_gun1.wav", RANDOM_FLOAT(0.6, 0.8), ATTN_NORM);
		break;
	case 1:
		EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/pl_gun2.wav", RANDOM_FLOAT(0.6, 0.8), ATTN_NORM);
		break;
	}

	pev->effects |= EF_MUZZLEFLASH;

	Vector angDir = UTIL_VecToAngles( vecShootDir );
	SetBlending( 0, angDir.x );

	m_cAmmoLoaded--;
}


//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//
// Returns number of events handled, 0 if none.
//=========================================================
void CHAssassin :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
	case ASSASSIN_AE_SHOOT1:
		Shoot( );
		break;
	case ASSASSIN_AE_TOSS1:
		{
			UTIL_MakeVectors( pev->angles );
			CGrenade::ShootTimed( pev, pev->origin + gpGlobals->v_forward * 34 + Vector (0, 0, 32), m_vecTossVelocity, 2.0 );

			m_flNextGrenadeCheck = gpGlobals->time + 6;// wait six seconds before even looking again to see if a grenade can be thrown.
			m_fThrowGrenade = FALSE;
			// !!!LATER - when in a group, only try to throw grenade if ordered.
		}
		break;
	case ASSASSIN_AE_JUMP:
		{
			// ALERT( at_console, "jumping");
			UTIL_MakeAimVectors( pev->angles );
			pev->movetype = MOVETYPE_TOSS;
			pev->flags &= ~FL_ONGROUND;
			pev->velocity = m_vecJumpVelocity;
			m_flNextJump = gpGlobals->time + 3.0;
		}
		return;
	default:
		CBaseMonster::HandleAnimEvent( pEvent );
		break;
	}
}

//=========================================================
// Spawn
//=========================================================
void CHAssassin :: Spawn()
{
	Precache( );

	SET_MODEL(ENT(pev), "models/hassassin.mdl");
	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	pev->effects		= 0;
	pev->health			= gSkillData.hassassinHealth;
	m_flFieldOfView		= VIEW_FIELD_WIDE; // indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;
	m_afCapability		= bits_CAP_MELEE_ATTACK1 | bits_CAP_DOORS_GROUP;
	pev->friction		= 1;

	m_HackedGunPos		= Vector( 0, 24, 48 );

	m_iTargetRanderamt	= 20;
	pev->renderamt		= 20;
	pev->rendermode		= kRenderTransTexture;

	MonsterInit();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CHAssassin :: Precache()
{
	PRECACHE_MODEL("models/hassassin.mdl");

	PRECACHE_SOUND("weapons/pl_gun1.wav");
	PRECACHE_SOUND("weapons/pl_gun2.wav");

	PRECACHE_SOUND("debris/beamstart1.wav");

	m_iShell = PRECACHE_MODEL ("models/shell.mdl");// brass shell
}	
	


//=========================================================
// AI Schedules Specific to this monster
//=========================================================

//=========================================================
// Fail Schedule
//=========================================================
Task_t	tlAssassinFail[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT_FACE_ENEMY,		(float)2		},
	// { TASK_WAIT_PVS,			(float)0		},
	{ TASK_SET_SCHEDULE,		(float)SCHED_CHASE_ENEMY },
};

Schedule_t	slAssassinFail[] =
{
	{
		tlAssassinFail,
		ARRAYSIZE ( tlAssassinFail ),
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_PROVOKED			|
		bits_COND_CAN_RANGE_ATTACK1 |
		bits_COND_CAN_RANGE_ATTACK2 |
		bits_COND_CAN_MELEE_ATTACK1 |
		bits_COND_HEAR_SOUND,
	
		bits_SOUND_DANGER |
		bits_SOUND_PLAYER,
		"AssassinFail"
	},
};


//=========================================================
// Enemy exposed Agrunt's cover
//=========================================================
Task_t	tlAssassinExposed[] =
{
	{ TASK_STOP_MOVING,			(float)0							},
	{ TASK_RANGE_ATTACK1,		(float)0							},
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_ASSASSIN_JUMP			},
	{ TASK_SET_SCHEDULE,		(float)SCHED_TAKE_COVER_FROM_ENEMY	},
};

Schedule_t slAssassinExposed[] =
{
	{
		tlAssassinExposed,
		ARRAYSIZE ( tlAssassinExposed ),
		bits_COND_CAN_MELEE_ATTACK1,
		0,
		"AssassinExposed",
	},
};


//=========================================================
// Take cover from enemy! Tries lateral cover before node 
// cover! 
//=========================================================
Task_t	tlAssassinTakeCoverFromEnemy[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_WAIT,					(float)0.2					},
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_RANGE_ATTACK1	},
	{ TASK_FIND_COVER_FROM_ENEMY,	(float)0					},
	{ TASK_RUN_PATH,				(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0					},
	{ TASK_REMEMBER,				(float)bits_MEMORY_INCOVER	},
	{ TASK_FACE_ENEMY,				(float)0					},
};

Schedule_t	slAssassinTakeCoverFromEnemy[] =
{
	{ 
		tlAssassinTakeCoverFromEnemy,
		ARRAYSIZE ( tlAssassinTakeCoverFromEnemy ), 
		bits_COND_NEW_ENEMY |
		bits_COND_CAN_MELEE_ATTACK1		|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"AssassinTakeCoverFromEnemy"
	},
};


//=========================================================
// Take cover from enemy! Tries lateral cover before node 
// cover! 
//=========================================================
Task_t	tlAssassinTakeCoverFromEnemy2[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_WAIT,					(float)0.2					},
	{ TASK_FACE_ENEMY,				(float)0					},
	{ TASK_RANGE_ATTACK1,			(float)0					},
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_RANGE_ATTACK2	},
	{ TASK_FIND_FAR_NODE_COVER_FROM_ENEMY,	(float)384			},
	{ TASK_RUN_PATH,				(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0					},
	{ TASK_REMEMBER,				(float)bits_MEMORY_INCOVER	},
	{ TASK_FACE_ENEMY,				(float)0					},
};

Schedule_t	slAssassinTakeCoverFromEnemy2[] =
{
	{ 
		tlAssassinTakeCoverFromEnemy2,
		ARRAYSIZE ( tlAssassinTakeCoverFromEnemy2 ), 
		bits_COND_NEW_ENEMY |
		bits_COND_CAN_MELEE_ATTACK2		|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"AssassinTakeCoverFromEnemy2"
	},
};


//=========================================================
// hide from the loudest sound source
//=========================================================
Task_t	tlAssassinTakeCoverFromBestSound[] =
{
	{ TASK_SET_FAIL_SCHEDULE,			(float)SCHED_MELEE_ATTACK1	},
	{ TASK_STOP_MOVING,					(float)0					},
	{ TASK_FIND_COVER_FROM_BEST_SOUND,	(float)0					},
	{ TASK_RUN_PATH,					(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,			(float)0					},
	{ TASK_REMEMBER,					(float)bits_MEMORY_INCOVER	},
	{ TASK_TURN_LEFT,					(float)179					},
};

Schedule_t	slAssassinTakeCoverFromBestSound[] =
{
	{ 
		tlAssassinTakeCoverFromBestSound,
		ARRAYSIZE ( tlAssassinTakeCoverFromBestSound ), 
		bits_COND_NEW_ENEMY,
		0,
		"AssassinTakeCoverFromBestSound"
	},
};





//=========================================================
// AlertIdle Schedules
//=========================================================
Task_t	tlAssassinHide[] =
{
	{ TASK_STOP_MOVING,			0						 },
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE			 },
	{ TASK_WAIT,				(float)2				 },
	{ TASK_SET_SCHEDULE,		(float)SCHED_CHASE_ENEMY },
};

Schedule_t	slAssassinHide[] =
{
	{ 
		tlAssassinHide,
		ARRAYSIZE ( tlAssassinHide ), 
		bits_COND_NEW_ENEMY				|
		bits_COND_SEE_ENEMY				|
		bits_COND_SEE_FEAR				|
		bits_COND_LIGHT_DAMAGE			|
		bits_COND_HEAVY_DAMAGE			|
		bits_COND_PROVOKED		|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"AssassinHide"
	},
};



//=========================================================
// HUNT Schedules
//=========================================================
Task_t tlAssassinHunt[] = 
{
	{ TASK_GET_PATH_TO_ENEMY,	(float)0		},
	{ TASK_RUN_PATH,			(float)0		},
	{ TASK_WAIT_FOR_MOVEMENT,	(float)0		},
};

Schedule_t slAssassinHunt[] =
{
	{ 
		tlAssassinHunt,
		ARRAYSIZE ( tlAssassinHunt ),
		bits_COND_NEW_ENEMY			|
		// bits_COND_SEE_ENEMY			|
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"AssassinHunt"
	},
};


//=========================================================
// Jumping Schedules
//=========================================================
Task_t	tlAssassinJump[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_HOP	},
	{ TASK_SET_SCHEDULE,		(float)SCHED_ASSASSIN_JUMP_ATTACK },
};

Schedule_t	slAssassinJump[] =
{
	{ 
		tlAssassinJump,
		ARRAYSIZE ( tlAssassinJump ), 
		0, 
		0, 
		"AssassinJump"
	},
};


//=========================================================
// repel 
//=========================================================
Task_t	tlAssassinJumpAttack[] =
{
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_ASSASSIN_JUMP_LAND	},
	// { TASK_SET_ACTIVITY,		(float)ACT_FLY	},
	{ TASK_ASSASSIN_FALL_TO_GROUND, (float)0		},
};


Schedule_t	slAssassinJumpAttack[] =
{
	{ 
		tlAssassinJumpAttack,
		ARRAYSIZE ( tlAssassinJumpAttack ), 
		0, 
		0,
		"AssassinJumpAttack"
	},
};


//=========================================================
// repel 
//=========================================================
Task_t	tlAssassinJumpLand[] =
{
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_ASSASSIN_EXPOSED	},
	// { TASK_SET_FAIL_SCHEDULE,		(float)SCHED_MELEE_ATTACK1	},
	{ TASK_SET_ACTIVITY,			(float)ACT_IDLE				},
	{ TASK_REMEMBER,				(float)bits_MEMORY_BADJUMP	},
	{ TASK_FIND_NODE_COVER_FROM_ENEMY,	(float)0					},
	{ TASK_RUN_PATH,				(float)0					},
	{ TASK_FORGET,					(float)bits_MEMORY_BADJUMP	},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0					},
	{ TASK_REMEMBER,				(float)bits_MEMORY_INCOVER	},
	{ TASK_FACE_ENEMY,				(float)0					},
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_RANGE_ATTACK1	},
};

Schedule_t	slAssassinJumpLand[] =
{
	{ 
		tlAssassinJumpLand,
		ARRAYSIZE ( tlAssassinJumpLand ), 
		0, 
		0,
		"AssassinJumpLand"
	},
};

DEFINE_CUSTOM_SCHEDULES( CHAssassin )
{
	slAssassinFail,
	slAssassinExposed,
	slAssassinTakeCoverFromEnemy,
	slAssassinTakeCoverFromEnemy2,
	slAssassinTakeCoverFromBestSound,
	slAssassinHide,
	slAssassinHunt,
	slAssassinJump,
	slAssassinJumpAttack,
	slAssassinJumpLand,
};

IMPLEMENT_CUSTOM_SCHEDULES( CHAssassin, CBaseMonster );


//=========================================================
// CheckMeleeAttack1 - jump like crazy if the enemy gets too close. 
//=========================================================
BOOL CHAssassin :: CheckMeleeAttack1 ( float flDot, float flDist )
{
	if ( m_flNextJump < gpGlobals->time && (flDist <= 128 || HasMemory( bits_MEMORY_BADJUMP )) && m_hEnemy != NULL )
	{
		TraceResult	tr;

		Vector vecDest = pev->origin + Vector( RANDOM_FLOAT( -64, 64), RANDOM_FLOAT( -64, 64 ), 160 );

		UTIL_TraceHull( pev->origin + Vector( 0, 0, 36 ), vecDest + Vector( 0, 0, 36 ), dont_ignore_monsters, human_hull, ENT(pev), &tr);

		if ( tr.fStartSolid || tr.flFraction < 1.0)
		{
			return FALSE;
		}

		float flGravity = g_psv_gravity->value;

		float time = sqrt( 160 / (0.5 * flGravity));
		float speed = flGravity * time / 160;
		m_vecJumpVelocity = (vecDest - pev->origin) * speed;

		return TRUE;
	}
	return FALSE;
}

//=========================================================
// CheckRangeAttack1  - drop a cap in their ass
//
//=========================================================
BOOL CHAssassin :: CheckRangeAttack1 ( float flDot, float flDist )
{
	if ( !HasConditions( bits_COND_ENEMY_OCCLUDED ) && flDist > 64 && flDist <= 2048 /* && flDot >= 0.5 */ /* && NoFriendlyFire() */ )
	{
		TraceResult	tr;

		Vector vecSrc = GetGunPosition();

		// verify that a bullet fired from the gun will hit the enemy before the world.
		UTIL_TraceLine( vecSrc, m_hEnemy->BodyTarget(vecSrc), dont_ignore_monsters, ENT(pev), &tr);

		if ( tr.flFraction == 1 || tr.pHit == m_hEnemy->edict() )
		{
			return TRUE;
		}
	}
	return FALSE;
}

//=========================================================
// CheckRangeAttack2 - toss grenade is enemy gets in the way and is too close. 
//=========================================================
BOOL CHAssassin :: CheckRangeAttack2 ( float flDot, float flDist )
{
	m_fThrowGrenade = FALSE;
	if ( !FBitSet ( m_hEnemy->pev->flags, FL_ONGROUND ) )
	{
		// don't throw grenades at anything that isn't on the ground!
		return FALSE;
	}

	// don't get grenade happy unless the player starts to piss you off
	if ( m_iFrustration <= 2)
		return FALSE;

	if ( m_flNextGrenadeCheck < gpGlobals->time && !HasConditions( bits_COND_ENEMY_OCCLUDED ) && flDist <= 512 /* && flDot >= 0.5 */ /* && NoFriendlyFire() */ )
	{
		Vector vecToss = VecCheckThrow( pev, GetGunPosition( ), m_hEnemy->Center(), flDist, 0.5 ); // use dist as speed to get there in 1 second

		if ( vecToss != g_vecZero )
		{
			m_vecTossVelocity = vecToss;

			// throw a hand grenade
			m_fThrowGrenade = TRUE;

			return TRUE;
		}
	}

	return FALSE;
}


//=========================================================
// RunAI
//=========================================================
void CHAssassin :: RunAI( void )
{
	CBaseMonster :: RunAI();

	// always visible if moving
	// always visible is not on hard
	if (g_iSkillLevel != SKILL_HARD || m_hEnemy == NULL || pev->deadflag != DEAD_NO || m_Activity == ACT_RUN || m_Activity == ACT_WALK || !(pev->flags & FL_ONGROUND))
		m_iTargetRanderamt = 255;
	else
		m_iTargetRanderamt = 20;

	if (pev->renderamt > m_iTargetRanderamt)
	{
		if (pev->renderamt == 255)
		{
			EMIT_SOUND (ENT(pev), CHAN_BODY, "debris/beamstart1.wav", 0.2, ATTN_NORM );
		}

		pev->renderamt = max( pev->renderamt - 50, m_iTargetRanderamt );
		pev->rendermode = kRenderTransTexture;
	}
	else if (pev->renderamt < m_iTargetRanderamt)
	{
		pev->renderamt = min( pev->renderamt + 50, m_iTargetRanderamt );
		if (pev->renderamt == 255)
			pev->rendermode = kRenderNormal;
	}

	if (m_Activity == ACT_RUN || m_Activity == ACT_WALK)
	{
		static int iStep = 0;
		iStep = ! iStep;
		if (iStep)
		{
			switch( RANDOM_LONG( 0, 3 ) )
			{
			case 0:	EMIT_SOUND( ENT(pev), CHAN_BODY, "player/pl_step1.wav", 0.5, ATTN_NORM);	break;
			case 1:	EMIT_SOUND( ENT(pev), CHAN_BODY, "player/pl_step3.wav", 0.5, ATTN_NORM);	break;
			case 2:	EMIT_SOUND( ENT(pev), CHAN_BODY, "player/pl_step2.wav", 0.5, ATTN_NORM);	break;
			case 3:	EMIT_SOUND( ENT(pev), CHAN_BODY, "player/pl_step4.wav", 0.5, ATTN_NORM);	break;
			}
		}
	}
}


//=========================================================
// StartTask
//=========================================================
void CHAssassin :: StartTask ( Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_RANGE_ATTACK2:
		if (!m_fThrowGrenade)
		{
			TaskComplete( );
		}
		else
		{
			CBaseMonster :: StartTask ( pTask );
		}
		break;
	case TASK_ASSASSIN_FALL_TO_GROUND:
		break;
	default:
		CBaseMonster :: StartTask ( pTask );
		break;
	}
}


//=========================================================
// RunTask 
//=========================================================
void CHAssassin :: RunTask ( Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_ASSASSIN_FALL_TO_GROUND:
		MakeIdealYaw( m_vecEnemyLKP );
		ChangeYaw( pev->yaw_speed );

		if (m_fSequenceFinished)
		{
			if (pev->velocity.z > 0)
			{
				pev->sequence = LookupSequence( "fly_up" );
			}
			else if (HasConditions ( bits_COND_SEE_ENEMY ))
			{
				pev->sequence = LookupSequence( "fly_attack" );
				pev->frame = 0;
			}
			else
			{
				pev->sequence = LookupSequence( "fly_down" );
				pev->frame = 0;
			}
			
			ResetSequenceInfo( );
			SetYawSpeed();
		}
		if (pev->flags & FL_ONGROUND)
		{
			// ALERT( at_console, "on ground\n");
			TaskComplete( );
		}
		break;
	default: 
		CBaseMonster :: RunTask ( pTask );
		break;
	}
}

//=========================================================
// GetSchedule - Decides which type of schedule best suits
// the monster's current state and conditions. Then calls
// monster's member function to get a pointer to a schedule
// of the proper type.
//=========================================================
Schedule_t *CHAssassin :: GetSchedule ( void )
{
	switch	( m_MonsterState )
	{
	case MONSTERSTATE_IDLE:
	case MONSTERSTATE_ALERT:
		{
			if ( HasConditions ( bits_COND_HEAR_SOUND ))
			{
				CSound *pSound;
				pSound = PBestSound();

				ASSERT( pSound != NULL );
				if ( pSound && (pSound->m_iType & bits_SOUND_DANGER) )
				{
					return GetScheduleOfType( SCHED_TAKE_COVER_FROM_BEST_SOUND );
				}
				if ( pSound && (pSound->m_iType & bits_SOUND_COMBAT) )
				{
					return GetScheduleOfType( SCHED_INVESTIGATE_SOUND );
				}
			}
		}
		break;

	case MONSTERSTATE_COMBAT:
		{
// dead enemy
			if ( HasConditions( bits_COND_ENEMY_DEAD ) )
			{
				// call base class, all code to handle dead enemies is centralized there.
				return CBaseMonster :: GetSchedule();
			}

			// flying?
			if ( pev->movetype == MOVETYPE_TOSS)
			{
				if (pev->flags & FL_ONGROUND)
				{
					// ALERT( at_console, "landed\n");
					// just landed
					pev->movetype = MOVETYPE_STEP;
					return GetScheduleOfType ( SCHED_ASSASSIN_JUMP_LAND );
				}
				else
				{
					// ALERT( at_console, "jump\n");
					// jump or jump/shoot
					if ( m_MonsterState == MONSTERSTATE_COMBAT )
						return GetScheduleOfType ( SCHED_ASSASSIN_JUMP );
					else
						return GetScheduleOfType ( SCHED_ASSASSIN_JUMP_ATTACK );
				}
			}

			if ( HasConditions ( bits_COND_HEAR_SOUND ))
			{
				CSound *pSound;
				pSound = PBestSound();

				ASSERT( pSound != NULL );
				if ( pSound && (pSound->m_iType & bits_SOUND_DANGER) )
				{
					return GetScheduleOfType( SCHED_TAKE_COVER_FROM_BEST_SOUND );
				}
			}

			if ( HasConditions ( bits_COND_LIGHT_DAMAGE ) )
			{
				m_iFrustration++;
			}
			if ( HasConditions ( bits_COND_HEAVY_DAMAGE ) )
			{
				m_iFrustration++;
			}

		// jump player!
			if ( HasConditions ( bits_COND_CAN_MELEE_ATTACK1 ) )
			{
				// ALERT( at_console, "melee attack 1\n");
				return GetScheduleOfType ( SCHED_MELEE_ATTACK1 );
			}

		// throw grenade
			if ( HasConditions ( bits_COND_CAN_RANGE_ATTACK2 ) )
			{
				// ALERT( at_console, "range attack 2\n");
				return GetScheduleOfType ( SCHED_RANGE_ATTACK2 );
			}

		// spotted
			if ( HasConditions ( bits_COND_SEE_ENEMY ) && HasConditions ( bits_COND_ENEMY_FACING_ME ) )
			{
				// ALERT( at_console, "exposed\n");
				m_iFrustration++;
				return GetScheduleOfType ( SCHED_ASSASSIN_EXPOSED );
			}

		// can attack
			if ( HasConditions ( bits_COND_CAN_RANGE_ATTACK1 ) )
			{
				// ALERT( at_console, "range attack 1\n");
				m_iFrustration = 0;
				return GetScheduleOfType ( SCHED_RANGE_ATTACK1 );
			}

			if ( HasConditions ( bits_COND_SEE_ENEMY ) )
			{
				// ALERT( at_console, "face\n");
				return GetScheduleOfType ( SCHED_COMBAT_FACE );
			}

		// new enemy
			if ( HasConditions ( bits_COND_NEW_ENEMY ) )
			{
				// ALERT( at_console, "take cover\n");
				return GetScheduleOfType ( SCHED_TAKE_COVER_FROM_ENEMY );
			}

			// ALERT( at_console, "stand\n");
			return GetScheduleOfType ( SCHED_ALERT_STAND );
		}
		break;
	}

	return CBaseMonster :: GetSchedule();
}

//=========================================================
//=========================================================
Schedule_t* CHAssassin :: GetScheduleOfType ( int Type ) 
{
	// ALERT( at_console, "%d\n", m_iFrustration );
	switch	( Type )
	{
	case SCHED_TAKE_COVER_FROM_ENEMY:
		if (pev->health > 30)
			return slAssassinTakeCoverFromEnemy;
		else
			return slAssassinTakeCoverFromEnemy2;
	case SCHED_TAKE_COVER_FROM_BEST_SOUND:
		return slAssassinTakeCoverFromBestSound;
	case SCHED_ASSASSIN_EXPOSED:
		return slAssassinExposed;
	case SCHED_FAIL:
		if (m_MonsterState == MONSTERSTATE_COMBAT)
			return slAssassinFail;
		break;
	case SCHED_ALERT_STAND:
		if (m_MonsterState == MONSTERSTATE_COMBAT)
			return slAssassinHide;
		break;
	case SCHED_CHASE_ENEMY:
		return slAssassinHunt;
	case SCHED_MELEE_ATTACK1:
		if (pev->flags & FL_ONGROUND)
		{
			if (m_flNextJump > gpGlobals->time)
			{
				// can't jump yet, go ahead and fail
				return slAssassinFail;
			}
			else
			{
				return slAssassinJump;
			}
		}
		else
		{
			return slAssassinJumpAttack;
		}
	case SCHED_ASSASSIN_JUMP:
	case SCHED_ASSASSIN_JUMP_ATTACK:
		return slAssassinJumpAttack;
	case SCHED_ASSASSIN_JUMP_LAND:
		return slAssassinJumpLand;
	}

	return CBaseMonster :: GetScheduleOfType( Type );
}

#endif