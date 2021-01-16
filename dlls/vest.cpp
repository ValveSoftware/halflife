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
#include "gamerules.h"

enum satchel_radio_e {
	SATCHEL_RADIO_IDLE1 = 0,
	SATCHEL_RADIO_FIDGET1,
	SATCHEL_RADIO_DRAW,
	SATCHEL_RADIO_FIRE,
	SATCHEL_RADIO_HOLSTER
};

int CVest::AddToPlayer( CBasePlayer *pPlayer )
{
	int bResult = CBasePlayerItem::AddToPlayer( pPlayer );

	pPlayer->pev->weapons |= (1<<m_iId);

	if ( bResult )
	{
		return AddWeapon( );
	}

	return FALSE;
}

void CVest::Spawn( )
{
	Precache( );
	m_iId = WEAPON_VEST;
	SET_MODEL(ENT(pev), "models/w_vest.mdl");

	m_iDefaultAmmo = SATCHEL_DEFAULT_GIVE;

	FallInit();
}

void CVest::Precache( void )
{
	PRECACHE_MODEL("models/v_vest_radio.mdl");
	PRECACHE_MODEL("models/w_vest.mdl");
	PRECACHE_MODEL("models/p_vest.mdl");

	PRECACHE_SOUND("vest_attack.wav");
}

int CVest::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "Satchel Charge";
	p->iMaxAmmo1 = SATCHEL_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 4;
	p->iPosition = 4;
	p->iFlags = ITEM_FLAG_SELECTONEMPTY | ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;
	p->iId = m_iId = WEAPON_VEST;
	p->iWeight = SATCHEL_WEIGHT;

	return 1;
}

BOOL CVest::IsUseable( void )
{
	if ( m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ] > 0 ) 
	{
		return TRUE;
	}

	return FALSE;
}

BOOL CVest::CanDeploy( void )
{
	if ( m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ] > 0 ) 
	{
		return TRUE;
	}

	return FALSE;
}

BOOL CVest::Deploy( )
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );

	return DefaultDeploy( "models/v_vest_radio.mdl", "models/p_vest.mdl", SATCHEL_RADIO_DRAW, "hive" );
}

void CVest::Holster( int skiplocal )
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	
	SendWeaponAnim( SATCHEL_RADIO_HOLSTER );
}

void CVest::PrimaryAttack()
{
	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "vest_attack.wav", 1, ATTN_NORM);

	SetThink( &CVest::BlowThink );
	pev->nextthink = gpGlobals->time + 1.0;

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 2.0;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.0;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0;
}

void CVest::BlowThink() {
	SendWeaponAnim( SATCHEL_RADIO_FIRE );
	SetThink( &CVest::GoneThink );

	for ( int i = 0; i < RANDOM_LONG(1,3); i++ ) {
		Create( "spark_shower", pev->origin, pev->angles, NULL );
	}

	m_pPlayer->SetAnimation( PLAYER_JUMP );
	pev->nextthink = gpGlobals->time + 0.5;
}

void CVest::GoneThink() {
	m_pPlayer->	pev->health = 0; // without this, player can walk as a ghost.
	m_pPlayer->Killed(m_pPlayer->pev, pev, GIB_ALWAYS);
	CGrenade::Vest( m_pPlayer->pev, pev->origin );
}

void CVest::WeaponIdle( void )
{
	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	SendWeaponAnim( SATCHEL_RADIO_FIDGET1 );
	strcpy( m_pPlayer->m_szAnimExtention, "hive" );

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );// how long till we do this again.
}

LINK_ENTITY_TO_CLASS( weapon_vest, CVest );
