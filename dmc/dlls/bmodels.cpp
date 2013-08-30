/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
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

===== bmodels.cpp ========================================================

  spawn, think, and use functions for entities that use brush models

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "doors.h"

extern DLL_GLOBAL Vector		g_vecAttackDir;

#define		SF_BRUSH_ACCDCC	16// brush should accelerate and decelerate when toggled
#define		SF_BRUSH_HURT		32// rotating brush that inflicts pain based on rotation speed
#define		SF_ROTATING_NOT_SOLID	64	// some special rotating objects are not solid.

// covering cheesy noise1, noise2, & noise3 fields so they make more sense (for rotating fans)
#define		noiseStart		noise1
#define		noiseStop		noise2
#define		noiseRunning	noise3

#define		SF_PENDULUM_SWING		2	// spawnflag that makes a pendulum a rope swing.
//
// BModelOrigin - calculates origin of a bmodel from absmin/size because all bmodel origins are 0 0 0
//
Vector VecBModelOrigin( entvars_t* pevBModel )
{
	return pevBModel->absmin + ( pevBModel->size * 0.5 );
}

// =================== FUNC_WALL ==============================================

/*QUAKED func_wall (0 .5 .8) ?
This is just a solid wall if not inhibited
*/
class CFuncWall : public CBaseEntity
{
public:
	void	Spawn( void );
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	// Bmodels don't go across transitions
	virtual int	ObjectCaps( void ) { return CBaseEntity :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
};

LINK_ENTITY_TO_CLASS( func_wall, CFuncWall );

void CFuncWall :: Spawn( void )
{
	pev->angles		= g_vecZero;
	pev->movetype	= MOVETYPE_PUSH;  // so it doesn't get pushed by anything
	pev->solid		= SOLID_BSP;
	SET_MODEL( ENT(pev), STRING(pev->model) );
	
	// If it can't move/go away, it's really part of the world
	pev->flags |= FL_WORLDBRUSH;
}


void CFuncWall :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( ShouldToggle( useType, (int)(pev->frame)) )
		pev->frame = 1 - pev->frame;
}


#define SF_WALL_START_OFF		0x0001

class CFuncWallToggle : public CFuncWall
{
public:
	void	Spawn( void );
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void	TurnOff( void );
	void	TurnOn( void );
	BOOL	IsOn( void );
};

LINK_ENTITY_TO_CLASS( func_wall_toggle, CFuncWallToggle );

void CFuncWallToggle :: Spawn( void )
{
	CFuncWall::Spawn();
	if ( pev->spawnflags & SF_WALL_START_OFF )
		TurnOff();
}


void CFuncWallToggle :: TurnOff( void )
{
	pev->solid = SOLID_NOT;
	pev->effects |= EF_NODRAW;
	UTIL_SetOrigin( pev, pev->origin );
}


void CFuncWallToggle :: TurnOn( void )
{
	pev->solid = SOLID_BSP;
	pev->effects &= ~EF_NODRAW;
	UTIL_SetOrigin( pev, pev->origin );
}


BOOL CFuncWallToggle :: IsOn( void )
{
	if ( pev->solid == SOLID_NOT )
		return FALSE;
	return TRUE;
}


void CFuncWallToggle :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	int status = IsOn();

	if ( ShouldToggle( useType, status ) )
	{
		if ( status )
			TurnOff();
		else
			TurnOn();
	}
}


#define SF_CONVEYOR_VISUAL		0x0001
#define SF_CONVEYOR_NOTSOLID	0x0002

class CFuncConveyor : public CFuncWall
{
public:
	void	Spawn( void );
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void	UpdateSpeed( float speed );
};

LINK_ENTITY_TO_CLASS( func_conveyor, CFuncConveyor );
void CFuncConveyor :: Spawn( void )
{
	SetMovedir( pev );
	CFuncWall::Spawn();

	if ( !(pev->spawnflags & SF_CONVEYOR_VISUAL) )
		SetBits( pev->flags, FL_CONVEYOR );

	// HACKHACK - This is to allow for some special effects
	if ( pev->spawnflags & SF_CONVEYOR_NOTSOLID )
	{
		pev->solid = SOLID_NOT;
		pev->skin = 0;		// Don't want the engine thinking we've got special contents on this brush
	}

	if ( pev->speed == 0 )
		pev->speed = 100;

	UpdateSpeed( pev->speed );
}


// HACKHACK -- This is ugly, but encode the speed in the rendercolor to avoid adding more data to the network stream
void CFuncConveyor :: UpdateSpeed( float speed )
{
	// Encode it as an integer with 4 fractional bits
	int speedCode = (int)(fabs(speed) * 16.0);

	if ( speed < 0 )
		pev->rendercolor.x = 1;
	else
		pev->rendercolor.x = 0;

	pev->rendercolor.y = (speedCode >> 8);
	pev->rendercolor.z = (speedCode & 0xFF);
}


void CFuncConveyor :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
    pev->speed = -pev->speed;
	UpdateSpeed( pev->speed );
}



// =================== FUNC_ILLUSIONARY ==============================================


/*QUAKED func_illusionary (0 .5 .8) ?
A simple entity that looks solid but lets you walk through it.
*/
class CFuncIllusionary : public CBaseToggle 
{
public:
	void Spawn( void );
	void EXPORT SloshTouch( CBaseEntity *pOther );
	void KeyValue( KeyValueData *pkvd );
	virtual int	ObjectCaps( void ) { return CBaseEntity :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
};

LINK_ENTITY_TO_CLASS( func_illusionary, CFuncIllusionary );

void CFuncIllusionary :: KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "skin"))//skin is used for content type
	{
		pev->skin = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseToggle::KeyValue( pkvd );
}

void CFuncIllusionary :: Spawn( void )
{
	pev->angles = g_vecZero;
	pev->movetype = MOVETYPE_NONE;  
	pev->solid = SOLID_NOT;// always solid_not 
	SET_MODEL( ENT(pev), STRING(pev->model) );
	
	// I'd rather eat the network bandwidth of this than figure out how to save/restore
	// these entities after they have been moved to the client, or respawn them ala Quake
	// Perhaps we can do this in deathmatch only.
	//	MAKE_STATIC(ENT(pev));
}


// -------------------------------------------------------------------------------
//
// Monster only clip brush
// 
// This brush will be solid for any entity who has the FL_MONSTERCLIP flag set
// in pev->flags
//
// otherwise it will be invisible and not solid.  This can be used to keep 
// specific monsters out of certain areas
//
// -------------------------------------------------------------------------------
class CFuncMonsterClip : public CFuncWall
{
public:
	void	Spawn( void );
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value ) {}		// Clear out func_wall's use function
};

LINK_ENTITY_TO_CLASS( func_monsterclip, CFuncMonsterClip );

void CFuncMonsterClip::Spawn( void )
{
	CFuncWall::Spawn();
	if ( CVAR_GET_FLOAT("showtriggers") == 0 )
		pev->effects = EF_NODRAW;
	pev->flags |= FL_MONSTERCLIP;
}


// =================== FUNC_ROTATING ==============================================
class CFuncRotating : public CBaseEntity
{
public:
	// basic functions
	void Spawn( void  );
	void Precache( void  );
	void EXPORT SpinUp ( void );
	void EXPORT SpinDown ( void );
	void KeyValue( KeyValueData* pkvd);
	void EXPORT HurtTouch ( CBaseEntity *pOther );
	void EXPORT RotatingUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT Rotate( void );
	void RampPitchVol (int fUp );
	void Blocked( CBaseEntity *pOther );
	virtual int	ObjectCaps( void ) { return CBaseEntity :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	
	static	TYPEDESCRIPTION m_SaveData[];

	float m_flFanFriction;
	float m_flAttenuation;
	float m_flVolume;
	float m_pitch;
	int	  m_sounds;
};

TYPEDESCRIPTION	CFuncRotating::m_SaveData[] = 
{
	DEFINE_FIELD( CFuncRotating, m_flFanFriction, FIELD_FLOAT ),
	DEFINE_FIELD( CFuncRotating, m_flAttenuation, FIELD_FLOAT ),
	DEFINE_FIELD( CFuncRotating, m_flVolume, FIELD_FLOAT ),
	DEFINE_FIELD( CFuncRotating, m_pitch, FIELD_FLOAT ),
	DEFINE_FIELD( CFuncRotating, m_sounds, FIELD_INTEGER )
};

IMPLEMENT_SAVERESTORE( CFuncRotating, CBaseEntity );


LINK_ENTITY_TO_CLASS( func_rotating, CFuncRotating );

void CFuncRotating :: KeyValue( KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "fanfriction"))
	{
		m_flFanFriction = atof(pkvd->szValue)/100;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "Volume"))
	{
		m_flVolume = atof(pkvd->szValue)/10.0;

		if (m_flVolume > 1.0)
			m_flVolume = 1.0;
		if (m_flVolume < 0.0)
			m_flVolume = 0.0;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "spawnorigin"))
	{
		Vector tmp;
		UTIL_StringToVector( (float *)tmp, pkvd->szValue );
		if ( tmp != g_vecZero )
			pev->origin = tmp;
	}
	else if (FStrEq(pkvd->szKeyName, "sounds"))
	{
		m_sounds = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else 
		CBaseEntity::KeyValue( pkvd );
}

/*QUAKED func_rotating (0 .5 .8) ? START_ON REVERSE X_AXIS Y_AXIS
You need to have an origin brush as part of this entity.  The  
center of that brush will be
the point around which it is rotated. It will rotate around the Z  
axis by default.  You can
check either the X_AXIS or Y_AXIS box to change that.

"speed" determines how fast it moves; default value is 100.
"dmg"	damage to inflict when blocked (2 default)

REVERSE will cause the it to rotate in the opposite direction.
*/


void CFuncRotating :: Spawn( )
{
	// set final pitch.  Must not be PITCH_NORM, since we
	// plan on pitch shifting later.

	m_pitch = PITCH_NORM - 1;

	// maintain compatibility with previous maps
	if (m_flVolume == 0.0)
		m_flVolume = 1.0;

	// if the designer didn't set a sound attenuation, default to one.
	m_flAttenuation = ATTN_NORM;
	
	if ( FBitSet ( pev->spawnflags, SF_BRUSH_ROTATE_SMALLRADIUS) )
	{
		m_flAttenuation = ATTN_IDLE;
	}
	else if ( FBitSet ( pev->spawnflags, SF_BRUSH_ROTATE_MEDIUMRADIUS) )
	{
		m_flAttenuation = ATTN_STATIC;
	}
	else if ( FBitSet ( pev->spawnflags, SF_BRUSH_ROTATE_LARGERADIUS) )
	{
		m_flAttenuation = ATTN_NORM;
	}

	// prevent divide by zero if level designer forgets friction!
	if ( m_flFanFriction == 0 )
	{
		m_flFanFriction = 1;
	}
	
	if ( FBitSet(pev->spawnflags, SF_BRUSH_ROTATE_Z_AXIS) )
		pev->movedir = Vector(0,0,1);
	else if ( FBitSet(pev->spawnflags, SF_BRUSH_ROTATE_X_AXIS) )
		pev->movedir = Vector(1,0,0);
	else 
		pev->movedir = Vector(0,1,0);	// y-axis

	// check for reverse rotation
	if ( FBitSet(pev->spawnflags, SF_BRUSH_ROTATE_BACKWARDS) )
		pev->movedir = pev->movedir * -1;

	// some rotating objects like fake volumetric lights will not be solid.
	if ( FBitSet(pev->spawnflags, SF_ROTATING_NOT_SOLID) )
	{
		pev->solid = SOLID_NOT;
		pev->skin = CONTENTS_EMPTY;
		pev->movetype	= MOVETYPE_PUSH;
	}
	else
	{
		pev->solid		= SOLID_BSP;
		pev->movetype	= MOVETYPE_PUSH;
	}

	UTIL_SetOrigin(pev, pev->origin);
	SET_MODEL( ENT(pev), STRING(pev->model) );

	SetUse( &CFuncRotating::RotatingUse );
	// did level designer forget to assign speed?
	if (pev->speed <= 0)
		pev->speed = 0;

	// Removed this per level designers request.  -- JAY
	//	if (pev->dmg == 0)
	//		pev->dmg = 2;

	// instant-use brush?
	if ( FBitSet( pev->spawnflags, SF_BRUSH_ROTATE_INSTANT) )
	{		
		SetThink( &CFuncRotating::SUB_CallUseToggle );
		pev->nextthink = pev->ltime + 1.5;	// leave a magic delay for client to start up
	}	
	// can this brush inflict pain?
	if ( FBitSet (pev->spawnflags, SF_BRUSH_HURT) )
	{
		SetTouch( &CFuncRotating::HurtTouch );
	}
	
	Precache( );
}


void CFuncRotating :: Precache( void )
{
	char* szSoundFile = (char*) STRING(pev->message);

	// set up fan sounds

	if (!FStringNull( pev->message ) && strlen( szSoundFile ) > 0)
	{
		// if a path is set for a wave, use it

		PRECACHE_SOUND(szSoundFile);
			
		pev->noiseRunning = ALLOC_STRING(szSoundFile);
	} else
	{
		// otherwise use preset sound
		switch (m_sounds)
		{
		case 1:
			PRECACHE_SOUND ("fans/fan1.wav");
			pev->noiseRunning = ALLOC_STRING("fans/fan1.wav");
			break;
		case 2:
			PRECACHE_SOUND ("fans/fan2.wav");
			pev->noiseRunning = ALLOC_STRING("fans/fan2.wav");
			break;
		case 3:
			PRECACHE_SOUND ("fans/fan3.wav");
			pev->noiseRunning = ALLOC_STRING("fans/fan3.wav");
			break;
		case 4:
			PRECACHE_SOUND ("fans/fan4.wav");
			pev->noiseRunning = ALLOC_STRING("fans/fan4.wav");
			break;
		case 5:
			PRECACHE_SOUND ("fans/fan5.wav");
			pev->noiseRunning = ALLOC_STRING("fans/fan5.wav");
			break;

		case 0:
		default:
			if (!FStringNull( pev->message ) && strlen( szSoundFile ) > 0)
			{
				PRECACHE_SOUND(szSoundFile);
				
				pev->noiseRunning = ALLOC_STRING(szSoundFile);
				break;
			} else
			{
				pev->noiseRunning = ALLOC_STRING("common/null.wav");
				break;
			}
		}
	}
	
	if (pev->avelocity != g_vecZero )
	{
		// if fan was spinning, and we went through transition or save/restore,
		// make sure we restart the sound.  1.5 sec delay is magic number. KDB

		SetThink ( &CFuncRotating::SpinUp );
		pev->nextthink = pev->ltime + 1.5;
	}
}



//
// Touch - will hurt others based on how fast the brush is spinning
//
void CFuncRotating :: HurtTouch ( CBaseEntity *pOther )
{
	entvars_t	*pevOther = pOther->pev;

	// we can't hurt this thing, so we're not concerned with it
	if ( !pevOther->takedamage )
		return;

	// calculate damage based on rotation speed
	pev->dmg = pev->avelocity.Length() / 10;

	pOther->TakeDamage( pev, pev, pev->dmg, DMG_CRUSH);
	
	pevOther->velocity = (pevOther->origin - VecBModelOrigin(pev) ).Normalize() * pev->dmg;
}

//
// RampPitchVol - ramp pitch and volume up to final values, based on difference
// between how fast we're going vs how fast we plan to go
//
#define FANPITCHMIN		30
#define FANPITCHMAX		100

void CFuncRotating :: RampPitchVol (int fUp)
{

	Vector vecAVel = pev->avelocity;
	vec_t vecCur;
	vec_t vecFinal;
	float fpct;
	float fvol;
	float fpitch;
	int pitch;
	
	// get current angular velocity

	vecCur = abs(vecAVel.x != 0 ? vecAVel.x : (vecAVel.y != 0 ? vecAVel.y : vecAVel.z));
	
	// get target angular velocity

	vecFinal = (pev->movedir.x != 0 ? pev->movedir.x : (pev->movedir.y != 0 ? pev->movedir.y : pev->movedir.z));
	vecFinal *= pev->speed;
	vecFinal = abs(vecFinal);

	// calc volume and pitch as % of final vol and pitch

	fpct = vecCur / vecFinal;
//	if (fUp)
//		fvol = m_flVolume * (0.5 + fpct/2.0); // spinup volume ramps up from 50% max vol
//	else
		fvol = m_flVolume * fpct;			  // slowdown volume ramps down to 0

	fpitch = FANPITCHMIN + (FANPITCHMAX - FANPITCHMIN) * fpct;	
	
	pitch = (int) fpitch;
	if (pitch == PITCH_NORM)
		pitch = PITCH_NORM-1;

	// change the fan's vol and pitch

	EMIT_SOUND_DYN(ENT(pev), CHAN_STATIC, (char *)STRING(pev->noiseRunning), 
		fvol, m_flAttenuation, SND_CHANGE_PITCH | SND_CHANGE_VOL, pitch);

}

//
// SpinUp - accelerates a non-moving func_rotating up to it's speed
//
void CFuncRotating :: SpinUp( void )
{
	Vector	vecAVel;//rotational velocity

	pev->nextthink = pev->ltime + 0.1;
	pev->avelocity = pev->avelocity + ( pev->movedir * ( pev->speed * m_flFanFriction ) );

	vecAVel = pev->avelocity;// cache entity's rotational velocity

	// if we've met or exceeded target speed, set target speed and stop thinking
	if (	abs(vecAVel.x) >= abs(pev->movedir.x * pev->speed)	&&
			abs(vecAVel.y) >= abs(pev->movedir.y * pev->speed)	&&
			abs(vecAVel.z) >= abs(pev->movedir.z * pev->speed) )
	{
		pev->avelocity = pev->movedir * pev->speed;// set speed in case we overshot
		EMIT_SOUND_DYN(ENT(pev), CHAN_STATIC, (char *)STRING(pev->noiseRunning), 
			m_flVolume, m_flAttenuation, SND_CHANGE_PITCH | SND_CHANGE_VOL, FANPITCHMAX);
		
		SetThink( &CFuncRotating::Rotate );
		Rotate();
	} 
	else
	{
		RampPitchVol(TRUE);
	}
}

//
// SpinDown - decelerates a moving func_rotating to a standstill.
//
void CFuncRotating :: SpinDown( void )
{
	Vector	vecAVel;//rotational velocity
	vec_t vecdir;

	pev->nextthink = pev->ltime + 0.1;

	pev->avelocity = pev->avelocity - ( pev->movedir * ( pev->speed * m_flFanFriction ) );//spin down slower than spinup

	vecAVel = pev->avelocity;// cache entity's rotational velocity

	if (pev->movedir.x != 0)
		vecdir = pev->movedir.x;
	else if (pev->movedir.y != 0)
		vecdir = pev->movedir.y;
	else
		vecdir = pev->movedir.z;

	// if we've met or exceeded target speed, set target speed and stop thinking
	// (note: must check for movedir > 0 or < 0)
	if (((vecdir > 0) && (vecAVel.x <= 0 && vecAVel.y <= 0 && vecAVel.z <= 0)) || 
		((vecdir < 0) && (vecAVel.x >= 0 && vecAVel.y >= 0 && vecAVel.z >= 0)))
	{
		pev->avelocity = g_vecZero;// set speed in case we overshot
		
		// stop sound, we're done
		EMIT_SOUND_DYN(ENT(pev), CHAN_STATIC, (char *)STRING(pev->noiseRunning /* Stop */), 
				0, 0, SND_STOP, m_pitch);

		SetThink( &CFuncRotating::Rotate );
		Rotate();
	} 
	else
	{
		RampPitchVol(FALSE);
	}
}

void CFuncRotating :: Rotate( void )
{
	pev->nextthink = pev->ltime + 10;
}

//=========================================================
// Rotating Use - when a rotating brush is triggered
//=========================================================
void CFuncRotating :: RotatingUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	// is this a brush that should accelerate and decelerate when turned on/off (fan)?
	if ( FBitSet ( pev->spawnflags, SF_BRUSH_ACCDCC ) )
	{
		// fan is spinning, so stop it.
		if ( pev->avelocity != g_vecZero )
		{
			SetThink ( &CFuncRotating::SpinDown );
			//EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, (char *)STRING(pev->noiseStop), 
			//	m_flVolume, m_flAttenuation, 0, m_pitch);

			pev->nextthink = pev->ltime + 0.1;
		}
		else// fan is not moving, so start it
		{
			SetThink ( &CFuncRotating::SpinUp );
			EMIT_SOUND_DYN(ENT(pev), CHAN_STATIC, (char *)STRING(pev->noiseRunning), 
				0.01, m_flAttenuation, 0, FANPITCHMIN);

			pev->nextthink = pev->ltime + 0.1;
		}
	}
	else if ( !FBitSet ( pev->spawnflags, SF_BRUSH_ACCDCC ) )//this is a normal start/stop brush.
	{
		if ( pev->avelocity != g_vecZero )
		{
			// play stopping sound here
			SetThink ( &CFuncRotating::SpinDown );

			// EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, (char *)STRING(pev->noiseStop), 
			//	m_flVolume, m_flAttenuation, 0, m_pitch);
			
			pev->nextthink = pev->ltime + 0.1;
			// pev->avelocity = g_vecZero;
		}
		else
		{
			EMIT_SOUND_DYN(ENT(pev), CHAN_STATIC, (char *)STRING(pev->noiseRunning), 
				m_flVolume, m_flAttenuation, 0, FANPITCHMAX);
			pev->avelocity = pev->movedir * pev->speed;

			SetThink( &CFuncRotating::Rotate );
			Rotate();
		}
	}
}


//
// RotatingBlocked - An entity has blocked the brush
//
void CFuncRotating :: Blocked( CBaseEntity *pOther )

{
	pOther->TakeDamage( pev, pev, pev->dmg, DMG_CRUSH);
}






//#endif


class CPendulum : public CBaseEntity
{
public:
	void	Spawn ( void );
	void	KeyValue( KeyValueData *pkvd );
	void	EXPORT Swing( void );
	void	EXPORT PendulumUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void	EXPORT Stop( void );
	void	Touch( CBaseEntity *pOther );
	void	EXPORT RopeTouch ( CBaseEntity *pOther );// this touch func makes the pendulum a rope
	virtual int	ObjectCaps( void ) { return CBaseEntity :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	void	Blocked( CBaseEntity *pOther );

	static	TYPEDESCRIPTION m_SaveData[];
	
	float	m_accel;			// Acceleration
	float	m_distance;			// 
	float	m_time;
	float	m_damp;
	float	m_maxSpeed;
	float	m_dampSpeed;
	vec3_t	m_center;
	vec3_t	m_start;
};

LINK_ENTITY_TO_CLASS( func_pendulum, CPendulum );

TYPEDESCRIPTION	CPendulum::m_SaveData[] = 
{
	DEFINE_FIELD( CPendulum, m_accel, FIELD_FLOAT ),
	DEFINE_FIELD( CPendulum, m_distance, FIELD_FLOAT ),
	DEFINE_FIELD( CPendulum, m_time, FIELD_TIME ),
	DEFINE_FIELD( CPendulum, m_damp, FIELD_FLOAT ),
	DEFINE_FIELD( CPendulum, m_maxSpeed, FIELD_FLOAT ),
	DEFINE_FIELD( CPendulum, m_dampSpeed, FIELD_FLOAT ),
	DEFINE_FIELD( CPendulum, m_center, FIELD_VECTOR ),
	DEFINE_FIELD( CPendulum, m_start, FIELD_VECTOR ),
};

IMPLEMENT_SAVERESTORE( CPendulum, CBaseEntity );



void CPendulum :: KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "distance"))
	{
		m_distance = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "damp"))
	{
		m_damp = atof(pkvd->szValue) * 0.001;
		pkvd->fHandled = TRUE;
	}
	else 
		CBaseEntity::KeyValue( pkvd );
}


void CPendulum :: Spawn( void )
{
	// set the axis of rotation
	CBaseToggle :: AxisDir( pev );

	if ( FBitSet (pev->spawnflags, SF_DOOR_PASSABLE) )
		pev->solid		= SOLID_NOT;
	else
		pev->solid		= SOLID_BSP;
	pev->movetype	= MOVETYPE_PUSH;
	UTIL_SetOrigin(pev, pev->origin);
	SET_MODEL(ENT(pev), STRING(pev->model) );

	if ( m_distance == 0 )
		return;

	if (pev->speed == 0)
		pev->speed = 100;

	m_accel = (pev->speed * pev->speed) / (2 * fabs(m_distance));	// Calculate constant acceleration from speed and distance
	m_maxSpeed = pev->speed;
	m_start = pev->angles;
	m_center = pev->angles + (m_distance * 0.5) * pev->movedir;

	if ( FBitSet( pev->spawnflags, SF_BRUSH_ROTATE_INSTANT) )
	{		
		SetThink( &CPendulum::SUB_CallUseToggle );
		pev->nextthink = gpGlobals->time + 0.1;
	}
	pev->speed = 0;
	SetUse( &CPendulum::PendulumUse );

	if ( FBitSet( pev->spawnflags, SF_PENDULUM_SWING ) )
	{
		SetTouch ( &CPendulum::RopeTouch );
	}
}


void CPendulum :: PendulumUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( pev->speed )		// Pendulum is moving, stop it and auto-return if necessary
	{
		if ( FBitSet( pev->spawnflags, SF_PENDULUM_AUTO_RETURN ) )
		{		
			float	delta;

			delta = CBaseToggle :: AxisDelta( pev->spawnflags, pev->angles, m_start );

			pev->avelocity = m_maxSpeed * pev->movedir;
			pev->nextthink = pev->ltime + (delta / m_maxSpeed);
			SetThink( &CPendulum::Stop );
		}
		else
		{
			pev->speed = 0;		// Dead stop
			SetThink( NULL );
			pev->avelocity = g_vecZero;
		}
	}
	else
	{
		pev->nextthink = pev->ltime + 0.1;		// Start the pendulum moving
		m_time = gpGlobals->time;		// Save time to calculate dt
		SetThink( &CPendulum::Swing );
		m_dampSpeed = m_maxSpeed;
	}
}


void CPendulum :: Stop( void )
{
	pev->angles = m_start;
	pev->speed = 0;
	SetThink( NULL );
	pev->avelocity = g_vecZero;
}


void CPendulum::Blocked( CBaseEntity *pOther )
{
	m_time = gpGlobals->time;
}


void CPendulum :: Swing( void )
{
	float delta, dt;
	
	delta = CBaseToggle :: AxisDelta( pev->spawnflags, pev->angles, m_center );
	dt = gpGlobals->time - m_time;	// How much time has passed?
	m_time = gpGlobals->time;		// Remember the last time called

	if ( delta > 0 && m_accel > 0 )
		pev->speed -= m_accel * dt;	// Integrate velocity
	else 
		pev->speed += m_accel * dt;

	if ( pev->speed > m_maxSpeed )
		pev->speed = m_maxSpeed;
	else if ( pev->speed < -m_maxSpeed )
		pev->speed = -m_maxSpeed;
	// scale the destdelta vector by the time spent traveling to get velocity
	pev->avelocity = pev->speed * pev->movedir;

	// Call this again
	pev->nextthink = pev->ltime + 0.1;

	if ( m_damp )
	{
		m_dampSpeed -= m_damp * m_dampSpeed * dt;
		if ( m_dampSpeed < 30.0 )
		{
			pev->angles = m_center;
			pev->speed = 0;
			SetThink( NULL );
			pev->avelocity = g_vecZero;
		}
		else if ( pev->speed > m_dampSpeed )
			pev->speed = m_dampSpeed;
		else if ( pev->speed < -m_dampSpeed )
			pev->speed = -m_dampSpeed;

	}
}


void CPendulum :: Touch ( CBaseEntity *pOther )
{
	entvars_t	*pevOther = pOther->pev;

	if ( pev->dmg <= 0 )
		return;

	// we can't hurt this thing, so we're not concerned with it
	if ( !pevOther->takedamage )
		return;

	// calculate damage based on rotation speed
	float damage = pev->dmg * pev->speed * 0.01;

	if ( damage < 0 )
		damage = -damage;

	pOther->TakeDamage( pev, pev, damage, DMG_CRUSH );
	
	pevOther->velocity = (pevOther->origin - VecBModelOrigin(pev) ).Normalize() * damage;
}

void CPendulum :: RopeTouch ( CBaseEntity *pOther )
{
	entvars_t	*pevOther = pOther->pev;

	if ( !pOther->IsPlayer() )
	{// not a player!
		ALERT ( at_console, "Not a client\n" );
		return;
	}

	if ( ENT(pevOther) == pev->enemy )
	{// this player already on the rope.
		return;
	}

	pev->enemy = pOther->edict();
	pevOther->velocity = g_vecZero;
	pevOther->movetype = MOVETYPE_NONE;
}


