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
#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"monsters.h"
#include	"schedule.h"
#include	"talkmonster.h"
#include	"defaultai.h"
#include	"scripted.h"
#include	"soundent.h"
#include	"animation.h"

//=========================================================
// Talking monster base class
// Used for scientists and barneys
//=========================================================
float	CTalkMonster::g_talkWaitTime = 0;		// time delay until it's ok to speak: used so that two NPCs don't talk at once

// NOTE: m_voicePitch & m_szGrp should be fixed up by precache each save/restore

TYPEDESCRIPTION	CTalkMonster::m_SaveData[] = 
{
	DEFINE_FIELD( CTalkMonster, m_bitsSaid, FIELD_INTEGER ),
	DEFINE_FIELD( CTalkMonster, m_nSpeak, FIELD_INTEGER ),

	// Recalc'ed in Precache()
	//	DEFINE_FIELD( CTalkMonster, m_voicePitch, FIELD_INTEGER ),
	//	DEFINE_FIELD( CTalkMonster, m_szGrp, FIELD_??? ),
	DEFINE_FIELD( CTalkMonster, m_useTime, FIELD_TIME ),
	DEFINE_FIELD( CTalkMonster, m_iszUse, FIELD_STRING ),
	DEFINE_FIELD( CTalkMonster, m_iszUnUse, FIELD_STRING ),
	DEFINE_FIELD( CTalkMonster, m_flLastSaidSmelled, FIELD_TIME ),
	DEFINE_FIELD( CTalkMonster, m_flStopTalkTime, FIELD_TIME ),
	DEFINE_FIELD( CTalkMonster, m_hTalkTarget, FIELD_EHANDLE ),
};

IMPLEMENT_SAVERESTORE( CTalkMonster, CBaseMonster );

// array of friend names
char *CTalkMonster::m_szFriends[TLK_CFRIENDS] = 
{
	"monster_barney",
	"monster_scientist",
	"monster_sitting_scientist",
};


//=========================================================
// AI Schedules Specific to talking monsters
//=========================================================

Task_t	tlIdleResponse[] =
{
	{ TASK_SET_ACTIVITY,	(float)ACT_IDLE	},// Stop and listen
	{ TASK_WAIT,			(float)0.5		},// Wait until sure it's me they are talking to
	{ TASK_TLK_EYECONTACT,	(float)0		},// Wait until speaker is done
	{ TASK_TLK_RESPOND,		(float)0		},// Wait and then say my response
	{ TASK_TLK_IDEALYAW,	(float)0		},// look at who I'm talking to
	{ TASK_FACE_IDEAL,		(float)0		}, 
	{ TASK_SET_ACTIVITY,	(float)ACT_SIGNAL3	},
	{ TASK_TLK_EYECONTACT,	(float)0		},// Wait until speaker is done
};

Schedule_t	slIdleResponse[] =
{
	{ 
		tlIdleResponse,
		ARRAYSIZE ( tlIdleResponse ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE,
		0,
		"Idle Response"

	},
};

Task_t	tlIdleSpeak[] =
{
	{ TASK_TLK_SPEAK,		(float)0		},// question or remark
	{ TASK_TLK_IDEALYAW,	(float)0		},// look at who I'm talking to
	{ TASK_FACE_IDEAL,		(float)0		}, 
	{ TASK_SET_ACTIVITY,	(float)ACT_SIGNAL3	},
	{ TASK_TLK_EYECONTACT,	(float)0		},
	{ TASK_WAIT_RANDOM,		(float)0.5		},
};

Schedule_t	slIdleSpeak[] =
{
	{ 
		tlIdleSpeak,
		ARRAYSIZE ( tlIdleSpeak ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_CLIENT_PUSH	|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE,
		0,
		"Idle Speak"
	},
};

Task_t	tlIdleSpeakWait[] =
{
	{ TASK_SET_ACTIVITY,	(float)ACT_SIGNAL3	},// Stop and talk
	{ TASK_TLK_SPEAK,		(float)0		},// question or remark
	{ TASK_TLK_EYECONTACT,	(float)0		},// 
	{ TASK_WAIT,			(float)2		},// wait - used when sci is in 'use' mode to keep head turned
};

Schedule_t	slIdleSpeakWait[] =
{
	{ 
		tlIdleSpeakWait,
		ARRAYSIZE ( tlIdleSpeakWait ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_CLIENT_PUSH	|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE,
		0,
		"Idle Speak Wait"
	},
};

Task_t	tlIdleHello[] =
{
	{ TASK_SET_ACTIVITY,	(float)ACT_SIGNAL3	},// Stop and talk
	{ TASK_TLK_HELLO,		(float)0		},// Try to say hello to player
	{ TASK_TLK_EYECONTACT,	(float)0		},
	{ TASK_WAIT,			(float)0.5		},// wait a bit
	{ TASK_TLK_HELLO,		(float)0		},// Try to say hello to player
	{ TASK_TLK_EYECONTACT,	(float)0		},
	{ TASK_WAIT,			(float)0.5		},// wait a bit
	{ TASK_TLK_HELLO,		(float)0		},// Try to say hello to player
	{ TASK_TLK_EYECONTACT,	(float)0		},
	{ TASK_WAIT,			(float)0.5		},// wait a bit
	{ TASK_TLK_HELLO,		(float)0		},// Try to say hello to player
	{ TASK_TLK_EYECONTACT,	(float)0		},
	{ TASK_WAIT,			(float)0.5		},// wait a bit

};

Schedule_t	slIdleHello[] =
{
	{ 
		tlIdleHello,
		ARRAYSIZE ( tlIdleHello ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_CLIENT_PUSH	|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND	|
		bits_COND_PROVOKED,

		bits_SOUND_COMBAT,
		"Idle Hello"
	},
};

Task_t	tlIdleStopShooting[] =
{
	{ TASK_TLK_STOPSHOOTING,	(float)0		},// tell player to stop shooting friend
	// { TASK_TLK_EYECONTACT,		(float)0		},// look at the player
};

Schedule_t	slIdleStopShooting[] =
{
	{ 
		tlIdleStopShooting,
		ARRAYSIZE ( tlIdleStopShooting ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND,
		0,
		"Idle Stop Shooting"
	},
};

Task_t	tlMoveAway[] =
{
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_MOVE_AWAY_FAIL },
	{ TASK_STORE_LASTPOSITION,		(float)0		},
	{ TASK_MOVE_AWAY_PATH,			(float)100		},
	{ TASK_WALK_PATH_FOR_UNITS,		(float)100		},
	{ TASK_STOP_MOVING,				(float)0		},
	{ TASK_FACE_PLAYER,				(float)0.5 },
};

Schedule_t	slMoveAway[] =
{
	{
		tlMoveAway,
		ARRAYSIZE ( tlMoveAway ),
		0,
		0,
		"MoveAway"
	},
};


Task_t	tlMoveAwayFail[] =
{
	{ TASK_STOP_MOVING,				(float)0		},
	{ TASK_FACE_PLAYER,				(float)0.5		},
};

Schedule_t	slMoveAwayFail[] =
{
	{
		tlMoveAwayFail,
		ARRAYSIZE ( tlMoveAwayFail ),
		0,
		0,
		"MoveAwayFail"
	},
};



Task_t	tlMoveAwayFollow[] =
{
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_TARGET_FACE },
	{ TASK_STORE_LASTPOSITION,		(float)0		},
	{ TASK_MOVE_AWAY_PATH,			(float)100				},
	{ TASK_WALK_PATH_FOR_UNITS,		(float)100		},
	{ TASK_STOP_MOVING,				(float)0		},
	{ TASK_SET_SCHEDULE,			(float)SCHED_TARGET_FACE },
};

Schedule_t	slMoveAwayFollow[] =
{
	{
		tlMoveAwayFollow,
		ARRAYSIZE ( tlMoveAwayFollow ),
		0,
		0,
		"MoveAwayFollow"
	},
};

Task_t	tlTlkIdleWatchClient[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE	},
	{ TASK_TLK_LOOK_AT_CLIENT,	(float)6		},
};

Task_t	tlTlkIdleWatchClientStare[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE	},
	{ TASK_TLK_CLIENT_STARE,	(float)6		},
	{ TASK_TLK_STARE,			(float)0		},
	{ TASK_TLK_IDEALYAW,		(float)0		},// look at who I'm talking to
	{ TASK_FACE_IDEAL,			(float)0		}, 
	{ TASK_SET_ACTIVITY,		(float)ACT_SIGNAL3	},
	{ TASK_TLK_EYECONTACT,		(float)0		},
};

Schedule_t	slTlkIdleWatchClient[] =
{
	{ 
		tlTlkIdleWatchClient,
		ARRAYSIZE ( tlTlkIdleWatchClient ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND	|
		bits_COND_SMELL			|
		bits_COND_CLIENT_PUSH	|
		bits_COND_CLIENT_UNSEEN	|
		bits_COND_PROVOKED,

		bits_SOUND_COMBAT		|// sound flags - change these, and you'll break the talking code.
		//bits_SOUND_PLAYER		|
		//bits_SOUND_WORLD		|
		
		bits_SOUND_DANGER		|
		bits_SOUND_MEAT			|// scents
		bits_SOUND_CARCASS		|
		bits_SOUND_GARBAGE,
		"TlkIdleWatchClient"
	},

	{ 
		tlTlkIdleWatchClientStare,
		ARRAYSIZE ( tlTlkIdleWatchClientStare ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND	|
		bits_COND_SMELL			|
		bits_COND_CLIENT_PUSH	|
		bits_COND_CLIENT_UNSEEN	|
		bits_COND_PROVOKED,

		bits_SOUND_COMBAT		|// sound flags - change these, and you'll break the talking code.
		//bits_SOUND_PLAYER		|
		//bits_SOUND_WORLD		|
		
		bits_SOUND_DANGER		|
		bits_SOUND_MEAT			|// scents
		bits_SOUND_CARCASS		|
		bits_SOUND_GARBAGE,
		"TlkIdleWatchClientStare"
	},
};


Task_t	tlTlkIdleEyecontact[] =
{
	{ TASK_TLK_IDEALYAW,	(float)0		},// look at who I'm talking to
	{ TASK_FACE_IDEAL,		(float)0		}, 
	{ TASK_SET_ACTIVITY,	(float)ACT_SIGNAL3	},
	{ TASK_TLK_EYECONTACT,	(float)0		},// Wait until speaker is done
};

Schedule_t	slTlkIdleEyecontact[] =
{
	{ 
		tlTlkIdleEyecontact,
		ARRAYSIZE ( tlTlkIdleEyecontact ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_CLIENT_PUSH	|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE,
		0,
		"TlkIdleEyecontact"
	},
};


DEFINE_CUSTOM_SCHEDULES( CTalkMonster )
{
	slIdleResponse,
	slIdleSpeak,
	slIdleHello,
	slIdleSpeakWait,
	slIdleStopShooting,
	slMoveAway,
	slMoveAwayFollow,
	slMoveAwayFail,
	slTlkIdleWatchClient,
	&slTlkIdleWatchClient[ 1 ],
	slTlkIdleEyecontact,
};

IMPLEMENT_CUSTOM_SCHEDULES( CTalkMonster, CBaseMonster );


void CTalkMonster :: SetActivity ( Activity newActivity )
{
	if (newActivity == ACT_IDLE && IsTalking() )
		newActivity = ACT_SIGNAL3;
	
	if ( newActivity == ACT_SIGNAL3 && (LookupActivity ( ACT_SIGNAL3 ) == ACTIVITY_NOT_AVAILABLE))
		newActivity = ACT_IDLE;

	CBaseMonster::SetActivity( newActivity );
}


void CTalkMonster :: StartTask( Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_TLK_SPEAK:
		// ask question or make statement
		FIdleSpeak();
		TaskComplete();
		break;

	case TASK_TLK_RESPOND:
		// respond to question
		IdleRespond();
		TaskComplete();
		break;

	case TASK_TLK_HELLO:
		// greet player
		FIdleHello();
		TaskComplete();
		break;
	

	case TASK_TLK_STARE:
		// let the player know I know he's staring at me.
		FIdleStare();
		TaskComplete();
		break;

	case TASK_FACE_PLAYER:
	case TASK_TLK_LOOK_AT_CLIENT:
	case TASK_TLK_CLIENT_STARE:
		// track head to the client for a while.
		m_flWaitFinished = gpGlobals->time + pTask->flData;
		break;

	case TASK_TLK_EYECONTACT:
		break;

	case TASK_TLK_IDEALYAW:
		if (m_hTalkTarget != NULL)
		{
			pev->yaw_speed = 60;
			float yaw = VecToYaw(m_hTalkTarget->pev->origin - pev->origin) - pev->angles.y;

			if (yaw > 180) yaw -= 360;
			if (yaw < -180) yaw += 360;

			if (yaw < 0)
			{
				pev->ideal_yaw = min( yaw + 45, 0 ) + pev->angles.y;
			}
			else
			{
				pev->ideal_yaw = max( yaw - 45, 0 ) + pev->angles.y;
			}
		}
		TaskComplete();
		break;

	case TASK_TLK_HEADRESET:
		// reset head position after looking at something
		m_hTalkTarget = NULL;
		TaskComplete();
		break;

	case TASK_TLK_STOPSHOOTING:
		// tell player to stop shooting
		PlaySentence( m_szGrp[TLK_NOSHOOT], RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_NORM );
		TaskComplete();
		break;

	case TASK_CANT_FOLLOW:
		StopFollowing( FALSE );
		PlaySentence( m_szGrp[TLK_STOP], RANDOM_FLOAT(2, 2.5), VOL_NORM, ATTN_NORM );
		TaskComplete();
		break;

	case TASK_WALK_PATH_FOR_UNITS:
		m_movementActivity = ACT_WALK;
		break;

	case TASK_MOVE_AWAY_PATH:
		{
			Vector dir = pev->angles;
			dir.y = pev->ideal_yaw + 180;
			Vector move;

			UTIL_MakeVectorsPrivate( dir, move, NULL, NULL );
			dir = pev->origin + move * pTask->flData;
			if ( MoveToLocation( ACT_WALK, 2, dir ) )
			{
				TaskComplete();
			}
			else if ( FindCover( pev->origin, pev->view_ofs, 0, CoverRadius() ) )
			{
				// then try for plain ole cover
				m_flMoveWaitFinished = gpGlobals->time + 2;
				TaskComplete();
			}
			else
			{
				// nowhere to go?
				TaskFail();
			}
		}
		break;

	case TASK_PLAY_SCRIPT:
		m_hTalkTarget = NULL;
		CBaseMonster::StartTask( pTask );
		break;

	default:
		CBaseMonster::StartTask( pTask );
	}
}


void CTalkMonster :: RunTask( Task_t *pTask )
{
	switch( pTask->iTask )
	{
	case TASK_TLK_CLIENT_STARE:
	case TASK_TLK_LOOK_AT_CLIENT:

		edict_t *pPlayer;

		// track head to the client for a while.
		if ( m_MonsterState == MONSTERSTATE_IDLE		&& 
			 !IsMoving()								&&
			 !IsTalking()								)
		{
			// Get edict for one player
			pPlayer = g_engfuncs.pfnPEntityOfEntIndex( 1 );

			if ( pPlayer )
			{
				IdleHeadTurn( pPlayer->v.origin );
			}
		}
		else
		{
			// started moving or talking
			TaskFail();
			return;
		}

		if ( pTask->iTask == TASK_TLK_CLIENT_STARE )
		{
			// fail out if the player looks away or moves away.
			if ( ( pPlayer->v.origin - pev->origin ).Length2D() > TLK_STARE_DIST )
			{
				// player moved away.
				TaskFail();
			}

			UTIL_MakeVectors( pPlayer->v.angles );
			if ( UTIL_DotPoints( pPlayer->v.origin, pev->origin, gpGlobals->v_forward ) < m_flFieldOfView )
			{
				// player looked away
				TaskFail();
			}
		}

		if ( gpGlobals->time > m_flWaitFinished )
		{
			TaskComplete();
		}
		break;

	case TASK_FACE_PLAYER:
		{
			// Get edict for one player
			edict_t *pPlayer = g_engfuncs.pfnPEntityOfEntIndex( 1 );

			if ( pPlayer )
			{
				MakeIdealYaw ( pPlayer->v.origin );
				ChangeYaw ( pev->yaw_speed );
				IdleHeadTurn( pPlayer->v.origin );
				if ( gpGlobals->time > m_flWaitFinished && FlYawDiff() < 10 )
				{
					TaskComplete();
				}
			}
			else
			{
				TaskFail();
			}
		}
		break;

	case TASK_TLK_EYECONTACT:
		if (!IsMoving() && IsTalking() && m_hTalkTarget != NULL)
		{
			// ALERT( at_console, "waiting %f\n", m_flStopTalkTime - gpGlobals->time );
			IdleHeadTurn( m_hTalkTarget->pev->origin );
		}
		else
		{
			TaskComplete();
		}
		break;

	case TASK_WALK_PATH_FOR_UNITS:
		{
			float distance;

			distance = (m_vecLastPosition - pev->origin).Length2D();

			// Walk path until far enough away
			if ( distance > pTask->flData || MovementIsComplete() )
			{
				TaskComplete();
				RouteClear();		// Stop moving
			}
		}
		break;
	case TASK_WAIT_FOR_MOVEMENT:
		if (IsTalking() && m_hTalkTarget != NULL)
		{
			// ALERT(at_console, "walking, talking\n");
			IdleHeadTurn( m_hTalkTarget->pev->origin );
		}
		else
		{
			IdleHeadTurn( pev->origin );
			// override so that during walk, a scientist may talk and greet player
			FIdleHello();
			if (RANDOM_LONG(0,m_nSpeak * 20) == 0)
			{
				FIdleSpeak();
			}
		}

		CBaseMonster::RunTask( pTask );
		if (TaskIsComplete())
			IdleHeadTurn( pev->origin );
		break;

	default:
		if (IsTalking() && m_hTalkTarget != NULL)
		{
			IdleHeadTurn( m_hTalkTarget->pev->origin );
		}
		else
		{
			SetBoneController( 0, 0 );
		}
		CBaseMonster::RunTask( pTask );
	}
}


void CTalkMonster :: Killed( entvars_t *pevAttacker, int iGib )
{
	// If a client killed me (unless I was already Barnacle'd), make everyone else mad/afraid of him
	if ( (pevAttacker->flags & FL_CLIENT) && m_MonsterState != MONSTERSTATE_PRONE )
	{
		AlertFriends();
		LimitFollowers( CBaseEntity::Instance(pevAttacker), 0 );
	}

	m_hTargetEnt = NULL;
	// Don't finish that sentence
	StopTalking();
	SetUse( NULL );
	CBaseMonster::Killed( pevAttacker, iGib );
}



CBaseEntity	*CTalkMonster::EnumFriends( CBaseEntity *pPrevious, int listNumber, BOOL bTrace )
{
	CBaseEntity *pFriend = pPrevious;
	char *pszFriend;
	TraceResult tr;
	Vector vecCheck;

	pszFriend = m_szFriends[ FriendNumber(listNumber) ];
	while (pFriend = UTIL_FindEntityByClassname( pFriend, pszFriend ))
	{
		if (pFriend == this || !pFriend->IsAlive())
			// don't talk to self or dead people
			continue;
		if ( bTrace )
		{
			vecCheck = pFriend->pev->origin;
			vecCheck.z = pFriend->pev->absmax.z;

			UTIL_TraceLine( pev->origin, vecCheck, ignore_monsters, ENT(pev), &tr);
		}
		else
			tr.flFraction = 1.0;

		if (tr.flFraction == 1.0)
		{
			return pFriend;
		}
	}

	return NULL;
}


void CTalkMonster::AlertFriends( void )
{
	CBaseEntity *pFriend = NULL;
	int i;

	// for each friend in this bsp...
	for ( i = 0; i < TLK_CFRIENDS; i++ )
	{
		while (pFriend = EnumFriends( pFriend, i, TRUE ))
		{
			CBaseMonster *pMonster = pFriend->MyMonsterPointer();
			if ( pMonster->IsAlive() )
			{
				// don't provoke a friend that's playing a death animation. They're a goner
				pMonster->m_afMemory |= bits_MEMORY_PROVOKED;
			}
		}
	}
}



void CTalkMonster::ShutUpFriends( void )
{
	CBaseEntity *pFriend = NULL;
	int i;

	// for each friend in this bsp...
	for ( i = 0; i < TLK_CFRIENDS; i++ )
	{
		while (pFriend = EnumFriends( pFriend, i, TRUE ))
		{
			CBaseMonster *pMonster = pFriend->MyMonsterPointer();
			if ( pMonster )
			{
				pMonster->SentenceStop();
			}
		}
	}
}


// UNDONE: Keep a follow time in each follower, make a list of followers in this function and do LRU
// UNDONE: Check this in Restore to keep restored monsters from joining a full list of followers
void CTalkMonster::LimitFollowers( CBaseEntity *pPlayer, int maxFollowers )
{
	CBaseEntity *pFriend = NULL;
	int i, count;

	count = 0;
	// for each friend in this bsp...
	for ( i = 0; i < TLK_CFRIENDS; i++ )
	{
		while (pFriend = EnumFriends( pFriend, i, FALSE ))
		{
			CBaseMonster *pMonster = pFriend->MyMonsterPointer();
			if ( pMonster )
			{
				if ( pMonster->m_hTargetEnt == pPlayer )
				{
					count++;
					if ( count > maxFollowers )
						pMonster->StopFollowing( TRUE );
				}
			}
		}
	}
}


float CTalkMonster::TargetDistance( void )
{
	// If we lose the player, or he dies, return a really large distance
	if ( m_hTargetEnt == NULL || !m_hTargetEnt->IsAlive() )
		return 1e6;

	return (m_hTargetEnt->pev->origin - pev->origin).Length();
}


//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CTalkMonster :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{		
	case SCRIPT_EVENT_SENTENCE_RND1:		// Play a named sentence group 25% of the time
		if (RANDOM_LONG(0,99) < 75)
			break;
		// fall through...
	case SCRIPT_EVENT_SENTENCE:				// Play a named sentence group
		ShutUpFriends();
		PlaySentence( pEvent->options, RANDOM_FLOAT(2.8, 3.4), VOL_NORM, ATTN_IDLE );
		//ALERT(at_console, "script event speak\n");
		break;

	default:
		CBaseMonster::HandleAnimEvent( pEvent );
		break;
	}
}

// monsters derived from ctalkmonster should call this in precache()

void CTalkMonster :: TalkInit( void )
{
	// every new talking monster must reset this global, otherwise
	// when a level is loaded, nobody will talk (time is reset to 0)

	CTalkMonster::g_talkWaitTime = 0;

	m_voicePitch = 100;
}	
//=========================================================
// FindNearestFriend
// Scan for nearest, visible friend. If fPlayer is true, look for
// nearest player
//=========================================================
CBaseEntity *CTalkMonster :: FindNearestFriend(BOOL fPlayer)
{
	CBaseEntity *pFriend = NULL;
	CBaseEntity *pNearest = NULL;
	float range = 10000000.0;
	TraceResult tr;
	Vector vecStart = pev->origin;
	Vector vecCheck;
	int i;
	char *pszFriend;
	int cfriends;

	vecStart.z = pev->absmax.z;
	
	if (fPlayer)
		cfriends = 1;
	else
		cfriends = TLK_CFRIENDS;

	// for each type of friend...

	for (i = cfriends-1; i > -1; i--)
	{
		if (fPlayer)
			pszFriend = "player";
		else
			pszFriend = m_szFriends[FriendNumber(i)];

		if (!pszFriend)
			continue;

		// for each friend in this bsp...
		while (pFriend = UTIL_FindEntityByClassname( pFriend, pszFriend ))
		{
			if (pFriend == this || !pFriend->IsAlive())
				// don't talk to self or dead people
				continue;

			CBaseMonster *pMonster = pFriend->MyMonsterPointer();

			// If not a monster for some reason, or in a script, or prone
			if ( !pMonster || pMonster->m_MonsterState == MONSTERSTATE_SCRIPT || pMonster->m_MonsterState == MONSTERSTATE_PRONE )
				continue;

			vecCheck = pFriend->pev->origin;
			vecCheck.z = pFriend->pev->absmax.z;

			// if closer than previous friend, and in range, see if he's visible

			if (range > (vecStart - vecCheck).Length())
			{
				UTIL_TraceLine(vecStart, vecCheck, ignore_monsters, ENT(pev), &tr);

				if (tr.flFraction == 1.0)
				{
					// visible and in range, this is the new nearest scientist
					if ((vecStart - vecCheck).Length() < TALKRANGE_MIN)
					{
						pNearest = pFriend;
						range = (vecStart - vecCheck).Length();
					}
				}
			}
		}
	}
	return pNearest;
}

int CTalkMonster :: GetVoicePitch( void )
{
	return m_voicePitch + RANDOM_LONG(0,3);
}


void CTalkMonster :: Touch( CBaseEntity *pOther )
{
	// Did the player touch me?
	if ( pOther->IsPlayer() )
	{
		// Ignore if pissed at player
		if ( m_afMemory & bits_MEMORY_PROVOKED )
			return;

		// Stay put during speech
		if ( IsTalking() )
			return;

		// Heuristic for determining if the player is pushing me away
		float speed = fabs(pOther->pev->velocity.x) + fabs(pOther->pev->velocity.y);
		if ( speed > 50 )
		{
			SetConditions( bits_COND_CLIENT_PUSH );
			MakeIdealYaw( pOther->pev->origin );
		}
	}
}



//=========================================================
// IdleRespond
// Respond to a previous question
//=========================================================
void CTalkMonster :: IdleRespond( void )
{
	int pitch = GetVoicePitch();
	
	// play response
	PlaySentence( m_szGrp[TLK_ANSWER], RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_IDLE );
}

int CTalkMonster :: FOkToSpeak( void )
{
	// if in the grip of a barnacle, don't speak
	if ( m_MonsterState == MONSTERSTATE_PRONE || m_IdealMonsterState == MONSTERSTATE_PRONE )
	{
		return FALSE;
	}

	// if not alive, certainly don't speak
	if ( pev->deadflag != DEAD_NO )
	{
		return FALSE;
	}

	// if someone else is talking, don't speak
	if (gpGlobals->time <= CTalkMonster::g_talkWaitTime)
		return FALSE;

	if ( pev->spawnflags & SF_MONSTER_GAG )
		return FALSE;

	if ( m_MonsterState == MONSTERSTATE_PRONE )
		return FALSE;

	// if player is not in pvs, don't speak
	if (!IsAlive() || FNullEnt(FIND_CLIENT_IN_PVS(edict())))
		return FALSE;

	// don't talk if you're in combat
	if (m_hEnemy != NULL && FVisible( m_hEnemy ))
		return FALSE;

	return TRUE;
}


int CTalkMonster::CanPlaySentence( BOOL fDisregardState ) 
{ 
	if ( fDisregardState )
		return CBaseMonster::CanPlaySentence( fDisregardState );
	return FOkToSpeak(); 
}

//=========================================================
// FIdleStare
//=========================================================
int CTalkMonster :: FIdleStare( void )
{
	if (!FOkToSpeak())
		return FALSE;

	PlaySentence( m_szGrp[TLK_STARE], RANDOM_FLOAT(5, 7.5), VOL_NORM, ATTN_IDLE );

	m_hTalkTarget = FindNearestFriend( TRUE );
	return TRUE;
}

//=========================================================
// IdleHello
// Try to greet player first time he's seen
//=========================================================
int CTalkMonster :: FIdleHello( void )
{
	if (!FOkToSpeak())
		return FALSE;

	// if this is first time scientist has seen player, greet him
	if (!FBitSet(m_bitsSaid, bit_saidHelloPlayer))
	{
		// get a player
		CBaseEntity *pPlayer = FindNearestFriend(TRUE);

		if (pPlayer)
		{
			if (FInViewCone(pPlayer) && FVisible(pPlayer))
			{
				m_hTalkTarget = pPlayer;

				if (FBitSet(pev->spawnflags, SF_MONSTER_PREDISASTER))
					PlaySentence( m_szGrp[TLK_PHELLO], RANDOM_FLOAT(3, 3.5), VOL_NORM,  ATTN_IDLE );
				else
					PlaySentence( m_szGrp[TLK_HELLO], RANDOM_FLOAT(3, 3.5), VOL_NORM,  ATTN_IDLE );

				SetBits(m_bitsSaid, bit_saidHelloPlayer);
				
				return TRUE;
			}
		}
	}
	return FALSE;
}


// turn head towards supplied origin
void CTalkMonster :: IdleHeadTurn( Vector &vecFriend )
{
	 // turn head in desired direction only if ent has a turnable head
	if (m_afCapability & bits_CAP_TURN_HEAD)
	{
		float yaw = VecToYaw(vecFriend - pev->origin) - pev->angles.y;

		if (yaw > 180) yaw -= 360;
		if (yaw < -180) yaw += 360;

		// turn towards vector
		SetBoneController( 0, yaw );
	}
}

//=========================================================
// FIdleSpeak
// ask question of nearby friend, or make statement
//=========================================================
int CTalkMonster :: FIdleSpeak ( void )
{ 
	// try to start a conversation, or make statement
	int pitch;
	const char *szIdleGroup;
	const char *szQuestionGroup;
	float duration;

	if (!FOkToSpeak())
		return FALSE;

	// set idle groups based on pre/post disaster
	if (FBitSet(pev->spawnflags, SF_MONSTER_PREDISASTER))
	{
		szIdleGroup = m_szGrp[TLK_PIDLE];
		szQuestionGroup = m_szGrp[TLK_PQUESTION];
		// set global min delay for next conversation
		duration = RANDOM_FLOAT(4.8, 5.2);
	}
	else
	{
		szIdleGroup = m_szGrp[TLK_IDLE];
		szQuestionGroup = m_szGrp[TLK_QUESTION];
		// set global min delay for next conversation
		duration = RANDOM_FLOAT(2.8, 3.2);

	}

	pitch = GetVoicePitch();
		
	// player using this entity is alive and wounded?
	CBaseEntity *pTarget = m_hTargetEnt;

	if ( pTarget != NULL )
	{
		if ( pTarget->IsPlayer() )
		{
			if ( pTarget->IsAlive() )
			{
				m_hTalkTarget = m_hTargetEnt;
				if (!FBitSet(m_bitsSaid, bit_saidDamageHeavy) && 
					(m_hTargetEnt->pev->health <= m_hTargetEnt->pev->max_health / 8))
				{
					//EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, m_szGrp[TLK_PLHURT3], 1.0, ATTN_IDLE, 0, pitch);
					PlaySentence( m_szGrp[TLK_PLHURT3], duration, VOL_NORM, ATTN_IDLE );
					SetBits(m_bitsSaid, bit_saidDamageHeavy);
					return TRUE;
				}
				else if (!FBitSet(m_bitsSaid, bit_saidDamageMedium) && 
					(m_hTargetEnt->pev->health <= m_hTargetEnt->pev->max_health / 4))
				{
					//EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, m_szGrp[TLK_PLHURT2], 1.0, ATTN_IDLE, 0, pitch);
					PlaySentence( m_szGrp[TLK_PLHURT2], duration, VOL_NORM, ATTN_IDLE );
					SetBits(m_bitsSaid, bit_saidDamageMedium);
					return TRUE;
				}
				else if (!FBitSet(m_bitsSaid, bit_saidDamageLight) &&
					(m_hTargetEnt->pev->health <= m_hTargetEnt->pev->max_health / 2))
				{
					//EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, m_szGrp[TLK_PLHURT1], 1.0, ATTN_IDLE, 0, pitch);
					PlaySentence( m_szGrp[TLK_PLHURT1], duration, VOL_NORM, ATTN_IDLE );
					SetBits(m_bitsSaid, bit_saidDamageLight);
					return TRUE;
				}
			}
			else
			{
				//!!!KELLY - here's a cool spot to have the talkmonster talk about the dead player if we want.
				// "Oh dear, Gordon Freeman is dead!" -Scientist
				// "Damn, I can't do this without you." -Barney
			}
		}
	}

	// if there is a friend nearby to speak to, play sentence, set friend's response time, return
	CBaseEntity *pFriend = FindNearestFriend(FALSE);

	if (pFriend && !(pFriend->IsMoving()) && (RANDOM_LONG(0,99) < 75))
	{
		PlaySentence( szQuestionGroup, duration, VOL_NORM, ATTN_IDLE );
		//SENTENCEG_PlayRndSz( ENT(pev), szQuestionGroup, 1.0, ATTN_IDLE, 0, pitch );

		// force friend to answer
		CTalkMonster *pTalkMonster = (CTalkMonster *)pFriend;
		m_hTalkTarget = pFriend;
		pTalkMonster->SetAnswerQuestion( this ); // UNDONE: This is EVIL!!!
		pTalkMonster->m_flStopTalkTime = m_flStopTalkTime;

		m_nSpeak++;
		return TRUE;
	}

	// otherwise, play an idle statement, try to face client when making a statement.
	if ( RANDOM_LONG(0,1) )
	{
		//SENTENCEG_PlayRndSz( ENT(pev), szIdleGroup, 1.0, ATTN_IDLE, 0, pitch );
		CBaseEntity *pFriend = FindNearestFriend(TRUE);

		if ( pFriend )
		{
			m_hTalkTarget = pFriend;
			PlaySentence( szIdleGroup, duration, VOL_NORM, ATTN_IDLE );
			m_nSpeak++;
			return TRUE;
		}
	}

	// didn't speak
	Talk( 0 );
	CTalkMonster::g_talkWaitTime = 0;
	return FALSE;
}

void CTalkMonster::PlayScriptedSentence( const char *pszSentence, float duration, float volume, float attenuation, BOOL bConcurrent, CBaseEntity *pListener )
{
	if ( !bConcurrent )
		ShutUpFriends();

	ClearConditions( bits_COND_CLIENT_PUSH );	// Forget about moving!  I've got something to say!
	m_useTime = gpGlobals->time + duration;
	PlaySentence( pszSentence, duration, volume, attenuation );

	m_hTalkTarget = pListener;
}

void CTalkMonster::PlaySentence( const char *pszSentence, float duration, float volume, float attenuation )
{
	if ( !pszSentence )
		return;

	Talk ( duration );

	CTalkMonster::g_talkWaitTime = gpGlobals->time + duration + 2.0;
	if ( pszSentence[0] == '!' )
		EMIT_SOUND_DYN( edict(), CHAN_VOICE, pszSentence, volume, attenuation, 0, GetVoicePitch());
	else
		SENTENCEG_PlayRndSz( edict(), pszSentence, volume, attenuation, 0, GetVoicePitch() );

	// If you say anything, don't greet the player - you may have already spoken to them
	SetBits(m_bitsSaid, bit_saidHelloPlayer);
}

//=========================================================
// Talk - set a timer that tells us when the monster is done
// talking.
//=========================================================
void CTalkMonster :: Talk( float flDuration )
{
	if ( flDuration <= 0 )
	{
		// no duration :( 
		m_flStopTalkTime = gpGlobals->time + 3;
	}
	else
	{
		m_flStopTalkTime = gpGlobals->time + flDuration;
	}
}

// Prepare this talking monster to answer question
void CTalkMonster :: SetAnswerQuestion( CTalkMonster *pSpeaker )
{
	if ( !m_pCine )
		ChangeSchedule( slIdleResponse );
	m_hTalkTarget = (CBaseMonster *)pSpeaker;
}

int CTalkMonster :: TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	if ( IsAlive() )
	{
		// if player damaged this entity, have other friends talk about it
		if (pevAttacker && m_MonsterState != MONSTERSTATE_PRONE && FBitSet(pevAttacker->flags, FL_CLIENT))
		{
			CBaseEntity *pFriend = FindNearestFriend(FALSE);

			if (pFriend && pFriend->IsAlive())
			{
				// only if not dead or dying!
				CTalkMonster *pTalkMonster = (CTalkMonster *)pFriend;
				pTalkMonster->ChangeSchedule( slIdleStopShooting );
			}
		}
	}
	return CBaseMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}


Schedule_t* CTalkMonster :: GetScheduleOfType ( int Type )
{
	switch( Type )
	{
	case SCHED_MOVE_AWAY:
		return slMoveAway;

	case SCHED_MOVE_AWAY_FOLLOW:
		return slMoveAwayFollow;

	case SCHED_MOVE_AWAY_FAIL:
		return slMoveAwayFail;

	case SCHED_TARGET_FACE:
		// speak during 'use'
		if (RANDOM_LONG(0,99) < 2)
			//ALERT ( at_console, "target chase speak\n" );
			return slIdleSpeakWait;
		else
			return slIdleStand;
		
	case SCHED_IDLE_STAND:
		{	
			// if never seen player, try to greet him
			if (!FBitSet(m_bitsSaid, bit_saidHelloPlayer))
			{
				return slIdleHello;
			}

			// sustained light wounds?
			if (!FBitSet(m_bitsSaid, bit_saidWoundLight) && (pev->health <= (pev->max_health * 0.75)))
			{
				//SENTENCEG_PlayRndSz( ENT(pev), m_szGrp[TLK_WOUND], 1.0, ATTN_IDLE, 0, GetVoicePitch() );
				//CTalkMonster::g_talkWaitTime = gpGlobals->time + RANDOM_FLOAT(2.8, 3.2);
				PlaySentence( m_szGrp[TLK_WOUND], RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_IDLE );
				SetBits(m_bitsSaid, bit_saidWoundLight);
				return slIdleStand;
			}
			// sustained heavy wounds?
			else if (!FBitSet(m_bitsSaid, bit_saidWoundHeavy) && (pev->health <= (pev->max_health * 0.5)))
			{
				//SENTENCEG_PlayRndSz( ENT(pev), m_szGrp[TLK_MORTAL], 1.0, ATTN_IDLE, 0, GetVoicePitch() );
				//CTalkMonster::g_talkWaitTime = gpGlobals->time + RANDOM_FLOAT(2.8, 3.2);
				PlaySentence( m_szGrp[TLK_MORTAL], RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_IDLE );
				SetBits(m_bitsSaid, bit_saidWoundHeavy);
				return slIdleStand;
			}

			// talk about world
			if (FOkToSpeak() && RANDOM_LONG(0,m_nSpeak * 2) == 0)
			{
				//ALERT ( at_console, "standing idle speak\n" );
				return slIdleSpeak;
			}
			
			if ( !IsTalking() && HasConditions ( bits_COND_SEE_CLIENT ) && RANDOM_LONG( 0, 6 ) == 0 )
			{
				edict_t *pPlayer = g_engfuncs.pfnPEntityOfEntIndex( 1 );

				if ( pPlayer )
				{
					// watch the client.
					UTIL_MakeVectors ( pPlayer->v.angles );
					if ( ( pPlayer->v.origin - pev->origin ).Length2D() < TLK_STARE_DIST	&& 
						 UTIL_DotPoints( pPlayer->v.origin, pev->origin, gpGlobals->v_forward ) >= m_flFieldOfView )
					{
						// go into the special STARE schedule if the player is close, and looking at me too.
						return &slTlkIdleWatchClient[ 1 ];
					}

					return slTlkIdleWatchClient;
				}
			}
			else
			{
				if (IsTalking())
					// look at who we're talking to
					return slTlkIdleEyecontact;
				else
					// regular standing idle
					return slIdleStand;
			}


			// NOTE - caller must first CTalkMonster::GetScheduleOfType, 
			// then check result and decide what to return ie: if sci gets back
			// slIdleStand, return slIdleSciStand
		}
		break;
	}

	return CBaseMonster::GetScheduleOfType( Type );
}

//=========================================================
// IsTalking - am I saying a sentence right now?
//=========================================================
BOOL CTalkMonster :: IsTalking( void )
{
	if ( m_flStopTalkTime > gpGlobals->time )
	{
		return TRUE;
	}

	return FALSE;
}

//=========================================================
// If there's a player around, watch him.
//=========================================================
void CTalkMonster :: PrescheduleThink ( void )
{
	if ( !HasConditions ( bits_COND_SEE_CLIENT ) )
	{
		SetConditions ( bits_COND_CLIENT_UNSEEN );
	}
}

// try to smell something
void CTalkMonster :: TrySmellTalk( void )
{
	if ( !FOkToSpeak() )
		return;

	// clear smell bits periodically
	if ( gpGlobals->time > m_flLastSaidSmelled  )
	{
//		ALERT ( at_aiconsole, "Clear smell bits\n" );
		ClearBits(m_bitsSaid, bit_saidSmelled);
	}
	// smelled something?
	if (!FBitSet(m_bitsSaid, bit_saidSmelled) && HasConditions ( bits_COND_SMELL ))
	{
		PlaySentence( m_szGrp[TLK_SMELL], RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_IDLE );
		m_flLastSaidSmelled = gpGlobals->time + 60;// don't talk about the stinky for a while.
		SetBits(m_bitsSaid, bit_saidSmelled);
	}
}



int CTalkMonster::IRelationship( CBaseEntity *pTarget )
{
	if ( pTarget->IsPlayer() )
		if ( m_afMemory & bits_MEMORY_PROVOKED )
			return R_HT;
	return CBaseMonster::IRelationship( pTarget );
}


void CTalkMonster::StopFollowing( BOOL clearSchedule )
{
	if ( IsFollowing() )
	{
		if ( !(m_afMemory & bits_MEMORY_PROVOKED) )
		{
			PlaySentence( m_szGrp[TLK_UNUSE], RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_IDLE );
			m_hTalkTarget = m_hTargetEnt;
		}

		if ( m_movementGoal == MOVEGOAL_TARGETENT )
			RouteClear(); // Stop him from walking toward the player
		m_hTargetEnt = NULL;
		if ( clearSchedule )
			ClearSchedule();
		if ( m_hEnemy != NULL )
			m_IdealMonsterState = MONSTERSTATE_COMBAT;
	}
}


void CTalkMonster::StartFollowing( CBaseEntity *pLeader )
{
	if ( m_pCine )
		m_pCine->CancelScript();

	if ( m_hEnemy != NULL )
		m_IdealMonsterState = MONSTERSTATE_ALERT;

	m_hTargetEnt = pLeader;
	PlaySentence( m_szGrp[TLK_USE], RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_IDLE );
	m_hTalkTarget = m_hTargetEnt;
	ClearConditions( bits_COND_CLIENT_PUSH );
	ClearSchedule();
}


BOOL CTalkMonster::CanFollow( void )
{
	if ( m_MonsterState == MONSTERSTATE_SCRIPT )
	{
		if ( !m_pCine->CanInterrupt() )
			return FALSE;
	}
	
	if ( !IsAlive() )
		return FALSE;

	return !IsFollowing();
}


void CTalkMonster :: FollowerUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	// Don't allow use during a scripted_sentence
	if ( m_useTime > gpGlobals->time )
		return;

	if ( pCaller != NULL && pCaller->IsPlayer() )
	{
		// Pre-disaster followers can't be used
		if ( pev->spawnflags & SF_MONSTER_PREDISASTER )
		{
			DeclineFollowing();
		}
		else if ( CanFollow() )
		{
			LimitFollowers( pCaller , 1 );

			if ( m_afMemory & bits_MEMORY_PROVOKED )
				ALERT( at_console, "I'm not following you, you evil person!\n" );
			else
			{
				StartFollowing( pCaller );
				SetBits(m_bitsSaid, bit_saidHelloPlayer);	// Don't say hi after you've started following
			}
		}
		else
		{
			StopFollowing( TRUE );
		}
	}
}

void CTalkMonster::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "UseSentence"))
	{
		m_iszUse = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "UnUseSentence"))
	{
		m_iszUnUse = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else 
		CBaseMonster::KeyValue( pkvd );
}


void CTalkMonster::Precache( void )
{
	if ( m_iszUse )
		m_szGrp[TLK_USE] = STRING( m_iszUse );
	if ( m_iszUnUse )
		m_szGrp[TLK_UNUSE] = STRING( m_iszUnUse );
}

