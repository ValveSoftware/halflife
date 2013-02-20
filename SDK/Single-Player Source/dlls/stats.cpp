//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: New version of the slider bar
//
// $NoKeywords: $
//=============================================================================

#include "extdll.h"
#include "util.h"

#include "cbase.h"
#include "player.h"
#include "trains.h"
#include "nodes.h"
#include "weapons.h"
#include "soundent.h"
#include "monsters.h"
#include "..\engine\shake.h"
#include "decals.h"
#include "gamerules.h"


float AmmoDamage( const char *pName )
{
	if ( !pName )
		return 0;

	if ( !strcmp( pName, "9mm" ) )
		return gSkillData.plrDmg9MM;
	if ( !strcmp( pName, "357" ) )
		return gSkillData.plrDmg357;
	if ( !strcmp( pName, "ARgrenades" ) )
		return gSkillData.plrDmgM203Grenade;
	if ( !strcmp( pName, "buckshot" ) )
		return gSkillData.plrDmgBuckshot;
	if ( !strcmp( pName, "bolts") )
		return gSkillData.plrDmgCrossbowMonster;
	if ( !strcmp( pName, "rockets") )
		return gSkillData.plrDmgRPG;
	if ( !strcmp( pName, "uranium") )
		return gSkillData.plrDmgGauss;
	if ( !strcmp( pName, "Hand Grenade") )
		return gSkillData.plrDmgHandGrenade;
	if ( !strcmp( pName, "Satchel Charge") )
		return gSkillData.plrDmgSatchel;
	if ( !strcmp( pName, "Trip Mine") )
		return gSkillData.plrDmgTripmine;

	return 0;
}


void UpdateStatsFile( float dataTime, char *pMapname, float health, float ammo, int skillLevel )
{
	FILE *fp;

	fp = fopen( "stats.txt", "a" );
	if ( !fp )
		return;
	fprintf( fp, "%6.2f, %6.2f, %6.2f, %s, %2d\n", dataTime, health, ammo, pMapname, skillLevel );
	fclose( fp );
}


#define AMMO_THRESHOLD		10			// This much ammo goes by before it is "interesting"
#define HEALTH_THRESHOLD	10			// Same for health
#define OUTPUT_LATENCY		3			// This many seconds for ammo/health to settle

typedef struct
{
	int		lastAmmo;
	float	lastHealth;
	float	lastOutputTime; // NOTE: These times are in "game" time -- a running total of elapsed time since the game started
	float	nextOutputTime;
	float	dataTime;
	float	gameTime;
	float	lastGameTime;
} TESTSTATS;

TESTSTATS gStats = {0,0,0,0,0,0,0};

void UpdateStats( CBasePlayer *pPlayer )
{
	int i;

	int ammoCount[ MAX_AMMO_SLOTS ];
	memcpy( ammoCount, pPlayer->m_rgAmmo, MAX_AMMO_SLOTS * sizeof(int) );

	// Keep a running time, so the graph doesn't overlap
	
	if ( gpGlobals->time < gStats.lastGameTime )	// Changed level or died, don't b0rk
	{
		gStats.lastGameTime = gpGlobals->time;
		gStats.dataTime = gStats.gameTime;
	}

	gStats.gameTime += gpGlobals->time - gStats.lastGameTime;
	gStats.lastGameTime = gpGlobals->time;

	for (i = 0; i < MAX_ITEM_TYPES; i++)
	{
		CBasePlayerItem *p = pPlayer->m_rgpPlayerItems[i];
		while (p)
		{
			ItemInfo II;
			
			memset(&II, 0, sizeof(II));
			p->GetItemInfo(&II);

			int index = pPlayer->GetAmmoIndex(II.pszAmmo1);
			if ( index >= 0 )
				ammoCount[ index ] += ((CBasePlayerWeapon *)p)->m_iClip;
			
			p = p->m_pNext;
		}
	}

	float ammo = 0;
	for (i = 1; i < MAX_AMMO_SLOTS; i++)
	{
		ammo += ammoCount[i] * AmmoDamage( CBasePlayerItem::AmmoInfoArray[i].pszName );
	}

	float health = pPlayer->pev->health + pPlayer->pev->armorvalue * 2;	// Armor is 2X health
	float ammoDelta = fabs( ammo - gStats.lastAmmo );
	float healthDelta = fabs( health - gStats.lastHealth );
	int forceWrite = 0;
	if ( health <= 0 && gStats.lastHealth > 0 )
		forceWrite = 1;

	if ( (ammoDelta > AMMO_THRESHOLD || healthDelta > HEALTH_THRESHOLD) && !forceWrite )
	{
		if ( gStats.nextOutputTime == 0 )
			gStats.dataTime = gStats.gameTime;

		gStats.lastAmmo = ammo;
		gStats.lastHealth = health;

		gStats.nextOutputTime = gStats.gameTime + OUTPUT_LATENCY;
	}
	else if ( (gStats.nextOutputTime != 0 && gStats.nextOutputTime < gStats.gameTime) || forceWrite )
	{
		UpdateStatsFile( gStats.dataTime, (char *)STRING(gpGlobals->mapname), health, ammo, (int)CVAR_GET_FLOAT("skill") );

		gStats.lastAmmo = ammo;
		gStats.lastHealth = health;
		gStats.lastOutputTime = gStats.gameTime;
		gStats.nextOutputTime = 0;
	}
}

void InitStats( CBasePlayer *pPlayer )
{
	gStats.lastGameTime = gpGlobals->time;	// Fixup stats time
}

