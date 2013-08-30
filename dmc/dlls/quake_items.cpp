//=========== (C) Copyright 1996-2002, Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: Quake world items
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "shake.h"
#include "../engine/studio.h"
#include "weapons.h"
#include "quake_gun.h"
#include "hltv.h"

extern unsigned short g_usPowerUp;

class CQuakeItem : public CBaseEntity
{
public:
	void	Spawn( void );

	// Respawning
	void	EXPORT Materialize( void );
	void	Respawn( float flTime );

	virtual void SetObjectCollisionBox ( void );

	// Touch
	void	EXPORT ItemTouch( CBaseEntity *pOther );
	virtual	BOOL MyTouch( CBasePlayer *pOther ) { return FALSE; };

	float	m_flRespawnTime;
};

//-----------------------------------------------------------------------------
// Purpose: Spawn and drop to the floor
//-----------------------------------------------------------------------------

void CQuakeItem :: SetObjectCollisionBox( void )
{
	pev->absmin = pev->origin + Vector(-32, -32, 0);
	pev->absmax = pev->origin + Vector(32, 32, 56); 
}

void CQuakeItem::Spawn()
{ 
	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_TRIGGER;
	
	SetTouch(&CQuakeItem::ItemTouch);

	if (DROP_TO_FLOOR(ENT(pev)) == 0)
	{
		ALERT(at_error, "Item %s fell out of level at %f,%f,%f", STRING( pev->classname ), pev->origin.x, pev->origin.y, pev->origin.z);
		UTIL_Remove( this );
		return;
	}

	//UTIL_SetOrigin( pev, pev->origin + Vector(0,0,16) );

	if (!m_flRespawnTime)
		m_flRespawnTime = 20;
}

//-----------------------------------------------------------------------------
// Purpose: Bring the item back
//-----------------------------------------------------------------------------
void CQuakeItem::Materialize()
{
	// Become visible and touchable
	pev->effects &= ~EF_NODRAW;
	SetTouch( &CQuakeItem::ItemTouch );

	// Play respawn sound
	EMIT_SOUND( ENT(pev), CHAN_WEAPON, "items/itembk2.wav", 1, ATTN_NORM );
}

//-----------------------------------------------------------------------------
// Purpose: Setup the item's respawn in the time set
//-----------------------------------------------------------------------------
void CQuakeItem::Respawn( float flTime )
{
	pev->effects |= EF_NODRAW;
	SetTouch( NULL );

	// Come back in time
	SetThink ( &CQuakeItem::Materialize );
	pev->nextthink = gpGlobals->time + flTime;
}


//-----------------------------------------------------------------------------
// Purpose: Touch function that calls the virtual touch function
//-----------------------------------------------------------------------------
void CQuakeItem::ItemTouch( CBaseEntity *pOther )
{
	// if it's not a player, ignore
	if ( !pOther->IsPlayer() )
		return;

	//Dead?
	if (pOther->pev->health <= 0)
		return;

	CBasePlayer *pPlayer = (CBasePlayer *)pOther;

	// Call the virtual touch function
	if ( MyTouch( pPlayer ) )
	{
		SUB_UseTargets( pOther, USE_TOGGLE, 0 );

		// Respawn if it's not DM==2
		if (gpGlobals->deathmatch != 2)
		{
			Respawn( m_flRespawnTime );
		}
		else
		{
			UTIL_Remove( this );
		}
	}
}

//======================================================================================
// HEALTH ITEMS
//======================================================================================
#define H_ROTTEN	1
#define H_MEGA 		2

class CItemHealth : public CQuakeItem
{
public:
	void Spawn( void );
	void Precache( void );
	BOOL MyTouch( CBasePlayer *pPlayer );
	void EXPORT MegahealthRot( void );

	EHANDLE	m_hRotTarget;
	int		m_iHealAmount;
	int		m_iHealType;
};
LINK_ENTITY_TO_CLASS(item_health, CItemHealth);

//--------------------------------------------
// Spawn
void CItemHealth::Spawn( void )
{
	Precache();

	// Setup healing method
	if (pev->spawnflags & H_ROTTEN)
	{
		SET_MODEL(ENT(pev), "models/w_medkits.mdl");
		pev->noise = MAKE_STRING( "items/r_item1.wav" );
		m_iHealAmount = 15;
		m_iHealType = H_ROTTEN;
	}
	else if (pev->spawnflags & H_MEGA)
	{
		SET_MODEL(ENT(pev), "models/w_medkitl.mdl");
		pev->noise = MAKE_STRING( "items/r_item2.wav" );
		m_iHealAmount = 100;
		m_iHealType = H_MEGA;
	}
	else
	{
		SET_MODEL(ENT(pev), "models/w_medkit.mdl");
		pev->noise = MAKE_STRING( "items/health1.wav" );
		m_iHealAmount = 25;
		m_iHealType = H_ROTTEN;
	}

	CQuakeItem::Spawn();
}

//--------------------------------------------
// Precache
void CItemHealth::Precache()
{
	PRECACHE_MODEL("models/w_medkitl.mdl");
	PRECACHE_MODEL("models/w_medkits.mdl");
	PRECACHE_MODEL("models/w_medkit.mdl");
	PRECACHE_SOUND("items/r_item1.wav");
	PRECACHE_SOUND("items/r_item2.wav");
	PRECACHE_SOUND("items/health1.wav");
}

//--------------------------------------------
// Health Touch
BOOL CItemHealth::MyTouch( CBasePlayer *pPlayer )
{
	// Don't heal in DM==4 if they're invincible
	if (gpGlobals->deathmatch == 4 && pPlayer->m_flInvincibleFinished > 0)
		return FALSE;

	if (pPlayer->pev->health <= 0)
		return FALSE;

	if (m_iHealType == H_MEGA)
	{
		if (pPlayer->pev->health >= 250)
			return FALSE;
		if ( !pPlayer->TakeHealth( m_iHealAmount, DMG_GENERIC | DMG_IGNORE_MAXHEALTH) )
			return FALSE;
	}
	else
	{
		// Heal the Player
		if ( !pPlayer->TakeHealth( m_iHealAmount, DMG_GENERIC ) )
			return FALSE;
	}

	ClientPrint( pPlayer->pev, HUD_PRINTNOTIFY, "#Get_Health", UTIL_dtos1(m_iHealAmount) );
	EMIT_SOUND( ENT(pev), CHAN_ITEM, STRING(pev->noise), 1, ATTN_NORM );

	// Setup for respawn
	if (m_iHealType == H_MEGA)
	{
		// Go invisible and fire targets
		pev->effects |= EF_NODRAW;
		SetTouch( NULL );
		SUB_UseTargets( pPlayer, USE_TOGGLE, 0 );

		pPlayer->m_iQuakeItems |= IT_SUPERHEALTH;
		if (gpGlobals->deathmatch != 4)
		{
			SetThink( &CItemHealth::MegahealthRot );
			pev->nextthink = gpGlobals->time + 5;
		}
		m_hRotTarget = pPlayer;

		// Return FALSE, because we want to handle our respawn ourselves
		return FALSE;
	}

	// Respawn as normal
	return TRUE;
}

//--------------------------------------------
// Megahealth Rot function. Reduce player's health until it's below 100. Then respawn.
void CItemHealth::MegahealthRot( void )
{
	if (m_hRotTarget)
	{
		CBasePlayer *pPlayer = ((CBasePlayer *)((CBaseEntity *)m_hRotTarget));

		if (pPlayer->pev->health > pPlayer->pev->max_health )
		{
			pPlayer->pev->health--;
			pev->nextthink = gpGlobals->time + 1;
			return;
		}
		
		pPlayer->m_iQuakeItems &= ~IT_SUPERHEALTH;
	}

	// Respawn if it's not DM==2
	if (gpGlobals->deathmatch != 2)
	{
		SetThink ( &CItemHealth::Materialize );
		pev->nextthink = gpGlobals->time + 20;
	}
	else
	{
		UTIL_Remove( this );
	}
}

//======================================================================================
// ARMOR ITEMS
//======================================================================================
class CItemArmor : public CQuakeItem
{
public:
	BOOL MyTouch( CBasePlayer *pPlayer );

	float m_flArmorValue;
	float m_flArmorType;
	int	  m_iArmorBit;
};

// Armor Touch
BOOL CItemArmor::MyTouch( CBasePlayer *pPlayer )
{
	if (pPlayer->pev->health <= 0)
		return FALSE;

	// Don't pickup in DM==4 if they're invincible
	if (gpGlobals->deathmatch == 4 && pPlayer->m_flInvincibleFinished > 0)
		return FALSE;

	// Don't pickup if this armor isn't as good as the stuff we've got
	if ( (pPlayer->pev->armortype * pPlayer->pev->armorvalue) >= (m_flArmorType * m_flArmorValue) )
		return FALSE;
		
	pPlayer->pev->armortype = m_flArmorType;
	pPlayer->pev->armorvalue = m_flArmorValue;
	pPlayer->m_iQuakeItems &= ~(IT_ARMOR1 | IT_ARMOR2 | IT_ARMOR3);
	pPlayer->m_iQuakeItems |= m_iArmorBit;

	EMIT_SOUND( ENT( pPlayer->pev ), CHAN_ITEM, "items/armor1.wav", 1, ATTN_NORM );

	return TRUE;
}

//===============
// Green Armor
class CItemArmorGreen : public CItemArmor
{
public:
	void Spawn( void );
	void Precache( void );
};
LINK_ENTITY_TO_CLASS(item_armor1, CItemArmorGreen);

// Spawn
void CItemArmorGreen::Spawn( void )
{
	Precache();	
	SET_MODEL(ENT(pev), "models/armour_g.mdl");
	CItemArmor::Spawn();

	m_flArmorValue = 100;
	m_flArmorType = 0.3;
	m_iArmorBit = IT_ARMOR1;
}

// Precache
void CItemArmorGreen::Precache( void )
{
	PRECACHE_MODEL( "models/armour_g.mdl" );
	PRECACHE_SOUND( "items/armor1.wav" );
}

//===============
// Yellow Armor
class CItemArmorYellow : public CItemArmor
{
public:
	void Spawn( void );
	void Precache( void );
};
LINK_ENTITY_TO_CLASS(item_armor2, CItemArmorYellow);

// Spawn
void CItemArmorYellow::Spawn( void )
{
	Precache();
	SET_MODEL(ENT(pev), "models/armour_y.mdl");
	CItemArmor::Spawn();

	m_flArmorValue = 150;
	m_flArmorType = 0.6;
	m_iArmorBit = IT_ARMOR2;
}

// Precache
void CItemArmorYellow::Precache( void )
{
	PRECACHE_MODEL( "models/armour_y.mdl" );
	PRECACHE_SOUND( "items/armor1.wav" );
}

//===============
// Red Armor
class CItemArmorRed : public CItemArmor
{
public:
	void Spawn( void );
	void Precache( void );
};
LINK_ENTITY_TO_CLASS(item_armor3, CItemArmorRed);
LINK_ENTITY_TO_CLASS(item_armorInv, CItemArmorRed);

// Spawn
void CItemArmorRed::Spawn( void )
{
	Precache();
	SET_MODEL(ENT(pev), "models/armour_r.mdl");
	CItemArmor::Spawn();

	m_flArmorValue = 200;
	m_flArmorType = 0.8;
	m_iArmorBit = IT_ARMOR3;
}

// Precache
void CItemArmorRed::Precache( void )
{
	PRECACHE_MODEL( "models/armour_r.mdl" );
	PRECACHE_SOUND( "items/armor1.wav" );
}

//======================================================================================
// WEAPON ITEMS
//======================================================================================
void CBasePlayer::CheckAmmo()
{
	if (m_iAmmoShells > 100)
		m_iAmmoShells = 100;
	if (m_iAmmoNails > 200)
		m_iAmmoNails = 200;
	if (m_iAmmoRockets > 100)
		m_iAmmoRockets = 100;               
	if (m_iAmmoCells > 100)
		m_iAmmoCells = 100;         
}

int RankForWeapon(int iWeapon)
{
	switch (iWeapon)
	{
	case IT_LIGHTNING:
		return 1; break;
	case IT_ROCKET_LAUNCHER:
		return 2; break;
	case IT_SUPER_NAILGUN:
		return 3; break;
	case IT_GRENADE_LAUNCHER:
		return 4; break;
	case IT_SUPER_SHOTGUN:
		return 5; break;
	case IT_NAILGUN:
		return 6; break;

	default:
		break;
	}

	return 7;
}

int WeaponCode(int iWeapon)
{
	switch (iWeapon)
	{
	case IT_SUPER_SHOTGUN:
		return 3; break;
	case IT_NAILGUN:
		return 4; break;
	case IT_SUPER_NAILGUN:
		return 5; break;
	case IT_GRENADE_LAUNCHER:
		return 6; break;
	case IT_ROCKET_LAUNCHER:
		return 7; break;
	case IT_LIGHTNING:
		return 8; break;

	default:
		break;
	}

	return 1;
}

int GetWeaponValue ( int iWeapon )
{
	int iWepValue;

	switch ( iWeapon )
	{
		case IT_AXE: iWepValue = 1; break;
		case IT_SHOTGUN: iWepValue = 2; break;
		case IT_SUPER_SHOTGUN: iWepValue = 3; break;
		case IT_NAILGUN: iWepValue = 4; break;
		case IT_SUPER_NAILGUN: iWepValue = 5; break;
		case IT_GRENADE_LAUNCHER: iWepValue = 6; break;
		case IT_ROCKET_LAUNCHER: iWepValue = 7; break;
		case IT_LIGHTNING: iWepValue = 8; break;
	}

	return iWepValue;
}
// Change weapon only if the new one's better
void CBasePlayer::Deathmatch_Weapon(int iOldWeapon, int iNewWeapon)
{
	int iPickedWep = GetWeaponValue( iNewWeapon );
	int iOldWep = GetWeaponValue( m_iQuakeWeapon );

	switch ( m_iAutoWepSwitch )
	{
		case 0: return; break;
		case 1: 
			W_ChangeWeapon( iPickedWep ); break;
		case 2:

			if ( iPickedWep == 8 && !FBitSet(pev->flags , FL_INWATER) || iPickedWep > iOldWep )
				W_ChangeWeapon( iPickedWep );
			break;
	}
		

}

//-----------------------------------------------
// Base Quake Weapon object
class CItemWeapon : public CQuakeItem
{
public:
	BOOL MyTouch( CBasePlayer *pPlayer );

	int	m_iWeapon;
};

BOOL CItemWeapon::MyTouch( CBasePlayer *pPlayer )
{
	BOOL bLeaveWeapon = FALSE;

	if (gpGlobals->deathmatch == 2 || gpGlobals->deathmatch == 3 || gpGlobals->deathmatch == 5 || CVAR_GET_FLOAT("mp_weaponstay") > 0  )
		bLeaveWeapon = TRUE;

	// Leave the weapon if the player's already got it
	if ( bLeaveWeapon && (pPlayer->m_iQuakeItems & m_iWeapon) )
		return FALSE;

	if ( pPlayer->pev->health <= 0)
		return FALSE;

	// Give the player some ammo
	switch (m_iWeapon)
	{
	case IT_NAILGUN:
		pPlayer->m_iAmmoNails += 30;
		break;
	case IT_SUPER_NAILGUN:
		pPlayer->m_iAmmoNails += 30;
		break;
	case IT_SUPER_SHOTGUN:
		pPlayer->m_iAmmoShells += 5;
		break;
	case IT_ROCKET_LAUNCHER:
		pPlayer->m_iAmmoRockets += 5;
		break;
	case IT_GRENADE_LAUNCHER:
		pPlayer->m_iAmmoRockets += 5;
		break;
	case IT_LIGHTNING:
		pPlayer->m_iAmmoCells += 15;
		break;
	default:
		break;
	}
	pPlayer->CheckAmmo();

	EMIT_SOUND( ENT(pev), CHAN_ITEM, "weapons/pkup.wav", 1, ATTN_NORM );

	// Change to new weapon?
	int iOldItems = pPlayer->m_iQuakeWeapon;
	pPlayer->m_iQuakeItems |= m_iWeapon;
	
	pPlayer->Deathmatch_Weapon(iOldItems, m_iWeapon);


	// Update HUD
	pPlayer->W_SetCurrentAmmo();
	pPlayer->m_iClientQuakeWeapon  = -1;
	pPlayer->m_fWeapon = FALSE;
	pPlayer->m_fKnownItem = FALSE;
	pPlayer->UpdateClientData();

	if (bLeaveWeapon)
		return FALSE;

	// Respawn
	m_flRespawnTime = 30;

	return TRUE;
}

//===============
// Super Shotgun
class CItemWeaponSuperShotgun : public CItemWeapon
{
public:
	void Spawn( void );
	void Precache( void );
};
LINK_ENTITY_TO_CLASS(weapon_supershotgun, CItemWeaponSuperShotgun);

// Spawn
void CItemWeaponSuperShotgun::Spawn( void )
{
	if ( gpGlobals->deathmatch > 3)
	{
		UTIL_Remove(this);
		return;
	}

	Precache();
	SET_MODEL(ENT(pev), "models/g_shot2.mdl");
	CItemWeapon::Spawn();

	m_iWeapon = IT_SUPER_SHOTGUN;
	pev->netname = MAKE_STRING("Double-barrelled Shotgun");
}

// Precache
void CItemWeaponSuperShotgun::Precache( void )
{
	PRECACHE_MODEL( "models/g_shot2.mdl" );
}

//===============
// Nailgun
class CItemWeaponNailgun : public CItemWeapon
{
public:
	void Spawn( void );
	void Precache( void );
};
LINK_ENTITY_TO_CLASS(weapon_nailgun, CItemWeaponNailgun);

// Spawn
void CItemWeaponNailgun::Spawn( void )
{
	if ( gpGlobals->deathmatch > 3)
	{
		UTIL_Remove(this);
		return;
	}

	Precache();
	SET_MODEL(ENT(pev), "models/g_nail.mdl");
	CItemWeapon::Spawn();

	m_iWeapon = IT_NAILGUN;
	pev->netname = MAKE_STRING("Nailgun");
}

// Precache
void CItemWeaponNailgun::Precache( void )
{
	PRECACHE_MODEL( "models/g_nail.mdl" );
}

//===============
// Super Nailgun
class CItemWeaponSuperNailgun : public CItemWeapon
{
public:
	void Spawn( void );
	void Precache( void );
};
LINK_ENTITY_TO_CLASS(weapon_supernailgun, CItemWeaponSuperNailgun);

// Spawn
void CItemWeaponSuperNailgun::Spawn( void )
{
	if ( gpGlobals->deathmatch > 3)
	{
		UTIL_Remove(this);
		return;
	}

	Precache();
	SET_MODEL(ENT(pev), "models/g_nail2.mdl");
	CItemWeapon::Spawn();

	m_iWeapon = IT_SUPER_NAILGUN;
	pev->netname = MAKE_STRING("Super Nailgun");
}

// Precache
void CItemWeaponSuperNailgun::Precache( void )
{
	PRECACHE_MODEL( "models/g_nail2.mdl" );
}

//===============
// Grenade Launcher
class CItemWeaponGrenadeLauncher : public CItemWeapon
{
public:
	void Spawn( void );
	void Precache( void );
};
LINK_ENTITY_TO_CLASS(weapon_grenadelauncher, CItemWeaponGrenadeLauncher);

// Spawn
void CItemWeaponGrenadeLauncher::Spawn( void )
{
	if ( gpGlobals->deathmatch > 3)
	{
		UTIL_Remove(this);
		return;
	}

	Precache();
	SET_MODEL(ENT(pev), "models/g_rock.mdl");
	CItemWeapon::Spawn();

	m_iWeapon = IT_GRENADE_LAUNCHER;
	pev->netname = MAKE_STRING("Grenade Launcher");
}

// Precache
void CItemWeaponGrenadeLauncher::Precache( void )
{
	PRECACHE_MODEL( "models/g_rock.mdl" );
}

//===============
// Rocket Launcher
class CItemWeaponRocketLauncher : public CItemWeapon
{
public:
	void Spawn( void );
	void Precache( void );
};
LINK_ENTITY_TO_CLASS(weapon_rocketlauncher, CItemWeaponRocketLauncher);

// Spawn
void CItemWeaponRocketLauncher::Spawn( void )
{
	if ( gpGlobals->deathmatch > 3)
	{
		UTIL_Remove(this);
		return;
	}

	Precache();
	SET_MODEL(ENT(pev), "models/g_rock2.mdl");
	CItemWeapon::Spawn();

	m_iWeapon = IT_ROCKET_LAUNCHER;
	pev->netname = MAKE_STRING("Rocket Launcher");
}

// Precache
void CItemWeaponRocketLauncher::Precache( void )
{
	PRECACHE_MODEL( "models/g_rock2.mdl" );
}

//===============
// Lightning Gun
class CItemWeaponLightning : public CItemWeapon
{
public:
	void Spawn( void );
	void Precache( void );
};
LINK_ENTITY_TO_CLASS(weapon_lightning, CItemWeaponLightning);

// Spawn
void CItemWeaponLightning::Spawn( void )
{
	if ( gpGlobals->deathmatch > 3)
	{
		UTIL_Remove(this);
		return;
	}

	Precache();
	SET_MODEL(ENT(pev), "models/g_light.mdl");
	CItemWeapon::Spawn();

	m_iWeapon = IT_LIGHTNING;
	pev->netname = MAKE_STRING("Thunderbolt");
}

// Precache
void CItemWeaponLightning::Precache( void )
{
	PRECACHE_MODEL( "models/g_light.mdl" );
}

//======================================================================================
// AMMO ITEMS
//======================================================================================
#define BIG_AMMOBOX		1 

class CItemAmmo : public CQuakeItem
{
public:
	void Spawn( void );
	void Precache( void );
	BOOL MyTouch( CBasePlayer *pPlayer );

	int	 m_isSmallBox;
	int  m_isLargeBox;
	int	 ammo_shells;
	int	 ammo_nails;
	int  ammo_rockets;
	int	 ammo_cells;
};

// Spawn
void CItemAmmo::Spawn( void )
{
	Precache();

	// Set the box size
	if (pev->spawnflags & BIG_AMMOBOX)
	{
		SET_MODEL( ENT(pev), STRING(m_isLargeBox) );
		ammo_shells *= 2;
		ammo_nails *= 2;
		ammo_rockets *= 2;
		ammo_cells *= 2;
	}
	else
	{
		SET_MODEL( ENT(pev), STRING(m_isSmallBox) );
	}

	// Halve respawn times in DM==3 and DM==5
	if (gpGlobals->deathmatch == 3 || gpGlobals->deathmatch == 5)        
		m_flRespawnTime = 15;
	else
		m_flRespawnTime = 30;

	CQuakeItem::Spawn();
}

// Precache
void CItemAmmo::Precache( void )
{
	if (pev->spawnflags & BIG_AMMOBOX)
		PRECACHE_MODEL( (char*)STRING(m_isLargeBox) );
	else
		PRECACHE_MODEL( (char*)STRING(m_isSmallBox) );
}

BOOL CItemAmmo::MyTouch( CBasePlayer *pPlayer )
{
	if (pPlayer->pev->health <= 0)
		return FALSE;

	// Find the player's best weapon
	int iBestWeapon = pPlayer->W_BestWeapon();

	// Return if the player can't carry
	if (ammo_shells && pPlayer->m_iAmmoShells >= 100)
		return FALSE;
	if (ammo_nails && pPlayer->m_iAmmoNails >= 200)
		return FALSE;
	if (ammo_rockets && pPlayer->m_iAmmoRockets >= 100)
		return FALSE;
	if (ammo_cells && pPlayer->m_iAmmoCells >= 100)
		return FALSE;

	pPlayer->m_iAmmoShells += ammo_shells;
	pPlayer->m_iAmmoNails += ammo_nails;
	pPlayer->m_iAmmoRockets += ammo_rockets;
	pPlayer->m_iAmmoCells += ammo_cells;
	pPlayer->CheckAmmo();

	EMIT_SOUND( ENT(pev), CHAN_ITEM, "weapons/lock4.wav", 1, ATTN_NORM );

	// Change to a better weapon if possible
	if ( pPlayer->m_iQuakeWeapon == iBestWeapon )
	{
		 pPlayer->m_iQuakeWeapon = pPlayer->W_BestWeapon();
	}

	pPlayer->W_SetCurrentAmmo();
	return TRUE;
}

//===============
// Shells
class CItemAmmoShells : public CItemAmmo
{
public:
	void Spawn( void );
};
LINK_ENTITY_TO_CLASS(item_shells, CItemAmmoShells);

// Spawn
void CItemAmmoShells::Spawn( void )
{
	if ( gpGlobals->deathmatch == 4)
	{
		UTIL_Remove(this);
		return;
	}

	m_isSmallBox = MAKE_STRING("models/w_shotbox.mdl");
	m_isLargeBox = MAKE_STRING("models/w_shotbox_big.mdl");
	pev->netname = MAKE_STRING("shells");
	ammo_shells = 20;
	
	CItemAmmo::Spawn();
}

//===============
// Spikes
class CItemAmmoSpikes : public CItemAmmo
{
public:
	void Spawn( void );
};
LINK_ENTITY_TO_CLASS(item_spikes, CItemAmmoSpikes);

// Spawn
void CItemAmmoSpikes::Spawn( void )
{
	if ( gpGlobals->deathmatch == 4)
	{
		UTIL_Remove(this);
		return;
	}

	m_isSmallBox = MAKE_STRING("models/b_nail0.mdl");
	m_isLargeBox = MAKE_STRING("models/b_nail1.mdl");
	pev->netname = MAKE_STRING("nails");
	ammo_nails = 25;
	
	CItemAmmo::Spawn();
}

//===============
// Rockets
class CItemAmmoRockets : public CItemAmmo
{
public:
	void Spawn( void );
};
LINK_ENTITY_TO_CLASS(item_rockets, CItemAmmoRockets);

// Spawn
void CItemAmmoRockets::Spawn( void )
{
	if ( gpGlobals->deathmatch == 4)
	{
		UTIL_Remove(this);
		return;
	}

	m_isSmallBox = MAKE_STRING("models/w_rpgammo.mdl");
	m_isLargeBox = MAKE_STRING("models/w_rpgammo_big.mdl");
	pev->netname = MAKE_STRING("rockets");
	ammo_rockets = 5;
	
	CItemAmmo::Spawn();
}

//===============
// Cells
class CItemAmmoCells : public CItemAmmo
{
public:
	void Spawn( void );
};
LINK_ENTITY_TO_CLASS(item_cells, CItemAmmoCells);

// Spawn
void CItemAmmoCells::Spawn( void )
{
	if ( gpGlobals->deathmatch == 4)
	{
		UTIL_Remove(this);
		return;
	}

	m_isSmallBox = MAKE_STRING("models/w_battery.mdl");
	m_isLargeBox = MAKE_STRING("models/w_battery.mdl");
	pev->netname = MAKE_STRING("cells");
	ammo_cells = 6;
	
	CItemAmmo::Spawn();
}

//===============
// Weapon ammo
// Another method of placing ammo. Quake still uses it in some maps.
#define AW_SHOTGUN	1
#define AW_ROCKET	2
#define AW_SPIKES	4
#define AW_BIG		8

class CItemAmmoWeapon : public CItemAmmo
{
public:
	void Spawn( void );
};
LINK_ENTITY_TO_CLASS(item_weapon, CItemAmmoWeapon);

// Spawn
void CItemAmmoWeapon::Spawn( void )
{
	if ( gpGlobals->deathmatch == 4)
	{
		UTIL_Remove(this);
		return;
	}

	// Shells
	if (pev->spawnflags & AW_SHOTGUN)
	{
		m_isSmallBox = MAKE_STRING("models/w_shotbox.mdl");
		m_isLargeBox = MAKE_STRING("models/w_shotbox_big.mdl");
		pev->netname = MAKE_STRING("shells");
		ammo_shells = 20;
	}

	// Nails
	if (pev->spawnflags & AW_SPIKES)
	{
		m_isSmallBox = MAKE_STRING("models/b_nail0.mdl");
		m_isLargeBox = MAKE_STRING("models/b_nail1.mdl");
		pev->netname = MAKE_STRING("nails");
		ammo_nails = 25;
	}

	// Rockets
	if (pev->spawnflags & AW_ROCKET)
	{
		m_isSmallBox = MAKE_STRING("models/w_rpgammo.mdl");
		m_isLargeBox = MAKE_STRING("models/w_rpgammo_big.mdl");
		pev->netname = MAKE_STRING("rockets");
		ammo_rockets = 5;
	}

	// Big?
	if (pev->spawnflags & AW_BIG)
		pev->spawnflags = BIG_AMMOBOX;
	else
		pev->spawnflags = 0;

	CItemAmmo::Spawn();
}

//===============================================================================
// POWERUPS
//===============================================================================
class CItemPowerup : public CQuakeItem
{
public:
	BOOL MyTouch( CBasePlayer *pPlayer );

	int		m_iPowerupBit;
	float	invincible_finished;
	float	radsuit_finished;
	float	invisible_finished;
	float	super_damage_finished;
};

// Powerup Touch
BOOL CItemPowerup::MyTouch( CBasePlayer *pPlayer )
{
	if (pPlayer->pev->health <= 0)
		return FALSE;

	EMIT_SOUND( ENT(pev), CHAN_ITEM, STRING(pev->noise), 1, ATTN_NORM );

	pPlayer->m_iQuakeItems |= m_iPowerupBit;
	
	int iPowerUp = 0;

	// Invincibility
	if (invincible_finished)
	{
		// Make them glow red

		if ( pPlayer->m_iQuakeItems & IT_QUAD )
		{
			pPlayer->pev->renderfx = kRenderFxGlowShell;
			pPlayer->pev->rendercolor = Vector( 255, 125, 255 );	// RGB
			pPlayer->pev->renderamt = 100;	// Shell size
			
			iPowerUp = 3;
		}
		else
		{
			pPlayer->pev->renderfx = kRenderFxGlowShell;
			pPlayer->pev->rendercolor = Vector( 255, 128, 0 );	// RGB
			pPlayer->pev->renderamt = 100;	// Shell size

			iPowerUp = 2;
		}

		if ( pPlayer->m_iQuakeItems & IT_INVISIBILITY )
		{
			pPlayer->pev->rendermode = kRenderTransColor;
			pPlayer->pev->renderamt = 1;
		}
		pPlayer->m_flInvincibleFinished = gpGlobals->time + invincible_finished;

	}
	
	// Quad Damage
	if (super_damage_finished)
	{
		// Make them glow blue

		if ( pPlayer->m_iQuakeItems & IT_INVULNERABILITY )
		{
			pPlayer->pev->renderfx = kRenderFxGlowShell;
			pPlayer->pev->rendercolor = Vector( 255, 125, 255 );	// RGB
			pPlayer->pev->renderamt = 100;	// Shell size

			iPowerUp = 3;
		}
		else
		{
			pPlayer->pev->renderfx = kRenderFxGlowShell;
			pPlayer->pev->rendercolor = Vector( 128, 128, 255 );	// RGB
			pPlayer->pev->renderamt = 100;	// Shell size

			iPowerUp = 1;
		}

		if ( pPlayer->m_iQuakeItems & IT_INVISIBILITY )
		{
			pPlayer->pev->rendermode = kRenderTransColor;
			pPlayer->pev->renderamt = 1;
		}


		pPlayer->m_flSuperDamageFinished = gpGlobals->time + super_damage_finished;

		// Remove armor and cells if DM==4
		if (gpGlobals->deathmatch == 4)
		{
			pPlayer->pev->armortype = 0;
			pPlayer->pev->armorvalue = 0; 
			pPlayer->m_iAmmoCells = 0;
		}
	}

	// Radiation suit
	if (radsuit_finished)
		pPlayer->m_flRadsuitFinished = gpGlobals->time + radsuit_finished;

	// Invisibility
	if (invisible_finished)
	{
		pPlayer->m_flInvisibleFinished = gpGlobals->time + invisible_finished;

		pPlayer->pev->renderfx = kRenderFxGlowShell;
		pPlayer->pev->rendercolor = Vector( 128, 128, 128 );	// RGB
		pPlayer->pev->renderamt = 5;	// Shell size
	
	}
	
	// tell director about it
	MESSAGE_BEGIN( MSG_SPEC, SVC_DIRECTOR );
		WRITE_BYTE ( 9 );	// command length in bytes
		WRITE_BYTE ( DRC_CMD_EVENT );	// powerup pickup
		WRITE_SHORT( ENTINDEX(pPlayer->edict()) );	// player is primary target
		WRITE_SHORT( ENTINDEX(this->edict()) );	// powerup as second target
		WRITE_LONG( 9 );   // highst prio in game
	MESSAGE_END();

	pPlayer->W_SetCurrentAmmo();

	PLAYBACK_EVENT_FULL( FEV_GLOBAL | FEV_RELIABLE, 
	pPlayer->edict(), g_usPowerUp, 0, (float *)&g_vecZero, (float *)&g_vecZero, 
	(float)iPowerUp, 0.0, pPlayer->entindex(), pPlayer->pev->team, 0, 0 );

	return TRUE;
}


//===============
// Pentagram
class CItemPowerupInvincible : public CItemPowerup
{
public:
	void Spawn( void );
	void Precache( void );
};
LINK_ENTITY_TO_CLASS(item_artifact_invulnerability, CItemPowerupInvincible);

// Spawn
void CItemPowerupInvincible::Spawn( void )
{
	Precache();
	CQuakeItem::Spawn();

	m_flRespawnTime = 300;
	invincible_finished = 30;

	SET_MODEL(ENT(pev), "models/pow_invuln.mdl");
	pev->netname = MAKE_STRING("Pentagram of Protection");
	pev->noise = MAKE_STRING("items/protect.wav");
	m_iPowerupBit = IT_INVULNERABILITY;

	// Make it glow red
	pev->renderfx = kRenderFxGlowShell;
	pev->rendercolor = Vector( 255, 128, 0 );	// RGB
	pev->renderamt = 100; // Shellsize
}

// Precache
void CItemPowerupInvincible::Precache( void )
{
	PRECACHE_MODEL("models/pow_invuln.mdl");
	PRECACHE_SOUND("items/protect.wav");
	PRECACHE_SOUND("items/protect2.wav");
	PRECACHE_SOUND("items/protect3.wav");
}

//===============
// Radiation Suit
class CItemPowerupRadsuit : public CItemPowerup
{
public:
	void Spawn( void );
	void Precache( void );
};
LINK_ENTITY_TO_CLASS(item_artifact_envirosuit, CItemPowerupRadsuit);

// Spawn
void CItemPowerupRadsuit::Spawn( void )
{
	Precache();
	CQuakeItem::Spawn();

	m_flRespawnTime = 60;
	radsuit_finished = 30;

	SET_MODEL(ENT(pev), "models/suit.mdl");
	pev->netname = MAKE_STRING("Biosuit");
	pev->noise = MAKE_STRING("items/suit.wav");
	m_iPowerupBit = IT_SUIT;
}

// Precache
void CItemPowerupRadsuit::Precache( void )
{
	PRECACHE_MODEL("models/suit.mdl");
	PRECACHE_SOUND("items/suit.wav");
	PRECACHE_SOUND("items/suit2.wav");
}

//===============
// Ring of Invisibility
class CItemPowerupInvisibility : public CItemPowerup
{
public:
	void Spawn( void );
	void Precache( void );
};
LINK_ENTITY_TO_CLASS(item_artifact_invisibility, CItemPowerupInvisibility);

// Spawn
void CItemPowerupInvisibility::Spawn( void )
{
	Precache();
	CQuakeItem::Spawn();

	m_flRespawnTime = 300;
	invisible_finished = 30;

	SET_MODEL(ENT(pev), "models/pow_invis.mdl");
	pev->netname = MAKE_STRING("Ring of Shadows");
	pev->noise = MAKE_STRING("items/inv1.wav");
	m_iPowerupBit = IT_INVISIBILITY;

	pev->renderfx = kRenderFxGlowShell;
	pev->rendercolor = Vector( 128, 128, 128 );	// RGB
	pev->renderamt = 25;	// Shell size

	pev->rendermode = kRenderTransColor;
	pev->renderamt = 30;
}

// Precache
void CItemPowerupInvisibility::Precache( void )
{
	PRECACHE_MODEL("models/pow_invis.mdl");
	PRECACHE_SOUND("items/inv1.wav");
	PRECACHE_SOUND("items/inv2.wav");
	PRECACHE_SOUND("items/inv3.wav");
}

//===============
// Quad Damage
class CItemPowerupQuad : public CItemPowerup
{
public:
	void Spawn( void );
	void Precache( void );
};
LINK_ENTITY_TO_CLASS(item_artifact_super_damage, CItemPowerupQuad);

// Spawn
void CItemPowerupQuad::Spawn( void )
{
	Precache();
	CQuakeItem::Spawn();

	m_flRespawnTime = 60;
	super_damage_finished = 30;

	SET_MODEL(ENT(pev), "models/pow_quad.mdl");
	pev->netname = MAKE_STRING("Quad Damage");
	pev->noise = MAKE_STRING("items/damage.wav");
	m_iPowerupBit = IT_QUAD;

	// Make it glow blue
	pev->renderfx = kRenderFxGlowShell;
	pev->rendercolor = Vector( 128, 128, 255 );	// RGB
	pev->renderamt = 100;	// Shell size
}

// Precache
void CItemPowerupQuad::Precache( void )
{
	PRECACHE_MODEL("models/pow_quad.mdl");
	PRECACHE_SOUND("items/damage.wav");
	PRECACHE_SOUND("items/damage2.wav");
	PRECACHE_SOUND("items/damage3.wav");
}

//===============================================================================
// PLAYER BACKPACKS
//===============================================================================
class CItemBackpack : public CQuakeItem
{
public:
	void Spawn( void );
//	void SetBox ( void );
	virtual void SetObjectCollisionBox ( void );

	BOOL MyTouch( CBasePlayer *pPlayer );

	int  m_iItems;
	int	 ammo_shells;
	int	 ammo_nails;
	int  ammo_rockets;
	int	 ammo_cells;
};
LINK_ENTITY_TO_CLASS(item_backpack, CItemBackpack);

void CItemBackpack :: SetObjectCollisionBox( void )
{
	pev->absmin = pev->origin + Vector(-32, -32, 0);
	pev->absmax = pev->origin + Vector(32, 32, 56); 
}

// Spawn
void CItemBackpack::Spawn()
{
	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_TRIGGER;
	UTIL_SetOrigin( pev, pev->origin );
	SET_MODEL(ENT(pev), "models/backpack.mdl");

	SetTouch(&CItemBackpack::ItemTouch);
}

// Drop a backpack containing this player's ammo/weapons
void CBasePlayer::DropBackpack()
{
	// Any ammo to drop?
	if ( !(m_iAmmoShells + m_iAmmoNails + m_iAmmoRockets + m_iAmmoCells) )
		return;

	// Create the pack
	CItemBackpack *pPack = (CItemBackpack *)CBaseEntity::Create( "item_backpack", pev->origin - Vector(0, 0, 24), g_vecZero, edict() );
	pPack->pev->velocity = Vector( RANDOM_FLOAT(-100,100), RANDOM_FLOAT(-100,100), 300 );
	pPack->Spawn();
	
	// Put the player's weapon in the pack
	pPack->m_iItems = m_iQuakeWeapon;
	switch (pPack->m_iItems)
	{
	case IT_AXE:
		pPack->pev->netname = MAKE_STRING("Crowbar"); break;
	case IT_SHOTGUN:
		pPack->pev->netname = MAKE_STRING("Shotgun"); break;
	case IT_SUPER_SHOTGUN:
		pPack->pev->netname = MAKE_STRING("Double-barrelled Shotgun"); break;
	case IT_NAILGUN:
		pPack->pev->netname = MAKE_STRING("Nailgun"); break;
	case IT_SUPER_NAILGUN:
		pPack->pev->netname = MAKE_STRING("Super Nailgun"); break;
	case IT_GRENADE_LAUNCHER:
		pPack->pev->netname = MAKE_STRING("Grenade Launcher"); break;
	case IT_ROCKET_LAUNCHER:
		pPack->pev->netname = MAKE_STRING("Rocket Launcher"); break;
	case IT_LIGHTNING:
		pPack->pev->netname = MAKE_STRING("Thunderbolt"); break;
	default:
		pPack->pev->netname = MAKE_STRING("Invalid weapon."); break;
	}

	// Put the ammo in
	pPack->ammo_shells = m_iAmmoShells;
	pPack->ammo_nails = m_iAmmoNails;
	pPack->ammo_rockets = m_iAmmoRockets;
	pPack->ammo_cells = m_iAmmoCells;

	//Remove them from the player
	m_iAmmoShells = m_iAmmoNails = m_iAmmoRockets = m_iAmmoCells = 0;

	// Remove after 2 mins
	pPack->pev->nextthink = gpGlobals->time + 120;
	pPack->SetThink( &CItemBackpack::SUB_Remove );

	// Remove all weapons
	m_iQuakeItems = 0;
	m_iQuakeWeapon = 0;
}

// Pickup backpack
BOOL CItemBackpack::MyTouch( CBasePlayer *pPlayer )
{
	if (pPlayer->pev->health <= 0)
		return FALSE;
	if (gpGlobals->deathmatch == 4 && pPlayer->m_flInvincibleFinished > 0)
		return FALSE;


 
	if (gpGlobals->deathmatch == 4)
	{       
		pPlayer->pev->health += 10;
		ClientPrint( pPlayer->pev, HUD_PRINTNOTIFY, "#Additional_Health" );
		if ((pPlayer->pev->health > 250) && (pPlayer->pev->health < 300))
			EMIT_SOUND( ENT(pPlayer->pev), CHAN_ITEM, "items/protect3.wav", 1, ATTN_NORM );
		else
			EMIT_SOUND( ENT(pPlayer->pev), CHAN_ITEM, "weapons/lock4.wav", 1, ATTN_NORM );
		
		// Become invulnerable if the player's reached 300 health
		if (pPlayer->pev->health > 299)
		{               
			if (pPlayer->m_flInvincibleFinished == 0)
			{                       
				// Give player invincibility and quad
				pPlayer->m_flInvincibleFinished = gpGlobals->time + 30;
				pPlayer->m_flSuperDamageFinished = gpGlobals->time + 30;
				pPlayer->m_iQuakeItems |= (IT_INVULNERABILITY | IT_QUAD);
				pPlayer->m_iAmmoCells = 0;		

				// Make player glow red
				pPlayer->pev->renderfx = kRenderFxGlowShell;
				pPlayer->pev->rendercolor = Vector( 255, 128, 0 );	// RGB
				pPlayer->pev->renderamt = 100;	// Shell size

				EMIT_SOUND( ENT(pPlayer->pev), CHAN_VOICE, "items/sight1.wav", 1, ATTN_NORM );
				UTIL_ClientPrintAll( HUD_PRINTNOTIFY, "#Bonus_Power", STRING(pPlayer->pev->netname) );
			}
		}

		UTIL_Remove( this );

		// We've removed ourself, so don't let CQuakeItem handle respawn
		return FALSE;
	}

	BOOL bPrintComma = FALSE;

	// Get the weapon from the pack
	if (m_iItems)
	{
		if ( !(pPlayer->m_iQuakeItems & m_iItems) )
		{
			bPrintComma = TRUE;

			switch ( m_iItems )
			{
			case IT_SUPER_SHOTGUN:
				ClientPrint( pPlayer->pev, HUD_PRINTNOTIFY, "#You_Get_SS", UTIL_dtos1( ammo_shells ), UTIL_dtos2 ( ammo_nails ), UTIL_dtos3 ( ammo_rockets ), UTIL_dtos4 ( ammo_cells ) ); break;
			case IT_NAILGUN:
				ClientPrint( pPlayer->pev, HUD_PRINTNOTIFY, "#You_Get_NG", UTIL_dtos1( ammo_shells ), UTIL_dtos2 ( ammo_nails ), UTIL_dtos3 ( ammo_rockets ), UTIL_dtos4 ( ammo_cells ) ); break;
			case IT_SUPER_NAILGUN:
				ClientPrint( pPlayer->pev, HUD_PRINTNOTIFY, "#You_Get_SG", UTIL_dtos1( ammo_shells ), UTIL_dtos2 ( ammo_nails ), UTIL_dtos3 ( ammo_rockets ), UTIL_dtos4 ( ammo_cells ) ); break;
			case IT_GRENADE_LAUNCHER:
				ClientPrint( pPlayer->pev, HUD_PRINTNOTIFY, "#You_Get_GL", UTIL_dtos1( ammo_shells ), UTIL_dtos2 ( ammo_nails ), UTIL_dtos3 ( ammo_rockets ), UTIL_dtos4 ( ammo_cells ) ); break;
			case IT_ROCKET_LAUNCHER:
				ClientPrint( pPlayer->pev, HUD_PRINTNOTIFY, "#You_Get_RL", UTIL_dtos1( ammo_shells ), UTIL_dtos2 ( ammo_nails ), UTIL_dtos3 ( ammo_rockets ), UTIL_dtos4 ( ammo_cells ) ); break;
			case IT_LIGHTNING:
				ClientPrint( pPlayer->pev, HUD_PRINTNOTIFY, "#You_Get_LG", UTIL_dtos1( ammo_shells ), UTIL_dtos2 ( ammo_nails ), UTIL_dtos3 ( ammo_rockets ), UTIL_dtos4 ( ammo_cells ) ); break;
			}
		}
		else
			ClientPrint( pPlayer->pev, HUD_PRINTNOTIFY, "#You_Get_NoGun", UTIL_dtos1( ammo_shells ), UTIL_dtos2 ( ammo_nails ), UTIL_dtos3 ( ammo_rockets ), UTIL_dtos4 ( ammo_cells ) );
	}
 
	// Get ammo from pack
	pPlayer->m_iAmmoShells += ammo_shells;
	pPlayer->m_iAmmoNails += ammo_nails;
	pPlayer->m_iAmmoRockets += ammo_rockets;
	pPlayer->m_iAmmoCells += ammo_cells;
	pPlayer->CheckAmmo();

	int iNewWeapon = m_iItems;
	if (!iNewWeapon)
		iNewWeapon = pPlayer->m_iQuakeWeapon;
	int iOldWeapon = pPlayer->m_iQuakeItems;
	pPlayer->m_iQuakeItems |= m_iItems;

	// Give them at least 5 rockets in DM==3 and DM==5
	if ( (gpGlobals->deathmatch==3 || gpGlobals->deathmatch == 5) & ( (WeaponCode(iNewWeapon)==6) || (WeaponCode(iNewWeapon)==7) ) & (pPlayer->m_iAmmoRockets < 5) )
		pPlayer->m_iAmmoRockets = 5;

	EMIT_SOUND( ENT(pPlayer->pev), CHAN_ITEM, "weapons/lock4.wav", 1, ATTN_NORM );

	// Switch to a better weapon
	if ( WeaponCode(iNewWeapon) <= pPlayer->m_iBackpackSwitch )
	{
		if (pPlayer->pev->flags & FL_INWATER)
		{
			if (iNewWeapon != IT_LIGHTNING)
			{
				pPlayer->Deathmatch_Weapon(iOldWeapon, iNewWeapon);
			}
		}
		else
		{                
			pPlayer->Deathmatch_Weapon(iOldWeapon, iNewWeapon);
		}
	}
	pPlayer->W_SetCurrentAmmo();
	pPlayer->m_iClientQuakeWeapon  = -1;
	pPlayer->m_fWeapon = FALSE;
	pPlayer->m_fKnownItem = FALSE;
	pPlayer->UpdateClientData();

	UTIL_Remove( this );

	// We've removed ourself, so don't let CQuakeItem handle respawn
	return FALSE;
}

#if 0

/*
===============================================================================
KEYS
===============================================================================
*/

void() key_touch =
{
local entity    stemp;
local float             best;

	if (other.classname != "player")
		return;
	if (other.health <= 0)
		return;
	if (other.items & self.items)
		return;

	sprint (other, PRINT_LOW, "You got the ");
	sprint (other, PRINT_LOW, self.netname);
	sprint (other,PRINT_LOW, "\n");

	sound (other, CHAN_ITEM, self.noise, 1, ATTN_NORM);
	stuffcmd (other, "bf\n");
	other.items = other.items | self.items;

	self.solid = SOLID_NOT;
	self.model = string_null;

	activator = other;
	SUB_UseTargets();                               // fire all targets / killtargets
};


void() key_setsounds =
{
	if (world.worldtype == 0)
	{
		precache_sound ("misc/medkey.wav");
		self.noise = "misc/medkey.wav";
	}
	if (world.worldtype == 1)
	{
		precache_sound ("misc/runekey.wav");
		self.noise = "misc/runekey.wav";
	}
	if (world.worldtype == 2)
	{
		precache_sound2 ("misc/basekey.wav");
		self.noise = "misc/basekey.wav";
	}
};

/*QUAKED item_key1 (0 .5 .8) (-16 -16 -24) (16 16 32)
SILVER key
In order for keys to work
you MUST set your maps
worldtype to one of the
following:
0: medieval
1: metal
2: base
*/

void() item_key1 =
{
	if (world.worldtype == 0)
	{
		precache_model ("progs/w_s_key.mdl");
		setmodel (self, "progs/w_s_key.mdl");
		self.netname = "silver key";
	}
	else if (world.worldtype == 1)
	{
		precache_model ("progs/m_s_key.mdl");
		setmodel (self, "progs/m_s_key.mdl");
		self.netname = "silver runekey";
	}
	else if (world.worldtype == 2)
	{
		precache_model2 ("progs/b_s_key.mdl");
		setmodel (self, "progs/b_s_key.mdl");
		self.netname = "silver keycard";
	}
	key_setsounds();
	self.touch = key_touch;
	self.items = IT_KEY1;
	setsize (self, '-16 -16 -24', '16 16 32');
	StartItem ();
};

/*QUAKED item_key2 (0 .5 .8) (-16 -16 -24) (16 16 32)
GOLD key
In order for keys to work
you MUST set your maps
worldtype to one of the
following:
0: medieval
1: metal
2: base
*/

void() item_key2 =
{
	if (world.worldtype == 0)
	{
		precache_model ("progs/w_g_key.mdl");
		setmodel (self, "progs/w_g_key.mdl");
		self.netname = "gold key";
	}
	if (world.worldtype == 1)
	{
		precache_model ("progs/m_g_key.mdl");
		setmodel (self, "progs/m_g_key.mdl");
		self.netname = "gold runekey";
	}
	if (world.worldtype == 2)
	{
		precache_model2 ("progs/b_g_key.mdl");
		setmodel (self, "progs/b_g_key.mdl");
		self.netname = "gold keycard";
	}
	key_setsounds();
	self.touch = key_touch;
	self.items = IT_KEY2;
	setsize (self, '-16 -16 -24', '16 16 32');
	StartItem ();
};



/*
===============================================================================

END OF LEVEL RUNES

===============================================================================
*/

void() sigil_touch =
{
local entity    stemp;
local float             best;

	if (other.classname != "player")
		return;
	if (other.health <= 0)
		return;

	centerprint (other, "You got the rune!");

	sound (other, CHAN_ITEM, self.noise, 1, ATTN_NORM);
	stuffcmd (other, "bf\n");
	self.solid = SOLID_NOT;
	self.model = string_null;
	serverflags = serverflags | (self.spawnflags & 15);
	self.classname = "";            // so rune doors won't find it
	
	activator = other;
	SUB_UseTargets();                               // fire all targets / killtargets
};


/*QUAKED item_sigil (0 .5 .8) (-16 -16 -24) (16 16 32) E1 E2 E3 E4
End of level sigil, pick up to end episode and return to jrstart.
*/

void() item_sigil =
{
	if (!self.spawnflags)
		objerror ("no spawnflags");

	precache_sound ("misc/runekey.wav");
	self.noise = "misc/runekey.wav";

	if (self.spawnflags & 1)
	{
		precache_model ("progs/end1.mdl");
		setmodel (self, "progs/end1.mdl");
	}
	if (self.spawnflags & 2)
	{
		precache_model2 ("progs/end2.mdl");
		setmodel (self, "progs/end2.mdl");
	}
	if (self.spawnflags & 4)
	{
		precache_model2 ("progs/end3.mdl");
		setmodel (self, "progs/end3.mdl");
	}
	if (self.spawnflags & 8)
	{
		precache_model2 ("progs/end4.mdl");
		setmodel (self, "progs/end4.mdl");
	}
	
	self.touch = sigil_touch;
	setsize (self, '-16 -16 -24', '16 16 32');
	StartItem ();
};

void() q_touch =
{
local entity    stemp;
local float     best;
local string    s;

	if (other.classname != "player")
		return;
	if (other.health <= 0)
		return;

	self.mdl = self.model;

	sound (other, CHAN_VOICE, self.noise, 1, ATTN_NORM);
	stuffcmd (other, "bf\n");
	self.solid = SOLID_NOT;
	other.items = other.items | IT_QUAD;
	self.model = string_null;
		if (deathmatch == 4)
		{
			other.armortype = 0;
			other.armorvalue = 0 * 0.01;
			other.ammo_cells = 0;
		}

// do the apropriate action
	other.super_time = 1;
	other.super_damage_finished = self.cnt;

	s=ftos(rint(other.super_damage_finished - time));

	bprint (PRINT_LOW, other.netname);
	if (deathmatch == 4)
		bprint (PRINT_LOW, " recovered an OctaPower with ");
	else 
		bprint (PRINT_LOW, " recovered a Quad with ");
	bprint (PRINT_LOW, s);
	bprint (PRINT_LOW, " seconds remaining!\n");

	activator = other;
	SUB_UseTargets();                               // fire all targets / killtargets
};


void(float timeleft) DropQuad =
{
	local entity    item;

	item = spawn();
	item.origin = self.origin - '0 0 24';
	
	item.velocity_z = 300;
	item.velocity_x = -100 + (random() * 200);
	item.velocity_y = -100 + (random() * 200);
	
	item.flags = FL_ITEM;
	item.solid = SOLID_TRIGGER;
	item.movetype = MOVETYPE_TOSS;
	item.noise = "items/damage.wav";
	setmodel (item, "progs/quaddama.mdl");
	setsize (item, '-16 -16 -24', '16 16 32');
	item.cnt = time + timeleft;
	item.touch = q_touch;
	item.nextthink = time + timeleft;    // remove it with the time left on it
	item.think = SUB_Remove;
};


void() r_touch;

void() r_touch =
{
local entity    stemp;
local float     best;
local string    s;

	if (other.classname != "player")
		return;
	if (other.health <= 0)
		return;

	self.mdl = self.model;

	sound (other, CHAN_VOICE, self.noise, 1, ATTN_NORM);
	stuffcmd (other, "bf\n");
	self.solid = SOLID_NOT;
	other.items = other.items | IT_INVISIBILITY;
	self.model = string_null;

// do the apropriate action
	other.invisible_time = 1;
	other.invisible_finished = self.cnt;
	s=ftos(rint(other.invisible_finished - time));
	bprint (PRINT_LOW, other.netname);
	bprint (PRINT_LOW, " recovered a Ring with ");
	bprint (PRINT_LOW, s);
	bprint (PRINT_LOW, " seconds remaining!\n");
      

	activator = other;
	SUB_UseTargets();                               // fire all targets / killtargets
};


void(float timeleft) DropRing =
{
	local entity    item;

	item = spawn();
	item.origin = self.origin - '0 0 24';
	
	item.velocity_z = 300;
	item.velocity_x = -100 + (random() * 200);
	item.velocity_y = -100 + (random() * 200);
	
	item.flags = FL_ITEM;
	item.solid = SOLID_TRIGGER;
	item.movetype = MOVETYPE_TOSS;
	item.noise = "items/inv1.wav";
	setmodel (item, "progs/invisibl.mdl");
	setsize (item, '-16 -16 -24', '16 16 32');
	item.cnt = time + timeleft;
	item.touch = r_touch;
	item.nextthink = time + timeleft;    // remove after 30 seconds
	item.think = SUB_Remove;
};
#endif
