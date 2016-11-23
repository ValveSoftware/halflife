//=========== (C) Copyright 1996-2002, Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: Quake nail entity
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
#include "decals.h"
#include "gamerules.h"

LINK_ENTITY_TO_CLASS( quake_nail, CQuakeNail );

//=========================================================
CQuakeNail *CQuakeNail::CreateNail( Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner )
{
	CQuakeNail *pNail = GetClassPtr( (CQuakeNail *)NULL );

	UTIL_SetOrigin( pNail->pev, vecOrigin );

	pNail->pev->velocity = vecAngles * 1000;
	pNail->pev->angles = UTIL_VecToAngles(vecAngles);
	pNail->pev->owner = pOwner->edict();
	pNail->Spawn();
	pNail->pev->classname = MAKE_STRING("spike");

	// don't send to clients.
	pNail->pev->effects |= EF_NODRAW;

	return pNail;
}

CQuakeNail *CQuakeNail::CreateSuperNail( Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner )
{
	CQuakeNail *pNail = CreateNail( vecOrigin, vecAngles, pOwner );
	pNail->pev->classname = MAKE_STRING("superspike");

	// Super nails simply do more damage
	pNail->pev->dmg = 18;
	return pNail;
}

//=========================================================
void CQuakeNail::Spawn( void )
{
	Precache();

	// Setup
	pev->movetype = MOVETYPE_FLYMISSILE;
	pev->solid = SOLID_BBOX;
	
	// Safety removal
	pev->nextthink = gpGlobals->time + 6;
	SetThink( SUB_Remove );
	
	// Touch
	SetTouch( NailTouch );

	// Model
	SET_MODEL( ENT(pev), "models/spike.mdl" );
	UTIL_SetSize(pev, Vector( 0, 0, 0), Vector(0, 0, 0));
	UTIL_SetOrigin( pev, pev->origin );

	// Damage
	pev->dmg = 9;
}

//=========================================================
void CQuakeNail::NailTouch( CBaseEntity *pOther )
{
	if (pOther->pev->solid == SOLID_TRIGGER)
		return;

	// Remove if we've hit skybrush
	if ( UTIL_PointContents(pev->origin) == CONTENT_SKY )
	{
		UTIL_Remove( this );
		return;
	}

	// Hit something that bleeds
	if (pOther->pev->takedamage)
	{
		CBaseEntity *pOwner = CBaseEntity::Instance(pev->owner);
		
		if ( g_pGameRules->PlayerRelationship( pOther, pOwner ) != GR_TEAMMATE )
			SpawnBlood( pev->origin, pOther->BloodColor(), pev->dmg );

		pOther->TakeDamage( pev, pOwner->pev, pev->dmg, DMG_GENERIC );
	}
	else
	{
		if ( pOther->pev->solid == SOLID_BSP || pOther->pev->movetype == MOVETYPE_PUSHSTEP )
		{
			TraceResult tr;
			tr.vecEndPos = pev->origin;
			tr.pHit = pOther->edict();

			//Arent we doing this client side?
			//UTIL_GunshotDecalTrace( &tr, DECAL_GUNSHOT1 + RANDOM_LONG( 0, 4 ) );
		}
	}

	UTIL_Remove( this );
}



