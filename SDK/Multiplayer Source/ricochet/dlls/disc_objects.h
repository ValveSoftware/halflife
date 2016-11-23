//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================

#ifndef DISK_OBJECTS_H
#define DISK_OBJECTS_H
#pragma once

// Disc objects
class CDiscWeapon;

class CDisc : public CGrenade
{
public:
	void	Spawn( void );
	void	Precache( void );
	void	EXPORT DiscTouch( CBaseEntity *pOther );
	void	EXPORT DiscThink( void );
	static	CDisc *CreateDisc( Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner, CDiscWeapon *pLauncher, bool bDecapitator, int iPowerupFlags );

	//void	SetObjectCollisionBox( void );
	void	ReturnToThrower( void );

	virtual BOOL	IsDisc( void ) { return TRUE; };

	float		m_fDontTouchEnemies;	// Prevent enemy touches for a bit
	float		m_fDontTouchOwner;		// Prevent friendly touches for a bit
	int			m_iBounces;		// Number of bounces
	EHANDLE		m_hOwner;		// Don't store in pev->owner, because it needs to hit its owner
	CDiscWeapon *m_pLauncher;	// pointer back to the launcher that fired me. 
	int			m_iTrail;
	int			m_iSpriteTexture;
	bool		m_bDecapitate;	// True if this is a decapitating shot
	bool		m_bRemoveSelf;  // True if the owner of this disc has died
	int			m_iPowerupFlags;// Flags for any powerups active on this disc
	bool		m_bTeleported;  // Disc has gone through a teleport

	EHANDLE m_pLockTarget;
	
	Vector	m_vecActualVelocity;
	Vector	m_vecSideVelocity;
	Vector	m_vecOrg;
};
 
//===============================================================================
// DISCWAR OBJECTS
//===============================================================================
class CBaseTrigger : public CBaseToggle
{
public:
	void EXPORT TeleportTouch ( CBaseEntity *pOther );
	void KeyValue( KeyValueData *pkvd );
	void EXPORT MultiTouch( CBaseEntity *pOther );
	void EXPORT HurtTouch ( CBaseEntity *pOther );
	void EXPORT CDAudioTouch ( CBaseEntity *pOther );
	void ActivateMultiTrigger( CBaseEntity *pActivator );
	void EXPORT MultiWaitOver( void );
	void EXPORT CounterUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT ToggleUse ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void InitTrigger( void );

	virtual int	ObjectCaps( void ) { return CBaseToggle :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
};

// Brush that's status gets toggled by a disc hit
#define LAST_HITBY_FRIENDLY		1
#define LAST_HITBY_ENEMY		2

class CDiscTarget : public CBaseTrigger
{
public:
	void KeyValue( KeyValueData *pkvd );
	void Spawn( void );
	void Reset( void );

	void EXPORT	DiscToggleTouch( CBaseEntity *pOther );

	int	m_iszFriendlyHit;
	int	m_iszEnemyHit;
	int m_iState;
};

//=========================================================
// Powerup object
class CDiscwarPowerup : public CBaseAnimating
{
public:
	void Spawn( void );
	void Activate( void );
	void Precache( void );
	void EXPORT PowerupTouch( CBaseEntity *pOther );
	void EXPORT ChoosePowerupThink( void );
	void EXPORT RemovePowerupThink( void );
	void EXPORT AnimateThink( void );
	void SetObjectCollisionBox( void );

	void Disable();
	void Enable();

	EHANDLE	m_hPlayerIGaveTo;
	int		m_iPowerupType;
};

//===============================================================================
// Brush that toggles between gone/there
#define PLAT_FADE_TIME		2.0
class CPlatToggleRemove : public CBaseEntity
{
public:
	void Spawn( void );
	void KeyValue( KeyValueData *pkvd );
	void Reset( void );

	void EXPORT PlatToggleRemoveUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT PlatRemoveThink( void );

	float	m_flRemoveAt;
};

//===============================================================================
// Trigger that jumps a player to a target point
class CTriggerJump : public CBaseTrigger
{
public:
	void Spawn( void );
	void KeyValue( KeyValueData *pkvd );
	void Activate( void );
	void Precache( void );
	void EXPORT JumpTouch( CBaseEntity *pOther );
	void EXPORT JumpUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	Vector m_vecTargetOrg;
	float  m_flHeight;
	int	   m_iState;

private:
	unsigned short m_usJump;
};

//===============================================================================
// Trigger that returns discs to their thrower immediately
class CTriggerDiscReturn : public CBaseTrigger
{
public:
	void Spawn( void );
	void Precache( void );
	void EXPORT DiscReturnTouch( CBaseEntity *pOther );
};

//===============================================================================
// Trigger that starts the fall animation for players
class CTriggerFall : public CBaseTrigger
{
public:
	void Spawn( void );
	void EXPORT FallTouch( CBaseEntity *pOther );
};

#endif // DISK_OBJECTS_H
