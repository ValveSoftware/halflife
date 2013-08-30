/***
*
*	Copyright (c) 1996-2002,, Valve LLC. All rights reserved.
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

===== quake_player.cpp ========================================================

  Quake Classic player functionality.

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "gamerules.h"
#include "hltv.h"

extern entvars_t *g_pevLastInflictor;
extern int gmsgStatusText;
extern int gmsgStatusValue; 
extern DLL_GLOBAL Vector		g_vecAttackDir;

/*************************************
			  STATUS BAR 
/*************************************/

// Initialise the player's status bar
void CBasePlayer::InitStatusBar()
{
	m_flStatusBarDisappearDelay = 0;
	m_SbarString1[0] = m_SbarString0[0] = 0; 
}

void CBasePlayer::UpdateStatusBar()
{
	int newSBarState[ SBAR_END ];
	memset( newSBarState, 0, sizeof(newSBarState) );

	// Find an ID Target
	TraceResult tr;
	UTIL_MakeVectors( pev->v_angle + pev->punchangle );
	Vector vecSrc = EyePosition();
	Vector vecEnd = vecSrc + (gpGlobals->v_forward * MAX_ID_RANGE);
	UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, edict(), &tr);

	if (tr.flFraction != 1.0)
	{
		if ( !FNullEnt( tr.pHit ) )
		{
			CBaseEntity *pEntity = CBaseEntity::Instance( tr.pHit );

			if ( pEntity->Classify() == CLASS_PLAYER )
			{
				newSBarState[ SBAR_ID_TARGETNAME ] = ENTINDEX( pEntity->edict() );
				newSBarState[ SBAR_ID_TARGETTEAM ] = FALSE;
					
				m_flStatusBarDisappearDelay = gpGlobals->time + 1.0;
			}
		}
		else if ( m_flStatusBarDisappearDelay > gpGlobals->time )
		{
			// hold the values for a short amount of time after viewing the object
			newSBarState[ SBAR_ID_TARGETNAME ] = m_izSBarState[ SBAR_ID_TARGETNAME ];
			newSBarState[ SBAR_ID_TARGETHEALTH ] = m_izSBarState[ SBAR_ID_TARGETHEALTH ];
			newSBarState[ SBAR_ID_TARGETARMOR ] = m_izSBarState[ SBAR_ID_TARGETARMOR ];
			newSBarState[ SBAR_ID_TARGETTEAM ] = m_izSBarState[ SBAR_ID_TARGETTEAM ];
		}
	}

	// Check values and send if they don't match
	for (int i = 1; i < SBAR_END; i++)
	{
		if ( newSBarState[i] != m_izSBarState[i] )
		{
			MESSAGE_BEGIN( MSG_ONE, gmsgStatusValue, NULL, pev );
				WRITE_BYTE( i );
				WRITE_SHORT( newSBarState[i] );
			MESSAGE_END();

			m_izSBarState[i] = newSBarState[i];
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: Player has taken some damage. This is now using the Quake functionality.
//-----------------------------------------------------------------------------
int CBasePlayer::TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	if ( (pev->takedamage == DAMAGE_NO) || (IsAlive() == FALSE) )
		return 0;


	//We are wearing the suit and we want to be hurt by lava or slime
	if ( m_iQuakeItems & IT_SUIT )
	{
		if ( bitsDamageType & DMG_BURN || bitsDamageType & DMG_ACID )
			return 0;
	}

	CBaseEntity *pAttacker = CBaseEntity::Instance(pevAttacker);

	// keep track of amount of damage last sustained
	m_lastDamageAmount = flDamage;

	// check for quad damage powerup on the attacker
	if (pAttacker->IsPlayer())
	{
		if ( ((CBasePlayer*)pAttacker)->m_flSuperDamageFinished > gpGlobals->time )
		{
			if (gpGlobals->deathmatch == 4)
				flDamage *= 8;
			else
				flDamage *= 4;
		}
	}

	// team play damage avoidance
	if ( g_pGameRules->PlayerRelationship( this, pAttacker ) == GR_TEAMMATE )
	{
		// Teamplay 3 you can still hurt yourself
		if ( CVAR_GET_FLOAT( "mp_teamplay" ) == 3 && pAttacker != this )
			return 0;
		// Teamplay 1 can't hurt any teammates, including yourself
		if ( CVAR_GET_FLOAT( "mp_teamplay" ) == 1 )
			return 0;
		// Teamplay 2 you can still hurt teammates
	}


	// save damage based on the target's armor level
	float flSave = ceil(pev->armortype * flDamage);
	if (flSave >= pev->armorvalue)
	{
		flSave = pev->armorvalue;
		pev->armortype = 0;     // lost all armor
		m_iQuakeItems &= ~(IT_ARMOR1 | IT_ARMOR2 | IT_ARMOR3);
	}
	pev->armorvalue -= flSave;
	float flTake = ceil(flDamage - flSave);

	// add to the damage total for clients, which will be sent as a single message at the end of the frame
	pev->dmg_take = pev->dmg_take + flTake;
	pev->dmg_inflictor = ENT(pevInflictor);

	Vector vecTemp;

	if ( pevAttacker == pevInflictor )	
	{
		vecTemp = pevAttacker->origin - ( VecBModelOrigin(pev) );
	}
	else
	// an actual missile was involved.
	{
		vecTemp = pevInflictor->origin - ( VecBModelOrigin(pev) );
	}

	// this global is still used for glass and other non-monster killables, along with decals.
	g_vecAttackDir = vecTemp.Normalize();


	// figure momentum add
	if ( (pevInflictor) && (pev->movetype == MOVETYPE_WALK) && !( FBitSet (bitsDamageType, DMG_BURN) ) && !( FBitSet (bitsDamageType, DMG_ACID) ) )
	{
		

		Vector vecPush = (pev->origin - (pevInflictor->absmin + pevInflictor->absmax) * 0.5).Normalize();
		// Set kickback for smaller weapons
		// Read: only if it's not yourself doing the damage
		if ( (flDamage < 60) && pAttacker->IsPlayer() && (pAttacker != this) ) 
			pev->velocity = pev->velocity + vecPush * flDamage * 11;
		else  
		{
			// Otherwise, these rules apply to rockets and grenades                        
			// for blast velocity
			if ( pAttacker == this )
			{
				if ( m_iQuakeWeapon != IT_LIGHTNING )
					pev->velocity = pev->velocity + vecPush * flDamage * 8;
			}
			else
				pev->velocity = pev->velocity + vecPush * flDamage * 8;
		}
		
		// Rocket Jump modifiers
		int iRocketJumpModifier = (int)CVAR_GET_FLOAT("rj");

		if ( (iRocketJumpModifier > 1) && (pAttacker == this) && m_iQuakeWeapon == ( IT_ROCKET_LAUNCHER | IT_GRENADE_LAUNCHER ) ) 
			pev->velocity = pev->velocity + vecPush * flDamage * iRocketJumpModifier;
	}

	// check for godmode or invincibility
	if (pev->flags & FL_GODMODE)
		return 0;
	if (m_flInvincibleFinished > gpGlobals->time)
	{
		if (m_fInvincSound < gpGlobals->time)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/protect3.wav", 1, ATTN_NORM);
			m_fInvincSound = gpGlobals->time + 2;
		}
		return 0;
	}


	// do the damage
	pev->health -= (int)flTake;

	// tell director about it
	MESSAGE_BEGIN( MSG_SPEC, SVC_DIRECTOR );
		WRITE_BYTE ( 9 );	// command length in bytes
		WRITE_BYTE ( DRC_CMD_EVENT );	// take damage event
		WRITE_SHORT( ENTINDEX(this->edict()) );	// index number of primary entity
		WRITE_SHORT( ENTINDEX(ENT(pevInflictor)) );	// index number of secondary entity
		WRITE_LONG( 5 );   // eventflags (priority and flags)
	MESSAGE_END();

	// react to the damage
	m_bitsDamageType |= bitsDamageType; // Save this so we can report it to the client
	m_bitsHUDDamage = -1;  // make sure the damage bits get resent
	
	if ( pev->health <= 0 )
	{
		g_pevLastInflictor = pevInflictor;

		Killed( pevAttacker, GIB_NORMAL );
	
		g_pevLastInflictor = NULL;
		return 0;
	}

	// play pain sound
	Pain( pAttacker );

	return flTake;
}

