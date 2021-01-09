#if defined( GRAPPLING_HOOK )

#include "extdll.h"
#include "util.h"

#include "cbase.h"
#include "weapons.h"
#include "gamerules.h"
#include "player.h"

#include "grapplinghook.h"

//================================================================
// Cold Ice Grapple Hook
// Also see: https://developer.valvesoftware.com/wiki/Grapple_Hook
//================================================================

#define HOOK_SPEED 1200

LINK_ENTITY_TO_CLASS( grapple_hook, CHook );

CHook *CHook::HookCreate( CBasePlayer *owner )
{
	CHook *pHook = GetClassPtr( (CHook *)NULL );
	pHook->pevOwner = owner;
	pHook->pev->classname = MAKE_STRING("hook");
	return pHook;
}

void CHook::Spawn( )
{
	Precache( );
	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_BBOX;
	pev->gravity = -1;

	SET_MODEL(ENT(pev), "models/crossbow_bolt.mdl");

	UTIL_SetOrigin( pev, pev->origin );
	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));
}

void CHook::Precache( )
{
	PRECACHE_MODEL ("models/crossbow_bolt.mdl");

	PRECACHE_SOUND("weapons/xbow_hitbod1.wav");
	PRECACHE_SOUND("weapons/xbow_hitbod2.wav");

	PRECACHE_SOUND("weapons/xbow_fly1.wav");
	PRECACHE_SOUND("weapons/xbow_hit1.wav");
	PRECACHE_SOUND("weapons/rocketfire1.wav");

	ropesprite = PRECACHE_MODEL("sprites/smoke.spr");
}

int CHook :: Classify ( void )
{
	return  CLASS_NONE;
}

void CHook::FireHook( ) {
	if ( m_fActiveHook ) {
		return;
	}

	EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/rocketfire1.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0,0xF));

	pev->owner = edict();
	Spawn();

	TraceResult tr;
	Vector anglesAim = pevOwner->pev->v_angle + pevOwner->pev->punchangle;
	UTIL_MakeVectors( anglesAim );
	anglesAim.x = -anglesAim.x;
	Vector vecDir = gpGlobals->v_forward;
	Vector trace_origin = pevOwner->GetGunPosition();
	if ( pevOwner->pev->flags & FL_DUCKING ) {
			trace_origin = trace_origin - ( VEC_HULL_MIN - VEC_DUCK_HULL_MIN );
	}
	UTIL_TraceLine( trace_origin + gpGlobals->v_forward * 20, trace_origin + gpGlobals->v_forward * 64, dont_ignore_monsters, NULL, &tr );

	pev->origin = tr.vecEndPos;
	pev->angles = anglesAim;
	pev->velocity = vecDir * HOOK_SPEED;
	m_vVecDirHookMove = vecDir;

	m_fActiveHook = TRUE;

	SetTouch( HookTouch );
	pev->nextthink = gpGlobals->time + 0.2;
}

void CHook::HookTouch( CBaseEntity *pOther )
{
	if (pOther == pevOwner) {
		pev->nextthink = gpGlobals->time + 0.2;
		return;
	}

	SetTouch( NULL );
	SetThink( NULL );

	if (pOther->pev->takedamage)
	{
		TraceResult tr = UTIL_GetGlobalTrace( );
		entvars_t *Owner;
		Owner = VARS( pev->owner );
		pOther->TraceAttack(Owner, 5, pev->velocity.Normalize(), &tr, DMG_NEVERGIB );
		ApplyMultiDamage( pev, Owner );
		pev->velocity = Vector( 0, 0, 0 );

		switch( RANDOM_LONG(0,1) )
		{
			case 0:
				EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/xbow_hitbod1.wav", 1, ATTN_NORM);
				break;
			case 1:
				EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/xbow_hitbod2.wav", 1, ATTN_NORM);
				break;
		}
		pevOwner->pev->movetype = MOVETYPE_WALK;
		pevOwner->pev->gravity = 1;

		m_fActiveHook = FALSE;
		m_fHookInWall = FALSE;
		m_fPlayerAtEnd = FALSE;
		SUB_Remove();

		if ( !g_pGameRules->IsMultiplayer() )
		{
			Killed( pev, GIB_NEVER );
		}
	}
	else
	{
		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/xbow_hit1.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 98 + RANDOM_LONG(0,7));

		Vector vecDir = pev->velocity.Normalize( );
		UTIL_SetOrigin( pev, pev->origin - vecDir * 12 );
		pev->angles = UTIL_VecToAngles( vecDir );
		pev->solid = SOLID_NOT;
		pev->movetype = MOVETYPE_FLY;
		pev->velocity = Vector( 0, 0, 0 );
		pev->avelocity.z = 0;
		pev->angles.z = RANDOM_LONG(0,360);
		SetThink( Think );
		pev->nextthink = gpGlobals->time + 0.01;

		m_fHookInWall = TRUE;
		m_fActiveHook = TRUE;
		pevOwner->pev->movetype = MOVETYPE_FLY;

		UTIL_Sparks( pev->origin );
	}
}

void CHook::KillHook( void )
{
	pevOwner->pev->movetype = MOVETYPE_WALK;
	pevOwner->pev->gravity = 1;

	m_fActiveHook = FALSE;
	m_fHookInWall = FALSE;
	m_fPlayerAtEnd = FALSE;

	SUB_Remove();
}

void CHook::Think ( void )
{
	if ( !pevOwner->IsAlive( ) )
	{
		KillHook();
		return;
	}

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_BEAMENTS );
		WRITE_SHORT( pevOwner->entindex() );
		WRITE_SHORT( this->entindex() );
		WRITE_SHORT( ropesprite );
		WRITE_BYTE( 1 );
		WRITE_BYTE( 0 );
		WRITE_BYTE( 1 );
		WRITE_BYTE( 10 );
		WRITE_BYTE( 0 );
		WRITE_BYTE( 0 ); // Red
		WRITE_BYTE( 113 ); // Green
		WRITE_BYTE( 230 ); // Blue
		WRITE_BYTE( 185 ); // Brightness
		WRITE_BYTE( 10 );
	MESSAGE_END( );

	if ( m_fHookInWall )
	{
		if ( !m_fPlayerAtEnd )
		{
			if (( pev->origin - pevOwner->pev->origin ).Length() >= 50.0)
			{
				pevOwner->pev->velocity = (( pev->origin - pevOwner->pev->origin ) + m_vVecDirHookMove) * 3.0;
				pevOwner->pev->speed = 150;
			}
			else
			{
				m_vPlayerHangOrigin = pevOwner->pev->origin;
				m_fPlayerAtEnd = TRUE;
			}
		}
		if ( m_fPlayerAtEnd )
		{
			pevOwner->pev->origin = m_vPlayerHangOrigin;
			pevOwner->pev->velocity = Vector(0, 0, 0);
			pevOwner->pev->gravity = -.001;
			pevOwner->pev->speed = -.001;
		}
	}

	pev->nextthink = gpGlobals->time + 0.01;
}
#endif
