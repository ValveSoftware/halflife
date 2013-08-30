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
/*

===== quake_weapons.cpp ========================================================

  Quake weaponry

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"
#include "quake_gun.h"

char gszQ_DeathType[128];
DLL_GLOBAL	short	g_sModelIndexNail;

#ifdef THREEWAVE
extern unsigned short g_usHook;	
extern unsigned short g_usCable;
extern unsigned short g_usCarried;
#endif

#ifdef CLIENT_DLL
#include "cl_entity.h"
struct cl_entity_s *GetViewEntity( void );
extern float g_flLightTime;
#endif

// called by worldspawn
void QuakeClassicPrecache( void )
{
	// Weapon sounds
	PRECACHE_SOUND("weapons/ax1.wav");
	PRECACHE_SOUND("player/axhit2.wav");
	PRECACHE_SOUND("player/axhitbod.wav");
	PRECACHE_SOUND("weapons/r_exp3.wav");  // new rocket explosion
	PRECACHE_SOUND("weapons/rocket1i.wav");// spike gun
	PRECACHE_SOUND("weapons/sgun1.wav");
	PRECACHE_SOUND("weapons/lhit.wav");
	PRECACHE_SOUND("weapons/guncock.wav"); // player shotgun
	PRECACHE_SOUND("weapons/ric1.wav");    // ricochet (used in c code)
	PRECACHE_SOUND("weapons/ric2.wav");    // ricochet (used in c code)
	PRECACHE_SOUND("weapons/ric3.wav");    // ricochet (used in c code)
	PRECACHE_SOUND("weapons/spike2.wav");  // super spikes
	PRECACHE_SOUND("weapons/tink1.wav");   // spikes tink (used in c code)
	PRECACHE_SOUND("weapons/grenade.wav"); // grenade launcher
	PRECACHE_SOUND("weapons/bounce.wav");  // grenade bounce
	PRECACHE_SOUND("weapons/shotgn2.wav"); // super shotgun
	PRECACHE_SOUND("weapons/lstart.wav");  // lightning start

	// Weapon models
	PRECACHE_MODEL("models/v_crowbar.mdl");
	PRECACHE_MODEL("models/v_shot.mdl");
	PRECACHE_MODEL("models/v_shot2.mdl");
	PRECACHE_MODEL("models/v_nail.mdl");
	PRECACHE_MODEL("models/v_nail2.mdl");
	PRECACHE_MODEL("models/v_rock.mdl");
	PRECACHE_MODEL("models/v_rock2.mdl");
	PRECACHE_MODEL("models/v_light.mdl");
 
	// Weapon player models
	PRECACHE_MODEL("models/p_crowbar.mdl");
	PRECACHE_MODEL("models/p_rock2.mdl");
	PRECACHE_MODEL("models/p_rock.mdl");
	PRECACHE_MODEL("models/p_shot2.mdl");
	PRECACHE_MODEL("models/p_nail.mdl");
	PRECACHE_MODEL("models/p_nail2.mdl");
	PRECACHE_MODEL("models/p_light.mdl");
	PRECACHE_MODEL("models/p_shot.mdl");
	

	// Weapon effect models
	PRECACHE_MODEL("models/rocket.mdl");	// rocket
	PRECACHE_MODEL("models/grenade.mdl");	// grenade
	g_sModelIndexNail = PRECACHE_MODEL("models/spike.mdl");
	g_sModelIndexLaser = PRECACHE_MODEL("sprites/laserbeam.spr");
	PRECACHE_MODEL("sprites/smoke.spr");

	// Powerup models

	// Powerup sounds
	PRECACHE_SOUND("items/damage3.wav");
	PRECACHE_SOUND("items/sight1.wav");

	// Teleport sounds
	PRECACHE_SOUND("misc/r_tele1.wav");
	PRECACHE_SOUND("misc/r_tele2.wav");
	PRECACHE_SOUND("misc/r_tele3.wav");
	PRECACHE_SOUND("misc/r_tele4.wav");
	PRECACHE_SOUND("misc/r_tele5.wav");

	// Misc
	PRECACHE_SOUND("weapons/lock4.wav");
	PRECACHE_SOUND("weapons/pkup.wav");
	PRECACHE_SOUND("items/itembk2.wav");
	PRECACHE_MODEL("models/backpack.mdl");

#ifdef THREEWAVE
	PRECACHE_MODEL("models/v_grapple.mdl");
#endif

}

//================================================================================================
// WEAPON SELECTION
//================================================================================================
// Return the ID of the best weapon being carried by the player
int CBasePlayer::W_BestWeapon()
{
	if (pev->waterlevel <= 1 && m_iAmmoCells >= 1 && (m_iQuakeItems & IT_LIGHTNING) )
		return IT_LIGHTNING;
	else if(m_iAmmoNails >= 2 && (m_iQuakeItems & IT_SUPER_NAILGUN) )
		return IT_SUPER_NAILGUN;
	else if(m_iAmmoShells >= 2 && (m_iQuakeItems & IT_SUPER_SHOTGUN) )
		return IT_SUPER_SHOTGUN;
	else if(m_iAmmoNails >= 1 && (m_iQuakeItems & IT_NAILGUN) )
		return IT_NAILGUN;
	else if(m_iAmmoShells >= 1 && (m_iQuakeItems & IT_SHOTGUN)  )
		return IT_SHOTGUN;
		
	return IT_AXE;
}

// Weapon setup after weapon switch
void CBasePlayer::W_SetCurrentAmmo( int sendanim /* = 1 */ )
{
	m_iQuakeItems &= ~(IT_SHELLS | IT_NAILS | IT_ROCKETS | IT_CELLS);
	int	iszViewModel = 0;
	char *viewmodel = "";
	int iszWeaponModel = 0;
	char *szAnimExt;
	
	// Find out what weapon the player's using
	if (m_iQuakeWeapon == IT_AXE)
	{
		m_pCurrentAmmo = NULL;
		viewmodel = "models/v_crowbar.mdl";
		iszViewModel = MAKE_STRING(viewmodel);
		szAnimExt = "crowbar";
		iszWeaponModel = MAKE_STRING("models/p_crowbar.mdl");
	}
	else if (m_iQuakeWeapon == IT_SHOTGUN)
	{
		m_pCurrentAmmo = &m_iAmmoShells;
		viewmodel = "models/v_shot.mdl";
		iszViewModel = MAKE_STRING(viewmodel);
		iszWeaponModel = MAKE_STRING("models/p_shot.mdl");
		m_iQuakeItems |= IT_SHELLS;
		szAnimExt = "shotgun";
	}
	else if (m_iQuakeWeapon == IT_SUPER_SHOTGUN)
	{
		m_pCurrentAmmo = &m_iAmmoShells;
		viewmodel = "models/v_shot2.mdl";
		iszViewModel = MAKE_STRING(viewmodel);
		iszWeaponModel = MAKE_STRING("models/p_shot2.mdl");
		m_iQuakeItems |= IT_SHELLS;
		szAnimExt = "shotgun";
	}
	else if (m_iQuakeWeapon == IT_NAILGUN)
	{
		m_pCurrentAmmo = &m_iAmmoNails;
		viewmodel = "models/v_nail.mdl";
		iszViewModel = MAKE_STRING(viewmodel);
		iszWeaponModel = MAKE_STRING("models/p_nail.mdl");
		m_iQuakeItems |= IT_NAILS;
		szAnimExt = "mp5";
	}
	else if (m_iQuakeWeapon == IT_SUPER_NAILGUN)
	{
		m_pCurrentAmmo = &m_iAmmoNails;
		viewmodel = "models/v_nail2.mdl";
		iszViewModel = MAKE_STRING(viewmodel);
		iszWeaponModel = MAKE_STRING("models/p_nail2.mdl");
		m_iQuakeItems |= IT_NAILS;
		szAnimExt = "mp5";
	}
	else if (m_iQuakeWeapon == IT_GRENADE_LAUNCHER)
	{
		m_pCurrentAmmo = &m_iAmmoRockets;
		viewmodel = "models/v_rock.mdl";
		iszViewModel = MAKE_STRING(viewmodel);
		m_iQuakeItems |= IT_ROCKETS;
		iszWeaponModel = MAKE_STRING("models/p_rock.mdl");
		szAnimExt = "gauss";
	}
	else if (m_iQuakeWeapon == IT_ROCKET_LAUNCHER)
	{
		m_pCurrentAmmo = &m_iAmmoRockets;
		viewmodel = "models/v_rock2.mdl";
		iszViewModel = MAKE_STRING(viewmodel);
		m_iQuakeItems |= IT_ROCKETS;
		iszWeaponModel = MAKE_STRING("models/p_rock2.mdl");
		szAnimExt = "gauss";
	}
	else if (m_iQuakeWeapon == IT_LIGHTNING)
	{
		m_pCurrentAmmo = &m_iAmmoCells;
		viewmodel = "models/v_light.mdl";
		iszViewModel = MAKE_STRING(viewmodel);
		iszWeaponModel = MAKE_STRING("models/p_light.mdl");
		m_iQuakeItems |= IT_CELLS;
		szAnimExt = "gauss";
	}
#ifdef THREEWAVE
	else if (m_iQuakeWeapon == IT_EXTRA_WEAPON)
	{
		m_pCurrentAmmo = NULL;
		viewmodel = "models/v_grapple.mdl";
		iszViewModel = MAKE_STRING(viewmodel);
		szAnimExt = "crowbar";
	}
#endif

	else
	{
		m_pCurrentAmmo = NULL;
	}

#if !defined( CLIENT_DLL )

	pev->viewmodel = iszViewModel;

	pev->weaponmodel = iszWeaponModel;
	strcpy( m_szAnimExtention, szAnimExt );

#else
	{

		int HUD_GetModelIndex( char *modelname );
		pev->viewmodel = HUD_GetModelIndex( viewmodel );
		
		cl_entity_t *view;
		view = GetViewEntity();

		//Adrian - The actual "magic" is done in the
		//Studio drawing code.
		if ( m_iQuakeItems & IT_INVISIBILITY )
		{
			if( view )
			{
				view->curstate.renderfx = kRenderFxGlowShell;
				view->curstate.renderamt = 5;
					
				view->curstate.rendercolor.r = 125;
				view->curstate.rendercolor.g = 125;
				view->curstate.rendercolor.b = 125;
			}
		}
		else
		{
			if ( m_iQuakeItems & IT_INVULNERABILITY )
			{
				if( view )
				{
					view->curstate.renderfx = kRenderFxGlowShell;
					view->curstate.renderamt = 15;
						
					view->curstate.rendercolor.r = 255;
					view->curstate.rendercolor.g = 125;
					view->curstate.rendercolor.b = 125;
				}
			}
			else if ( m_iQuakeItems & IT_QUAD )
			{
				if( view )
				{
					view->curstate.renderfx = kRenderFxGlowShell;
					view->curstate.renderamt = 15;
						
					view->curstate.rendercolor.r = 125;
					view->curstate.rendercolor.g = 125;
					view->curstate.rendercolor.b = 255;
				}
			}
			else if ( m_iQuakeItems & ( IT_INVULNERABILITY | IT_QUAD ) )
			{
				if( view )
				{
					view->curstate.renderfx = kRenderFxGlowShell;
					view->curstate.renderamt = 15;
						
					view->curstate.rendercolor.r = 255;
					view->curstate.rendercolor.g = 125;
					view->curstate.rendercolor.b = 255;
				}
			}
			else 
				view->curstate.renderfx = kRenderFxNone; // Clear it.
		}
	}
#endif
}

// Return TRUE if the weapon still has ammo
BOOL CBasePlayer::W_CheckNoAmmo()
{
	if ( m_pCurrentAmmo && *m_pCurrentAmmo > 0 )
		return TRUE;

	if ( m_iQuakeWeapon == IT_AXE )
		return TRUE;

#ifdef THREEWAVE
	if ( m_iQuakeWeapon == IT_EXTRA_WEAPON )
		return TRUE;
#endif
	
	if ( m_iQuakeWeapon == IT_LIGHTNING )
	{
		 PLAYBACK_EVENT_FULL( FEV_NOTHOST, edict(), m_usLightning, 0, (float *)&pev->origin, (float *)&pev->angles, 0.0, 0.0, 0, 1, 0, 0 );

		 if ( m_pActiveItem )
			  ((CQuakeGun*)m_pActiveItem)->DestroyEffect();
	}
	
	m_iQuakeWeapon = W_BestWeapon();
	W_SetCurrentAmmo();
	return FALSE;
}

// Change to the specified weapon
void CBasePlayer::W_ChangeWeapon( int iWeaponNumber )
{
	if ( m_iQuakeWeapon == IT_LIGHTNING )
	{
		 PLAYBACK_EVENT_FULL( FEV_NOTHOST, edict(), m_usLightning, 0, (float *)&pev->origin, (float *)&pev->angles, 0.0, 0.0, 0, 1, 0, 0 );

		 if ( m_pActiveItem )
			  ((CQuakeGun*)m_pActiveItem)->DestroyEffect();
	}
		 
	int iWeapon = 0;
	BOOL bHaveAmmo = TRUE;
	
	if (iWeaponNumber == 1)
	{
		iWeapon = IT_AXE;
	}
	else if (iWeaponNumber == 2)
	{
		iWeapon = IT_SHOTGUN;
		if (m_iAmmoShells < 1)
			bHaveAmmo = FALSE;
	}
	else if (iWeaponNumber == 3)
	{
		iWeapon = IT_SUPER_SHOTGUN;
		if (m_iAmmoShells < 2)
			bHaveAmmo = FALSE;
	}               
	else if (iWeaponNumber == 4)
	{
		iWeapon = IT_NAILGUN;
		if (m_iAmmoNails < 1)
			bHaveAmmo = FALSE;
	}
	else if (iWeaponNumber == 5)
	{
		iWeapon = IT_SUPER_NAILGUN;
		if (m_iAmmoNails < 2)
			bHaveAmmo = FALSE;
	}
	else if (iWeaponNumber == 6)
	{
		iWeapon = IT_GRENADE_LAUNCHER;
		if (m_iAmmoRockets < 1)
			bHaveAmmo = FALSE;
	}
	else if (iWeaponNumber == 7)
	{
		iWeapon = IT_ROCKET_LAUNCHER;
		if (m_iAmmoRockets < 1)
			bHaveAmmo = FALSE;
	}
	else if (iWeaponNumber == 8)
	{
		iWeapon = IT_LIGHTNING;
		
		if (m_iAmmoCells < 1)
			bHaveAmmo = FALSE;
	}
#ifdef THREEWAVE
	else if (iWeaponNumber == 9)
	{
		iWeapon = IT_EXTRA_WEAPON;
	}
#endif

	// Have the weapon?
	if ( !(m_iQuakeItems & iWeapon) )
	{       
		ClientPrint( pev, HUD_PRINTCONSOLE, "#No_Weapon" );
		return;
	}
	
	// Have ammo for it?
	if ( !bHaveAmmo )
	{
		ClientPrint( pev, HUD_PRINTCONSOLE, "#No_Ammo" );
		return;
	}

	// Set weapon, update ammo
	m_iQuakeWeapon = iWeapon;
	W_SetCurrentAmmo();

#ifdef CLIENT_DLL
	g_flLightTime = 0.0;
#endif
}

// Go to the next weapon with ammo
void CBasePlayer::W_CycleWeaponCommand( void )
{
	while (1)
	{
		BOOL bHaveAmmo = TRUE;

		if (m_iQuakeWeapon == IT_LIGHTNING)
		{
			m_iQuakeWeapon = IT_EXTRA_WEAPON;
		}
		else if (m_iQuakeWeapon == IT_EXTRA_WEAPON)
		{
			m_iQuakeWeapon = IT_AXE;
		}
		else if (m_iQuakeWeapon == IT_AXE)
		{
			m_iQuakeWeapon = IT_SHOTGUN;
			if (m_iAmmoShells < 1)
				bHaveAmmo = FALSE;
		}
		else if (m_iQuakeWeapon == IT_SHOTGUN)
		{
			m_iQuakeWeapon = IT_SUPER_SHOTGUN;
			if (m_iAmmoShells < 2)
				bHaveAmmo = FALSE;
		}               
		else if (m_iQuakeWeapon == IT_SUPER_SHOTGUN)
		{
			m_iQuakeWeapon = IT_NAILGUN;
			if (m_iAmmoNails < 1)
				bHaveAmmo = FALSE;
		}
		else if (m_iQuakeWeapon == IT_NAILGUN)
		{
			m_iQuakeWeapon = IT_SUPER_NAILGUN;
			if (m_iAmmoNails < 2)
				bHaveAmmo = FALSE;
		}
		else if (m_iQuakeWeapon == IT_SUPER_NAILGUN)
		{
			m_iQuakeWeapon = IT_GRENADE_LAUNCHER;
			if (m_iAmmoRockets < 1)
				bHaveAmmo = FALSE;
		}
		else if (m_iQuakeWeapon == IT_GRENADE_LAUNCHER)
		{
			m_iQuakeWeapon = IT_ROCKET_LAUNCHER;
			if (m_iAmmoRockets < 1)
				bHaveAmmo = FALSE;
		}
		else if (m_iQuakeWeapon == IT_ROCKET_LAUNCHER)
		{
			m_iQuakeWeapon = IT_LIGHTNING;
			if (m_iAmmoCells < 1)
				bHaveAmmo = FALSE;
		}
	
		if ( (m_iQuakeItems & m_iQuakeWeapon) && bHaveAmmo )
		{
			W_SetCurrentAmmo();
			return;
		}
	}

}

// Go to the prev weapon with ammo
void CBasePlayer::W_CycleWeaponReverseCommand()
{
	while (1)
	{
		BOOL bHaveAmmo = TRUE;

		if (m_iQuakeWeapon == IT_EXTRA_WEAPON)
		{
			m_iQuakeWeapon = IT_LIGHTNING;
		}
		else if (m_iQuakeWeapon == IT_LIGHTNING)
		{
			m_iQuakeWeapon = IT_ROCKET_LAUNCHER;
			if (m_iAmmoRockets < 1)
				bHaveAmmo = FALSE;
		}
		else if (m_iQuakeWeapon == IT_ROCKET_LAUNCHER)
		{
			m_iQuakeWeapon = IT_GRENADE_LAUNCHER;
			if (m_iAmmoRockets < 1)
				bHaveAmmo = FALSE;
		}
		else if (m_iQuakeWeapon == IT_GRENADE_LAUNCHER)
		{
			m_iQuakeWeapon = IT_SUPER_NAILGUN;
			if (m_iAmmoNails < 2)
				bHaveAmmo = FALSE;
		}
		else if (m_iQuakeWeapon == IT_SUPER_NAILGUN)
		{
			m_iQuakeWeapon = IT_NAILGUN;
			if (m_iAmmoNails < 1)
				bHaveAmmo = FALSE;
		}
		else if (m_iQuakeWeapon == IT_NAILGUN)
		{
			m_iQuakeWeapon = IT_SUPER_SHOTGUN;
			if (m_iAmmoShells < 2)
				bHaveAmmo = FALSE;
		}               
		else if (m_iQuakeWeapon == IT_SUPER_SHOTGUN)
		{
			m_iQuakeWeapon = IT_SHOTGUN;
			if (m_iAmmoShells < 1)
				bHaveAmmo = FALSE;
		}
		else if (m_iQuakeWeapon == IT_SHOTGUN)
		{
			m_iQuakeWeapon = IT_AXE;
		}
		else if (m_iQuakeWeapon == IT_AXE)
		{
			m_iQuakeWeapon = IT_EXTRA_WEAPON;
		}
		else if (m_iQuakeWeapon == IT_EXTRA_WEAPON)
		{
			m_iQuakeWeapon = IT_LIGHTNING;
			if (m_iAmmoCells < 1)
				bHaveAmmo = FALSE;
		}

	
		if ( (m_iQuakeItems & m_iQuakeWeapon) && bHaveAmmo )
		{
			W_SetCurrentAmmo();
			return;
		}
	}

}

//================================================================================================
// WEAPON FUNCTIONS
//================================================================================================
// Returns true if the inflictor can directly damage the target.  Used for explosions and melee attacks.
float Q_CanDamage(CBaseEntity *pTarget, CBaseEntity *pInflictor) 
{
	TraceResult trace;

	// bmodels need special checking because their origin is 0,0,0
	if (pTarget->pev->movetype == MOVETYPE_PUSH)
	{
		UTIL_TraceLine( pInflictor->pev->origin, 0.5 * (pTarget->pev->absmin + pTarget->pev->absmax), ignore_monsters, NULL, &trace );
		if (trace.flFraction == 1)
			return TRUE;
		CBaseEntity *pEntity = CBaseEntity::Instance(trace.pHit);
		if (pEntity == pTarget)
			return TRUE;
		return FALSE;
	}
	
	UTIL_TraceLine( pInflictor->pev->origin, pTarget->pev->origin, ignore_monsters, NULL, &trace );
	if (trace.flFraction == 1)
		return TRUE;
	UTIL_TraceLine( pInflictor->pev->origin, pTarget->pev->origin + Vector(15,15,0), ignore_monsters, NULL, &trace );
	if (trace.flFraction == 1)
		return TRUE;
	UTIL_TraceLine( pInflictor->pev->origin, pTarget->pev->origin + Vector(-15,-15,0), ignore_monsters, NULL, &trace );
	if (trace.flFraction == 1)
		return TRUE;
	UTIL_TraceLine( pInflictor->pev->origin, pTarget->pev->origin + Vector(-15,15,0), ignore_monsters, NULL, &trace );
	if (trace.flFraction == 1)
		return TRUE;
	UTIL_TraceLine( pInflictor->pev->origin, pTarget->pev->origin + Vector(15,-15,0), ignore_monsters, NULL, &trace );
	if (trace.flFraction == 1)
		return TRUE;

	return FALSE;
}

// Quake Bullet firing
void CBasePlayer::Q_FireBullets(int iShots, Vector vecDir, Vector vecSpread)
{
	TraceResult trace;
	UTIL_MakeVectors(pev->v_angle);

	Vector vecSrc = pev->origin + (gpGlobals->v_forward * 10);
	vecSrc.z = pev->absmin.z + (pev->size.z * 0.7);
	ClearMultiDamage();

	while ( iShots > 0 )
	{
		Vector vecPath = vecDir + ( RANDOM_FLOAT( -1, 1 ) * vecSpread.x * gpGlobals->v_right ) + ( RANDOM_FLOAT( -1, 1 ) * vecSpread.y * gpGlobals->v_up );
		Vector vecEnd = vecSrc + ( vecPath * 2048 );
		UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, ENT(pev), &trace );
		if (trace.flFraction != 1.0)
		{
			CBaseEntity *pEntity = CBaseEntity::Instance(trace.pHit);
			if (pEntity && pEntity->pev->takedamage && pEntity->IsPlayer() )
			{
				pEntity->TraceAttack(pev, 4, vecPath, &trace, DMG_BULLET);
				//AddMultiDamage(pev, pEntity, 4, DMG_BULLET);
			}
			else if ( pEntity && pEntity->pev->takedamage )
			{
				pEntity->TakeDamage( pev, pev, 4, DMG_BULLET );
			}
		}

		iShots--;
	}

	ApplyMultiDamage( pev, pev );
}

#if !defined( CLIENT_DLL )
// Quake Radius damage
void Q_RadiusDamage( CBaseEntity *pInflictor, CBaseEntity *pAttacker, float flDamage, CBaseEntity *pIgnore )
{
	CBaseEntity *pEnt = NULL;

	while ( (pEnt = UTIL_FindEntityInSphere( pEnt, pInflictor->pev->origin, flDamage+40 )) != NULL )
	{
		if (pEnt != pIgnore)
		{
			if (pEnt->pev->takedamage)
			{
				Vector vecOrg = pEnt->pev->origin + ((pEnt->pev->mins + pEnt->pev->maxs) * 0.5);
				float flPoints = 0.5 * (pInflictor->pev->origin - vecOrg).Length();
				if (flPoints < 0)
					flPoints = 0;
				flPoints = flDamage - flPoints;
				
				if (pEnt == pAttacker)
					flPoints = flPoints * 0.5;
				if (flPoints > 0)
				{
					if ( Q_CanDamage( pEnt, pInflictor ) )
						pEnt->TakeDamage( pInflictor->pev, pAttacker->pev, flPoints, DMG_GENERIC );
				}
			}
		}
	}
}
#endif

// Lightning hit a target
void LightningHit(CBaseEntity *pTarget, CBaseEntity *pAttacker, Vector vecHitPos, float flDamage, TraceResult *ptr, Vector vecDir ) 
{

#ifndef CLIENT_DLL
	SpawnBlood( vecHitPos, BLOOD_COLOR_RED, flDamage * 1.5 );

	if ( g_pGameRules->PlayerRelationship( pTarget, pAttacker ) != GR_TEAMMATE )
	{

		pTarget->TakeDamage( pAttacker->pev, pAttacker->pev, flDamage, DMG_GENERIC );
		pTarget->TraceBleed( flDamage, vecDir, ptr, DMG_BULLET ); // have to use DMG_BULLET or it wont spawn.

	}
#endif
}

// Lightning Damage
void CBasePlayer::LightningDamage( Vector p1, Vector p2, CBaseEntity *pAttacker, float flDamage, Vector vecDir)
{
#if !defined( CLIENT_DLL )
	TraceResult trace;
	Vector vecThru = (p2 - p1).Normalize();
	vecThru.x = 0 - vecThru.y;
	vecThru.y = vecThru.x;
	vecThru.z = 0;
	vecThru = vecThru * 16;

	CBaseEntity *pEntity1 = NULL;
	CBaseEntity *pEntity2 = NULL;

	// Hit first target?
	UTIL_TraceLine( p1, p2, dont_ignore_monsters, ENT(pev), &trace );
	CBaseEntity *pEntity = CBaseEntity::Instance(trace.pHit);
	if (pEntity && pEntity->pev->takedamage)
	{
		LightningHit(pEntity, pAttacker, trace.vecEndPos, flDamage, &trace, vecDir );
	}
	pEntity1 = pEntity;

	// Hit second target?
	UTIL_TraceLine( p1 + vecThru, p2 + vecThru, dont_ignore_monsters, ENT(pev), &trace );
	pEntity = CBaseEntity::Instance(trace.pHit);
	if (pEntity && pEntity != pEntity1 && pEntity->pev->takedamage)
	{
		LightningHit(pEntity, pAttacker, trace.vecEndPos, flDamage, &trace, vecDir );
	}
	pEntity2 = pEntity;

	// Hit third target?
	UTIL_TraceLine( p1 - vecThru, p2 - vecThru, dont_ignore_monsters, ENT(pev), &trace );
	pEntity = CBaseEntity::Instance(trace.pHit);
	if (pEntity && pEntity != pEntity1 && pEntity != pEntity2 && pEntity->pev->takedamage)
	{
		LightningHit(pEntity, pAttacker, trace.vecEndPos, flDamage, &trace, vecDir );
	}
#endif
}

//================================================================================================
// WEAPON FIRING
//================================================================================================
// Axe
void CBasePlayer::W_FireAxe()
{
	TraceResult trace;
	Vector vecSrc = pev->origin + Vector(0, 0, 16);

	// Swing forward 64 units
	UTIL_MakeVectors(pev->v_angle);
	UTIL_TraceLine( vecSrc, vecSrc + (gpGlobals->v_forward * 64), dont_ignore_monsters, ENT(pev), &trace );
	if (trace.flFraction == 1.0)
		return;
	
	Vector vecOrg = trace.vecEndPos - gpGlobals->v_forward * 4;

	CBaseEntity *pEntity = CBaseEntity::Instance(trace.pHit);
	if (pEntity && pEntity->pev->takedamage)
	{
		pEntity->m_bAxHitMe = TRUE;
		int iDmg = 20;
		if (gpGlobals->deathmatch > 3)
			iDmg = 75;

		pEntity->TakeDamage( pev, pev, iDmg, DMG_GENERIC );

#ifndef CLIENT_DLL
		if ( g_pGameRules->PlayerRelationship( this, pEntity ) != GR_TEAMMATE )
			 SpawnBlood( vecOrg, BLOOD_COLOR_RED, iDmg * 4 ); // Make a lot of Blood!
#endif
	}
}

// Single barrel shotgun
void CBasePlayer::W_FireShotgun( int iQuadSound )
{
	PLAYBACK_EVENT_FULL( FEV_NOTHOST, edict(), m_usShotgunSingle, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, iQuadSound, 0, 0, 0 );

	if (gpGlobals->deathmatch != 4 )
		*m_pCurrentAmmo -= 1;

	Vector vecDir = GetAutoaimVector( AUTOAIM_5DEGREES );
	Q_FireBullets(6, vecDir, Vector(0.04, 0.04, 0) );
}


#ifdef THREEWAVE
void CBasePlayer::W_FireHook( void )
{
	PLAYBACK_EVENT_FULL( FEV_NOTHOST | FEV_GLOBAL, edict(), g_usHook, 0, (float *)&pev->origin, (float *)&pev->angles, 0.0, 0.0, 0, 0, 0, 0 );

	Throw_Grapple();
}
#endif

#ifdef THREEWAVE

#ifdef CLIENT_DLL
unsigned short g_usCable;
unsigned short g_usHook;	
unsigned short g_usCarried;

void CBasePlayer::Throw_Grapple( void )
{
}
#endif
#endif

// Double barrel shotgun
void CBasePlayer::W_FireSuperShotgun( int iQuadSound )
{
	if (*m_pCurrentAmmo == 1)
	{
		W_FireShotgun( iQuadSound );
		return;
	}

	PLAYBACK_EVENT_FULL( FEV_NOTHOST, edict(), m_usShotgunDouble, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, iQuadSound, 0, 0, 0 );

	if (gpGlobals->deathmatch != 4 )
		*m_pCurrentAmmo -= 2;
	Vector vecDir = GetAutoaimVector( AUTOAIM_5DEGREES );
	Q_FireBullets(14, vecDir, Vector(0.14, 0.08, 0) );
};

// Rocket launcher
void CBasePlayer::W_FireRocket( int iQuadSound )
{
	PLAYBACK_EVENT_FULL( FEV_NOTHOST, edict(), m_usRocket, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, iQuadSound, 0, 0, 0 );

	if (gpGlobals->deathmatch != 4 )
		*m_pCurrentAmmo -= 1;

	// Create the rocket
	UTIL_MakeVectors( pev->v_angle );
	Vector vecOrg = pev->origin + (gpGlobals->v_forward * 8) + Vector(0,0,16);
	Vector vecDir = GetAutoaimVector( AUTOAIM_5DEGREES );
	CQuakeRocket *pRocket = CQuakeRocket::CreateRocket( vecOrg, vecDir, this );
}

// Grenade launcher
void CBasePlayer::W_FireGrenade( int iQuadSound )
{       
	PLAYBACK_EVENT_FULL( FEV_NOTHOST, edict(), m_usGrenade, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, iQuadSound, 0, 0, 0 );

	if (gpGlobals->deathmatch != 4 )
		*m_pCurrentAmmo -= 1;

	// Get initial velocity
	UTIL_MakeVectors( pev->v_angle );
	Vector vecVelocity;
	if ( pev->v_angle.x )
	{
		vecVelocity = gpGlobals->v_forward * 600 + gpGlobals->v_up * 200 + RANDOM_FLOAT(-1,1) * gpGlobals->v_right * 10 + RANDOM_FLOAT(-1,1) * gpGlobals->v_up * 10;
	}
	else
	{
		vecVelocity = GetAutoaimVector( AUTOAIM_5DEGREES );
		vecVelocity = vecVelocity * 600;
		vecVelocity.z = 200;
	}

	// Create the grenade
	CQuakeRocket *pRocket = CQuakeRocket::CreateGrenade( pev->origin, vecVelocity, this );
}

// Lightning Gun
void CBasePlayer::W_FireLightning( int iQuadSound )
{
	if (*m_pCurrentAmmo < 1)
	{
		//This should already be IT_LIGHTNING but what the heck.
		if ( m_iQuakeWeapon == IT_LIGHTNING )
		{
			 PLAYBACK_EVENT_FULL( FEV_NOTHOST, edict(), m_usLightning, 0, (float *)&pev->origin, (float *)&pev->angles, 0.0, 0.0, 0, 1, 0, 0 );

			 if ( m_pActiveItem )
				  ((CQuakeGun*)m_pActiveItem)->DestroyEffect();
		}
		
		m_iQuakeWeapon = W_BestWeapon ();
		W_SetCurrentAmmo();
		return;
	}

	bool playsound = false;

	// Make lightning sound every 0.6 seconds
	if ( m_flLightningTime <= gpGlobals->time )
	{
		playsound = true;
		m_flLightningTime = gpGlobals->time + 0.6;
	}

	// explode if under water
	if (pev->waterlevel > 1)
	{
		if ( (gpGlobals->deathmatch > 3) && (RANDOM_FLOAT(0, 1) <= 0.5) )
		{
			strcpy( gszQ_DeathType, "selfwater" );
			TakeDamage( pev, pev, 4000, DMG_GENERIC );
		}
		else
		{
			float flCellsBurnt = *m_pCurrentAmmo;
			*m_pCurrentAmmo = 0;
			W_SetCurrentAmmo();
#if !defined( CLIENT_DLL )
			Q_RadiusDamage( this, this, 35 * flCellsBurnt, NULL );
#endif
			return;
		}
	}

	PLAYBACK_EVENT_FULL( FEV_NOTHOST, edict(), m_usLightning, 0, (float *)&pev->origin, (float *)&pev->angles, 0.0, 0.0, iQuadSound, 0, playsound, 0 );

#if !defined( CLIENT_DLL )
	
	if (gpGlobals->deathmatch != 4 )
		*m_pCurrentAmmo -= 1;

	// Lightning bolt effect
	TraceResult trace;
	Vector vecOrg = pev->origin + Vector(0,0,16);
	UTIL_MakeVectors( pev->v_angle );
	UTIL_TraceLine( vecOrg, vecOrg + (gpGlobals->v_forward * 600), ignore_monsters, ENT(pev), &trace );

	Vector vecDir = gpGlobals->v_forward + ( 0.001 * gpGlobals->v_right ) + ( 0.001 * gpGlobals->v_up );
	// Do damage
	LightningDamage(pev->origin, trace.vecEndPos + (gpGlobals->v_forward * 4), this, 30, vecDir );
	
#endif
}

// Super Nailgun
void CBasePlayer::W_FireSuperSpikes( int iQuadSound )
{
	PLAYBACK_EVENT_FULL( FEV_NOTHOST, edict(), m_usSuperSpike, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, iQuadSound, 0, m_iNailOffset > 0.0 ? 1 : 0, 0 );

	if (gpGlobals->deathmatch != 4 )
		*m_pCurrentAmmo -= 2;
	m_flNextAttack = UTIL_WeaponTimeBase() + 0.1;

	// Fire the Nail
	Vector vecDir = GetAutoaimVector( AUTOAIM_5DEGREES );
	CQuakeNail *pNail = CQuakeNail::CreateSuperNail( pev->origin + Vector(0,0,16), vecDir, this );
}

// Nailgun
void CBasePlayer::W_FireSpikes( int iQuadSound )
{
	// If we're wielding the Super nailgun and we've got ammo for it, fire Super nails
	if (*m_pCurrentAmmo >= 2 && m_iQuakeWeapon == IT_SUPER_NAILGUN)
	{
		W_FireSuperSpikes( iQuadSound );
		return;
	}

	// Swap to next best weapon if this one just ran out
	if (*m_pCurrentAmmo < 1)
	{
		m_iQuakeWeapon = W_BestWeapon ();
		W_SetCurrentAmmo();
		return;
	}

	PLAYBACK_EVENT_FULL( FEV_NOTHOST, edict(), m_usSpike, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, iQuadSound, 0, m_iNailOffset > 0.0 ? 1 : 0, 0 );

	// Fire left then right
	if (m_iNailOffset == 2)
		m_iNailOffset = -2;
	else
		m_iNailOffset = 2;

	if (gpGlobals->deathmatch != 4 )
		*m_pCurrentAmmo -= 1;
	m_flNextAttack = UTIL_WeaponTimeBase() + 0.1;

	// Fire the nail
	UTIL_MakeVectors( pev->v_angle );
	Vector vecDir = GetAutoaimVector( AUTOAIM_5DEGREES );
	CQuakeNail *pNail = CQuakeNail::CreateNail( pev->origin + Vector(0,0,10) + (gpGlobals->v_right * m_iNailOffset), vecDir, this );
}

//===============================================================================
// PLAYER WEAPON USE
//===============================================================================
void CBasePlayer::W_Attack( int iQuadSound )
{
	// Out of ammo?
	if ( !W_CheckNoAmmo() )
		return;

	if ( m_iQuakeWeapon != IT_LIGHTNING )
		((CBasePlayerWeapon*)m_pActiveItem)->SendWeaponAnim( 1, 1 );

	if (m_iQuakeWeapon == IT_AXE)
	{
#ifdef THREEWAVE
		if ( m_iRuneStatus == ITEM_RUNE3_FLAG )
			m_flNextAttack = UTIL_WeaponTimeBase() + 0.3;
		else
#endif
			m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

		PLAYBACK_EVENT_FULL( FEV_NOTHOST, edict(), m_usAxeSwing, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, iQuadSound, 0, 0, 0 );

		// Delay attack for 0.3
		m_flAxeFire = gpGlobals->time + 0.3;

		PLAYBACK_EVENT_FULL( FEV_NOTHOST, edict(), m_usAxe, 0.3, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, iQuadSound, 0, 0, 0 );

	}
	else if (m_iQuakeWeapon == IT_SHOTGUN)
	{
#ifdef THREEWAVE
		if ( m_iRuneStatus == ITEM_RUNE3_FLAG )
			m_flNextAttack = UTIL_WeaponTimeBase() + 0.3;
		else
#endif
			m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

		W_FireShotgun( iQuadSound );
	}
	else if (m_iQuakeWeapon == IT_SUPER_SHOTGUN)
	{
#ifdef THREEWAVE
		if ( m_iRuneStatus == ITEM_RUNE3_FLAG )
			m_flNextAttack = UTIL_WeaponTimeBase() + 0.4;
		else
#endif
			m_flNextAttack = UTIL_WeaponTimeBase() + 0.7;

		W_FireSuperShotgun( iQuadSound );
	}
	else if (m_iQuakeWeapon == IT_NAILGUN)
	{	
		m_flNextAttack = UTIL_WeaponTimeBase() + 0.1;
	
		W_FireSpikes( iQuadSound );
	}
	else if (m_iQuakeWeapon == IT_SUPER_NAILGUN)
	{
		m_flNextAttack = UTIL_WeaponTimeBase() + 0.1;
		W_FireSpikes( iQuadSound );
	}
	else if (m_iQuakeWeapon == IT_GRENADE_LAUNCHER)
	{
#ifdef THREEWAVE
		if ( m_iRuneStatus == ITEM_RUNE3_FLAG )
			m_flNextAttack = UTIL_WeaponTimeBase() + 0.3;
		else
#endif
			m_flNextAttack = UTIL_WeaponTimeBase() + 0.6;

		W_FireGrenade( iQuadSound );
	}
	else if (m_iQuakeWeapon == IT_ROCKET_LAUNCHER)
	{
#ifdef THREEWAVE
		if ( m_iRuneStatus == ITEM_RUNE3_FLAG )
			m_flNextAttack = UTIL_WeaponTimeBase() + 0.4;
		else
#endif
			m_flNextAttack = UTIL_WeaponTimeBase() + 0.8;
			
		W_FireRocket( iQuadSound );
	}
	else if (m_iQuakeWeapon == IT_LIGHTNING)
	{
		m_flNextAttack = UTIL_WeaponTimeBase() + 0.1;

		// Play the lightning start sound if gun just started firing
		if (m_afButtonPressed & IN_ATTACK)
			EMIT_SOUND(ENT(pev), CHAN_AUTO, "weapons/lstart.wav", 1, ATTN_NORM);

		W_FireLightning( iQuadSound );
	}

#ifdef THREEWAVE
	else if ( m_iQuakeWeapon == IT_EXTRA_WEAPON )
	{

		if ( !m_bHook_Out )
			W_FireHook ();
				
		m_flNextAttack = UTIL_WeaponTimeBase() + 0.1;
	}
#endif

	// Make player attack
	if ( pev->health >= 0 )
		SetAnimation( PLAYER_ATTACK1 );
}
