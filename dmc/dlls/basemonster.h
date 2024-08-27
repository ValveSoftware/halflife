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

#ifndef BASEMONSTER_H
#define BASEMONSTER_H

//
// generic Monster
//
class CBaseMonster : public CBaseToggle
{
private:
		int					m_afConditions;

public:
		typedef enum
		{
			SCRIPT_PLAYING = 0,		// Playing the sequence
			SCRIPT_WAIT,				// Waiting on everyone in the script to be ready
			SCRIPT_CLEANUP,					// Cancelling the script / cleaning up
			SCRIPT_WALK_TO_MARK,
			SCRIPT_RUN_TO_MARK,
		} SCRIPTSTATE;


	
		// these fields have been added in the process of reworking the state machine. (sjb)
		EHANDLE				m_hEnemy;		 // the entity that the monster is fighting.
		EHANDLE				m_hTargetEnt;	 // the entity that the monster is trying to reach
		EHANDLE				m_hOldEnemy[ MAX_OLD_ENEMIES ];
		Vector				m_vecOldEnemy[ MAX_OLD_ENEMIES ];

		float				m_flFieldOfView;// width of monster's field of view ( dot product )
		float				m_flWaitFinished;// if we're told to wait, this is the time that the wait will be over.
		float				m_flMoveWaitFinished;

		Activity			m_Activity;// what the monster is doing (animation)
		Activity			m_IdealActivity;// monster should switch to this activity
		
		int					m_LastHitGroup; // the last body region that took damage
		
		MONSTERSTATE		m_MonsterState;// monster's current state
		MONSTERSTATE		m_IdealMonsterState;// monster should change to this state
	
		int					m_iTaskStatus;
		Schedule_t			*m_pSchedule;
		int					m_iScheduleIndex;

		WayPoint_t			m_Route[ ROUTE_SIZE ];	// Positions of movement
		int					m_movementGoal;			// Goal that defines route
		int					m_iRouteIndex;			// index into m_Route[]
		float				m_moveWaitTime;			// How long I should wait for something to move

		Vector				m_vecMoveGoal; // kept around for node graph moves, so we know our ultimate goal
		Activity			m_movementActivity;	// When moving, set this activity

		int					m_iAudibleList; // first index of a linked list of sounds that the monster can hear.
		int					m_afSoundTypes;

		Vector				m_vecLastPosition;// monster sometimes wants to return to where it started after an operation.

		int					m_iHintNode; // this is the hint node that the monster is moving towards or performing active idle on.

		int					m_afMemory;

		int					m_iMaxHealth;// keeps track of monster's maximum health value (for re-healing, etc)

	Vector				m_vecEnemyLKP;// last known position of enemy. (enemy's origin)

	int					m_cAmmoLoaded;		// how much ammo is in the weapon (used to trigger reload anim sequences)

	int					m_afCapability;// tells us what a monster can/can't do.

	float				m_flNextAttack;		// cannot attack again until this time

	int					m_bitsDamageType;	// what types of damage has monster (player) taken
	BYTE				m_rgbTimeBasedDamage[CDMG_TIMEBASED];

	int					m_lastDamageAmount;// how much damage did monster (player) last take
											// time based damage counters, decr. 1 per 2 seconds
	int					m_bloodColor;		// color of blood particless

	int					m_failSchedule;				// Schedule type to choose if current schedule fails

	float				m_flHungryTime;// set this is a future time to stop the monster from eating for a while. 

	float				m_flDistTooFar;	// if enemy farther away than this, bits_COND_ENEMY_TOOFAR set in CheckEnemy
	float				m_flDistLook;	// distance monster sees (Default 2048)

	int					m_iTriggerCondition;// for scripted AI, this is the condition that will cause the activation of the monster's TriggerTarget
	string_t			m_iszTriggerTarget;// name of target that should be fired. 

	Vector				m_HackedGunPos;	// HACK until we can query end of gun

// Scripted sequence Info
	SCRIPTSTATE			m_scriptState;		// internal cinematic state
	CCineMonster		*m_pCine;

	float 				m_flLastYawTime;	// Last time yaw change was computed

	virtual int		Save( CSave &save ); 
	virtual int		Restore( CRestore &restore );
	
	static	TYPEDESCRIPTION m_SaveData[];

	void KeyValue( KeyValueData *pkvd );

// monster use function
	void EXPORT			MonsterUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT			CorpseUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

// overrideable Monster member functions
	
	virtual int	 BloodColor( void ) { return m_bloodColor; }

	virtual CBaseMonster *MyMonsterPointer( void ) { return this; }
	virtual void Look ( int iDistance );// basic sight function for monsters
	virtual void RunAI ( void );// core ai function!	
	void Listen ( void );

	virtual BOOL	IsAlive( void ) { return (pev->deadflag != DEAD_DEAD); }
	virtual BOOL	ShouldFadeOnDeath( void );

// Basic Monster AI functions
	virtual float ChangeYaw ( int speed );
	float VecToYaw( Vector vecDir );
	float FlYawDiff ( void ); 

	float DamageForce( float damage );

// stuff written for new state machine
		virtual void MonsterThink( void );
		void EXPORT	CallMonsterThink( void ) { this->MonsterThink(); }
		virtual int IRelationship ( CBaseEntity *pTarget );
		virtual void MonsterInit ( void );
		virtual void MonsterInitDead( void );	// Call after animation/pose is set up
		virtual void BecomeDead( void );
		void EXPORT CorpseFallThink( void );

		void EXPORT MonsterInitThink ( void );
		virtual void StartMonster ( void );
		virtual CBaseEntity* BestVisibleEnemy ( void );// finds best visible enemy for attack
		virtual BOOL FInViewCone ( CBaseEntity *pEntity );// see if pEntity is in monster's view cone
		virtual BOOL FInViewCone ( Vector *pOrigin );// see if given location is in monster's view cone
		virtual void HandleAnimEvent( MonsterEvent_t *pEvent );

		virtual int CheckLocalMove ( const Vector &vecStart, const Vector &vecEnd, CBaseEntity *pTarget, float *pflDist );// check validity of a straight move through space
		virtual void Move( float flInterval = 0.1 );
		virtual void MoveExecute( CBaseEntity *pTargetEnt, const Vector &vecDir, float flInterval );
		virtual BOOL ShouldAdvanceRoute( float flWaypointDist );

		virtual Activity GetStoppedActivity( void ) { return ACT_IDLE; }
		virtual void Stop( void ) { m_IdealActivity = GetStoppedActivity(); }

		// This will stop animation until you call ResetSequenceInfo() at some point in the future
		inline void StopAnimation( void ) { pev->framerate = 0; }

		// these functions will survey conditions and set appropriate conditions bits for attack types.
		virtual BOOL CheckRangeAttack1( float flDot, float flDist );
		virtual BOOL CheckRangeAttack2( float flDot, float flDist );
		virtual BOOL CheckMeleeAttack1( float flDot, float flDist );
		virtual BOOL CheckMeleeAttack2( float flDot, float flDist );

		BOOL FHaveSchedule( void );
		BOOL FScheduleValid ( void );
		void ClearSchedule( void );
		BOOL FScheduleDone ( void );
		void ChangeSchedule ( Schedule_t *pNewSchedule );
		void NextScheduledTask ( void );
		Schedule_t *ScheduleInList( const char *pName, Schedule_t **pList, int listCount );

		virtual Schedule_t *ScheduleFromName( const char *pName );
		static Schedule_t *m_scheduleList[];
		
		void MaintainSchedule ( void );
		virtual void StartTask ( Task_t *pTask );
		virtual void RunTask ( Task_t *pTask );
		virtual Schedule_t *GetScheduleOfType( int Type );
		virtual Schedule_t *GetSchedule( void );
		virtual void ScheduleChange( void ) {}
		// virtual int CanPlaySequence( void ) { return ((m_pCine == NULL) && (m_MonsterState == MONSTERSTATE_NONE || m_MonsterState == MONSTERSTATE_IDLE || m_IdealMonsterState == MONSTERSTATE_IDLE)); }
		virtual int CanPlaySequence( BOOL fDisregardState, int interruptLevel );
		virtual int CanPlaySentence( BOOL fDisregardState ) { return IsAlive(); }
		virtual void PlaySentence( const char *pszSentence, float duration, float volume, float attenuation );
		virtual void PlayScriptedSentence( const char *pszSentence, float duration, float volume, float attenuation, BOOL bConcurrent, CBaseEntity *pListener );

		virtual void SentenceStop( void );

		Task_t *GetTask ( void );
		virtual MONSTERSTATE GetIdealState ( void );
		virtual void SetActivity ( Activity NewActivity );
		void SetSequenceByName ( char *szSequence );
		void SetState ( MONSTERSTATE State );
		virtual void ReportAIState( void );

		void CheckAttacks ( CBaseEntity *pTarget, float flDist );
		virtual int CheckEnemy ( CBaseEntity *pEnemy );
		void PushEnemy( CBaseEntity *pEnemy, Vector &vecLastKnownPos );
		BOOL PopEnemy( void );

		BOOL FGetNodeRoute ( Vector vecDest );
		
		inline void TaskComplete( void ) { if ( !HasConditions(bits_COND_TASK_FAILED) ) m_iTaskStatus = TASKSTATUS_COMPLETE; }
		void MovementComplete( void );
		inline void TaskFail( void ) { SetConditions(bits_COND_TASK_FAILED); }
		inline void TaskBegin( void ) { m_iTaskStatus = TASKSTATUS_RUNNING; }
		int TaskIsRunning( void );
		inline int TaskIsComplete( void ) { return (m_iTaskStatus == TASKSTATUS_COMPLETE); }
		inline int MovementIsComplete( void ) { return (m_movementGoal == MOVEGOAL_NONE); }

		int IScheduleFlags ( void );
		BOOL FRefreshRoute( void );
		BOOL FRouteClear ( void );
		void RouteSimplify( CBaseEntity *pTargetEnt );
		void AdvanceRoute ( float distance );
		virtual BOOL FTriangulate ( const Vector &vecStart , const Vector &vecEnd, float flDist, CBaseEntity *pTargetEnt, Vector *pApex );
		void MakeIdealYaw( Vector vecTarget );
		virtual void SetYawSpeed ( void ) { return; };// allows different yaw_speeds for each activity
		BOOL BuildRoute ( const Vector &vecGoal, int iMoveFlag, CBaseEntity *pTarget );
		virtual BOOL BuildNearestRoute ( Vector vecThreat, Vector vecViewOffset, float flMinDist, float flMaxDist );
		int RouteClassify( int iMoveFlag );
		void InsertWaypoint ( Vector vecLocation, int afMoveFlags );
		
		BOOL FindLateralCover ( const Vector &vecThreat, const Vector &vecViewOffset );
		virtual BOOL FindCover ( Vector vecThreat, Vector vecViewOffset, float flMinDist, float flMaxDist );
		virtual BOOL FValidateCover ( const Vector &vecCoverLocation ) { return TRUE; };
		virtual float CoverRadius( void ) { return 784; } // Default cover radius

		virtual BOOL FCanCheckAttacks ( void );
		virtual void CheckAmmo( void ) { return; };
		virtual int IgnoreConditions ( void );
		
		inline void	SetConditions( int iConditions ) { m_afConditions |= iConditions; }
		inline void	ClearConditions( int iConditions ) { m_afConditions &= ~iConditions; }
		inline BOOL HasConditions( int iConditions ) { if ( m_afConditions & iConditions ) return TRUE; return FALSE; }
		inline BOOL HasAllConditions( int iConditions ) { if ( (m_afConditions & iConditions) == iConditions ) return TRUE; return FALSE; }

		virtual BOOL FValidateHintType( short sHint );
		int FindHintNode ( void );
		virtual BOOL FCanActiveIdle ( void );
		void SetTurnActivity ( void );
		float FLSoundVolume ( CSound *pSound );

		BOOL MoveToNode( Activity movementAct, float waitTime, const Vector &goal );
		BOOL MoveToTarget( Activity movementAct, float waitTime );
		BOOL MoveToLocation( Activity movementAct, float waitTime, const Vector &goal );
		BOOL MoveToEnemy( Activity movementAct, float waitTime );

		// Returns the time when the door will be open
		float	OpenDoorAndWait( entvars_t *pevDoor );

		virtual int ISoundMask( void );
		virtual CSound* PBestSound ( void );
		virtual CSound* PBestScent ( void );
		virtual float HearingSensitivity( void ) { return 1.0; };

		BOOL FBecomeProne ( void );
		virtual void BarnacleVictimBitten( entvars_t *pevBarnacle );
		virtual void BarnacleVictimReleased( void );

		void SetEyePosition ( void );

		BOOL FShouldEat( void );// see if a monster is 'hungry'
		void Eat ( float flFullDuration );// make the monster 'full' for a while.

		CBaseEntity *CheckTraceHullAttack( float flDist, int iDamage, int iDmgType );
		BOOL FacingIdeal( void );

		BOOL FCheckAITrigger( void );// checks and, if necessary, fires the monster's trigger target. 
		BOOL NoFriendlyFire( void );

		BOOL BBoxFlat( void );

		// PrescheduleThink 
		virtual void PrescheduleThink( void ) { return; };

		BOOL GetEnemy ( void );
		void MakeDamageBloodDecal ( int cCount, float flNoise, TraceResult *ptr, const Vector &vecDir );
		void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);

	// combat functions
	float UpdateTarget ( entvars_t *pevTarget );
	virtual Activity GetDeathActivity ( void );
	Activity GetSmallFlinchActivity( void );
	virtual void Killed( entvars_t *pevAttacker, int iGib );
	virtual void GibMonster( void );
	BOOL		 ShouldGibMonster( int iGib );
	void		 CallGibMonster( void );
	virtual BOOL	HasHumanGibs( void );
	virtual BOOL	HasAlienGibs( void );
	virtual void	FadeMonster( void );	// Called instead of GibMonster() when gibs are disabled

	Vector ShootAtEnemy( const Vector &shootOrigin );
	virtual Vector BodyTarget( const Vector &posSrc ) { return Center( ) * 0.75 + EyePosition() * 0.25; };		// position to shoot at

	virtual	Vector  GetGunPosition( void );

	virtual int TakeHealth( float flHealth, int bitsDamageType );
	virtual int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
	int			DeadTakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );

	void RadiusDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType );
	void RadiusDamage(Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType );
	virtual int		IsMoving( void ) { return m_movementGoal != MOVEGOAL_NONE; }

	void RouteClear( void );
	void RouteNew( void );
	
	virtual void DeathSound ( void ) { return; };
	virtual void AlertSound ( void ) { return; };
	virtual void IdleSound ( void ) { return; };
	virtual void PainSound ( void ) { return; };
	
	virtual void StopFollowing( BOOL clearSchedule ) {}

	inline void	Remember( int iMemory ) { m_afMemory |= iMemory; }
	inline void	Forget( int iMemory ) { m_afMemory &= ~iMemory; }
	inline BOOL HasMemory( int iMemory ) { if ( m_afMemory & iMemory ) return TRUE; return FALSE; }
	inline BOOL HasAllMemories( int iMemory ) { if ( (m_afMemory & iMemory) == iMemory ) return TRUE; return FALSE; }

	BOOL ExitScriptedSequence( );
	BOOL CineCleanup( );

	CBaseEntity* DropItem ( char *pszItemName, const Vector &vecPos, const Vector &vecAng );// drop an item.
};



#endif // BASEMONSTER_H
