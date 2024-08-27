#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "trains.h"
#include "saverestore.h"
#include "weapons.h"
#include "minmax.h"

static float Fix2( float angle )
{
	while ( angle < 0 )
		angle += 360;
	while ( angle > 360 )
		angle -= 360;

	return angle;
}


static void FixupAngles2( Vector &v )
{
	v.x = Fix2( v.x );
	v.y = Fix2( v.y );
	v.z = Fix2( v.z );
}

// ---------------------------------------------------------------------
//
// Counter-Strike drivable vehicles.
//
// ---------------------------------------------------------------------

#define VEHICLE_STARTPITCH	60
#define VEHICLE_MAXPITCH		200
#define VEHICLE_MAXSPEED		1500	// approx max speed for sound pitch calculation


TYPEDESCRIPTION	CFuncVehicle::m_SaveData[] = 
{
	DEFINE_FIELD( CFuncVehicle, m_ppath, FIELD_CLASSPTR ),
	DEFINE_FIELD( CFuncVehicle, m_length, FIELD_FLOAT ),
	DEFINE_FIELD( CFuncVehicle, m_height, FIELD_FLOAT ),
	DEFINE_FIELD( CFuncVehicle, m_speed, FIELD_FLOAT ),
	DEFINE_FIELD( CFuncVehicle, m_dir, FIELD_FLOAT ),
	DEFINE_FIELD( CFuncVehicle, m_startSpeed, FIELD_FLOAT ),
	DEFINE_FIELD( CFuncVehicle, m_controlMins, FIELD_VECTOR ),
	DEFINE_FIELD( CFuncVehicle, m_controlMaxs, FIELD_VECTOR ),
	DEFINE_FIELD( CFuncVehicle, m_sounds, FIELD_INTEGER ),
	DEFINE_FIELD( CFuncVehicle, m_flVolume, FIELD_FLOAT ),
	DEFINE_FIELD( CFuncVehicle, m_flBank, FIELD_FLOAT ),
	DEFINE_FIELD( CFuncVehicle, m_oldSpeed, FIELD_FLOAT ),
};

IMPLEMENT_SAVERESTORE( CFuncVehicle, CBaseEntity );
LINK_ENTITY_TO_CLASS( func_vehicle, CFuncVehicle );

void CFuncVehicle :: KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "length"))
	{
		m_length = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	if (FStrEq(pkvd->szKeyName, "width"))
	{
		m_width = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "height"))
	{
		m_height = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "startspeed"))
	{
		m_startSpeed = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "sounds"))
	{
		m_sounds = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "volume"))
	{
		m_flVolume = (float) (atoi(pkvd->szValue));
		m_flVolume *= 0.1;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "bank"))
	{
		m_flBank = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "acceleration"))
	{
		m_acceleration = atoi(pkvd->szValue);
		if (m_acceleration < 1)
			m_acceleration = 1;
		else if (m_acceleration > 10)
			m_acceleration = 10;
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue( pkvd );
}


void CFuncVehicle :: NextThink( float thinkTime, BOOL alwaysThink )
{
	if ( alwaysThink )
		pev->flags |= FL_ALWAYSTHINK;
	else
		pev->flags &= ~FL_ALWAYSTHINK;

	pev->nextthink = thinkTime;
}


void CFuncVehicle :: Blocked( CBaseEntity *pOther )
{
	entvars_t	*pevOther = pOther->pev;

	// Blocker is on-ground on the train
	if ( FBitSet( pevOther->flags, FL_ONGROUND ) && VARS(pevOther->groundentity) == pev )
	{
		pevOther->velocity = pev->velocity;
		return;

		float deltaSpeed = fabs(pev->speed);
		if ( deltaSpeed > 50 )
			deltaSpeed = 50;
		if ( !pevOther->velocity.z )
			pevOther->velocity.z += deltaSpeed;
		return;
	}
	else
	{
		pevOther->velocity = (pevOther->origin - pev->origin ).Normalize() * pev->dmg;
		pevOther->velocity.z += 300;
		
		//slow down vehicle
		pev->velocity = pev->velocity * 0.85;
	}

	ALERT( at_console, "TRAIN(%s): Blocked by %s (dmg:%.2f)\n", STRING(pev->targetname), STRING(pOther->pev->classname), pev->dmg );
	// we can't hurt this thing, so we're not concerned with it

	UTIL_MakeVectors( pev->angles );
	Vector vFrontLeft, vFrontRight, vBackLeft, vBackRight;
	Vector forward = (gpGlobals->v_forward * -1) * (m_length/2);  //for some stupid reason I have to invert this...
	Vector right = (gpGlobals->v_right * -1) * (m_width/2);

	vFrontLeft = pev->origin + forward - right;
	vFrontRight = pev->origin + forward + right;
	vBackLeft = pev->origin - forward - right;
	vBackRight = pev->origin - forward + right;

	Vector vOrigin = pOther->pev->origin;

	float minx, miny, minz;
	float maxx, maxy, maxz;

	minx = min(vFrontLeft.x, vBackRight.x);
	maxx = max(vFrontLeft.x, vBackRight.x);
	miny = min(vFrontLeft.y, vBackRight.y);
	maxy = max(vFrontLeft.y, vBackRight.y);
	minz = pev->origin.z;
	maxz = pev->origin.z + abs(pev->mins.z - pev->maxs.z) * 2;

	// Check if the target is out of the bounds of the vehicle...
	if (	((vOrigin.x < minx) || (vOrigin.x > maxx))
		||	((vOrigin.y < miny) || (vOrigin.y > maxy))
		||	((vOrigin.z < minz) || (vOrigin.z > maxz))	)
		pOther->TakeDamage(pev, pev, 150, DMG_CRUSH);
}


void CFuncVehicle :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( useType != USE_SET )
	{
		if ( !ShouldToggle( useType, (pev->speed != 0) ) )
			return;

		if ( pev->speed == 0 )
		{
			pev->speed = m_speed * m_dir;
			
			Next();
		}
		else
		{
			pev->speed = 0;
			pev->velocity = g_vecZero;
			pev->avelocity = g_vecZero;
			StopSound();
			SetThink( NULL );
		}
	}
	else
	{
		if (value < 10)  // It's either forward or backward
		{
			if (	(value < 0) && (pev->speed > 145)	)
			{
				StopSound();
			}

			float delta = value;
			float flSpeedRatio;

			if (delta > 0)  // accelerating
			{
				flSpeedRatio = (pev->speed / m_speed);

				if (pev->speed < 0)
					delta = flSpeedRatio + (0.015/3) + (m_acceleration * 0.0005);
				else if (pev->speed < 10)
					delta = flSpeedRatio + (0.015/7) + (m_acceleration * 0.0006);
				else if (pev->speed < 20)
					delta = flSpeedRatio + (0.02/6) + (m_acceleration * 0.0007);
				else if (pev->speed < 30)
					delta = flSpeedRatio + (0.025/6) + (m_acceleration * 0.0007);
				else if (pev->speed < 45)
					delta = flSpeedRatio + (0.02/5) + (m_acceleration * 0.0007);
				else if (pev->speed < 60)
					delta = flSpeedRatio + (0.019/5) + (m_acceleration * 0.0008);
				else if (pev->speed < 80)
					delta = flSpeedRatio + (0.018/4) + (m_acceleration * 0.0008);
				else if (pev->speed < 100)
					delta = flSpeedRatio + (0.017/4) + (m_acceleration * 0.0009);
				else if (pev->speed < 150)
					delta = flSpeedRatio + (0.016/6) + (m_acceleration * 0.0008);
				else if (pev->speed < 225)
					delta = flSpeedRatio + (0.016/7) + (m_acceleration * 0.0007);
				else if (pev->speed < 300)
					delta = flSpeedRatio + (0.015/8) + (m_acceleration * 0.0006);
				else if (pev->speed < 400)
					delta = flSpeedRatio + (0.013/9) + (m_acceleration * 0.0005);
				else if (pev->speed < 550)
					delta = flSpeedRatio + (0.012/10) + (m_acceleration * 0.0005);
				else if (pev->speed < 800)
					delta = flSpeedRatio + (0.011/12) + (m_acceleration * 0.0005);
			}
			else if (delta < 0) // braking
			{
				flSpeedRatio = pev->speed / m_speed;

				if (flSpeedRatio > 0)
					delta = flSpeedRatio - 0.05 / 4;
				else if (	(flSpeedRatio <= 0) && (flSpeedRatio > -0.05)	)   // Just started accelerating
					delta = flSpeedRatio - 0.03 / 4;
				else if (	(flSpeedRatio <= 0.05) && (flSpeedRatio > -0.1)	)
					delta = flSpeedRatio - 0.04 / 4;
				else if (	(flSpeedRatio <= 0.15) && (flSpeedRatio > -0.15)	)
					delta = flSpeedRatio - 0.05 / 4;
				else if (	(flSpeedRatio <= 0.15) && (flSpeedRatio > -0.22)	)
					delta = flSpeedRatio - 0.055 / 4;
				else if (	(flSpeedRatio <= 0.22) && (flSpeedRatio > -0.3)	)
					delta = flSpeedRatio - 0.07 / 4;
				else if (flSpeedRatio <= 0.3)
					delta = flSpeedRatio - 0.05 / 4;
			}
			
			if (delta > 1)
				delta = 1;
			else if (delta < -0.35)
				delta = -0.35;

			pev->speed = m_speed * delta;

			Next();	

			m_flAcceleratorDecay = gpGlobals->time + 0.25;
		}
		else
		{
			if (m_flCanTurnNow < gpGlobals->time)
			{
				if (value == 20)  // turn left!
				{
					m_iTurnAngle += 1;
					m_flSteeringWheelDecay = gpGlobals->time + 0.075;
					if (m_iTurnAngle > 8)
						m_iTurnAngle = 8;
				}
				else if (value == 30)  // turn right
				{
					m_iTurnAngle -= 1;
					m_flSteeringWheelDecay = gpGlobals->time + 0.075;
					if (m_iTurnAngle < -8)
						m_iTurnAngle = -8;
				}

				m_flCanTurnNow = gpGlobals->time + 0.05;
			}
		}
	}
}


void CFuncVehicle :: StopSound( void )
{
	// if sound playing, stop it
	if (m_soundPlaying && pev->noise)
	{
		unsigned short us_encode;
		unsigned short us_sound  = ( ( unsigned short )( m_sounds ) & 0x0007 ) << 12;

		us_encode = us_sound;

		PLAYBACK_EVENT_FULL( FEV_RELIABLE | FEV_UPDATE, edict(), m_usAdjustPitch, 0.0, 
			(float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, us_encode, 0, 1, 0 );
	}

	m_soundPlaying = 0;
}

// update pitch based on speed, start sound if not playing
// NOTE: when train goes through transition, m_soundPlaying should go to 0, 
// which will cause the looped sound to restart.

void CFuncVehicle :: UpdateSound( void )
{
	float flpitch;
	
	if (!pev->noise)
		return;

	flpitch = VEHICLE_STARTPITCH + (abs(pev->speed) * (VEHICLE_MAXPITCH - VEHICLE_STARTPITCH) / VEHICLE_MAXSPEED);

	if (flpitch > VEHICLE_MAXPITCH)
		flpitch = VEHICLE_MAXPITCH;

	if (!m_soundPlaying)
	{
		// play startup sound for train
		if (m_sounds < 5)
			EMIT_SOUND_DYN(ENT(pev), CHAN_ITEM, "plats/vehicle_brake1.wav", m_flVolume, ATTN_NORM, 0, 100);
		EMIT_SOUND_DYN(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noise), m_flVolume, ATTN_NORM, 0, (int) flpitch);
		m_soundPlaying = 1;
	} 
	else
	{
/*
		// update pitch
		EMIT_SOUND_DYN(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noise), m_flVolume, ATTN_NORM, SND_CHANGE_PITCH, (int) flpitch);
*/
		// volume 0.0 - 1.0 - 6 bits
		// m_sounds 3 bits
		// flpitch = 6 bits
		// 15 bits total

		unsigned short us_encode;
		unsigned short us_sound  = ( ( unsigned short )( m_sounds ) & 0x0007 ) << 12;
		unsigned short us_pitch  = ( ( unsigned short )( flpitch / 10.0 ) & 0x003f ) << 6;
		unsigned short us_volume = ( ( unsigned short )( m_flVolume * 40.0 ) & 0x003f );

		us_encode = us_sound | us_pitch | us_volume;

		PLAYBACK_EVENT_FULL( FEV_UPDATE, edict(), m_usAdjustPitch, 0.0, 
			(float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, us_encode, 0, 0, 0 );

	}
}

void CFuncVehicle :: CheckTurning (void)
{
	float maxspeed;
	TraceResult tr;

	bool bTurnIntoWall = false;
	// Check to see if he's trying to turn into a wall
	if (m_iTurnAngle < 0)  // He's trying to turn right
	{
		if (pev->speed > 0)
		{
			UTIL_TraceLine ( m_vFrontRight, m_vFrontRight - (gpGlobals->v_right * 16), ignore_monsters, dont_ignore_glass, ENT(pev), &tr);
			if (tr.flFraction != 1.0)
			{
				m_iTurnAngle = 1;
			}
		}
		else if (pev->speed < 0)
		{
			UTIL_TraceLine ( m_vBackLeft, m_vBackLeft + (gpGlobals->v_right * 16), ignore_monsters, dont_ignore_glass, ENT(pev), &tr);
			if (tr.flFraction != 1.0)
			{
				m_iTurnAngle = 1;
			}
		}
	}
	else if (m_iTurnAngle > 0)
	{
		if (pev->speed > 0)
		{
			UTIL_TraceLine ( m_vFrontLeft, m_vFrontLeft + (gpGlobals->v_right * 16), ignore_monsters, dont_ignore_glass, ENT(pev), &tr);
			if (tr.flFraction != 1.0)
			{
				m_iTurnAngle = -1;
			}
		}
		else if (pev->speed < 0)
		{
			UTIL_TraceLine ( m_vBackRight, m_vBackRight - (gpGlobals->v_right * 16), ignore_monsters, dont_ignore_glass, ENT(pev), &tr);
			if (tr.flFraction != 1.0)
			{
				m_iTurnAngle = -1;
			}
		}
	}
	
	if (pev->speed > 0)
	{
		if ( abs(m_iTurnAngle) > 4	) // Doing a hard turn
		{
			if (m_flTurnStartTime == -1)  //just started turning
			{
				m_flTurnStartTime = gpGlobals->time;
				maxspeed = (float)m_speed;
			}
			else if (	(gpGlobals->time - m_flTurnStartTime) >= 0.0	)
			{
				maxspeed = (float)m_speed * 0.98;
			}
			else if (	(gpGlobals->time - m_flTurnStartTime) > 0.3	)
			{
				maxspeed = (float)m_speed * 0.95;
			}
			else if (	(gpGlobals->time - m_flTurnStartTime) > 0.6	)
			{
				maxspeed = (float)m_speed * 0.9;
			}
			else if (	(gpGlobals->time - m_flTurnStartTime) > 0.8	)
			{
				maxspeed = (float)m_speed * 0.8;
			}
			else if (	(gpGlobals->time - m_flTurnStartTime) > 1	)
			{
				maxspeed = (float)m_speed * 0.7;
			}
			else if (	(gpGlobals->time - m_flTurnStartTime) > 1.2	)
			{
				maxspeed = (float)m_speed * 0.5;
			}
		}
		else if ( abs(m_iTurnAngle) > 2 )
		{
			m_flTurnStartTime = -1;
			maxspeed = (float)m_speed * 0.9;
		}
		else
		{
			maxspeed = (float)m_speed;
			m_flTurnStartTime = -1;
		}

		if (pev->speed > maxspeed)
		{
			pev->speed -= m_speed / 10;
		}
	}
}

void CFuncVehicle :: CollisionDetection (void)
{
	TraceResult tr;
	bool bHitSomething = false;

	// Check to see if there's anything in front of us
	if (pev->speed < 0)  // We're going backwards
	{
		// Check back left portion of vehicle if we hit a wall
		UTIL_TraceLine ( m_vBackLeft, m_vBackLeft + (gpGlobals->v_forward * 16), ignore_monsters, dont_ignore_glass, ENT(pev), &tr);
		if (tr.flFraction != 1.0)
		{
			bHitSomething = true;
			if (	( DotProduct(tr.vecPlaneNormal,gpGlobals->v_forward*-1) < 0.7) && (tr.vecPlaneNormal.z < 0.1)	) // slightly vertical wall... skid along it
			{				
				m_vSurfaceNormal = tr.vecPlaneNormal;
				m_vSurfaceNormal.z = 0;
				pev->speed = pev->speed * 0.99;
			}
			else if (	(tr.vecPlaneNormal.z < 0.65) || (tr.fStartSolid)	)
				pev->speed = pev->speed * -1;
			else
				m_vSurfaceNormal = tr.vecPlaneNormal;

			CBaseEntity *pHit = CBaseEntity::Instance(tr.pHit);
			if (pHit && (pHit->Classify() == CLASS_VEHICLE))
				ALERT(at_console, "I hit another vehicle\n");
		}
		if (bHitSomething == false)
		{
			UTIL_TraceLine ( m_vBackRight, m_vBackRight + (gpGlobals->v_forward * 16), ignore_monsters, dont_ignore_glass, ENT(pev), &tr);
			if (tr.flFraction != 1.0)
			{
				bHitSomething = true;
				if (	( DotProduct(tr.vecPlaneNormal,gpGlobals->v_forward*-1) < 0.7) && (tr.vecPlaneNormal.z < 0.1)	) // slightly vertical wall... skid along it
				{
					m_vSurfaceNormal = tr.vecPlaneNormal;
					m_vSurfaceNormal.z = 0;
					pev->speed = pev->speed * 0.99;
				}
				else if (	(tr.vecPlaneNormal.z < 0.65) || (tr.fStartSolid)	)
					pev->speed = pev->speed * -1;
				else
					m_vSurfaceNormal = tr.vecPlaneNormal;
			}
		}
		if (bHitSomething == false)
		{
			UTIL_TraceLine ( m_vBack, m_vBack + (gpGlobals->v_forward * 16), ignore_monsters, dont_ignore_glass, ENT(pev), &tr);
			if (tr.flFraction != 1.0)
			{
				if (	( DotProduct(tr.vecPlaneNormal,gpGlobals->v_forward*-1) < 0.7) && (tr.vecPlaneNormal.z < 0.1)	) // slightly vertical wall... skid along it
				{
					m_vSurfaceNormal = tr.vecPlaneNormal;
					m_vSurfaceNormal.z = 0;
					pev->speed = pev->speed * 0.99;
				}
				else if (	(tr.vecPlaneNormal.z < 0.65) || (tr.fStartSolid)	)
					pev->speed = pev->speed * -1;
				else
					m_vSurfaceNormal = tr.vecPlaneNormal;
			}
		}
	}
	else if (pev->speed > 0)
	{
		// Check front left portion of vehicle
		UTIL_TraceLine ( m_vFrontLeft, m_vFrontLeft - (gpGlobals->v_forward * 16), dont_ignore_monsters, dont_ignore_glass, ENT(pev), &tr);
		if (tr.flFraction != 1.0)
		{
			bHitSomething = true;
			if (	( DotProduct(tr.vecPlaneNormal,gpGlobals->v_forward*-1) > -0.7) && (tr.vecPlaneNormal.z < 0.1)	) // slightly vertical wall... skid along it
			{				
				m_vSurfaceNormal = tr.vecPlaneNormal;
				m_vSurfaceNormal.z = 0;
				pev->speed = pev->speed * 0.99;
			}
			else if (	(tr.vecPlaneNormal.z < 0.65) || (tr.fStartSolid)	)
				pev->speed = pev->speed * -1;
			else
				m_vSurfaceNormal = tr.vecPlaneNormal;

		}
		if (bHitSomething == false)
		{
			UTIL_TraceLine ( m_vFrontRight, m_vFrontRight - (gpGlobals->v_forward * 16), ignore_monsters, dont_ignore_glass, ENT(pev), &tr);
			if (tr.flFraction != 1.0)
			{
				bHitSomething = true;
				if (	( DotProduct(tr.vecPlaneNormal,gpGlobals->v_forward*-1) > -0.7) && (tr.vecPlaneNormal.z < 0.1)	) // slightly vertical wall... skid along it
				{
					m_vSurfaceNormal = tr.vecPlaneNormal;
					m_vSurfaceNormal.z = 0;
					pev->speed = pev->speed * 0.99;
				}
				else if (	(tr.vecPlaneNormal.z < 0.65) || (tr.fStartSolid)	)
					pev->speed = pev->speed * -1;
				else
					m_vSurfaceNormal = tr.vecPlaneNormal;
			}
		}
		if (bHitSomething == false)
		{
			UTIL_TraceLine ( m_vFront, m_vFront - (gpGlobals->v_forward * 16), ignore_monsters, dont_ignore_glass, ENT(pev), &tr);
			if (tr.flFraction != 1.0)
			{
				bHitSomething = true;
				if (	( DotProduct(tr.vecPlaneNormal,gpGlobals->v_forward*-1) > -0.7) && (tr.vecPlaneNormal.z < 0.1)	) // slightly vertical wall... skid along it
				{
					m_vSurfaceNormal = tr.vecPlaneNormal;
					m_vSurfaceNormal.z = 0;
					pev->speed = pev->speed * 0.99;
				}
				else if (	(tr.vecPlaneNormal.z < 0.65) || (tr.fStartSolid)	)
					pev->speed = pev->speed * -1;
				else
					m_vSurfaceNormal = tr.vecPlaneNormal;
			}
		}
	}
}

void CFuncVehicle :: TerrainFollowing (void)
{
	TraceResult tr;

	//Get the normal of the surface we're right on top of
	UTIL_TraceLine ( pev->origin, pev->origin + Vector(0,0,-1 * (m_height + 48) ), ignore_monsters, dont_ignore_glass, ENT(pev), &tr);
	if (tr.flFraction != 1.0)  // we hit something
	{
		m_vSurfaceNormal = tr.vecPlaneNormal;
	}
	else if (tr.fInWater)
	{
		m_vSurfaceNormal = Vector(0,0,1);
	}
	else if (tr.fStartSolid)
	{
//		ALERT(at_console,"I'm underground\n!");
	}
}


void CFuncVehicle :: Next( void )
{
	Vector vGravityVector = g_vecZero;

	UTIL_MakeVectors( pev->angles );

	Vector forward = (gpGlobals->v_forward * -1) * (m_length/2);  //for some stupid reason I have to invert this...
	Vector right = (gpGlobals->v_right * -1) * (m_width/2);
	Vector up = (gpGlobals->v_up) * 16;

	m_vFrontLeft = pev->origin + forward - right + up;
	m_vFrontRight = pev->origin + forward + right + up;
	m_vFront = pev->origin + forward + up;
	m_vBackLeft = pev->origin - forward - right + up;
	m_vBackRight = pev->origin - forward + right + up;
	m_vBack = pev->origin - forward + up;

	m_vSurfaceNormal = g_vecZero;

	
	// Check to see if we're hitting something...and alter the vehicles velocity.
	CheckTurning ();


	float time = 0.1;

	if (m_flSteeringWheelDecay < gpGlobals->time)
	{
		m_flSteeringWheelDecay = gpGlobals->time + 0.1;
		if (m_iTurnAngle < 0)
			m_iTurnAngle++;
		else if (m_iTurnAngle > 0)
			m_iTurnAngle--;
	}

	if (m_flAcceleratorDecay < gpGlobals->time)
	{
		m_flAcceleratorDecay = gpGlobals->time + 0.1;
		if (pev->speed < 0)
		{
			pev->speed += 20;
			if (pev->speed > 0)
				pev->speed = 0;
		}
		else if (pev->speed > 0)
		{
			pev->speed -= 20;
			if (pev->speed < 0)
				pev->speed = 0;
		}
	
	}

	if ( !pev->speed )
	{
		m_iTurnAngle = 0;
		pev->avelocity = g_vecZero;
		pev->velocity = g_vecZero;
//		StopSound();

		SetThink( &CFuncVehicle::Next );
		NextThink( pev->ltime + time, TRUE );
		return;
	}
	else
	{
		TerrainFollowing();
		CollisionDetection();

		if (m_vSurfaceNormal != g_vecZero)
		{
			m_vVehicleDirection = CrossProduct(m_vSurfaceNormal, (gpGlobals->v_forward));
			m_vVehicleDirection = CrossProduct(m_vSurfaceNormal, m_vVehicleDirection);

			Vector vTargetAngle, vAngle;
			float vx,vy;

			vTargetAngle = UTIL_VecToAngles (m_vVehicleDirection);
			vTargetAngle.y += 180;
			vAngle	= pev->angles;

			if (m_iTurnAngle != 0)
				vTargetAngle.y += m_iTurnAngle;

			FixupAngles2(vTargetAngle);
			FixupAngles2(vAngle);

			vx = UTIL_AngleDistance( vTargetAngle.x, vAngle.x );
			vy = UTIL_AngleDistance( vTargetAngle.y, vAngle.y );

			if (vx > 10)
				vx = 10;
			else if (vx < -10)
				vx = -10;
			if (vy > 10)
				vy = 10;
			else if (vy < -10)
				vy = -10;
			pev->avelocity.y = (int)(vy * 10);
			pev->avelocity.x = (int)(vx * 10);

			// reset launch time to -1
			m_flLaunchTime = -1;
			m_flLastNormalZ = m_vSurfaceNormal.z;
		}
		else
		{
			time = 0.1;
			if (m_flLaunchTime == -1)
			{
				m_flLaunchTime = gpGlobals->time;
				vGravityVector = Vector (0,0,0);
				pev->velocity = pev->velocity * 1.5;
			}
			else
			{
				float flAirTime;

				flAirTime = gpGlobals->time - m_flLaunchTime;

				vGravityVector = Vector (0,0,  flAirTime * -35);
				if (vGravityVector.z < -400)
					vGravityVector.z = -400;
			}
			
			m_vVehicleDirection = gpGlobals->v_forward * -1;
		}

		Vector temp;
		temp = UTIL_VecToAngles (m_vVehicleDirection);

		if (m_flUpdateSound < gpGlobals->time)
		{
			UpdateSound();
			m_flUpdateSound = gpGlobals->time + 1;
		}

		if (m_vSurfaceNormal != g_vecZero)
		{
			pev->velocity = pev->speed * m_vVehicleDirection.Normalize();
		}
		else
		{
			pev->velocity = pev->velocity + vGravityVector;
		}
		SetThink( &CFuncVehicle::Next );
		NextThink( pev->ltime + time, TRUE );
		return;
	}

/************************************************/

}


void CFuncVehicle::DeadEnd( void )
{
	// Fire the dead-end target if there is one
	CPathTrack *pTrack, *pNext;

	pTrack = m_ppath;

	ALERT( at_aiconsole, "TRAIN(%s): Dead end ", STRING(pev->targetname) );
	// Find the dead end path node
	// HACKHACK -- This is bugly, but the train can actually stop moving at a different node depending on it's speed
	// so we have to traverse the list to it's end.
	if ( pTrack )
	{
		if ( m_oldSpeed < 0 )
		{
			do
			{
				pNext = pTrack->ValidPath( pTrack->GetPrevious(), TRUE );
				if ( pNext )
					pTrack = pNext;
			} while ( pNext );
		}
		else
		{
			do
			{
				pNext = pTrack->ValidPath( pTrack->GetNext(), TRUE );
				if ( pNext )
					pTrack = pNext;
			} while ( pNext );
		}
	}

	pev->velocity = g_vecZero;
	pev->avelocity = g_vecZero;
	if ( pTrack )
	{
		ALERT( at_aiconsole, "at %s\n", STRING(pTrack->pev->targetname) );
		if ( pTrack->pev->netname )
			FireTargets( STRING(pTrack->pev->netname), this, this, USE_TOGGLE, 0 );
	}
	else
		ALERT( at_aiconsole, "\n" );
}


void CFuncVehicle :: SetControls( entvars_t *pevControls )
{
	Vector offset = pevControls->origin - pev->oldorigin;

	m_controlMins = pevControls->mins + offset;
	m_controlMaxs = pevControls->maxs + offset;
}


BOOL CFuncVehicle :: OnControls( entvars_t *pevTest )
{
	Vector offset = pevTest->origin - pev->origin;

	if ( pev->spawnflags & SF_TRACKTRAIN_NOCONTROL )
		return FALSE;

	// Transform offset into local coordinates
	UTIL_MakeVectors( pev->angles );
	Vector local;
	local.x = DotProduct( offset, gpGlobals->v_forward );
	local.y = -DotProduct( offset, gpGlobals->v_right );
	local.z = DotProduct( offset, gpGlobals->v_up );

	if ( local.x >= m_controlMins.x && local.y >= m_controlMins.y && local.z >= m_controlMins.z &&
		 local.x <= m_controlMaxs.x && local.y <= m_controlMaxs.y && local.z <= m_controlMaxs.z )
		 return TRUE;

	return FALSE;
}


void CFuncVehicle :: Find( void )
{
	m_ppath = CPathTrack::Instance(FIND_ENTITY_BY_TARGETNAME( NULL, STRING(pev->target) ));
	if ( !m_ppath )
		return;

	entvars_t *pevTarget = m_ppath->pev;
	if ( !FClassnameIs( pevTarget, "path_track" ) )
	{
		ALERT( at_console, "func_track_train must be on a path of path_track\n" );
		m_ppath = NULL;
		return;
	}

	Vector nextPos = pevTarget->origin;
	nextPos.z += m_height;

	Vector look = nextPos;
	look.z -= m_height;
	m_ppath->LookAhead( &look, m_length, 0 );
	look.z += m_height;

	pev->angles = UTIL_VecToAngles( look - nextPos );
	// The train actually points west
	pev->angles.y += 180;

	if ( pev->spawnflags & SF_TRACKTRAIN_NOPITCH )
		pev->angles.x = 0;
    UTIL_SetOrigin( pev, nextPos );
	NextThink( pev->ltime + 0.1, FALSE );
	SetThink( &CFuncVehicle::Next );
	pev->speed = m_startSpeed;

	UpdateSound();
}


void CFuncVehicle :: NearestPath( void )
{
	CBaseEntity *pTrack = NULL;
	CBaseEntity *pNearest = NULL;
	float dist, closest;

	closest = 1024;

	while ((pTrack = UTIL_FindEntityInSphere( pTrack, pev->origin, 1024 )) != NULL)
	{
		// filter out non-tracks
		if ( !(pTrack->pev->flags & (FL_CLIENT|FL_MONSTER)) && FClassnameIs( pTrack->pev, "path_track" ) )
		{
			dist = (pev->origin - pTrack->pev->origin).Length();
			if ( dist < closest )
			{
				closest = dist;
				pNearest = pTrack;
			}
		}
	}

	if ( !pNearest )
	{
		ALERT( at_console, "Can't find a nearby track !!!\n" );
		SetThink(NULL);
		return;
	}

	ALERT( at_aiconsole, "TRAIN: %s, Nearest track is %s\n", STRING(pev->targetname), STRING(pNearest->pev->targetname) );
	// If I'm closer to the next path_track on this path, then it's my real path
	pTrack = ((CPathTrack *)pNearest)->GetNext();
	if ( pTrack )
	{
		if ( (pev->origin - pTrack->pev->origin).Length() < (pev->origin - pNearest->pev->origin).Length() )
			pNearest = pTrack;
	}

	m_ppath = (CPathTrack *)pNearest;

	if ( pev->speed != 0 )
	{
		NextThink( pev->ltime + 0.1, FALSE );
		SetThink( &CFuncVehicle::Next );
	}
}


void CFuncVehicle::OverrideReset( void )
{
	NextThink( pev->ltime + 0.1, FALSE );
	SetThink( &CFuncVehicle::NearestPath );
}


CFuncVehicle *CFuncVehicle::Instance( edict_t *pent )
{ 
	if ( FClassnameIs( pent, "func_vehicle" ) )
		return (CFuncVehicle *)GET_PRIVATE(pent);
	return NULL;
}

/*QUAKED func_train (0 .5 .8) ?
Trains are moving platforms that players can ride.
The targets origin specifies the min point of the train at each corner.
The train spawns at the first target it is pointing at.
If the train is the target of a button or trigger, it will not begin moving until activated.
speed	default 100
dmg		default	2
sounds
1) ratchet metal
*/

int CFuncVehicle::Classify(void)
{
	return CLASS_VEHICLE;
}

void CFuncVehicle :: Spawn( void )
{
	if ( pev->speed == 0 )
		m_speed = 165;
	else
		m_speed = pev->speed;

	if (m_sounds == 0)
		m_sounds = 3;

	ALERT (at_console, "M_speed = %f\n", m_speed);
	
	pev->speed = 0;
	pev->velocity = g_vecZero;
	pev->avelocity = g_vecZero;
	pev->impulse = m_speed;
	m_acceleration = 5;

	m_dir = 1;
	m_flTurnStartTime = -1;

	if ( FStringNull(pev->target) )
		ALERT( at_console, "Vehicle with no target" );

	if ( pev->spawnflags & SF_TRACKTRAIN_PASSABLE )
		pev->solid = SOLID_NOT;
	else
		pev->solid = SOLID_BSP;
	pev->movetype = MOVETYPE_PUSH;
	
	SET_MODEL( ENT(pev), STRING(pev->model) );

	UTIL_SetSize( pev, pev->mins, pev->maxs );
	UTIL_SetOrigin( pev, pev->origin );

	// Cache off placed origin for train controls
	pev->oldorigin = pev->origin;

	m_controlMins = pev->mins;
	m_controlMaxs = pev->maxs;
	m_controlMaxs.z += 72;
// start trains on the next frame, to make sure their targets have had
// a chance to spawn/activate
	NextThink( pev->ltime + 0.1, FALSE );
	SetThink( &CFuncVehicle::Find );
	Precache();
}

// GOOSEMAN
void CFuncVehicle :: Restart( void )
{
	ALERT (at_console, "M_speed = %f\n", m_speed);

	pev->speed = 0;
	pev->velocity = g_vecZero;
	pev->avelocity = g_vecZero;
	pev->impulse = m_speed;

	m_dir = 1;
	m_flTurnStartTime = -1;
	m_flUpdateSound = -1;
	m_pDriver = NULL;

	if ( FStringNull(pev->target) )
		ALERT( at_console, "Vehicle with no target" );

	UTIL_SetOrigin( pev, pev->oldorigin );

	STOP_SOUND(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noise));
// start trains on the next frame, to make sure their targets have had
// a chance to spawn/activate
	NextThink( pev->ltime + 0.1, FALSE );
	SetThink( &CFuncVehicle::Find );
}


void CFuncVehicle :: Precache( void )
{
	if (m_flVolume == 0.0)
		m_flVolume = 1.0;

	switch (m_sounds)
	{
	case 1: PRECACHE_SOUND("plats/vehicle1.wav"); pev->noise = MAKE_STRING("plats/vehicle1.wav");break;
	case 2: PRECACHE_SOUND("plats/vehicle2.wav"); pev->noise = MAKE_STRING("plats/vehicle2.wav");break;
	case 3: PRECACHE_SOUND("plats/vehicle3.wav"); pev->noise = MAKE_STRING("plats/vehicle3.wav");break; 
	case 4: PRECACHE_SOUND("plats/vehicle4.wav"); pev->noise = MAKE_STRING("plats/vehicle4.wav");break;
	case 5: PRECACHE_SOUND("plats/vehicle6.wav"); pev->noise = MAKE_STRING("plats/vehicle6.wav");break;
	case 6: PRECACHE_SOUND("plats/vehicle7.wav"); pev->noise = MAKE_STRING("plats/vehicle7.wav");break;
	default: PRECACHE_SOUND("plats/vehicle7.wav"); pev->noise = MAKE_STRING("plats/vehicle7.wav");break;
	}

	PRECACHE_SOUND("plats/vehicle_brake1.wav");
	PRECACHE_SOUND("plats/vehicle_start1.wav");

	m_usAdjustPitch = PRECACHE_EVENT( 1, "events/vehicle.sc" );
}



// This class defines the volume of space that the player must stand in to control the vehicle
class CFuncVehicleControls : public CBaseEntity
{
public:
	virtual int	ObjectCaps( void ) { return CBaseEntity :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	void Spawn( void );
	void EXPORT Find( void );
};
LINK_ENTITY_TO_CLASS( func_vehiclecontrols, CFuncVehicleControls );


void CFuncVehicleControls :: Find( void )
{
	edict_t *pTarget = NULL;

	do 
	{
		pTarget = FIND_ENTITY_BY_TARGETNAME( pTarget, STRING(pev->target) );
	} while ( !FNullEnt(pTarget) && !FClassnameIs(pTarget, "func_vehicle") );

	if ( FNullEnt( pTarget ) )
	{
		ALERT( at_console, "No vehicle %s\n", STRING(pev->target) );
		return;
	}

	CFuncVehicle *pvehicle = CFuncVehicle::Instance(pTarget);
	pvehicle->SetControls( pev );
	UTIL_Remove( this );
}


void CFuncVehicleControls :: Spawn( void )
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	SET_MODEL( ENT(pev), STRING(pev->model) );

	UTIL_SetSize( pev, pev->mins, pev->maxs );
	UTIL_SetOrigin( pev, pev->origin );
	
	SetThink( &CFuncVehicleControls::Find );
	pev->nextthink = gpGlobals->time;
}

