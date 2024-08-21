/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"

enum glock_e {
	GLOCK_IDLE1 = 0,
	GLOCK_IDLE2,
	GLOCK_IDLE3,
	GLOCK_SHOOT,
	GLOCK_SHOOT_EMPTY,
	GLOCK_RELOAD,
	GLOCK_RELOAD_NOT_EMPTY,
	GLOCK_DRAW,
	GLOCK_HOLSTER,
	GLOCK_ADD_SILENCER
};

class CGlock : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 2; }
	int GetItemInfo(ItemInfo *p);

	void PrimaryAttack( void );
	void SecondaryAttack( void );
	void GlockFire( float flSpread, float flCycleTime, BOOL fUseAutoAim );
	BOOL Deploy( void );
	void Reload( void );
	void WeaponIdle( void );

private:
	int m_iShell;
	

	unsigned short m_usFireGlock1;
	unsigned short m_usFireGlock2;
};
LINK_ENTITY_TO_CLASS( weapon_glock, CGlock );
LINK_ENTITY_TO_CLASS( weapon_9mmhandgun, CGlock );


void CGlock::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_9mmhandgun"); // hack to allow for old names
	Precache( );
	m_iId = WEAPON_GLOCK;
	SET_MODEL(ENT(pev), "models/w_9mmhandgun.mdl");

	m_iDefaultAmmo = GLOCK_DEFAULT_GIVE;

	FallInit();// get ready to fall down.
}


void CGlock::Precache( void )
{
	PRECACHE_MODEL("models/v_9mmhandgun.mdl");
	PRECACHE_MODEL("models/w_9mmhandgun.mdl");
	PRECACHE_MODEL("models/p_9mmhandgun.mdl");

	m_iShell = PRECACHE_MODEL ("models/shell.mdl");// brass shell

	PRECACHE_SOUND("items/9mmclip1.wav");
	PRECACHE_SOUND("items/9mmclip2.wav");

	PRECACHE_SOUND ("weapons/pl_gun1.wav");//silenced handgun
	PRECACHE_SOUND ("weapons/pl_gun2.wav");//silenced handgun
	PRECACHE_SOUND ("weapons/pl_gun3.wav");//handgun

	m_usFireGlock1 = PRECACHE_EVENT( 1, "events/glock1.sc" );
	m_usFireGlock2 = PRECACHE_EVENT( 1, "events/glock2.sc" );
}

int CGlock::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = _9MM_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = GLOCK_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 0;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_GLOCK;
	p->iWeight = GLOCK_WEIGHT;

	return 1;
}

BOOL CGlock::Deploy( )
{
	// pev->body = 1;
	return DefaultDeploy( "models/v_9mmhandgun.mdl", "models/p_9mmhandgun.mdl", GLOCK_DRAW, "onehanded" );
}

void CGlock::SecondaryAttack( void )
{
	GlockFire( 0.1, 0.2, FALSE );
}

void CGlock::PrimaryAttack( void )
{
	GlockFire( 0.01, 0.3, TRUE );
}

void CGlock::GlockFire( float flSpread , float flCycleTime, BOOL fUseAutoAim )
{
	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = gpGlobals->time + 0.2;
		}

		return;
	}

	m_iClip--;

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

#if defined ( OLD_WEAPONS )
	if (m_iClip != 0)
		SendWeaponAnim( GLOCK_SHOOT );
	else
		SendWeaponAnim( GLOCK_SHOOT_EMPTY );
#endif

	if ( fUseAutoAim )
	{
		PLAYBACK_EVENT_FULL( 0, m_pPlayer->edict(), m_usFireGlock1, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, 0, 0, ( m_iClip == 0 ) ? 1 : 0, 0 );
	}
	else
	{
		PLAYBACK_EVENT_FULL( 0, m_pPlayer->edict(), m_usFireGlock2, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, 0, 0, ( m_iClip == 0 ) ? 1 : 0, 0 );
	}

	// player "shoot" animation
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

#if defined ( OLD_WEAPONS )
	UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );
		
	Vector	vecShellVelocity = m_pPlayer->pev->velocity 
							 + gpGlobals->v_right * RANDOM_FLOAT(50,70) 
							 + gpGlobals->v_up * RANDOM_FLOAT(100,150) 
							 + gpGlobals->v_forward * 25;
	EjectBrass ( pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_up * -12 + gpGlobals->v_forward * 32 + gpGlobals->v_right * 6 , vecShellVelocity, pev->angles.y, m_iShell, TE_BOUNCE_SHELL ); 
#endif

	// silenced
	if (pev->body == 1)
	{
		m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;
#if defined ( OLD_WEAPONS )
		switch(RANDOM_LONG(0,1))
		{
		case 0:
			EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/pl_gun1.wav", RANDOM_FLOAT(0.9, 1.0), ATTN_NORM);
			break;
		case 1:
			EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/pl_gun2.wav", RANDOM_FLOAT(0.9, 1.0), ATTN_NORM);
			break;
		}
#endif
	}
	else
	{
		// non-silenced
		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;
#if defined ( OLD_WEAPONS )
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/pl_gun3.wav", RANDOM_FLOAT(0.92, 1.0), ATTN_NORM, 0, 98 + RANDOM_LONG(0,3));
#endif
	}

	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecAiming;
	
	if ( fUseAutoAim )
	{
		vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );
	}
	else
	{
		vecAiming = gpGlobals->v_forward;
	}

	m_pPlayer->FireBullets( 1, vecSrc, vecAiming, Vector( flSpread, flSpread, flSpread ), 8192, BULLET_PLAYER_9MM, 0 );

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->time + flCycleTime;

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flTimeWeaponIdle = gpGlobals->time + RANDOM_FLOAT ( 10, 15 );

#if defined ( OLD_WEAPONS )
	m_pPlayer->pev->punchangle.x -= 2;
#endif
}


void CGlock::Reload( void )
{
	int iResult = DefaultReload( GLOCK_MAX_CLIP, m_iClip > 0 ? GLOCK_RELOAD_NOT_EMPTY : GLOCK_RELOAD, 1.5 );
	if (iResult)
	{
		m_flTimeWeaponIdle = gpGlobals->time + RANDOM_FLOAT ( 10, 15 );
	}
}



void CGlock::WeaponIdle( void )
{
	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

	if (m_flTimeWeaponIdle > gpGlobals->time)
		return;

	// only idle if the slid isn't back
	if (m_iClip != 0)
	{
		int iAnim;
		float flRand = RANDOM_FLOAT(0, 1);
		if (flRand <= 0.3 + 0 * 0.75)
		{
			iAnim = GLOCK_IDLE3;
			m_flTimeWeaponIdle = gpGlobals->time + 49.0 / 16;
		}
		else if (flRand <= 0.6 + 0 * 0.875)
		{
			iAnim = GLOCK_IDLE1;
			m_flTimeWeaponIdle = gpGlobals->time + 60.0 / 16.0;
		}
		else
		{
			iAnim = GLOCK_IDLE2;
			m_flTimeWeaponIdle = gpGlobals->time + 40.0 / 16.0;
		}
		SendWeaponAnim( iAnim );
	}
}








class CGlockAmmo : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_9mmclip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		if (pOther->GiveAmmo( AMMO_GLOCKCLIP_GIVE, "9mm", _9MM_MAX_CARRY ) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_glockclip, CGlockAmmo );
LINK_ENTITY_TO_CLASS( ammo_9mmclip, CGlockAmmo );















