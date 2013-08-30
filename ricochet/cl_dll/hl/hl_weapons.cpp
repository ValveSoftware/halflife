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
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "disc_weapon.h"
#include "discwar.h"
#include "nodes.h"
#include "player.h"

#include "usercmd.h"
#include "entity_state.h"
#include "demo_api.h"
#include "pm_defs.h"
#include "event_api.h"
#include "r_efx.h"

#include "../hud_iface.h"
#include "../com_weapons.h"
#include "../demo.h"

#include "r_studioint.h"

#include "../Ricochet_JumpPads.h"
#include "studio.h"
#include "com_model.h"

// Global engine <-> studio model rendering code interface
extern engine_studio_api_t IEngineStudio;

extern globalvars_t *gpGlobals;

// Pool of client side entities/entvars_t
static entvars_t	ev[ 32 ];
static int			num_ents = 0;

// The entity we'll use to represent the local client
static CBasePlayer	player;

// Local version of game .dll global variables ( time, etc. )
static globalvars_t	Globals; 

static CBasePlayerWeapon *g_pWpns[ 32 ];

// For storing predicted sequence and gaitsequence and origin/angles data
static int g_rseq = 0, g_gaitseq = 0;
static vec3_t g_clorg, g_clang;

// HLDM Weapon placeholder entities.
CDiscWeapon g_Disc;
extern int	g_iCannotFire;

/*
======================
AlertMessage

Print debug messages to console
======================
*/
void AlertMessage( ALERT_TYPE atype, char *szFmt, ... )
{
	va_list		argptr;
	static char	string[1024];
	
	va_start (argptr, szFmt);
	vsprintf (string, szFmt,argptr);
	va_end (argptr);

	gEngfuncs.Con_Printf( "cl:  " );
	gEngfuncs.Con_Printf( string );
}

/*
=====================
HUD_PrepEntity

Links the raw entity to an entvars_s holder.  If a player is passed in as the owner, then
we set up the m_pPlayer field.
=====================
*/
void HUD_PrepEntity( CBaseEntity *pEntity, CBasePlayer *pWeaponOwner )
{
	memset( &ev[ num_ents ], 0, sizeof( entvars_t ) );
	pEntity->pev = &ev[ num_ents++ ];

	pEntity->Precache();
	pEntity->Spawn();

	if ( pWeaponOwner )
	{
		ItemInfo info;
		
		((CBasePlayerWeapon *)pEntity)->m_pPlayer = pWeaponOwner;
		
		((CBasePlayerWeapon *)pEntity)->GetItemInfo( &info );

		g_pWpns[ info.iId ] = (CBasePlayerWeapon *)pEntity;
	}
}

/*
=====================
CBaseEntity :: Killed

If weapons code "kills" an entity, just set its effects to EF_NODRAW
=====================
*/
void CBaseEntity :: Killed( entvars_t *pevAttacker, int iGib )
{
	pev->effects |= EF_NODRAW;
}

/*
=====================
CBasePlayerWeapon :: DefaultReload
=====================
*/
BOOL CBasePlayerWeapon :: DefaultReload( int iClipSize, int iAnim, float fDelay )
{
#if 0 // FIXME, need to know primary ammo to get this right
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return FALSE;

	int j = min(iClipSize - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);	

	if (j == 0)
		return FALSE;
#endif

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + fDelay;

	//!!UNDONE -- reload sound goes here !!!
	SendWeaponAnim( iAnim );

	m_fInReload = TRUE;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3;
	return TRUE;
}

/*
=====================
CBasePlayerWeapon :: CanDeploy
=====================
*/
BOOL CBasePlayerWeapon :: CanDeploy( void ) 
{
	BOOL bHasAmmo = 0;

	if ( !pszAmmo1() )
	{
		// this weapon doesn't use ammo, can always deploy.
		return TRUE;
	}

	if ( pszAmmo1() )
	{
		bHasAmmo |= (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] != 0);
	}
	if ( pszAmmo2() )
	{
		bHasAmmo |= (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] != 0);
	}
	if (m_iClip > 0)
	{
		bHasAmmo |= 1;
	}
	if (!bHasAmmo)
	{
		return FALSE;
	}

	return TRUE;
}

/*
=====================
CBasePlayerWeapon :: DefaultDeploy

=====================
*/
BOOL CBasePlayerWeapon :: DefaultDeploy( char *szViewModel, char *szWeaponModel, int iAnim, char *szAnimExt, int skiplocal )
{
	if ( !CanDeploy() )
		return FALSE;

	gEngfuncs.CL_LoadModel( szViewModel, &m_pPlayer->pev->viewmodel );
	
	SendWeaponAnim( iAnim );

	m_pPlayer->m_flNextAttack = 0.5;
	m_flTimeWeaponIdle = 1.0;
	return TRUE;
}

/*
=====================
CBasePlayerWeapon :: PlayEmptySound

=====================
*/
BOOL CBasePlayerWeapon :: PlayEmptySound( void )
{
	if (m_iPlayEmptySound)
	{
		HUD_PlaySound( "weapons/357_cock1.wav", 0.8 );
		m_iPlayEmptySound = 0;
		return 0;
	}
	return 0;
}

/*
=====================
CBasePlayerWeapon :: ResetEmptySound

=====================
*/
void CBasePlayerWeapon :: ResetEmptySound( void )
{
	m_iPlayEmptySound = 1;
}

/*
=====================
CBasePlayerWeapon::Holster

Put away weapon
=====================
*/
void CBasePlayerWeapon::Holster( int skiplocal /* = 0 */ )
{ 
	m_fInReload = FALSE; // cancel any reload in progress.
	m_pPlayer->pev->viewmodel = 0; 
}

/*
=====================
CBasePlayerWeapon::SendWeaponAnim

Animate weapon model
=====================
*/
void CBasePlayerWeapon::SendWeaponAnim( int iAnim, int skiplocal )
{
	m_pPlayer->pev->weaponanim = iAnim;
	
	int body = 0;

	HUD_SendWeaponAnim( iAnim, body, 0 );
}

/*
=====================
CBasePlayerWeapon::ItemPostFrame

Handles weapon firing, reloading, etc.
=====================
*/
void CBasePlayerWeapon::ItemPostFrame( void )
{
	if ((m_fInReload) && (m_pPlayer->m_flNextAttack <= 0.0))
	{
#if 0 // FIXME, need ammo on client to make this work right
		// complete the reload. 
		int j = min( iMaxClip() - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);	

		// Add them to the clip
		m_iClip += j;
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= j;
#else	
		m_iClip += 10;
#endif
		m_fInReload = FALSE;
	}

	if ((m_pPlayer->pev->button & IN_ATTACK2) && (m_flNextSecondaryAttack <= 0.0))
	{
		if ( pszAmmo2() && !m_pPlayer->m_rgAmmo[SecondaryAmmoIndex()] )
		{
			m_fFireOnEmpty = TRUE;
		}

		SecondaryAttack();
		m_pPlayer->pev->button &= ~IN_ATTACK2;
	}
	else if ((m_pPlayer->pev->button & IN_ATTACK) && (m_flNextPrimaryAttack <= 0.0))
	{
		if ( (m_iClip == 0 && pszAmmo1()) || (iMaxClip() == -1 && !m_pPlayer->m_rgAmmo[PrimaryAmmoIndex()] ) )
		{
			m_fFireOnEmpty = TRUE;
		}

		PrimaryAttack();
	}
	else if ( m_pPlayer->pev->button & IN_RELOAD && iMaxClip() != WEAPON_NOCLIP && !m_fInReload ) 
	{
		// reload when reload is pressed, or if no buttons are down and weapon is empty.
		Reload();
	}
	else if ( !(m_pPlayer->pev->button & (IN_ATTACK|IN_ATTACK2) ) )
	{
		// no fire buttons down

		m_fFireOnEmpty = FALSE;

		// weapon is useable. Reload if empty and weapon has waited as long as it has to after firing
		if ( m_iClip == 0 && !(iFlags() & ITEM_FLAG_NOAUTORELOAD) && m_flNextPrimaryAttack < 0.0 )
		{
			Reload();
			return;
		}

		WeaponIdle( );
		return;
	}
	
	// catch all
	if ( ShouldWeaponIdle() )
	{
		WeaponIdle();
	}
}

/*
=====================
CBasePlayer::SelectItem

  Switch weapons
=====================
*/
void CBasePlayer::SelectItem(const char *pstr)
{
	if (!pstr)
		return;

	CBasePlayerItem *pItem = NULL;

	if (!pItem)
		return;

	
	if (pItem == m_pActiveItem)
		return;

	if (m_pActiveItem)
		m_pActiveItem->Holster( );
	
	m_pLastItem = m_pActiveItem;
	m_pActiveItem = pItem;

	if (m_pActiveItem)
	{
		m_pActiveItem->Deploy( );
	}
}

/*
=====================
CBasePlayer::SelectLastItem

=====================
*/
void CBasePlayer::SelectLastItem(void)
{
	if (!m_pLastItem)
	{
		return;
	}

	if ( m_pActiveItem && !m_pActiveItem->CanHolster() )
	{
		return;
	}

	if (m_pActiveItem)
		m_pActiveItem->Holster( );
	
	CBasePlayerItem *pTemp = m_pActiveItem;
	m_pActiveItem = m_pLastItem;
	m_pLastItem = pTemp;
	m_pActiveItem->Deploy( );
}

/*
=====================
CBasePlayer::Killed

=====================
*/
void CBasePlayer::Killed( entvars_t *pevAttacker, int iGib )
{
	// Holster weapon immediately, to allow it to cleanup
	if (m_pActiveItem)
		m_pActiveItem->Holster( );
}

/*
=====================
CBasePlayer::Spawn

=====================
*/
void CBasePlayer::Spawn( void )
{
	if (m_pActiveItem)
		m_pActiveItem->Deploy( );
}

/*
=====================
UTIL_TraceLine

Don't actually trace, but act like the trace didn't hit anything.
=====================
*/
void UTIL_TraceLine( const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, edict_t *pentIgnore, TraceResult *ptr )
{
	memset( ptr, 0, sizeof( *ptr ) );
	ptr->flFraction = 1.0;
}

/*
=====================
UTIL_ParticleBox

For debugging, draw a box around a player made out of particles
=====================
*/
void UTIL_ParticleBox( CBasePlayer *player, float *mins, float *maxs, float life, unsigned char r, unsigned char g, unsigned char b )
{
	int i;
	vec3_t mmin, mmax;

	for ( i = 0; i < 3; i++ )
	{
		mmin[ i ] = player->pev->origin[ i ] + mins[ i ];
		mmax[ i ] = player->pev->origin[ i ] + maxs[ i ];
	}

	gEngfuncs.pEfxAPI->R_ParticleBox( (float *)&mmin, (float *)&mmax, 5.0, 0, 255, 0 );
}

/*
=====================
UTIL_ParticleBoxes

For debugging, draw boxes for other collidable players
=====================
*/
void UTIL_ParticleBoxes( void )
{
	int idx;
	physent_t *pe;
	cl_entity_t *player;
	vec3_t mins, maxs;
	
	gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction( false, true );

	// Store off the old count
	gEngfuncs.pEventAPI->EV_PushPMStates();

	player = gEngfuncs.GetLocalPlayer();
	// Now add in all of the players.
	gEngfuncs.pEventAPI->EV_SetSolidPlayers ( player->index - 1 );	

	for ( idx = 1; idx < 100; idx++ )
	{
		pe = gEngfuncs.pEventAPI->EV_GetPhysent( idx );
		if ( !pe )
			break;

		if ( pe->info >= 1 && pe->info <= gEngfuncs.GetMaxClients() )
		{
			mins = pe->origin + pe->mins;
			maxs = pe->origin + pe->maxs;

			gEngfuncs.pEfxAPI->R_ParticleBox( (float *)&mins, (float *)&maxs, 0, 0, 255, 2.0 );
		}
	}

	gEngfuncs.pEventAPI->EV_PopPMStates();
}

/*
=====================
UTIL_ParticleLine

For debugging, draw a line made out of particles
=====================
*/
void UTIL_ParticleLine( CBasePlayer *player, float *start, float *end, float life, unsigned char r, unsigned char g, unsigned char b )
{
	gEngfuncs.pEfxAPI->R_ParticleLine( start, end, r, g, b, life );
}

/*
=====================
CBasePlayerWeapon::PrintState

For debugging, print out state variables to log file
=====================
*/
void CBasePlayerWeapon::PrintState( void )
{
	COM_Log( "c:\\hl.log", "%.4f ", gpGlobals->time );
	COM_Log( "c:\\hl.log", "%.4f ", m_pPlayer->m_flNextAttack );
	COM_Log( "c:\\hl.log", "%.4f ", m_flNextPrimaryAttack );
	COM_Log( "c:\\hl.log", "%.4f ", m_flTimeWeaponIdle - gpGlobals->time);
	COM_Log( "c:\\hl.log", "%i ", m_iClip );
}

/*
=====================
HUD_InitClientWeapons

Set up weapons, player and functions needed to run weapons code client-side.
=====================
*/
void HUD_InitClientWeapons( void )
{
	static int initialized = 0;
	if ( initialized )
		return;

	initialized = 1;

	// Set up pointer ( dummy object )
	gpGlobals = &Globals;

	// Fill in current time ( probably not needed )
	gpGlobals->time = gEngfuncs.GetClientTime();

	// Fake functions
	g_engfuncs.pfnPrecacheModel		= stub_PrecacheModel;
	g_engfuncs.pfnPrecacheSound		= stub_PrecacheSound;
	g_engfuncs.pfnPrecacheEvent		= stub_PrecacheEvent;
	g_engfuncs.pfnNameForFunction	= stub_NameForFunction;
	g_engfuncs.pfnSetModel			= stub_SetModel;
	g_engfuncs.pfnSetClientMaxspeed = HUD_SetMaxSpeed;

	// Handled locally
	g_engfuncs.pfnPlaybackEvent		= HUD_PlaybackEvent;
	g_engfuncs.pfnAlertMessage		= AlertMessage;

	// Pass through to engine
	g_engfuncs.pfnPrecacheEvent		= gEngfuncs.pfnPrecacheEvent;
	g_engfuncs.pfnRandomFloat		= gEngfuncs.pfnRandomFloat;
	g_engfuncs.pfnRandomLong		= gEngfuncs.pfnRandomLong;

	// Allocate a slot for the local player
	HUD_PrepEntity( &player		, NULL );

	// Allocate slot(s) for each weapon that we are going to be predicting
	HUD_PrepEntity( &g_Disc	, &player );
}

/*
==============================
LookupSequence

Find sequence # of named sequence
==============================
*/
int LookupSequence( void *pmodel, const char *label )
{
	studiohdr_t *pstudiohdr;
	
	pstudiohdr = (studiohdr_t *)pmodel;
	if (! pstudiohdr)
		return 0;

	mstudioseqdesc_t	*pseqdesc;

	pseqdesc = (mstudioseqdesc_t *)((byte *)pstudiohdr + pstudiohdr->seqindex);

	for (int i = 0; i < pstudiohdr->numseq; i++)
	{
		if (stricmp( pseqdesc[i].label, label ) == 0)
			return i;
	}

	return -1;
}

/*
==============================
LookupSequence

==============================
*/
int CBaseAnimating :: LookupSequence ( const char *label )
{
	cl_entity_t *current;

	current = gEngfuncs.GetLocalPlayer();
	if ( !current || !current->model )
		return 0;

	return ::LookupSequence( (studiohdr_t *)IEngineStudio.Mod_Extradata( current->model ), label );
}

/*
==============================
GetSequenceInfo

==============================
*/
void GetSequenceInfo( void *pmodel, entvars_t *pev, float *pflFrameRate, float *pflGroundSpeed )
{
	studiohdr_t *pstudiohdr;
	
	pstudiohdr = (studiohdr_t *)pmodel;
	if (! pstudiohdr)
		return;

	mstudioseqdesc_t	*pseqdesc;

	if (pev->sequence >= pstudiohdr->numseq)
	{
		*pflFrameRate = 0.0;
		*pflGroundSpeed = 0.0;
		return;
	}

	pseqdesc = (mstudioseqdesc_t *)((byte *)pstudiohdr + pstudiohdr->seqindex) + (int)pev->sequence;

	if (pseqdesc->numframes > 1)
	{
		*pflFrameRate = 256 * pseqdesc->fps / (pseqdesc->numframes - 1);
		*pflGroundSpeed = sqrt( pseqdesc->linearmovement[0]*pseqdesc->linearmovement[0]+ pseqdesc->linearmovement[1]*pseqdesc->linearmovement[1]+ pseqdesc->linearmovement[2]*pseqdesc->linearmovement[2] );
		*pflGroundSpeed = *pflGroundSpeed * pseqdesc->fps / (pseqdesc->numframes - 1);
	}
	else
	{
		*pflFrameRate = 256.0;
		*pflGroundSpeed = 0.0;
	}
}

/*
==============================
GetSequenceFlags

==============================
*/
int GetSequenceFlags( void *pmodel, entvars_t *pev )
{
	studiohdr_t *pstudiohdr;
	
	pstudiohdr = (studiohdr_t *)pmodel;
	if ( !pstudiohdr || pev->sequence >= pstudiohdr->numseq )
		return 0;

	mstudioseqdesc_t	*pseqdesc;
	pseqdesc = (mstudioseqdesc_t *)((byte *)pstudiohdr + pstudiohdr->seqindex) + (int)pev->sequence;

	return pseqdesc->flags;
}

/*
==============================
ResetSequenceInfo

==============================
*/
void CBaseAnimating :: ResetSequenceInfo ( )
{
	cl_entity_t *current;

	current = gEngfuncs.GetLocalPlayer();
	if ( !current || !current->model )
		return;

	void *pmodel = (studiohdr_t *)IEngineStudio.Mod_Extradata( current->model );

	GetSequenceInfo( pmodel, pev, &m_flFrameRate, &m_flGroundSpeed );
	m_fSequenceLoops = ((GetSequenceFlags() & STUDIO_LOOPING) != 0);
	pev->animtime = gpGlobals->time;
	pev->framerate = 1.0;
	m_fSequenceFinished = FALSE;
	m_flLastEventCheck = gpGlobals->time;
}

/*
==============================
UTIL_MakeVectors

==============================
*/
void UTIL_MakeVectors( const Vector &vecAngles )
{
	gEngfuncs.pfnAngleVectors ( (float *)&vecAngles, gpGlobals->v_forward, gpGlobals->v_right, gpGlobals->v_up);
}

/*
==============================
GetFallAnimation

==============================
*/
int CBasePlayer::GetFallAnimation( void )
{
	Vector vecNormVel = pev->velocity;
	vecNormVel.Normalize();
	int    fallAnim;

	UTIL_MakeVectors( pev->angles );
	float flDot = DotProduct( vecNormVel, gpGlobals->v_forward );
	float flSideDot = DotProduct( vecNormVel, gpGlobals->v_right );
	// Choose a falling animation based upon the velocity vector
	if ( flDot < -0.6 )
		fallAnim = LookupSequence( "fall_b" );
	else if ( flSideDot < -0.6 )
		fallAnim = LookupSequence( "fall_r" );
	else if ( flSideDot > 0.6 )
		fallAnim = LookupSequence( "fall_l" );
	else
		fallAnim = LookupSequence( "fall_f" );
	
	return fallAnim;
}

#define WALK_SPEED		100
// Set the activity based on an event or current state

/*
==============================
SetAnimation

==============================
*/
void CBasePlayer::SetAnimation( PLAYER_ANIM playerAnim )
{
	int animDesired;
	float speed;

	speed = pev->velocity.Length2D();

	switch (playerAnim) 
	{
	case PLAYER_JUMP:
		m_IdealActivity = ACT_HOP;
		break;
	
	case PLAYER_SUPERJUMP:
		m_IdealActivity = ACT_LEAP;
		break;
	
	case PLAYER_ATTACK1:
		m_IdealActivity = ACT_BASE_THROW;
		break;

	case PLAYER_FALL:
		m_IdealActivity = ACT_FALL;
		break;

	case PLAYER_IDLE:
	case PLAYER_WALK:
		if ( !FBitSet( pev->flags, FL_ONGROUND ) && (m_Activity == ACT_HOP) )	// Still jumping
		{
			m_IdealActivity = m_Activity;
		}
		else
		{
			m_IdealActivity = ACT_BASE_WALK;
		}
		break;
	}

	Vector vecNormVel;
	float  flDot, flSideDot, flVelDot;
	bool   bInReverse;
	int	   iFrame;

	// Decide which sequence to play based upon the activity
	switch (m_IdealActivity)
	{
	case ACT_DIEFORWARD:
	case ACT_FALL:
	default:
		if ( m_Activity == m_IdealActivity)
			return;
		m_Activity = ACT_FALL;

		animDesired = GetFallAnimation();
		// Already using the desired animation?
		if (pev->sequence == animDesired)
			return;

		pev->gaitsequence = 0;
		pev->sequence		= animDesired;
		pev->frame			= 0;
		ResetSequenceInfo( );
		return;

	case ACT_LEAP:
		UTIL_MakeVectors( pev->angles );
		vecNormVel = pev->velocity;
		vecNormVel.Normalize();
		flDot = DotProduct( vecNormVel, gpGlobals->v_forward );
		if ( flDot < -0.6 )
		{
			// Use non-blended backflip
			animDesired = LookupSequence( "backflip" );
			m_Activity = m_IdealActivity;
			pev->gaitsequence = 0;
			pev->sequence		= animDesired;
			pev->frame			= 0;
			ResetSequenceInfo( );
			return;
		}
		else
		{
			// Use blended longjump
			animDesired = LookupSequence( "longjump" );
			m_Activity = ACT_LEAP;
			pev->gaitsequence   = animDesired;
			pev->sequence		= animDesired;
			pev->frame			= 0;
			ResetSequenceInfo( );
			return;
		}
		break;

	case ACT_DIE_HEADSHOT:
		animDesired = LookupSequence( "die_simple" );
		m_Activity = m_IdealActivity;

		pev->gaitsequence = 0;
		pev->sequence		= animDesired;
		pev->frame			= 0;
		ResetSequenceInfo( );
		return;

	case ACT_HOP:
		iFrame = pev->frame / 18;
		if ( iFrame >= 2 && iFrame <= 11 )
			animDesired = LookupSequence( "jump" );
		else
			animDesired = LookupSequence( "jumpl" );
		// Already using the desired animation?
		if (pev->sequence == animDesired)
			return;

		pev->sequence		= animDesired;
		pev->frame			= 0;
		ResetSequenceInfo( );
		return;

	case ACT_BASE_THROW:
		// No throw animation during backflip
		if ( pev->sequence == LookupSequence( "backflip" ) )
			return;

		// If we're in the air, we need to use the blended longjump throw
		if ( pev->sequence == LookupSequence( "longjump" ) )
		{
			// Use blended longjump
			animDesired = LookupSequence( "longjump_throw" );
			m_Activity = ACT_FLINCH_CLOCKWISE;
			pev->gaitsequence   = animDesired;
			pev->sequence		= animDesired;
			pev->frame			= 0;
			ResetSequenceInfo( );
			return;
		}

		animDesired = GetThrowAnim();
		m_Activity = m_IdealActivity;
		m_flThrowTime = 0.25;
		break;

	case ACT_BASE_WALK:
		UTIL_MakeVectors( pev->angles );
		bInReverse = ( pev->sequence == LookupSequence("base_reverse") );
		vecNormVel = pev->velocity;
		vecNormVel.Normalize();
		flDot = DotProduct( vecNormVel, gpGlobals->v_forward );
		flSideDot = DotProduct( vecNormVel, gpGlobals->v_right );
		flVelDot = DotProduct( m_vecOldVelocity, vecNormVel );

		if ( ( m_flBackupTime <= 0 ) && (m_Activity != ACT_BASE_THROW) || m_fSequenceFinished )
		{
			if ( speed == 0 )
			{
				animDesired = LookupSequence( "base_stand" );
			}
			else if ( flDot < -0.6 )
			{
				animDesired = LookupSequence( "base_backup" );
			}
			else if ( ( flVelDot <= 0 ) && ( flDot <= 0.6 ) )
			{
				animDesired = LookupSequence( "base_reverse" );
				m_flBackupTime = 0.7;
				pev->effects |= EF_NOINTERP;
			}
			else
			{
				if ( speed > WALK_SPEED )
					animDesired = LookupSequence( "base_run" );
				else
					animDesired = LookupSequence( "base_walk" );
			}

			if (animDesired == -1)
			{
				animDesired = 0;
			}
			m_Activity = ACT_BASE_WALK;
		}
		else if ( bInReverse )
		{
			// Don't play the backup run if we're still in the backup run
			if ( DotProduct( m_vecOldVelocity, vecNormVel ) < 0 )
			{
				m_flBackupTime = 0;
				if ( speed > WALK_SPEED )
					animDesired = LookupSequence( "base_run" );
				else
					animDesired = LookupSequence( "base_walk" );
				pev->effects |= EF_NOINTERP;
			}
			else
			{
				animDesired = pev->sequence;
			}
		}
		else
		{
			animDesired = pev->sequence;
		}
		break;
	}

	// Set gait animation
	if ( m_flBackupTime > 0 )
	{
		pev->gaitsequence	= LookupSequence( "base_backup" );
	}
	else
	{ 
		if ( speed > WALK_SPEED )
		{
			pev->gaitsequence	= LookupSequence( "base_run" );
		}
		else if (speed > 0)
		{
			pev->gaitsequence	= LookupSequence( "base_walk" );
		}
	}

	// Idle?
	if (speed <= 0)
	{
		pev->gaitsequence	= LookupSequence( "base_stand" );
	}

	// Already using the desired animation?
	if (pev->sequence == animDesired)
		return;

	// Reset to first frame of desired animation
	pev->sequence		= animDesired;
	pev->frame			= 0;
	ResetSequenceInfo( );
}

int CBasePlayer::GetThrowAnim( void )
{
	int	throwAnim;

	if ( pev->velocity.Length2D() == 0 )
		throwAnim = LookupSequence( "base_stand_throw" );
	else
		throwAnim = LookupSequence( "base_throw" );

	return throwAnim;
}

int CBasePlayer::GetHoldAnim( void )
{
	int	holdAnim;

	// Choose hold anim based upon powerups.
	// Multiple Powerups can be had, in which case the one considered more dangerous has the animation.
	if ( m_iPowerups & POW_TRIPLE )
		holdAnim = LookupSequence( "triple_ready" );
	else if ( m_iPowerups & POW_FAST )
		holdAnim = LookupSequence( "kill_ready" );
	else if ( m_iPowerups & POW_HARD )
		holdAnim = LookupSequence( "hard_ready" );
	else if ( m_iPowerups & POW_FREEZE )
		holdAnim = LookupSequence( "freeze_ready" );
	else
		holdAnim = LookupSequence( "base_ready" );

	return holdAnim;
}

/*
==============================
PostThink

==============================
*/
void CBasePlayer::PostThink()
{
	if ( !g_runfuncs )
		return;

	// select the proper animation for the player character	
	if ( !CL_IsDead() && (m_flTouchedByJumpPad < gpGlobals->time) )
	{
		if (!pev->velocity.x && !pev->velocity.y)
			SetAnimation( PLAYER_IDLE );
		else if ((pev->velocity.x || pev->velocity.y) && (FBitSet(pev->flags, FL_ONGROUND)))
			SetAnimation( PLAYER_WALK );
		else if (pev->waterlevel > 1)
			SetAnimation( PLAYER_WALK );
	}

	if ( !CL_IsDead() && ( m_flThrowTime <= 0.0 ) )
	{
		m_Activity = ACT_BASE_WALK;
		m_flThrowTime = 0.0;
		if (!pev->velocity.x && !pev->velocity.y)
		{
			SetAnimation( PLAYER_IDLE );
		}
		else
		{
			SetAnimation( PLAYER_WALK );
		}
	}

	// Store old velocity for use in backpedalling animations
	m_vecOldVelocity = pev->velocity;
	m_vecOldVelocity.Normalize();
}

/*
=====================
HUD_WeaponsPostThink

Run Weapon firing code on client
=====================
*/
void HUD_WeaponsPostThink( local_state_s *from, local_state_s *to, usercmd_t *cmd, double time, unsigned int random_seed )
{
	int i;
	int buttonsChanged;
	CBasePlayerWeapon *pWeapon = NULL;
	CBasePlayerWeapon *pCurrent;
	weapon_data_t nulldata, *pfrom, *pto;
	static int lasthealth;

	memset( &nulldata, 0, sizeof( nulldata ) );

	HUD_InitClientWeapons();	

	// Get current clock
	gpGlobals->time = time;

	// Fill in data based on selected weapon
	// FIXME, make this a method in each weapon?  where you pass in an entity_state_t *?
	switch ( from->client.m_iId )
	{
	case WEAPON_DISC:
		pWeapon = &g_Disc;
		break;
	}

	// We are not predicting the current weapon, just bow out here.
	if ( !pWeapon )
		return;

	for ( i = 0; i < 32; i++ )
	{
		pCurrent = g_pWpns[ i ];
		if ( !pCurrent )
		{
			continue;
		}

		pfrom = &from->weapondata[ i ];
		
		pCurrent->m_fInReload			= pfrom->m_fInReload;
		pCurrent->m_iClip				= pfrom->m_iClip;
		pCurrent->m_flNextPrimaryAttack	= pfrom->m_flNextPrimaryAttack;
		pCurrent->m_flNextSecondaryAttack = pfrom->m_flNextSecondaryAttack;
		pCurrent->m_flTimeWeaponIdle	= pfrom->m_flTimeWeaponIdle;

		// Ricochet uses m_iClip to transmit current/primary ammo to client
		if ( pWeapon == pCurrent )
		{
			player.m_rgAmmo[pCurrent->m_iPrimaryAmmoType] = pfrom->m_iClip;
		}
	}

	// For random weapon events, use this seed to seed random # generator
	player.random_seed = random_seed;

	// Get old buttons from previous state.
	player.m_afButtonLast = from->playerstate.oldbuttons;

	// Which buttsons chave changed
	buttonsChanged = (player.m_afButtonLast ^ cmd->buttons);	// These buttons have changed this frame
	
	// Debounced button codes for pressed/released
	// The changed ones still down are "pressed"
	player.m_afButtonPressed =  buttonsChanged & cmd->buttons;	
	// The ones not down are "released"
	player.m_afButtonReleased = buttonsChanged & (~cmd->buttons);

	// Set player variables that weapons code might check/alter
	player.pev->button = cmd->buttons;

	player.pev->velocity = from->client.velocity;
	player.pev->flags = from->client.flags;

	player.pev->deadflag = from->client.deadflag;
	player.pev->waterlevel = from->client.waterlevel;
	player.pev->maxspeed    = from->client.maxspeed;
	player.pev->fov = from->client.fov;
	player.pev->weaponanim = from->client.weaponanim;
	player.pev->viewmodel = from->client.viewmodel;
	player.m_flNextAttack = from->client.m_flNextAttack;
	player.m_flBackupTime = from->client.fuser1;
	player.m_Activity	  = (Activity)(int)from->client.fuser2;
	player.m_flThrowTime = from->client.fuser3;

	player.m_vecOldVelocity = from->client.vuser1;

	player.pev->sequence = from->playerstate.sequence;
	player.pev->gaitsequence = from->playerstate.gaitsequence;
	player.pev->angles = from->playerstate.angles;

	// Point to current weapon object
	if ( from->client.m_iId )
	{
		player.m_pActiveItem = g_pWpns[ from->client.m_iId ];
	}

	// Store pointer to our destination entity_state_t so we can get our origin, etc. from it
	//  for setting up events on the client
	g_finalstate = to;

	// Don't go firing anything if we have died.
	// Or if we don't have a weapon model deployed
	if ( ( player.pev->deadflag != ( DEAD_DISCARDBODY + 1 ) ) && !CL_IsDead() && player.pev->viewmodel )
	{
		if ( player.m_flNextAttack <= 0 )
		{
			pWeapon->ItemPostFrame();
		}
	}

	// If we are running events/etc. go ahead and see if we
	//  managed to die between last frame and this one
	// If so, run the appropriate player killed or spawn function
	if ( g_runfuncs )
	{
		if ( to->client.health <= 0 && lasthealth > 0 )
		{
			player.Killed( NULL, 0 );
		}
		else if ( to->client.health > 0 && lasthealth <= 0 )
		{
			player.Spawn();
		}

		lasthealth = to->client.health;
	}

	// Fix up animations, etc.
	player.PostThink();

	// Assume that we are not going to switch weapons
	to->client.m_iId					= from->client.m_iId;

	// Now see if we issued a changeweapon command ( and we're not dead )
	if ( cmd->weaponselect && ( player.pev->deadflag != ( DEAD_DISCARDBODY + 1 ) ) )
	{
		// Switched to a different weapon?
		if ( from->weapondata[ cmd->weaponselect ].m_iId == cmd->weaponselect )
		{
			CBasePlayerWeapon *pNew = g_pWpns[ cmd->weaponselect ];
			if ( pNew && ( pNew != pWeapon ) )
			{
				// Put away old weapon
				if (player.m_pActiveItem)
					player.m_pActiveItem->Holster( );
				
				player.m_pLastItem = player.m_pActiveItem;
				player.m_pActiveItem = pNew;

				// Deploy new weapon
				if (player.m_pActiveItem)
				{
					player.m_pActiveItem->Deploy( );
				}

				// Update weapon id so we can predict things correctly.
				to->client.m_iId = cmd->weaponselect;
			}
		}
	}

	// Copy in results of predcition code
	to->client.viewmodel				= player.pev->viewmodel;
	to->client.fov						= player.pev->fov;
	to->client.weaponanim				= player.pev->weaponanim;
	to->client.m_flNextAttack			= player.m_flNextAttack;
	to->client.maxspeed					= player.pev->maxspeed;
	to->client.fuser1					= player.m_flBackupTime;
	to->client.fuser2					= (float)(int)player.m_Activity;
	to->client.fuser3					= player.m_flThrowTime;

	to->client.vuser1					= player.m_vecOldVelocity;

	to->playerstate.sequence = player.pev->sequence;
	to->playerstate.gaitsequence = player.pev->gaitsequence;

	// Make sure that weapon animation matches what the game .dll is telling us
	//  over the wire ( fixes some animation glitches )
	if ( g_runfuncs && ( HUD_GetWeaponAnim() != to->client.weaponanim ) )
	{
		int body = 2;
		// Force a fixed anim down to viewmodel
		HUD_SendWeaponAnim( to->client.weaponanim, body, 1 );
	}

	for ( i = 0; i < 32; i++ )
	{
		pCurrent = g_pWpns[ i ];

		pto = &to->weapondata[ i ];

		if ( !pCurrent )
		{
			memset( pto, 0, sizeof( weapon_data_t ) );
			continue;
		}
	
		pto->m_fInReload				= pCurrent->m_fInReload;
		pto->m_iClip					= pCurrent->m_iClip; 
		pto->m_flNextPrimaryAttack		= pCurrent->m_flNextPrimaryAttack;
		pto->m_flNextSecondaryAttack	= pCurrent->m_flNextSecondaryAttack;
		pto->m_flTimeWeaponIdle			= pCurrent->m_flTimeWeaponIdle;

		// Decrement weapon counters, server does this at same time ( during post think, after doing everything else )
		pto->m_flNextReload				-= cmd->msec / 1000.0;
		pto->m_fNextAimBonus			-= cmd->msec / 1000.0;
		pto->m_flNextPrimaryAttack		-= cmd->msec / 1000.0;
		pto->m_flNextSecondaryAttack	-= cmd->msec / 1000.0;
		pto->m_flTimeWeaponIdle			-= cmd->msec / 1000.0;

		if ( pto->m_flPumpTime != -9999 )
		{
			pto->m_flPumpTime -= cmd->msec / 1000.0;
			if ( pto->m_flPumpTime < -0.001 )
				pto->m_flPumpTime = -0.001;
		}

		if ( pto->m_fNextAimBonus < -1.0 )
		{
			pto->m_fNextAimBonus = -1.0;
		}

		if ( pto->m_flNextPrimaryAttack < -1.0 )
		{
			pto->m_flNextPrimaryAttack = -1.0;
		}

		if ( pto->m_flNextSecondaryAttack < -0.001 )
		{
			pto->m_flNextSecondaryAttack = -0.001;
		}

		if ( pto->m_flTimeWeaponIdle < -0.001 )
		{
			pto->m_flTimeWeaponIdle = -0.001;
		}

		if ( pto->m_flNextReload < -0.001 )
		{
			pto->m_flNextReload = -0.001;
		}
	}

	// m_flNextAttack is now part of the weapons, but is part of the player instead
	to->client.m_flNextAttack -= cmd->msec / 1000.0;
	if ( to->client.m_flNextAttack < -0.001 )
	{
		to->client.m_flNextAttack = -0.001;
	}

	to->client.fuser1 -= cmd->msec / 1000.0;
	if ( to->client.fuser1 < -0.001 )
	{
		to->client.fuser1 = -0.001;
	}

	to->client.fuser3 -= cmd->msec / 1000.0;
	if ( to->client.fuser3 < -0.001 )
	{
		to->client.fuser3 = -0.001;
	}

	// Wipe it so we can't use it after this frame
	g_finalstate = NULL;
}

/*
==============================
Ricochet_GetSequence

==============================
*/
void Ricochet_GetSequence( int *seq, int *gaitseq )
{
	*seq = g_rseq;
	*gaitseq = g_gaitseq;
}

/*
==============================
Ricochet_SetSequence

==============================
*/
void Ricochet_SetSequence( int seq, int gaitseq )
{
	g_rseq = seq;
	g_gaitseq = gaitseq;
}

/*
==============================
Ricochet_SetOrientation

==============================
*/
void Ricochet_SetOrientation( vec3_t o, vec3_t a )
{
	g_clorg = o;
	g_clang = a;
}

/*
==============================
Ricochet_GetOrientation

==============================
*/
void Ricochet_GetOrientation( float *o, float *a )
{
	int i;
	
	for ( i = 0; i < 3; i++ )
	{
		o[ i ] = g_clorg[ i ];
		a[ i ] = g_clang[ i ];
	}
}


/*
=====================
HUD_PostRunCmd

Client calls this during prediction, after it has moved the player and updated any info changed into to->
time is the current client clock based on prediction
cmd is the command that caused the movement, etc
runfuncs is 1 if this is the first time we've predicted this command.  If so, sounds and effects should play, otherwise, they should
be ignored
=====================
*/
void EXPORT HUD_PostRunCmd( struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed )
{
	g_runfuncs = runfuncs;

	// We'll always do prediction of disc throwing
	if ( cl_lw && cl_lw->value )
	{
		// Allowed to fire?
		if ( g_iCannotFire == FALSE )
			HUD_WeaponsPostThink( from, to, cmd, time, random_seed );
	}
	else
	{
		to->client.fov = g_lastFOV;
	}

	// Store of final sequence, etc. for client side animation
	if ( g_runfuncs )
	{
		Ricochet_SetSequence( to->playerstate.sequence, to->playerstate.gaitsequence );
		Ricochet_SetOrientation( to->playerstate.origin, cmd->viewangles );
	}

	// See if we stepped on a jump pad
	Ricochet_CheckJumpPads( from, to );

	// All games can use FOV state
	g_lastFOV = to->client.fov;
}
