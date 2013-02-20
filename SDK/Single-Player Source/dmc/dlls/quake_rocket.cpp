//=========== (C) Copyright 1996-2002, Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: Quake rocket entity
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
#include "weapons.h"

#define GRENADE_TRAIL 1
#define ROCKET_TRAIL 2

extern unsigned short g_sTrail;
extern unsigned short g_sExplosion;

LINK_ENTITY_TO_CLASS( quake_rocket, CQuakeRocket );

//=========================================================
CQuakeRocket *CQuakeRocket::CreateRocket( Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner )
{
	CQuakeRocket *pRocket = GetClassPtr( (CQuakeRocket *)NULL );
	
	UTIL_SetOrigin( pRocket->pev, vecOrigin );
	SET_MODEL(ENT(pRocket->pev), "models/rocket.mdl");
	pRocket->Spawn();
	pRocket->pev->classname = MAKE_STRING("missile");
	pRocket->pev->owner = pOwner->edict();

	// Setup
	pRocket->pev->movetype = MOVETYPE_FLYMISSILE;
	pRocket->pev->solid = SOLID_BBOX;
		
	// Velocity
	pRocket->pev->velocity = vecAngles * 1000;
	pRocket->pev->angles = UTIL_VecToAngles( vecAngles );
	
	// Touch
	pRocket->SetTouch( CQuakeRocket::RocketTouch );

	// Safety Remove
	pRocket->pev->nextthink = gpGlobals->time + 5;
	pRocket->SetThink( SUB_Remove );

	// Effects
//	pRocket->pev->effects |= EF_LIGHT;

	PLAYBACK_EVENT_FULL (FEV_GLOBAL, pRocket->edict(), g_sTrail, 0.0, 
	(float *)&pRocket->pev->origin, (float *)&pRocket->pev->angles, 0.7, 0.0, pRocket->entindex(), ROCKET_TRAIL, 0, 0);

	return pRocket;
} 

//=========================================================
CQuakeRocket *CQuakeRocket::CreateGrenade( Vector vecOrigin, Vector vecVelocity, CBaseEntity *pOwner )
{
	CQuakeRocket *pRocket = GetClassPtr( (CQuakeRocket *)NULL );

	UTIL_SetOrigin( pRocket->pev, vecOrigin );
	SET_MODEL(ENT(pRocket->pev), "models/grenade.mdl");
	pRocket->Spawn();
	pRocket->pev->classname = MAKE_STRING("grenade");
	pRocket->pev->owner = pOwner->edict();

	// Setup
	pRocket->pev->movetype = MOVETYPE_BOUNCE;
	pRocket->pev->solid = SOLID_BBOX;

	pRocket->pev->avelocity = Vector(300,300,300);
	
	// Velocity
	pRocket->pev->velocity = vecVelocity;
	pRocket->pev->angles = UTIL_VecToAngles(vecVelocity);
	pRocket->pev->friction = 0.5;

	// Touch
	pRocket->SetTouch( CQuakeRocket::GrenadeTouch );

	// set newmis duration
	if ( gpGlobals->deathmatch == 4 )
	{
		pRocket->m_flAttackFinished = gpGlobals->time + 1.1;	// What's this used for?
		if (pOwner)
			pOwner->TakeDamage( pOwner->pev, pOwner->pev, 10, DMG_GENERIC );
	}

	pRocket->pev->nextthink = gpGlobals->time + 2.5;
	pRocket->SetThink( CQuakeRocket::GrenadeExplode );

	PLAYBACK_EVENT_FULL (FEV_GLOBAL, pRocket->edict(), g_sTrail, 0.0, 
	(float *)&g_vecZero, (float *)&g_vecZero, 0.7, 0.0, pRocket->entindex(), GRENADE_TRAIL, 0, 0);


	return pRocket;
}

//=========================================================
void CQuakeRocket::Spawn( void )
{
	Precache();

	UTIL_SetSize(pev, Vector( 0, 0, 0), Vector(0, 0, 0));
	UTIL_SetOrigin( pev, pev->origin );
}

//=========================================================
void CQuakeRocket::Precache( void )
{
	m_iTrail = PRECACHE_MODEL("sprites/smoke.spr");
}

//=========================================================
void CQuakeRocket::RocketTouch ( CBaseEntity *pOther )
{
	// Remove if we've hit skybrush
	if ( UTIL_PointContents(pev->origin) == CONTENT_SKY )
	{
		UTIL_Remove( this );
		return;
	}

	// Do touch damage
	float flDmg = RANDOM_FLOAT( 100, 120 );
	CBaseEntity *pOwner = CBaseEntity::Instance(pev->owner);
	if (pOther->pev->health)
		pOther->TakeDamage( pev, pOwner->pev, flDmg, DMG_BULLET );

	// Don't do radius damage to the other, because all the damage was done in the impact
	Q_RadiusDamage(this, pOwner, 120, pOther);

	// Finish and remove
	Explode();
}

//=========================================================
void CQuakeRocket::GrenadeTouch( CBaseEntity *pOther )
{
	if (pOther->pev->takedamage == DAMAGE_AIM)
	{
		GrenadeExplode();
		return;
	}

	if (pev->flags & FL_ONGROUND)
		{
			// add a bit of static friction
			pev->velocity = pev->velocity * 0.75;

			if (pev->velocity.Length() <= 20)
			{
				pev->avelocity = g_vecZero;
			}
		}

	EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/bounce.wav", 1, ATTN_NORM);

	if (pev->velocity == g_vecZero)
		pev->avelocity = g_vecZero;
}

//=========================================================
void CQuakeRocket::GrenadeExplode()
{
	CBaseEntity *pOwner = CBaseEntity::Instance(pev->owner);
	Q_RadiusDamage(this, pOwner, 120, NULL);

	// Finish and remove
	Explode();
}

//=========================================================
void CQuakeRocket::Explode()
{
	//We use the angles field to send the rocket velocity.
	PLAYBACK_EVENT_FULL( FEV_GLOBAL, edict(), g_sExplosion, 0.0, (float *)&pev->origin, (float *)&pev->velocity, 0.0, 0.0, 0, 0, 0, 0 );

	UTIL_Remove( this );
}