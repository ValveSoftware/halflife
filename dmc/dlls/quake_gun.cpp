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

===== quake_gun.cpp ========================================================

  This is a half-life weapon that fires every one of the quake weapons.
  It's automatically given to all players.

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "quake_gun.h"

LINK_ENTITY_TO_CLASS( weapon_quakegun, CQuakeGun );


//===========================================================
void CQuakeGun::Spawn( )
{
	Precache( );
	SET_MODEL(ENT(pev), "models/v_crowbar.mdl");
	m_iDefaultAmmo = GLOCK_DEFAULT_GIVE;
	FallInit(); 
}

void CQuakeGun::Precache( void )
{
	PRECACHE_MODEL("models/v_crowbar.mdl");
	PRECACHE_MODEL("models/p_9mmhandgun.mdl");
}

int CQuakeGun::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = -1;
	p->iSlot = 1;
	p->iPosition = 1;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_GLOCK;
	p->iWeight = GLOCK_WEIGHT;

	return 1;
}

void CQuakeGun::DestroyEffect( void )
{

#ifndef CLIENT_DLL
	if ( m_pBeam )
	{
		UTIL_Remove( m_pBeam );
		m_pBeam = NULL;
	}
#endif

}

void CQuakeGun::CreateEffect( void )
{

#ifndef CLIENT_DLL
	DestroyEffect();

	m_pBeam = CBeam::BeamCreate( "sprites/laserbeam.spr", 40 );
	m_pBeam->PointEntInit( pev->origin, m_pPlayer->entindex() );
	m_pBeam->SetBrightness( 100 );
	m_pBeam->SetEndAttachment( 1 );
	m_pBeam->pev->spawnflags |= SF_BEAM_TEMPORARY;	// Flag these to be destroyed on save/restore or level transition
	m_pBeam->pev->flags |= FL_SKIPLOCALHOST;
	m_pBeam->pev->owner = m_pPlayer->edict();

	m_pBeam->SetScrollRate( 110 );
	m_pBeam->SetNoise( 5 );
#endif

}

void CQuakeGun::UpdateEffect( void )
{
#if !defined( CLIENT_DLL )
	UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );
	Vector vecAiming = gpGlobals->v_forward;
	Vector vecSrc	 = m_pPlayer->GetGunPosition( );

	Vector vecDest = vecSrc + vecAiming * 2048;
	edict_t		*pentIgnore;
	TraceResult tr;

	pentIgnore = m_pPlayer->edict();
	Vector tmpSrc = vecSrc + gpGlobals->v_up * -8 + gpGlobals->v_right * 3;

	// ALERT( at_console, "." );
	
	UTIL_TraceLine( vecSrc, vecDest, dont_ignore_monsters, pentIgnore, &tr );

	if (tr.fAllSolid)
		return;

	if ( !m_pBeam )
	{
		CreateEffect();
	}

	m_pBeam->SetStartPos( tr.vecEndPos );
#endif

}

#if !defined( CLIENT_DLL )
BOOL CQuakeGun::Deploy( )
{
	m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_crowbar.mdl");
	m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_9mmhandgun.mdl");
	strcpy( m_pPlayer->m_szAnimExtention, "onehanded" );

#ifdef CLIENT_DLL
	g_flLightTime = 0.0;
#endif

	SendWeaponAnim( 0 );
	return TRUE;
}
#endif

// Plays quad sound if needed
int CQuakeGun::SuperDamageSound()
{
	if ( m_pPlayer->m_iQuakeItems & IT_QUAD )
	{
		if ( m_pPlayer->m_flNextQuadSound < gpGlobals->time)
		{
			m_pPlayer->m_flNextQuadSound = gpGlobals->time + 1;
				return 1;
		}
	}

	return 0;
}

// Firing the Quakegun forces the player to fire the appropriate weapon
void CQuakeGun::PrimaryAttack( void )
{
	int iQuadSound = 0;
	iQuadSound = SuperDamageSound();
	m_pPlayer->W_Attack( iQuadSound );

#if !defined( CLIENT_DLL )
	if ( m_pPlayer->m_iQuakeWeapon == IT_LIGHTNING && m_pPlayer->pev->deadflag == DEAD_NO )
		 UpdateEffect();
#endif

	m_bPlayedIdleAnim = FALSE;
}
