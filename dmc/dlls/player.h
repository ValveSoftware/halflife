/***
*
*	Copyright (c) 1999, Valve LLC. All rights reserved.
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
#ifndef PLAYER_H
#define PLAYER_H

#define PLAYER_FATAL_FALL_SPEED		1024// approx 60 feet
#define PLAYER_MAX_SAFE_FALL_SPEED	580// approx 20 feet
#define DAMAGE_FOR_FALL_SPEED		(float) 100 / ( PLAYER_FATAL_FALL_SPEED - PLAYER_MAX_SAFE_FALL_SPEED )// damage per unit per second.
#define PLAYER_MIN_BOUNCE_SPEED		200
#define PLAYER_FALL_PUNCH_THRESHHOLD (float)350 // won't punch player's screen/make scrape noise unless player falling at least this fast.

//
// Player PHYSICS FLAGS bits
//
#define		PFLAG_ONLADDER		( 1<<0 )
#define		PFLAG_ONSWING		( 1<<0 )
#define		PFLAG_ONTRAIN		( 1<<1 )
#define		PFLAG_ONBARNACLE	( 1<<2 )
#define		PFLAG_DUCKING		( 1<<3 )		// In the process of ducking, but totally squatted yet
#define		PFLAG_USING			( 1<<4 )		// Using a continuous entity
#define		PFLAG_OBSERVER		( 1<<5 )		// player is locked in stationary cam mode. Spectators can move, observers can't.

//
// generic player
//
//-----------------------------------------------------
//This is Half-Life player entity
//-----------------------------------------------------
#define CSUITPLAYLIST	4		// max of 4 suit sentences queued up at any time

#define SUIT_GROUP			TRUE
#define	SUIT_SENTENCE		FALSE

#define	SUIT_REPEAT_OK		0
#define SUIT_NEXT_IN_30SEC	30
#define SUIT_NEXT_IN_1MIN	60
#define SUIT_NEXT_IN_5MIN	300
#define SUIT_NEXT_IN_10MIN	600
#define SUIT_NEXT_IN_30MIN	1800
#define SUIT_NEXT_IN_1HOUR	3600

#define CSUITNOREPEAT		32

#define	SOUND_FLASHLIGHT_ON		"items/flashlight1.wav"
#define	SOUND_FLASHLIGHT_OFF	"items/flashlight1.wav"

#define TEAM_NAME_LENGTH	16

typedef enum
{
	PLAYER_IDLE,
	PLAYER_WALK,
	PLAYER_JUMP,
	PLAYER_SUPERJUMP,
	PLAYER_DIE,
	PLAYER_ATTACK1,
} PLAYER_ANIM;

#ifdef THREEWAVE
enum Player_Menu {
    Team_Menu,
	Team_Menu_IG,
};
#endif

#define MAX_ID_RANGE 2048
#define SBAR_STRING_SIZE 128
enum sbar_data
{
SBAR_ID_TARGETNAME = 1,
SBAR_ID_TARGETHEALTH,
SBAR_ID_TARGETARMOR,
SBAR_ID_TARGETRUNE,
SBAR_ID_TARGETTEAM,
SBAR_END,
};

#define PLAYER_MAX_SPEED 300

class CBasePlayer : public CBaseMonster
{
public:
	int					random_seed;    // See that is shared between client & server for shared weapons code

	int					m_iPlayerSound;// the index of the sound list slot reserved for this player
	int					m_iTargetVolume;// ideal sound volume. 
	int					m_iWeaponVolume;// how loud the player's weapon is right now.
	int					m_iExtraSoundTypes;// additional classification for this weapon's sound
	int					m_iWeaponFlash;// brightness of the weapon flash
	float				m_flStopExtraSoundTime;
	
	float				m_flFlashLightTime;	// Time until next battery draw/Recharge
	int					m_iFlashBattery;		// Flashlight Battery Draw

	int					m_afButtonLast;
	int					m_afButtonPressed;
	int					m_afButtonReleased;
	
	edict_t			   *m_pentSndLast;			// last sound entity to modify player room type
	float				m_flSndRoomtype;		// last roomtype set by sound entity
	float				m_flSndRange;			// dist from player to sound entity

	float				m_flFallVelocity;
	
	int					m_rgItems[MAX_ITEMS];
	int					m_fKnownItem;		// True when a new item needs to be added
	int					m_fNewAmmo;			// True when a new item has been added

	unsigned int		m_afPhysicsFlags;	// physics flags - set when 'normal' physics should be revisited or overriden
	float				m_fNextSuicideTime; // the time after which the player can next use the suicide command


// these are time-sensitive things that we keep track of
	float				m_flTimeStepSound;	// when the last stepping sound was made
	float				m_flTimeWeaponIdle; // when to play another weapon idle animation.
	float				m_flSwimTime;		// how long player has been underwater
	float				m_flDuckTime;		// how long we've been ducking
	float				m_flWallJumpTime;	// how long until next walljump

	float				m_flSuitUpdate;					// when to play next suit update
	int					m_rgSuitPlayList[CSUITPLAYLIST];// next sentencenum to play for suit update
	int					m_iSuitPlayNext;				// next sentence slot for queue storage;
	int					m_rgiSuitNoRepeat[CSUITNOREPEAT];		// suit sentence no repeat list
	float				m_rgflSuitNoRepeatTime[CSUITNOREPEAT];	// how long to wait before allowing repeat
	int					m_lastDamageAmount;		// Last damage taken
	float				m_tbdPrev;				// Time-based damage timer

	float				m_flgeigerRange;		// range to nearest radiation source
	float				m_flgeigerDelay;		// delay per update of range msg to client
	int					m_igeigerRangePrev;
	int					m_iStepLeft;			// alternate left/right foot stepping sound
	char				m_szTextureName[CBTEXTURENAMEMAX];	// current texture name we're standing on
	char				m_chTextureType;		// current texture type

	int					m_idrowndmg;			// track drowning damage taken
	int					m_idrownrestored;		// track drowning damage restored

	int					m_bitsHUDDamage;		// Damage bits for the current fame. These get sent to 
												// the hude via the DAMAGE message
	BOOL				m_fInitHUD;				// True when deferred HUD restart msg needs to be sent
	BOOL				m_fGameHUDInitialized;
	int					m_iTrain;				// Train control position
	BOOL				m_fWeapon;				// Set this to FALSE to force a reset of the current weapon HUD info

	EHANDLE				m_pTank;				// the tank which the player is currently controlling,  NULL if no tank
	float				m_fDeadTime;			// the time at which the player died  (used in PlayerDeathThink())

	BOOL			m_fNoPlayerSound;	// a debugging feature. Player makes no sound if this is true. 
	BOOL			m_fLongJump; // does this player have the longjump module?

	float       m_tSneaking;
	int			m_iUpdateTime;		// stores the number of frame ticks before sending HUD update messages
	int			m_iClientHealth;	// the health currently known by the client.  If this changes, send a new
	int			m_iClientBattery;	// the Battery currently known by the client.  If this changes, send a new
	int			m_iHideHUD;		// the players hud weapon info is to be hidden
	int			m_iClientHideHUD;
	int			m_iFOV;			// field of view
	int			m_iClientFOV;	// client's known FOV
	// usable player items 
	CBasePlayerItem	*m_rgpPlayerItems[MAX_ITEM_TYPES];
	CBasePlayerItem *m_pActiveItem;
	CBasePlayerItem *m_pClientActiveItem;  // client version of the active item
	CBasePlayerItem *m_pLastItem;
	// shared ammo slots
	int	m_rgAmmo[MAX_AMMO_SLOTS];
	int	m_rgAmmoLast[MAX_AMMO_SLOTS];

	Vector				m_vecAutoAim;
	BOOL				m_fOnTarget;
	int					m_iDeaths;
	float				m_iRespawnFrames;	// used in PlayerDeathThink() to make sure players can always respawn

	int m_lastx, m_lasty;  // These are the previous update's crosshair angles, DON"T SAVE/RESTORE

	int m_nCustomSprayFrames;// Custom clan logo frames for this player
	float	m_flNextDecalTime;// next time this player can spray a decal

	char m_szTeamName[TEAM_NAME_LENGTH];

	virtual void Spawn( void );

//	virtual void Think( void );
	virtual void Jump( void );
	virtual void Duck( void );
	virtual void PreThink( void );
	virtual void PostThink( void );
	virtual Vector GetGunPosition( void );
	virtual int TakeHealth( float flHealth, int bitsDamageType );
	virtual void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);
	virtual int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
	virtual void	Killed( entvars_t *pevAttacker, int iGib );
	virtual Vector BodyTarget( const Vector &posSrc ) { return Center( ) + pev->view_ofs * RANDOM_FLOAT( 0.5, 1.1 ); };		// position to shoot at
	virtual void StartSneaking( void ) { m_tSneaking = gpGlobals->time - 1; }
	virtual void StopSneaking( void ) { m_tSneaking = gpGlobals->time + 30; }
	virtual BOOL IsSneaking( void ) { return m_tSneaking <= gpGlobals->time; }
	virtual BOOL IsAlive( void ) { return (pev->deadflag == DEAD_NO) && pev->health > 0; }
	virtual BOOL ShouldFadeOnDeath( void ) { return FALSE; }
	virtual	BOOL IsPlayer( void ) { return TRUE; }			// Spectators should return FALSE for this, they aren't "players" as far as game logic is concerned

	virtual BOOL IsNetClient( void ) { return TRUE; }		// Bots should return FALSE for this, they can't receive NET messages
															// Spectators should return TRUE for this
	virtual const char *TeamID( void );

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	void RenewItems(void);
	void RemoveAllItems( BOOL removeSuit );
	BOOL SwitchWeapon( CBasePlayerItem *pWeapon );

	// JOHN:  sends custom messages if player HUD data has changed  (eg health, ammo)
	virtual void UpdateClientData( void );
	
	static	TYPEDESCRIPTION m_playerSaveData[];

	// Player is moved across the transition by other means
	virtual int		ObjectCaps( void ) { return CBaseMonster :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	virtual void	Precache( void );
	BOOL			IsOnLadder( void );
	BOOL			FlashlightIsOn( void );
	void			FlashlightTurnOn( void );
	void			FlashlightTurnOff( void );
	
	void DeathSound ( void );

	int Classify ( void );
	void SetAnimation( PLAYER_ANIM playerAnim );
	void SetWeaponAnimType( const char *szExtention );
	char m_szAnimExtention[32];

	// custom player functions
	virtual void ImpulseCommands( void );
	void CheatImpulseCommands( int iImpulse );

	void StartDeathCam( void );
	void StartObserver( Vector vecPosition, Vector vecViewAngle );

	void AddPoints( int score, BOOL bAllowNegativeScore );
	void AddPointsToTeam( int score, BOOL bAllowNegativeScore );
	BOOL AddPlayerItem( CBasePlayerItem *pItem );
	BOOL RemovePlayerItem( CBasePlayerItem *pItem );
	void DropPlayerItem ( char *pszItemName );
	BOOL HasPlayerItem( CBasePlayerItem *pCheckItem );
	BOOL HasNamedPlayerItem( const char *pszItemName );
	BOOL HasWeapons( void );// do I have ANY weapons?
	void SelectPrevItem( int iItem );
	void SelectNextItem( int iItem );
	void SelectLastItem(void);
	void SelectItem(const char *pstr);
	void ItemPreFrame( void );
	void ItemPostFrame( void );
	void GiveNamedItem( const char *szName );
	void EnableControl(BOOL fControl);

	int  GiveAmmo( int iAmount, char *szName, int iMax );
	void SendAmmoUpdate(void);

	void WaterMove( void );
	void EXPORT PlayerDeathThink( void );
	void PlayerUse( void );

	void CheckSuitUpdate();
	void SetSuitUpdate(char *name, int fgroup, int iNoRepeat);
	void UpdateGeigerCounter( void );
	void CheckTimeBasedDamage( void );
	void UpdateStepSound( void );
	void PlayStepSound(int step, float fvol);

	BOOL FBecomeProne ( void );
	void BarnacleVictimBitten ( entvars_t *pevBarnacle );
	void BarnacleVictimReleased ( void );
	static int GetAmmoIndex(const char *psz);
	int AmmoInventory( int iAmmoIndex );
	int Illumination( void );

	void ResetAutoaim( void );
	Vector GetAutoaimVector( float flDelta  );
	Vector AutoaimDeflection( Vector &vecSrc, float flDist, float flDelta  );

	void ForceClientDllUpdate( void );  // Forces all client .dll specific data to be resent to client.

	void DeathMessage( entvars_t *pevKiller );

	void SetCustomDecalFrames( int nFrames );
	int GetCustomDecalFrames( void );

	// Observer camera
	void	Observer_FindNextPlayer();
	void	Observer_HandleButtons();
	void	Observer_SetMode( int iMode );
	EHANDLE	m_hObserverTarget;
	float	m_flNextObserverInput;
	int		IsObserver() { return pev->iuser1; };


	// QUAKECLASSIC
	// Player
	void	Pain( CBaseEntity *pAttacker );
	float	m_flPainSoundFinished;

	BOOL	m_bHadFirstSpawn;	// used to handle the MOTD

	// Weapon selection
	int		W_BestWeapon( void );
	void	W_SetCurrentAmmo( int sendanim = 1 );
	BOOL	W_CheckNoAmmo( void );
	void	W_ChangeWeapon( int iWeaponNumber );
	void	W_CycleWeaponCommand( void );
	void	W_CycleWeaponReverseCommand( void );

	// Weapon functionality
	void	Q_FireBullets(int iShots, Vector vecDir, Vector vecSpread);
	void	LightningDamage( Vector p1, Vector p2, CBaseEntity *pAttacker, float flDamage,Vector vecDir);

	// Weapons
	void	W_Attack( int iQuadSound );
	void	W_FireAxe( void );
	void	W_FireShotgun( int QuadSound );
	void	W_FireSuperShotgun( int QuadSound );
	void	W_FireRocket( int QuadSound );
	void	W_FireLightning( int QuadSound );
	void	W_FireGrenade( int QuadSound );
	void	W_FireSuperSpikes( int QuadSound );
	void	W_FireSpikes( int QuadSound );

	// Ammunition
	void	CheckAmmo( void );
	int		*m_pCurrentAmmo;		// Always points to one of the four ammo counts below
	int		m_iAmmoRockets;
	int		m_iAmmoCells;
	int		m_iAmmoShells;
	int		m_iAmmoNails;

	// Backpacks
	void	DropBackpack( void );

	// Weapons
	void	Deathmatch_Weapon(int iOldWeapon, int iNewWeapon);
	int		m_iQuakeWeapon;
	int		m_iClientQuakeWeapon;	// The last status of the m_iQuakeWeapon sent to the client.
	int		m_iQuakeItems;
	int		m_iClientQuakeItems;	// The last status of the m_iQuakeItems sent to the client.
	int		m_iWeaponSwitch;
	int		m_iBackpackSwitch;
	int		m_iAutoWepSwitch;

	// Weapon Data
	float	m_flAxeFire;
	float	m_flLightningTime;
	int		m_iNailOffset;
	float	m_flNextQuadSound;

	// Powerups
	float	m_flSuperDamageFinished;
	float	m_flInvincibleFinished;
	float	m_flInvisibleFinished;
	float	m_flRadsuitFinished;
	void	PowerUpThink( void ); //Checks powerup timers and hadles their effects
	char	m_chOldModel[64];			      //Save the player's model here
	bool	m_bPlayedQuadSound;
	bool	m_bPlayedEnvSound;
	bool	m_bPlayedInvSound;
	bool	m_bPlayedProtectSound;

	BOOL	m_bLostInvincSound;
	BOOL	m_bLostInvisSound;
	BOOL	m_bLostSuperSound;
	BOOL	m_bLostRadSound;
	float	m_fInvincSound;
	float	m_fSuperSound;

	void InitStatusBar( void );
	void UpdateStatusBar( void );
	int m_izSBarState[ SBAR_END ];
	float m_flNextSBarUpdateTime;
	float m_flStatusBarDisappearDelay;
	char m_SbarString0[ SBAR_STRING_SIZE ];
	char m_SbarString1[ SBAR_STRING_SIZE ];

	unsigned short m_usShotgunSingle;
	unsigned short m_usShotgunDouble;
	unsigned short m_usAxe;
	unsigned short m_usAxeSwing;
	unsigned short m_usRocket;
	unsigned short m_usGrenade;
	unsigned short m_usLightning;
	unsigned short m_usSpike;
	unsigned short m_usSuperSpike;	


#ifdef THREEWAVE
	int		m_bHasFlag;
	void ShowMenu ( int bitsValidSlots, int nDisplayTime, BOOL fNeedMore, char *pszText );
	int     m_iMenu;

	float	m_flNextTeamChange;

	CBasePlayer *pFlagCarrierKiller;
	CBasePlayer *pFlagReturner;
	CBasePlayer *pCarrierHurter;

	float	m_flCarrierHurtTime;
	float	m_flCarrierPickupTime;
	float	m_flFlagCarrierKillTime;
	float	m_flFlagReturnTime;
	float	m_flFlagStatusTime;

	float	m_flRegenTime;

	int		m_iRuneStatus;

	void	W_FireHook ( void );
	void	Throw_Grapple ( void );

	bool	m_bHook_Out;
	bool    m_bOn_Hook;
	CBaseEntity *m_ppHook;

	void Service_Grapple ( void );

#endif
	

//#ifdef THREEWAVE

//#endif

};

#define AUTOAIM_2DEGREES  0.0348994967025
#define AUTOAIM_5DEGREES  0.08715574274766
#define AUTOAIM_8DEGREES  0.1391731009601
#define AUTOAIM_10DEGREES 0.1736481776669






// QUAKECLASSIC
#define Q_SMALL_PUNCHANGLE_KICK		-2
#define Q_BIG_PUNCHANGLE_KICK		-4

#define IT_AXE                          (1 << 0)
#define IT_SHOTGUN                      (1 << 1)
#define IT_SUPER_SHOTGUN                (1 << 2)
#define IT_NAILGUN                      (1 << 3)
#define IT_SUPER_NAILGUN                (1 << 4)
#define IT_GRENADE_LAUNCHER             (1 << 5)
#define IT_ROCKET_LAUNCHER              (1 << 6)
#define IT_LIGHTNING                    (1 << 7)
#define IT_EXTRA_WEAPON                 (1 << 8)

#define IT_SHELLS                       (1 << 9)
#define IT_NAILS                        (1 << 10)
#define IT_ROCKETS                      (1 << 11)
#define IT_CELLS                        (1 << 12)

#define IT_ARMOR1                       (1 << 13)
#define IT_ARMOR2                       (1 << 14)
#define IT_ARMOR3                       (1 << 15)
#define IT_SUPERHEALTH                  (1 << 16)

#define IT_KEY1                         (1 << 17)
#define IT_KEY2                         (1 << 18)

#define IT_INVISIBILITY                 (1 << 19)
#define IT_INVULNERABILITY              (1 << 20)
#define IT_SUIT                         (1 << 21)
#define IT_QUAD                         (1 << 22)


#define ITEM_RUNE1_FLAG                 1
#define ITEM_RUNE2_FLAG                 2
#define ITEM_RUNE3_FLAG                 3
#define ITEM_RUNE4_FLAG                 4




extern int	gmsgHudText;
extern BOOL gInitHUD;

#define MAX_TELES 256

#endif // PLAYER_H
