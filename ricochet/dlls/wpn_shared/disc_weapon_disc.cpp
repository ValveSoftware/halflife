
//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: Weapon functionality for Discwar
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
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "effects.h"
#include "discwar.h"
#include "disc_objects.h"
#include "disc_arena.h"
 
// Disc trail colors
float g_iaDiscColors[33][3] =
{
	{ 255, 255, 255, },
	{ 250, 0, 0 },
	{ 0, 0, 250 },
	{ 0, 250, 0 },
	{ 128, 128, 0 },
	{ 128, 0, 128 },
	{ 0, 128, 128 },
	{ 128, 128, 128 },
	{ 64, 128, 0 },
	{ 128, 64, 0 },
	{ 128, 0, 64 },
	{ 64, 0, 128 },
	{ 0, 64, 128 },
	{ 64, 64, 128 },
	{ 128, 64, 64 },
	{ 64, 128, 64 },
	{ 128, 128, 64 },
	{ 128, 64, 128 },
	{ 64, 128, 128 },
	{ 250, 128, 0 },
	{ 128, 250, 0 },
	{ 128, 0, 250 },
	{ 250, 0, 128 },
	{ 0, 250, 128 },
	{ 250, 250, 128 },
	{ 250, 128, 250 },
	{ 128, 250, 250 },
	{ 250, 128, 64 },
	{ 250, 64, 128 },
	{ 128, 250, 64 },
	{ 64, 128, 250 },
	{ 128, 64, 250 },
};

enum disc_e 
{
	DISC_IDLE = 0,
	DISC_FIDGET,
	DISC_PINPULL,
	DISC_THROW1,	// toss
	DISC_THROW2,	// medium
	DISC_THROW3,	// hard
	DISC_HOLSTER,
	DISC_DRAW
};

#include "disc_weapon.h"

LINK_ENTITY_TO_CLASS( weapon_disc, CDiscWeapon );

#if !defined( CLIENT_DLL )
LINK_ENTITY_TO_CLASS( disc, CDisc );

//========================================================================================
// DISC
//========================================================================================
void CDisc::Spawn( void )
{
	Precache( );

	pev->classname = MAKE_STRING("disc");
	pev->movetype = MOVETYPE_BOUNCEMISSILE;
	pev->solid = SOLID_TRIGGER;

	// Setup model
	if ( m_iPowerupFlags & POW_HARD )
		SET_MODEL(ENT(pev), "models/disc_hard.mdl");
	else
		SET_MODEL(ENT(pev), "models/disc.mdl");
	UTIL_SetSize(pev, Vector( -4,-4,-4 ), Vector(4, 4, 4));

	UTIL_SetOrigin( pev, pev->origin );
	SetTouch( &CDisc::DiscTouch );
	SetThink( &CDisc::DiscThink );

	m_iBounces = 0;
	m_fDontTouchOwner = gpGlobals->time + 0.2;
	m_fDontTouchEnemies = 0;
	m_bRemoveSelf = false;
	m_bTeleported = false;
	m_pLockTarget = NULL;

	UTIL_MakeVectors( pev->angles );

	// Fast powerup makes discs go faster
	if ( m_iPowerupFlags & POW_FAST )
		pev->velocity = gpGlobals->v_forward * DISC_VELOCITY * 1.5;
	else
		pev->velocity = gpGlobals->v_forward * DISC_VELOCITY;

	// Pull our owner out so we will still touch it
	if ( pev->owner )
		m_hOwner = Instance(pev->owner);
	pev->owner = NULL;

	// Trail
	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_BEAMFOLLOW );
		WRITE_SHORT(entindex());	// entity
		WRITE_SHORT(m_iTrail );	// model

	if (m_bDecapitate)
		WRITE_BYTE( 5 ); // life
	else
		WRITE_BYTE( 3 ); // life

		WRITE_BYTE( 5 );  // width

		WRITE_BYTE( g_iaDiscColors[pev->team][0] ); // r, g, b
		WRITE_BYTE( g_iaDiscColors[pev->team][1] ); // r, g, b
		WRITE_BYTE( g_iaDiscColors[pev->team][2] ); // r, g, b

		WRITE_BYTE( 250 );	// brightness
	MESSAGE_END();

	// Decapitator's make sound
	if (m_bDecapitate)
		EMIT_SOUND( ENT(pev), CHAN_VOICE, "weapons/rocket1.wav", 0.5, 0.5 );

	// Highlighter
	pev->renderfx = kRenderFxGlowShell;
	for (int i = 0; i <= 2;i ++)
		pev->rendercolor[i] = g_iaDiscColors[pev->team][i];
	pev->renderamt = 100;

	pev->nextthink = gpGlobals->time + 0.1;
}

void CDisc::Precache( void )
{
	PRECACHE_MODEL("models/disc.mdl");
	PRECACHE_MODEL("models/disc_hard.mdl");
	PRECACHE_SOUND("weapons/cbar_hitbod1.wav");
	PRECACHE_SOUND("weapons/cbar_hitbod2.wav");
	PRECACHE_SOUND("weapons/cbar_hitbod3.wav");
	PRECACHE_SOUND("weapons/altfire.wav");
	PRECACHE_SOUND("items/gunpickup2.wav");
	PRECACHE_SOUND("weapons/electro5.wav");
	PRECACHE_SOUND("weapons/xbow_hit1.wav");
	PRECACHE_SOUND("weapons/xbow_hit2.wav");
	PRECACHE_SOUND("weapons/rocket1.wav");
	PRECACHE_SOUND("dischit.wav");
	m_iTrail = PRECACHE_MODEL("sprites/smoke.spr");
	m_iSpriteTexture = PRECACHE_MODEL( "sprites/lgtning.spr" );
}

/*
void CDisc::SetObjectCollisionBox( void )
{
	pev->absmin = pev->origin + Vector( -8, -8, 8 );
	pev->absmax = pev->origin + Vector( 8, 8, 8 );
}
*/

// Give the disc back to it's owner
void CDisc::ReturnToThrower( void )
{
	if (m_bDecapitate)
	{
		STOP_SOUND( edict(), CHAN_VOICE, "weapons/rocket1.wav" );
		if ( !m_bRemoveSelf )
			((CBasePlayer*)(CBaseEntity*)m_hOwner)->GiveAmmo( MAX_DISCS, "disc", MAX_DISCS );
	}
	else
	{
		if ( !m_bRemoveSelf )
			((CBasePlayer*)(CBaseEntity*)m_hOwner)->GiveAmmo( 1, "disc", MAX_DISCS );
	}

	UTIL_Remove( this );
}

void CDisc::DiscTouch ( CBaseEntity *pOther )
{
	// Push players backwards
	if ( pOther->IsPlayer() )
	{
		if ( ((CBaseEntity*)m_hOwner) == pOther )
		{
			if (m_fDontTouchOwner < gpGlobals->time)
			{
				// Play catch sound
				EMIT_SOUND_DYN( pOther->edict(), CHAN_WEAPON, "items/gunpickup2.wav", 1.0, ATTN_NORM, 0, 98 + RANDOM_LONG(0,3)); 

				ReturnToThrower();
			}

			return;
		}
		else if ( m_fDontTouchEnemies < gpGlobals->time)
		{
			if ( pev->team != pOther->pev->team )
			{
				((CBasePlayer*)pOther)->m_LastHitGroup = HITGROUP_GENERIC;

				// Do freeze seperately so you can freeze and shatter a person with a single shot
				if ( m_iPowerupFlags & POW_FREEZE && ((CBasePlayer*)pOther)->m_iFrozen == FALSE )
				{
					// Freeze the player and make them glow blue
					EMIT_SOUND_DYN( pOther->edict(), CHAN_WEAPON, "weapons/electro5.wav", 1.0, ATTN_NORM, 0, 98 + RANDOM_LONG(0,3)); 
					((CBasePlayer*)pOther)->Freeze();

					// If it's not a decap, return now. If it's a decap, continue to shatter
					if ( !m_bDecapitate )
					{
						m_fDontTouchEnemies = gpGlobals->time + 2.0;
						return;
					}
				}

				// Decap or push
				if (m_bDecapitate)
				{
					// Decapitate!
					if ( m_bTeleported )
						((CBasePlayer*)pOther)->m_flLastDiscHitTeleport = gpGlobals->time;
					((CBasePlayer*)pOther)->Decapitate( ((CBaseEntity*)m_hOwner)->pev );

					m_fDontTouchEnemies = gpGlobals->time + 0.5;
				}
				else 
				{
					// Play thwack sound
					switch( RANDOM_LONG(0,2) )
					{
					case 0:
						EMIT_SOUND_DYN( pOther->edict(), CHAN_ITEM, "weapons/cbar_hitbod1.wav", 1.0, ATTN_NORM, 0, 98 + RANDOM_LONG(0,3)); 
						break;
					case 1:
						EMIT_SOUND_DYN( pOther->edict(), CHAN_ITEM, "weapons/cbar_hitbod2.wav", 1.0, ATTN_NORM, 0, 98 + RANDOM_LONG(0,3)); 
						break;
					case 2:
						EMIT_SOUND_DYN( pOther->edict(), CHAN_ITEM, "weapons/cbar_hitbod3.wav", 1.0, ATTN_NORM, 0, 98 + RANDOM_LONG(0,3)); 
						break;
					}

					// Push the player
					Vector vecDir = pev->velocity.Normalize();
					pOther->pev->flags &= ~FL_ONGROUND;
					((CBasePlayer*)pOther)->m_vecHitVelocity = vecDir * DISC_PUSH_MULTIPLIER;

					// Shield flash only if the player isnt frozen
					if ( ((CBasePlayer*)pOther)->m_iFrozen == false )
					{
						pOther->pev->renderfx = kRenderFxGlowShell;
						pOther->pev->rendercolor.x = 255;
						pOther->pev->renderamt = 150;
					}

					((CBasePlayer*)pOther)->m_hLastPlayerToHitMe = m_hOwner;
					((CBasePlayer*)pOther)->m_flLastDiscHit = gpGlobals->time;
					((CBasePlayer*)pOther)->m_iLastDiscBounces = m_iBounces;
					if ( m_bTeleported )
						((CBasePlayer*)pOther)->m_flLastDiscHitTeleport = gpGlobals->time;

					m_fDontTouchEnemies = gpGlobals->time + 2.0;
				}
			}
		}
	}
	// Hit a disc?
	else if ( pOther->pev->iuser4 ) 
	{
		// Enemy Discs destroy each other
		if ( pOther->pev->iuser4 != pev->iuser4 )
		{
			// Play a warp sound and sprite
			CSprite *pSprite = CSprite::SpriteCreate( "sprites/discreturn.spr", pev->origin, TRUE );
			pSprite->AnimateAndDie( 60 );
			pSprite->SetTransparency( kRenderTransAdd, 255, 255, 255, 255, kRenderFxNoDissipation );
			pSprite->SetScale( 1 );
			EMIT_SOUND_DYN( edict(), CHAN_ITEM, "dischit.wav", 1.0, ATTN_NORM, 0, 98 + RANDOM_LONG(0,3));

			// Return both discs to their owners
			((CDisc*)pOther)->ReturnToThrower();
			ReturnToThrower();
		}
		else
		{
			// Friendly discs just pass through each other
		}
	}
	else
	{
		m_iBounces++;

		switch ( RANDOM_LONG( 0, 1 ) )
		{
		case 0:	EMIT_SOUND_DYN( edict(), CHAN_ITEM, "weapons/xbow_hit1.wav", 1.0, ATTN_NORM, 0, 98 + RANDOM_LONG(0,3));  break;
		case 1:	EMIT_SOUND_DYN( edict(), CHAN_ITEM, "weapons/xbow_hit2.wav", 1.0, ATTN_NORM, 0, 98 + RANDOM_LONG(0,3));  break;
		}

		UTIL_Sparks( pev->origin, edict() );
	}
}

void CDisc::DiscThink()
{
	// Make Freeze discs home towards any player ahead of them
	if ( (m_iPowerupFlags & POW_FREEZE) && (m_iBounces == 0) )
	{
		// Use an existing target if he's still in the view cone
		if ( m_pLockTarget != NULL )
		{
			Vector vecDir = (m_pLockTarget->pev->origin - pev->origin).Normalize();
			UTIL_MakeVectors( pev->angles );
			float flDot = DotProduct( gpGlobals->v_forward, vecDir );
			if ( flDot < 0.6 )
				m_pLockTarget = NULL;
		}

		// Get a new target if we don't have one
		if ( m_pLockTarget == NULL )
		{
			CBaseEntity *pOther = NULL;

			// Examine all entities within a reasonable radius
			while ((pOther = UTIL_FindEntityByClassname( pOther, "player" )) != NULL)
			{
				// Skip the guy who threw this
				if ( ((CBaseEntity*)m_hOwner) == pOther )
					continue;
				// Skip observers
				if ( ((CBasePlayer*)pOther)->IsObserver() )
					continue;

				// Make sure the enemy's in a cone ahead of us
				Vector vecDir = (pOther->pev->origin - pev->origin).Normalize();
				UTIL_MakeVectors( pev->angles );
				float flDot = DotProduct( gpGlobals->v_forward, vecDir );
				if ( flDot > 0.6 )
				{
					m_pLockTarget = pOther;
					break;
				}
			}
		}

		// Track towards our target
		if ( m_pLockTarget != NULL )
		{
			// Calculate new velocity
			Vector vecDir = (m_pLockTarget->pev->origin - pev->origin).Normalize();
			pev->velocity = ( pev->velocity.Normalize() + (vecDir.Normalize() * 0.25)).Normalize();
			pev->velocity = pev->velocity * DISC_VELOCITY;
			pev->angles = UTIL_VecToAngles( pev->velocity );
		}
	}

	// Track the player if we've bounced 3 or more times ( Fast discs remove immediately )
	if ( m_iBounces >= 3 || (m_iPowerupFlags & POW_FAST && m_iBounces >= 1) )
	{
		// Remove myself if my owner's died
		if (m_bRemoveSelf)
		{
			STOP_SOUND( edict(), CHAN_VOICE, "weapons/rocket1.wav" );
			UTIL_Remove( this );
			return;
		}

		// 7 Bounces, just remove myself
		if ( m_iBounces > 7 )
		{
			ReturnToThrower();
			return;
		}

		// Start heading for the player
		if ( m_hOwner )
		{
			Vector vecDir = ( m_hOwner->pev->origin - pev->origin );
			vecDir = vecDir.Normalize();
			pev->velocity = vecDir * DISC_VELOCITY;
			pev->nextthink = gpGlobals->time + 0.1;
		}
		else
		{
			UTIL_Remove( this ); 
		}
	}

	// Sanity check
	if ( pev->velocity == g_vecZero )
		ReturnToThrower();

	pev->nextthink = gpGlobals->time + 0.1;
}

CDisc *CDisc::CreateDisc( Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner, CDiscWeapon *pLauncher, bool bDecapitator, int iPowerupFlags )
{
	CDisc *pDisc = GetClassPtr( (CDisc *)NULL );

	UTIL_SetOrigin( pDisc->pev, vecOrigin );
	pDisc->m_iPowerupFlags = iPowerupFlags;
	// Hard shots always decapitate
	if ( pDisc->m_iPowerupFlags & POW_HARD )
		pDisc->m_bDecapitate = TRUE;
	else
		pDisc->m_bDecapitate = bDecapitator;

	pDisc->pev->angles = vecAngles;
	pDisc->pev->owner = pOwner->edict();
	pDisc->pev->team = pOwner->pev->team;
	pDisc->pev->iuser4 = pOwner->pev->iuser4;

	// Set the Group Info
	pDisc->pev->groupinfo = pOwner->pev->groupinfo;

	pDisc->m_pLauncher = pLauncher; 

	pDisc->Spawn();

	return pDisc;
}
#endif // !CLIENT_DLL

//========================================================================================
// DISC WEAPON
//========================================================================================
void CDiscWeapon::Spawn( )
{
	Precache( );
	m_iId = WEAPON_DISC;
	SET_MODEL(ENT(pev), "models/disc.mdl");

#if !defined( CLIENT_DLL )
	pev->dmg = gSkillData.plrDmgHandGrenade;
#endif

	m_iDefaultAmmo = STARTING_DISCS;
	m_iFastShotDiscs = NUM_FASTSHOT_DISCS;

	FallInit();// get ready to fall down.
}


void CDiscWeapon::Precache( void )
{
	PRECACHE_MODEL("models/disc.mdl");
	PRECACHE_MODEL("models/disc_hard.mdl");
	PRECACHE_MODEL("models/v_disc.mdl");
	PRECACHE_MODEL("models/p_disc.mdl");
	PRECACHE_SOUND("weapons/cbar_miss1.wav");
	m_iSpriteTexture = PRECACHE_MODEL( "sprites/lgtning.spr" );

	m_usFireDisc = PRECACHE_EVENT( 1, "events/firedisc.sc" );
}

int CDiscWeapon::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "disc";
	p->iMaxAmmo1 = MAX_DISCS;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 4;
	p->iPosition = 0;
	p->iId = WEAPON_DISC;
	p->iWeight = 100;
	p->iFlags = ITEM_FLAG_NOAUTORELOAD | ITEM_FLAG_NOAUTOSWITCHEMPTY;

	return 1;
}


BOOL CDiscWeapon::Deploy( )
{
	return DefaultDeploy( "models/v_disc.mdl", "models/p_disc.mdl", DISC_DRAW, "crowbar" );
}

BOOL CDiscWeapon::CanHolster( void )
{
	return TRUE;
}

void CDiscWeapon::Holster( int skiplocal /* = 0 */ )
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		SendWeaponAnim( DISC_HOLSTER, 1 );
	}
	else
	{
		// no more grenades!
		m_pPlayer->pev->weapons &= ~(1<<WEAPON_DISC);
		SetThink( &CDiscWeapon::DestroyItem );
		pev->nextthink = gpGlobals->time + 0.1;
	}

	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "common/null.wav", 1.0, ATTN_NORM);
}

CDisc *CDiscWeapon::FireDisc( bool bDecapitator )
{
	CDisc *pReturnDisc = NULL;

	SendWeaponAnim( DISC_THROW1, 1 );

	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

#if !defined( CLIENT_DLL )
	Vector vecFireDir = g_vecZero;
	vecFireDir[1] = m_pPlayer->pev->v_angle[1];
	UTIL_MakeVectors( vecFireDir );
	Vector vecSrc = m_pPlayer->pev->origin + (m_pPlayer->pev->view_ofs * 0.25) + gpGlobals->v_forward * 16;
	CDisc *pDisc = CDisc::CreateDisc( vecSrc, vecFireDir, m_pPlayer, this, bDecapitator, m_pPlayer->m_iPowerups );
	pReturnDisc = pDisc;

	// Triple shot fires 2 more disks
	if ( m_pPlayer->HasPowerup( POW_TRIPLE ) )
	{
		// The 2 extra discs from triple shot are removed after their 3rd bounce
		vecFireDir[1] = m_pPlayer->pev->v_angle[1] - 7;
		UTIL_MakeVectors( vecFireDir );
		vecSrc = m_pPlayer->pev->origin + (m_pPlayer->pev->view_ofs * 0.25) + gpGlobals->v_forward * 16;
		pDisc = CDisc::CreateDisc( vecSrc, vecFireDir, m_pPlayer, this, bDecapitator, POW_TRIPLE );
		pDisc->m_bRemoveSelf = true;

		vecFireDir[1] = m_pPlayer->pev->v_angle[1] + 7;
		UTIL_MakeVectors( vecFireDir );
		vecSrc = m_pPlayer->pev->origin + (m_pPlayer->pev->view_ofs * 0.25) + gpGlobals->v_forward * 16;
		pDisc = CDisc::CreateDisc( vecSrc, vecFireDir, m_pPlayer, this, bDecapitator, POW_TRIPLE );
		pDisc->m_bRemoveSelf = true;
	}

#endif

	// Fast shot allows faster throwing
	float flTimeToNextShot = 0.5;
	if ( m_pPlayer->HasPowerup( POW_FAST ) )
		flTimeToNextShot = 0.2;

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flTimeToNextShot;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + flTimeToNextShot;

	return pReturnDisc;
}

void CDiscWeapon::PrimaryAttack()
{
#if !defined( CLIENT_DLL )
	if ( m_pPlayer->m_pCurrentArena )
	{
		if ( m_pPlayer->m_pCurrentArena->AllowedToFire() == false )
			return;
	}
#endif

	if ( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] )
	{
		PLAYBACK_EVENT_FULL( FEV_NOTHOST, m_pPlayer->edict(), m_usFireDisc, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, 0, 0, 0, 0 );
		CDisc *pDisc = FireDisc( false );
		
		// Fast powerup has a number of discs per 1 normal disc
		if ( m_pPlayer->HasPowerup( POW_FAST ) )
		{
			m_iFastShotDiscs--;
			if ( m_iFastShotDiscs )
			{
				// Make this disc remove itself
				pDisc->m_bRemoveSelf = true;
				return;
			}

			m_iFastShotDiscs = NUM_FASTSHOT_DISCS;
		}

		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;

		// If we have powered discs, remove one
		if ( m_pPlayer->m_iPowerupDiscs )
		{
			m_pPlayer->m_iPowerupDiscs--;
			if ( !m_pPlayer->m_iPowerupDiscs )
				m_pPlayer->RemoveAllPowerups();
		}
	}
}

void CDiscWeapon::SecondaryAttack()
{
#if !defined( CLIENT_DLL )
	if ( m_pPlayer->m_pCurrentArena )
	{
		if ( m_pPlayer->m_pCurrentArena->AllowedToFire() == false )
			return;
	}
#endif

	// Fast powerup has a number of discs per 1 normal disc (so it can throw a decap when it has at least 1 real disc)
	if ( (m_pPlayer->HasPowerup( POW_FAST ) && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0 ) ||
		 ( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] == MAX_DISCS ) )
	{
		PLAYBACK_EVENT_FULL( FEV_NOTHOST, m_pPlayer->edict(), m_usFireDisc, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, 0, 0, 1, 0 );

		FireDisc( true );

		// Deduct MAX_DISCS from fast shot, or deduct all discs if we don't have fast shot
		if ( m_pPlayer->HasPowerup( POW_FAST ) )
		{
			for ( int i = 1; i <= MAX_DISCS; i++ )
			{
				m_iFastShotDiscs--;
				if ( m_iFastShotDiscs == 0 )
				{
					m_iFastShotDiscs = NUM_FASTSHOT_DISCS;
					m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;

					// Remove a powered disc
					m_pPlayer->m_iPowerupDiscs--;
					if ( !m_pPlayer->m_iPowerupDiscs )
						m_pPlayer->RemoveAllPowerups();
				}
			}
		}
		else
		{
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = 0;

			// If we have powered discs, remove one
			if ( m_pPlayer->m_iPowerupDiscs )
			{
				m_pPlayer->m_iPowerupDiscs--;
				if ( !m_pPlayer->m_iPowerupDiscs )
					m_pPlayer->RemoveAllPowerups();
			}
		}
	}
}


void CDiscWeapon::WeaponIdle( void )
{
#if !defined( CLIENT_DLL )
	if ( m_pPlayer->HasPowerup(POW_VISUALIZE_REBOUNDS) )
	{
		Vector vecFireDir = g_vecZero;
		Vector vecSrc = m_pPlayer->pev->origin + (m_pPlayer->pev->view_ofs * 0.25);
		vecFireDir[1] = m_pPlayer->pev->v_angle[1];

		// Draw beams to show where rebounds will go
		for (int i = 0; i < 3; i++)
		{
			TraceResult	tr;
			UTIL_MakeVectors( vecFireDir );
			UTIL_TraceLine( vecSrc, (vecSrc + gpGlobals->v_forward * 2048), ignore_monsters, ENT(pev), &tr );

			MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
				WRITE_BYTE( TE_BEAMPOINTS );
				WRITE_COORD( vecSrc.x);
				WRITE_COORD( vecSrc.y);
				WRITE_COORD( vecSrc.z);
				WRITE_COORD( tr.vecEndPos.x);
				WRITE_COORD( tr.vecEndPos.y);
				WRITE_COORD( tr.vecEndPos.z);
				WRITE_SHORT( m_iSpriteTexture );
				WRITE_BYTE( 0 ); // framerate
				WRITE_BYTE( 0 ); // framerate
				WRITE_BYTE( 1 ); // life
				WRITE_BYTE( 40 );  // width
				WRITE_BYTE( 0 );   // noise
				WRITE_BYTE( i * 50 );   // r, g, b
				WRITE_BYTE( i * 50 );   // r, g, b
				WRITE_BYTE( 200 - (i * 50));   // r, g, b
				WRITE_BYTE( 128 - (i * 30) );   // r, g, b
				WRITE_BYTE( 0 );		// speed
			MESSAGE_END();

			// Calculate rebound angle
			Vector vecOut;
			Vector vecIn = tr.vecEndPos - (tr.vecEndPos - (gpGlobals->v_forward * 5));
			float backoff = DotProduct( vecIn, tr.vecPlaneNormal ) * 2.0;
			for (int i=0 ; i<3 ; i++)
			{
				float change = tr.vecPlaneNormal[i] * backoff;
				vecOut[i] = vecIn[i] - change;
				if (vecOut[i] > -0.1 && vecOut[i] < 0.1)
					vecOut[i] = 0;
			}

			vecOut = vecOut.Normalize();
			vecSrc = tr.vecEndPos;
			vecFireDir = UTIL_VecToAngles(vecOut);
		}
	}
#endif

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		int iAnim;
		float flRand = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0, 1 );
		if (flRand <= 0.75)
		{
			iAnim = DISC_IDLE;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );// how long till we do this again.
		}
		else 
		{
			iAnim = DISC_FIDGET;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 75.0 / 30.0;
		}

		SendWeaponAnim( iAnim, 1 );
	}
}

// Prevent disc weapons lying around on the ground
int CDiscWeapon::AddDuplicate( CBasePlayerItem *pOriginal )
{
	pev->flags |= FL_KILLME;
	return FALSE;
}



