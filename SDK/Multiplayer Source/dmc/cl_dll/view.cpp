//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

// view/refresh setup functions

#include <string.h>
#include "hud.h"
#include "cl_util.h"
#include "cvardef.h"
#include "usercmd.h"
#include "const.h"

#include "entity_state.h"
#include "cl_entity.h"
#include "ref_params.h"
#include "in_defs.h" // PITCH YAW ROLL
#include "pm_movevars.h"
#include "pm_shared.h"
#include "pm_defs.h"
#include "event_api.h"
#include "pmtrace.h"
#include "hltv.h"


// QUAKECLASSIC
extern int iMouseInUse;
extern vec3_t  vecTempAngles;
extern bool	bChangeAngles;
 

#ifndef M_PI
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif

extern "C" 
{
	int CL_IsThirdPerson( void );
	void CL_CameraOffset( float *ofs );

	void DLLEXPORT V_CalcRefdef( struct ref_params_s *pparams );

	void PM_ParticleLine( float *start, float *end, int pcolor, float life, float vert);
	int PM_GetInfo( int ent );
	void	InterpolateAngles(  float * start, float * end, float * output, float frac );
	void	NormalizeAngles( float * angles );
	float	Distance(const float * v1, const float * v2);
	float	AngleBetweenVectors(  const float * v1,  const float * v2 );

	float	vJumpOrigin[3];
	float	vJumpAngles[3];
}

#include "r_studioint.h"
#include "com_model.h"

extern engine_studio_api_t IEngineStudio;

void V_DropPunchAngle ( float frametime, float *ev_punchangle );
void VectorAngles( const float *forward, float *angles );

/*
The view is allowed to move slightly from it's true position for bobbing,
but if it exceeds 8 pixels linear distance (spherical, not box), the list of
entities sent from the server may not include everything in the pvs, especially
when crossing a water boudnary.
*/

extern cvar_t	*cl_forwardspeed;
extern cvar_t	*chase_active;
extern cvar_t	*cl_vsmoothing;

vec3_t	v_origin, v_angles, v_cl_angles, v_sim_org, v_lastAngles;
float	v_frametime, v_lastDistance;	

vec3_t ev_punchangle;

cvar_t	*scr_ofsx;
cvar_t	*scr_ofsy;
cvar_t	*scr_ofsz;

cvar_t	*v_centermove;
cvar_t	*v_centerspeed;

cvar_t	*cl_bobcycle;
cvar_t	*cl_bob;
cvar_t	*cl_bobup;
cvar_t	*cl_waterdist;
cvar_t	*cl_chasedist;

// These cvars are not registered (so users can't cheat), so set the ->value field directly
// Register these cvars in V_Init() if needed for easy tweaking
cvar_t	v_iyaw_cycle		= {"v_iyaw_cycle", "2", 0, 2};
cvar_t	v_iroll_cycle		= {"v_iroll_cycle", "0.5", 0, 0.5};
cvar_t	v_ipitch_cycle		= {"v_ipitch_cycle", "1", 0, 1};
cvar_t	v_iyaw_level		= {"v_iyaw_level", "0.3", 0, 0.3};
cvar_t	v_iroll_level		= {"v_iroll_level", "0.1", 0, 0.1};
cvar_t	v_ipitch_level		= {"v_ipitch_level", "0.3", 0, 0.3};

float	v_idlescale;  // used by TFC for concussion grenade effect

/*
//=============================================================================
void V_NormalizeAngles( float *angles )
{
	int i;
	// Normalize angles
	for ( i = 0; i < 3; i++ )
	{
		if ( angles[i] > 180.0 )
		{
			angles[i] -= 360.0;
		}
		else if ( angles[i] < -180.0 )
		{
			angles[i] += 360.0;
		}
	}
}

/*
===================
V_InterpolateAngles

Interpolate Euler angles.
FIXME:  Use Quaternions to avoid discontinuities
Frac is 0.0 to 1.0 ( i.e., should probably be clamped, but doesn't have to be )
===================

void V_InterpolateAngles( float *start, float *end, float *output, float frac )
{
	int i;
	float ang1, ang2;
	float d;
	
	V_NormalizeAngles( start );
	V_NormalizeAngles( end );

	for ( i = 0 ; i < 3 ; i++ )
	{
		ang1 = start[i];
		ang2 = end[i];

		d = ang2 - ang1;
		if ( d > 180 )
		{
			d -= 360;
		}
		else if ( d < -180 )
		{	
			d += 360;
		}

		output[i] = ang1 + d * frac;
	}

	V_NormalizeAngles( output );
} */


// Quakeworld bob code, this fixes jitters in the mutliplayer since the clock (pparams->time) isn't quite linear
float V_CalcBob ( struct ref_params_s *pparams )
{
	static	double	bobtime;
	static float	bob;
	float	cycle;
	static float	lasttime;
	vec3_t	vel;
	

	if ( pparams->onground == -1 ||
		 pparams->time == lasttime )
	{
		// just use old value
		return bob;	
	}

	lasttime = pparams->time;

	bobtime += pparams->frametime;
	cycle = bobtime - (int)( bobtime / cl_bobcycle->value ) * cl_bobcycle->value;
	cycle /= cl_bobcycle->value;
	
	if ( cycle < cl_bobup->value )
	{
		cycle = M_PI * cycle / cl_bobup->value;
	}
	else
	{
		cycle = M_PI + M_PI * ( cycle - cl_bobup->value )/( 1.0 - cl_bobup->value );
	}

	// bob is proportional to simulated velocity in the xy plane
	// (don't count Z, or jumping messes it up)
	VectorCopy( pparams->simvel, vel );
	vel[2] = 0;

	bob = sqrt( vel[0] * vel[0] + vel[1] * vel[1] ) * cl_bob->value;
	bob = bob * 0.3 + bob * 0.7 * sin(cycle);
	bob = min( bob, 4 );
	bob = max( bob, -7 );
	return bob;
	
}

/*
===============
V_CalcRoll
Used by view and sv_user
===============
*/
float V_CalcRoll (vec3_t angles, vec3_t velocity, float rollangle, float rollspeed )
{
    float   sign;
    float   side;
    float   value;
	vec3_t  forward, right, up;
    
	AngleVectors ( angles, forward, right, up );
    
	side = DotProduct (velocity, right);
    sign = side < 0 ? -1 : 1;
    side = fabs( side );
    
	value = rollangle;
    if (side < rollspeed)
	{
		side = side * value / rollspeed;
	}
    else
	{
		side = value;
	}
	return side * sign;
}


typedef struct pitchdrift_s
{
	float		pitchvel;
	int			nodrift;
	float		driftmove;
	double		laststop;
} pitchdrift_t;

static pitchdrift_t pd;

void V_StartPitchDrift( void )
{
	if ( pd.laststop == gEngfuncs.GetClientTime() )
	{
		return;		// something else is keeping it from drifting
	}

	if ( pd.nodrift || !pd.pitchvel )
	{
		pd.pitchvel = v_centerspeed->value;
		pd.nodrift = 0;
		pd.driftmove = 0;
	}
}

void V_StopPitchDrift ( void )
{
	pd.laststop = gEngfuncs.GetClientTime();
	pd.nodrift = 1;
	pd.pitchvel = 0;
}

/*
===============
V_DriftPitch

Moves the client pitch angle towards idealpitch sent by the server.

If the user is adjusting pitch manually, either with lookup/lookdown,
mlook and mouse, or klook and keyboard, pitch drifting is constantly stopped.
===============
*/
#include "kbutton.h"
extern kbutton_t	in_mlook;

void V_DriftPitch ( struct ref_params_s *pparams )
{
	float		delta, move;

	if ( ( in_mlook.state & 1) || gEngfuncs.IsNoClipping() || !pparams->onground || pparams->demoplayback || pparams->spectator )
	{
		pd.driftmove = 0;
		pd.pitchvel = 0;
		return;
	}

	// don't count small mouse motion
	if (pd.nodrift)
	{
		if ( fabs( pparams->cmd->forwardmove ) < cl_forwardspeed->value )
			pd.driftmove = 0;
		else
			pd.driftmove += pparams->frametime;
	
		if ( pd.driftmove > v_centermove->value)
		{
			V_StartPitchDrift ();
		}
		return;
	}
	
	delta = pparams->idealpitch - pparams->cl_viewangles[PITCH];

	if (!delta)
	{
		pd.pitchvel = 0;
		return;
	}

	move = pparams->frametime * pd.pitchvel;
	pd.pitchvel += pparams->frametime * v_centerspeed->value;
	
//Con_Printf ("move: %f (%f)\n", move, pparams->frametime);

	if (delta > 0)
	{
		if (move > delta)
		{
			pd.pitchvel = 0;
			move = delta;
		}
		pparams->cl_viewangles[PITCH] += move;
	}
	else if (delta < 0)
	{
		if (move > -delta)
		{
			pd.pitchvel = 0;
			move = -delta;
		}
		pparams->cl_viewangles[PITCH] -= move;
	}
}

/* 
============================================================================== 
						VIEW RENDERING 
============================================================================== 
*/ 

/*
==================
V_CalcGunAngle
==================
*/
void V_CalcGunAngle ( struct ref_params_s *pparams )
{	
	cl_entity_t *viewent;
	
	viewent = gEngfuncs.GetViewModel();
	if ( !viewent )
		return;

	viewent->angles[YAW]   =  pparams->viewangles[YAW]   + pparams->crosshairangle[YAW];
	viewent->angles[PITCH] = -pparams->viewangles[PITCH] + pparams->crosshairangle[PITCH] * 0.25;
	viewent->angles[ROLL]  -= v_idlescale * sin(pparams->time*v_iroll_cycle.value) * v_iroll_level.value;
	
	// don't apply all of the v_ipitch to prevent normally unseen parts of viewmodel from coming into view.
	viewent->angles[PITCH] -= v_idlescale * sin(pparams->time*v_ipitch_cycle.value) * (v_ipitch_level.value * 0.5);
	viewent->angles[YAW]   -= v_idlescale * sin(pparams->time*v_iyaw_cycle.value) * v_iyaw_level.value;

	VectorCopy( viewent->angles, viewent->curstate.angles );
	VectorCopy( viewent->angles, viewent->latched.prevangles );
}

/*
==============
V_AddIdle

Idle swaying
==============
*/
void V_AddIdle ( struct ref_params_s *pparams )
{
	pparams->viewangles[ROLL] += v_idlescale * sin(pparams->time*v_iroll_cycle.value) * v_iroll_level.value;
	pparams->viewangles[PITCH] += v_idlescale * sin(pparams->time*v_ipitch_cycle.value) * v_ipitch_level.value;
	pparams->viewangles[YAW] += v_idlescale * sin(pparams->time*v_iyaw_cycle.value) * v_iyaw_level.value;
}


extern cvar_t *cl_rollspeed;
extern cvar_t *cl_rollangle;
/*
==============
V_CalcViewRoll

Roll is induced by movement and damage
==============
*/
void V_CalcViewRoll ( struct ref_params_s *pparams )
{
	cl_entity_t *viewentity;
	
	viewentity = gEngfuncs.GetEntityByIndex( pparams->viewentity );
	if ( !viewentity )
		return;

	//Roll the angles when strafing Quake style!
	pparams->viewangles[ROLL] = V_CalcRoll (pparams->viewangles, pparams->simvel, cl_rollangle->value, cl_rollspeed->value ) * 4;

	if ( pparams->health <= 0 && ( pparams->viewheight[2] != 0 ) )
	{
		// only roll the view if the player is dead and the viewheight[2] is nonzero 
		// this is so deadcam in multiplayer will work.
		pparams->viewangles[ROLL] = 80;	// dead view angle
		return;
	}
}


/*
==================
V_CalcIntermissionRefdef

==================
*/
void V_CalcIntermissionRefdef ( struct ref_params_s *pparams )
{
	cl_entity_t	*ent, *view;
	float		old;

// don't allow cheats in multiplayer
	if ( pparams->maxclients > 1 )
	{
		scr_ofsx->value = 0.0;
		scr_ofsy->value = 0.0;
		scr_ofsz->value = 0.0;
	}

	// ent is the player model ( visible when out of body )
	ent = gEngfuncs.GetLocalPlayer();
	
	// view is the weapon model (only visible from inside body )
	view = gEngfuncs.GetViewModel();

	VectorCopy ( pparams->simorg, pparams->vieworg );
	VectorCopy ( pparams->cl_viewangles, pparams->viewangles );

	view->model = NULL;

	// allways idle in intermission
	old = v_idlescale;
	v_idlescale = 1;

	V_AddIdle ( pparams );

	if ( gEngfuncs.IsSpectateOnly() )
	{
		// in HLTV we must go to 'intermission' position by ourself
		VectorCopy( gHUD.m_Spectator.m_cameraOrigin, pparams->vieworg );
		VectorCopy( gHUD.m_Spectator.m_cameraAngles, pparams->viewangles );
	}
	v_idlescale = old;

	v_cl_angles = pparams->cl_viewangles;
	v_origin = pparams->vieworg;
	v_angles = pparams->viewangles;
}

#define ORIGIN_BACKUP 64
#define ORIGIN_MASK ( ORIGIN_BACKUP - 1 )

typedef struct 
{
	float Origins[ ORIGIN_BACKUP ][3];
	float OriginTime[ ORIGIN_BACKUP ];

	float Angles[ ORIGIN_BACKUP ][3];
	float AngleTime[ ORIGIN_BACKUP ];

	int CurrentOrigin;
	int CurrentAngle;
} viewinterp_t;

/*
==================
V_CalcRefdef

==================
*/
void V_CalcNormalRefdef ( struct ref_params_s *pparams )
{
	cl_entity_t		*ent, *view;
	int				i;
	vec3_t			angles;
	float			bob, waterOffset;
	static viewinterp_t		ViewInterp;

	static float oldz = 0;
	static float lasttime;

	static float lastang[3];
	vec3_t angdelta;

	vec3_t camAngles, camForward, camRight, camUp;
	cl_entity_t *pwater;

	//Force angle change
	if ( bChangeAngles == true )
	{
		pparams->cl_viewangles[PITCH] = vecTempAngles[PITCH];
		pparams->cl_viewangles[YAW]	  = vecTempAngles[YAW];
		pparams->cl_viewangles[ROLL]  = vecTempAngles[ROLL];

		vecTempAngles = Vector ( 0, 0, 0 );
		bChangeAngles = false;
	}

	// don't allow cheats in multiplayer
	if ( pparams->maxclients > 1 )
	{
		gEngfuncs.Cvar_SetValue ("scr_ofsx", 0);
		gEngfuncs.Cvar_SetValue ("scr_ofsy", 0);
		gEngfuncs.Cvar_SetValue ("scr_ofsz", 0);
	}

	V_DriftPitch ( pparams );

	// ent is the player model ( visible when out of body )
	ent = gEngfuncs.GetLocalPlayer();
	
	// view is the weapon model (only visible from inside body )
	view = gEngfuncs.GetViewModel();

	// transform the view offset by the model's matrix to get the offset from
	// model origin for the view
	bob = V_CalcBob ( pparams );
	
	// refresh position
	VectorCopy ( pparams->simorg, pparams->vieworg );
	pparams->vieworg[2] += ( bob );
	VectorAdd( pparams->vieworg, pparams->viewheight, pparams->vieworg );

	VectorCopy ( pparams->cl_viewangles, pparams->viewangles );

	gEngfuncs.V_CalcShake();
	gEngfuncs.V_ApplyShake( pparams->vieworg, pparams->viewangles, 1.0 );

	// never let view origin sit exactly on a node line, because a water plane can
	// dissapear when viewed with the eye exactly on it.
	// FIXME, we send origin at 1/128 now, change this?
	// the server protocol only specifies to 1/16 pixel, so add 1/32 in each axis
	
	pparams->vieworg[0] += 1.0/32;
	pparams->vieworg[1] += 1.0/32;
	pparams->vieworg[2] += 1.0/32;

	// Check for problems around water, move the viewer artificially if necessary 
	// -- this prevents drawing errors in GL due to waves

	waterOffset = 0;
	if ( pparams->waterlevel >= 2 )
	{
		int		i, contents, waterDist, waterEntity;
		vec3_t	point;
		waterDist = cl_waterdist->value;

		if ( pparams->hardware )
		{
			waterEntity = gEngfuncs.PM_WaterEntity( pparams->simorg );
			if ( waterEntity >= 0 && waterEntity < pparams->max_entities )
			{
				pwater = gEngfuncs.GetEntityByIndex( waterEntity );
				if ( pwater && ( pwater->model != NULL ) )
				{
					waterDist += ( pwater->curstate.scale * 16 );	// Add in wave height
				}
			}
		}
		else
		{
			waterEntity = 0;	// Don't need this in software
		}
		
		VectorCopy( pparams->vieworg, point );

		// Eyes are above water, make sure we're above the waves
		if ( pparams->waterlevel == 2 )	
		{
			point[2] -= waterDist;
			for ( i = 0; i < waterDist; i++ )
			{
				contents = gEngfuncs.PM_PointContents( point, NULL );
				if ( contents > CONTENTS_WATER )
					break;
				point[2] += 1;
			}
			waterOffset = (point[2] + waterDist) - pparams->vieworg[2];
		}
		else
		{
			// eyes are under water.  Make sure we're far enough under
			point[2] += waterDist;

			for ( i = 0; i < waterDist; i++ )
			{
				contents = gEngfuncs.PM_PointContents( point, NULL );
				if ( contents <= CONTENTS_WATER )
					break;
				point[2] -= 1;
			}
			waterOffset = (point[2] - waterDist) - pparams->vieworg[2];
		}
	}

	pparams->vieworg[2] += waterOffset;
	
	V_CalcViewRoll ( pparams );
	
	V_AddIdle ( pparams );

	// offsets
	VectorCopy( pparams->cl_viewangles, angles );

	AngleVectors ( angles, pparams->forward, pparams->right, pparams->up );

	for ( i=0 ; i<3 ; i++ )
	{
		pparams->vieworg[i] += scr_ofsx->value*pparams->forward[i] + scr_ofsy->value*pparams->right[i] + scr_ofsz->value*pparams->up[i];
	}
	
	// Treating cam_ofs[2] as the distance
	if( CL_IsThirdPerson() )
	{
		vec3_t ofs;

		ofs[0] = ofs[1] = ofs[2] = 0.0;

		CL_CameraOffset( (float *)&ofs );

		VectorCopy( ofs, camAngles );
		camAngles[ ROLL ]	= 0;

		AngleVectors( camAngles, camForward, camRight, camUp );

		for ( i = 0; i < 3; i++ )
		{
			pparams->vieworg[ i ] += -ofs[2] * camForward[ i ];
		}
	}

	// Give gun our viewangles
	VectorCopy ( pparams->cl_viewangles, view->angles );
	
	// set up gun position
	V_CalcGunAngle ( pparams );

	// Use predicted origin as view origin.
	VectorCopy ( pparams->simorg, view->origin );      
	view->origin[2] += ( waterOffset );
	VectorAdd( view->origin, pparams->viewheight, view->origin );

	// Let the viewmodel shake at about 10% of the amplitude
	gEngfuncs.V_ApplyShake( view->origin, view->angles, 0.9 );

	for ( i = 0; i < 3; i++ )
	{
		view->origin[ i ] += bob * 0.4 * pparams->forward[ i ];
	}
	view->origin[2] += bob;

	// throw in a little tilt.
	view->angles[YAW]   -= bob * 0.5;
	view->angles[ROLL]  -= bob * 1;
	view->angles[PITCH] -= bob * 0.3;

	// pushing the view origin down off of the same X/Z plane as the ent's origin will give the
	// gun a very nice 'shifting' effect when the player looks up/down. If there is a problem
	// with view model distortion, this may be a cause. (SJB). 
	view->origin[2] -= 1;

	// fudge position around to keep amount of weapon visible
	// roughly equal with different FOV
	if (pparams->viewsize == 110)
	{
		view->origin[2] += 1;
	}
	else if (pparams->viewsize == 100)
	{
		view->origin[2] += 2;
	}
	else if (pparams->viewsize == 90)
	{
		view->origin[2] += 1;
	}
	else if (pparams->viewsize == 80)
	{
		view->origin[2] += 0.5;
	}

	// Add in the punchangle, if any
	VectorAdd ( pparams->viewangles, pparams->punchangle, pparams->viewangles );

	// Include client side punch, too
	VectorAdd ( pparams->viewangles, (float *)&ev_punchangle, pparams->viewangles);

	V_DropPunchAngle ( pparams->frametime, (float *)&ev_punchangle );

	// smooth out stair step ups
#if 1
	if ( !pparams->smoothing && pparams->onground && pparams->simorg[2] - oldz > 0)
	{
		float steptime;
		
		steptime = pparams->time - lasttime;
		if (steptime < 0)
	//FIXME		I_Error ("steptime < 0");
			steptime = 0;

		oldz += steptime * 150;
		if (oldz > pparams->simorg[2])
			oldz = pparams->simorg[2];
		if (pparams->simorg[2] - oldz > 18)
			oldz = pparams->simorg[2]- 18;
		pparams->vieworg[2] += oldz - pparams->simorg[2];
		view->origin[2] += oldz - pparams->simorg[2];
	}
	else
	{
		oldz = pparams->simorg[2];
	}
#endif

	{
		static float lastorg[3];
		vec3_t delta;

		VectorSubtract( pparams->simorg, lastorg, delta );

		if ( Length( delta ) != 0.0 )
		{
			VectorCopy( pparams->simorg, ViewInterp.Origins[ ViewInterp.CurrentOrigin & ORIGIN_MASK ] );
			ViewInterp.OriginTime[ ViewInterp.CurrentOrigin & ORIGIN_MASK ] = pparams->time;
			ViewInterp.CurrentOrigin++;

			VectorCopy( pparams->simorg, lastorg );
		}
	}

	// Smooth out whole view in multiplayer when on trains, lifts
	if ( cl_vsmoothing && cl_vsmoothing->value &&
		( pparams->smoothing && ( pparams->maxclients > 1 ) ) )
	{
		int foundidx;
		int i;
		float t;

		if ( cl_vsmoothing->value < 0.0 )
		{
			gEngfuncs.Cvar_SetValue( "cl_vsmoothing", 0.0 );
		}

		t = pparams->time - cl_vsmoothing->value;

		for ( i = 1; i < ORIGIN_MASK; i++ )
		{
			foundidx = ViewInterp.CurrentOrigin - 1 - i;
			if ( ViewInterp.OriginTime[ foundidx & ORIGIN_MASK ] <= t )
				break;
		}

		if ( i < ORIGIN_MASK &&  ViewInterp.OriginTime[ foundidx & ORIGIN_MASK ] != 0.0 )
		{
			// Interpolate
			vec3_t delta;
			double frac;
			double dt;
			vec3_t neworg;

			dt = ViewInterp.OriginTime[ (foundidx + 1) & ORIGIN_MASK ] - ViewInterp.OriginTime[ foundidx & ORIGIN_MASK ];
			if ( dt > 0.0 )
			{
				frac = ( t - ViewInterp.OriginTime[ foundidx & ORIGIN_MASK] ) / dt;
				frac = min( 1.0, frac );
				VectorSubtract( ViewInterp.Origins[ ( foundidx + 1 ) & ORIGIN_MASK ], ViewInterp.Origins[ foundidx & ORIGIN_MASK ], delta );
				VectorMA( ViewInterp.Origins[ foundidx & ORIGIN_MASK ], frac, delta, neworg );

				VectorSubtract( neworg, pparams->simorg, delta );

				VectorAdd( pparams->simorg, delta, pparams->simorg );
				VectorAdd( pparams->vieworg, delta, pparams->vieworg );
				VectorAdd( view->origin, delta, view->origin );
			}
		}
	}

	// Store off v_angles before munging for third person
	v_angles = pparams->viewangles;
	v_cl_angles = pparams->cl_viewangles;

	if ( CL_IsThirdPerson() )
	{
		VectorCopy( camAngles, pparams->viewangles);
	}

	// override all previous settings if the viewent isn't the client
	if ( pparams->viewentity > pparams->maxclients )
	{
		cl_entity_t *viewentity;
		viewentity = gEngfuncs.GetEntityByIndex( pparams->viewentity );
		if ( viewentity )
		{
			VectorCopy( viewentity->origin, pparams->vieworg );
			VectorCopy( viewentity->angles, pparams->viewangles );

			// Store off overridden viewangles
			v_angles = pparams->viewangles;
		}
	}

	lasttime = pparams->time;
	
	v_origin = pparams->vieworg;
}
void V_SmoothInterpolateAngles( float * startAngle, float * endAngle, float * finalAngle, float degreesPerSec )
{
	float frac,d;
	
	NormalizeAngles( startAngle );
	NormalizeAngles( endAngle );

	for ( int i = 0 ; i < 3 ; i++ )
	{
		d = endAngle[i] - startAngle[i];

		if ( d > 180.0f )
		{
			d -= 360.0f;
		}
		else if ( d < -180.0f )
		{	
			d += 360.0f;
		}

		float absd = fabs(d);

		if ( absd > 0.1f )
		{
			frac = (degreesPerSec * v_frametime ) / absd;
				
			if ( absd < 45.0f )
				frac*= absd / 45.0f; // slow down last 45 degrees

			if ( frac > 1.0f )
			{
				finalAngle[i] = endAngle[i];
			}
			else
			{
				finalAngle[i] = startAngle[i] + d * frac;
			}
		}
		else
		{
			finalAngle[i] = endAngle[i];
		}

	}

	NormalizeAngles( finalAngle );
}

// Get the origin of the Observer based around the target's position and angles
void V_GetChaseOrigin( float * angles, float * origin, float distance, qboolean worldOnly, float * returnvec )
{
	int tracefinished = false;
	vec3_t	vecEnd;
	vec3_t	forward;
	vec3_t	vecStart;
	pmtrace_t * trace;
	
	// Trace back from the target using the player's view angles
	AngleVectors(angles, forward, NULL, NULL);
	
	VectorScale(forward,-1,forward);

	VectorCopy( origin, vecStart );

	VectorMA(vecStart, distance , forward, vecEnd);

	while (!tracefinished)
	{

		trace = gEngfuncs.PM_TraceLine( vecStart, vecEnd, PM_TRACELINE_PHYSENTSONLY, 2, -1 );

		if ( trace->ent <= 0 || !worldOnly) 
		{
			tracefinished = true;
		}
		else
		{
			if( Distance(trace->endpos, vecEnd ) > 1.0f )
			{
				VectorAdd( trace->endpos, forward, vecStart);
			}
			else
			{
				tracefinished = true;
			}
		}
	}  

	// gEngfuncs.Con_Printf("Trace loop %i\n", trace->ent ); 

	VectorMA( trace->endpos, 4, trace->plane.normal, returnvec );

	v_lastDistance = Distance(trace->endpos, origin);	// real distance without offset
}

void V_GetDirectedChasePosition(cl_entity_t	 * ent1, cl_entity_t * ent2,float * angle, float * origin)
{
	float newAngle[3]; float newOrigin[3]; float tempVec[3];
	
	int flags 	   = gHUD.m_Spectator.m_iObserverFlags;

	qboolean deadPlayer = ent1->player && (ent1->curstate.solid == SOLID_NOT);

	float dfactor   = ( flags & DRC_FLAG_DRAMATIC )? -1.0f : 1.0f;

	if ( ent1->player && (ent1->curstate.solid == SOLID_NOT) )
		dfactor = 1.5f;	// zoom away if player dies 

	float distance = 112.0f + ( 16.0f * dfactor ); // get close if dramatic;
	
	// go away in final scenes
	if (flags & DRC_FLAG_FINAL )
		distance*=2.0f;	

	// let v_lastDistance float smoothly away
	v_lastDistance+= v_frametime * 24.0f;	// move unit per seconds back

	if ( distance > v_lastDistance )
		distance = v_lastDistance;

	
	VectorCopy(ent1->origin, newOrigin);

	if ( ent1->player )
	{
		if ( deadPlayer )  
			newOrigin[2]+= 2;	//laying on ground
		else
			newOrigin[2]+= 28; // head level of living player
			
	}
	else
		newOrigin[2]+= 8;	// object, tricky, must be above bomb in CS

	if ( ( ent2 == (cl_entity_t*)0xFFFFFFFF ) || deadPlayer ) // we have no second target or player just died
	{
		// we have no second target, choose view direction based on
		// show front of primary target
		VectorCopy(ent1->angles, newAngle);
		newAngle[1]+= 180.0f;

		newAngle[0]+= 12.5f * dfactor; // lower angle if dramatic

		// if final scene (bomb), show from real high pos
		if ( flags & DRC_FLAG_FINAL )
			newAngle[0] = 22.5f; 

		// choose side of object/player			
		if ( flags & DRC_FLAG_SIDE )
			newAngle[1]+=22.5f;
		else
			newAngle[1]-=22.5f;

		// if ( AngleBetweenVectors( tempVec, newAngle ) > 1.0f )
		V_SmoothInterpolateAngles( v_lastAngles, newAngle, angle, 120.0f );

		// HACK, if player is dead don't clip against his dead body, can't check this
		V_GetChaseOrigin( angle, newOrigin, distance, deadPlayer, origin );

	}
	else if ( ent2 )
	{
		// get new angle towards second target
		VectorSubtract( ent2->origin, ent1->origin, newAngle );

		VectorAngles( newAngle, newAngle );
		newAngle[0] = -newAngle[0];

		// set angle diffrent in Dramtaic scenes
		newAngle[0]+= 12.5f * dfactor; // lower angle if dramatic
				
		if ( flags & DRC_FLAG_SIDE )
			newAngle[1]+=22.5f;
		else
			newAngle[1]-=22.5f;

		V_GetChaseOrigin( newAngle, newOrigin, distance, false, origin );

		origin[2]+= 16.0f*( 1.0f - (v_lastDistance / distance) );

		// calculate angle to second target
		VectorSubtract( ent2->origin, origin, tempVec );
		VectorAngles( tempVec, tempVec );
		tempVec[0] = -tempVec[0];

		// take middle between two viewangles
		InterpolateAngles( newAngle, tempVec, angle, 0.5f);

	}
	else
	{
		// second target disappeard somehow (dead)
		// keep last good viewangle
		V_GetChaseOrigin( angle, newOrigin, distance, false, origin );
	}

	VectorCopy(angle, v_lastAngles);
}

void V_GetChasePos(int target, float * cl_angles, float * origin, float * angles)
{
	if ( !target)
	{
		// just copy a save in-map position
		VectorCopy ( vJumpAngles, angles );
		VectorCopy ( vJumpOrigin, origin );
		return;
	};
	
	cl_entity_t	 *	ent = gEngfuncs.GetEntityByIndex( target );

	if (!ent) return;
	
	
	if ( gHUD.m_Spectator.m_autoDirector->value )
	{
		if ( g_iUser3 )
			V_GetDirectedChasePosition( ent, gEngfuncs.GetEntityByIndex( g_iUser3 ),
				angles, origin );
		else
			V_GetDirectedChasePosition( ent, ( cl_entity_t*)0xFFFFFFFF,
				angles, origin );
	}
	else
	{
		if ( cl_angles == NULL )	// no mouse angles given, use entity angles ( locked mode )
		{
			VectorCopy ( ent->angles, angles);
			angles[0]*=-1;
		}
		else
			VectorCopy ( cl_angles, angles);


		VectorCopy ( ent->origin, origin);
		
		origin[2]+= 28; // DEFAULT_VIEWHEIGHT - some offset

		V_GetChaseOrigin( angles, origin, cl_chasedist->value, false, origin );
	}
}

void V_ResetChaseCam()
{
	v_lastDistance = 4096.0f;
}


void V_GetInEyePos(int target, float * origin, float * angles )
{
	if ( !target)
	{
		// just copy a save in-map position
		VectorCopy ( vJumpAngles, angles );
		VectorCopy ( vJumpOrigin, origin );
		return;
	};


	cl_entity_t	 * ent = gEngfuncs.GetEntityByIndex( target );

	if ( !ent )
		return;

	VectorCopy ( ent->origin, origin );
	VectorCopy ( ent->angles, angles );

	angles[0]*=-M_PI;

	if ( ent->curstate.solid == SOLID_NOT )
	{
		angles[ROLL] = 80;	// dead view angle
		origin[2]+= -8 ; // PM_DEAD_VIEWHEIGHT
	}
	else if (ent->curstate.usehull == 1 )
		origin[2]+= 12; // VEC_DUCK_VIEW;
	else
		// exacty eye position can't be caluculated since it depends on
		// client values like cl_bobcycle, this offset matches the default values
		origin[2]+= 28; // DEFAULT_VIEWHEIGHT
}

void V_GetMapFreePosition( float * cl_angles, float * origin, float * angles )
{
	vec3_t forward;
	vec3_t zScaledTarget;

	VectorCopy(cl_angles, angles);

	// modify angles since we don't wanna see map's bottom
	angles[0] = 51.25f + 38.75f*(angles[0]/90.0f);

	zScaledTarget[0] = gHUD.m_Spectator.m_mapOrigin[0];
	zScaledTarget[1] = gHUD.m_Spectator.m_mapOrigin[1];
	zScaledTarget[2] = gHUD.m_Spectator.m_mapOrigin[2] * (( 90.0f - angles[0] ) / 90.0f );
	

	AngleVectors(angles, forward, NULL, NULL);

	VectorNormalize(forward);

	VectorMA(zScaledTarget, -( 4096.0f / gHUD.m_Spectator.m_mapZoom ), forward , origin);
}

void V_GetMapChasePosition(int target, float * cl_angles, float * origin, float * angles)
{
	vec3_t forward;

	if ( target )
	{
		cl_entity_t	 *	ent = gEngfuncs.GetEntityByIndex( target );

		if ( gHUD.m_Spectator.m_autoDirector->value )
		{
			// this is done to get the angles made by director mode
			V_GetChasePos(target, cl_angles, origin, angles);
			VectorCopy(ent->origin, origin);
			
			// keep fix chase angle horizontal
			angles[0] = 45.0f;
		}
		else
		{
			VectorCopy(cl_angles, angles);
			VectorCopy(ent->origin, origin);

			// modify angles since we don't wanna see map's bottom
			angles[0] = 51.25f + 38.75f*(angles[0]/90.0f);
		}
	}
	else
	{
		// keep out roaming position, but modify angles
		VectorCopy(cl_angles, angles);
		angles[0] = 51.25f + 38.75f*(angles[0]/90.0f);
	}

	origin[2] *= (( 90.0f - angles[0] ) / 90.0f );
	angles[2] = 0.0f;	// don't roll angle (if chased player is dead)

	AngleVectors(angles, forward, NULL, NULL);

	VectorNormalize(forward);

	VectorMA(origin, -1536, forward, origin); 
}

int V_FindViewModelByWeaponModel(int weaponindex)
{

	static char * modelmap[][2] =	{

#ifdef THREEWAVE
	//	{ "models/p_grapple.mdl",			"models/v_grapple.mdl"	},
#endif

		{ "models/p_crowbar.mdl",		"models/v_crowbar.mdl"		},
		{ "models/p_shot.mdl",			"models/v_shot.mdl"			},
		{ "models/p_shot2.mdl",			"models/v_shot2.mdl"		},
		{ "models/p_nail.mdl",				"models/v_nail.mdl"	},
		{ "models/p_nail2.mdl",			"models/v_nail2.mdl"		},
		{ "models/p_rock.mdl",			"models/v_rock.mdl"			},
		{ "models/p_rock2.mdl",			"models/v_rock2.mdl"		},
		{ "models/p_light.mdl",			"models/v_light.mdl"		},
		{ NULL, NULL } };

	struct model_s * weaponModel = IEngineStudio.GetModelByIndex( weaponindex );

	if ( weaponModel )
	{
		int len = strlen( weaponModel->name );
		int i = 0;

		while ( *modelmap[i] != NULL )
		{
			if ( !strnicmp( weaponModel->name, modelmap[i][0], len ) )
			{
				return gEngfuncs.pEventAPI->EV_FindModelIndex( modelmap[i][1] );
			}
			i++;
		}

		return 0;
	}
	else
		return 0;

}


/*
==================
V_CalcSpectatorRefdef

==================
*/
void V_CalcSpectatorRefdef ( struct ref_params_s * pparams )
{

	vec3_t					angles;
	static viewinterp_t		ViewInterp;
	static float			bob = 0.0f;
	static vec3_t			velocity ( 0.0f, 0.0f, 0.0f);

	static int lastWeaponModelIndex = 0;
	static int lastViewModelIndex = 0;
		
	cl_entity_t	 * ent = gEngfuncs.GetEntityByIndex( g_iUser2 );
	cl_entity_t	 * gunModel = gEngfuncs.GetViewModel();
	static float lasttime;

	static float lastang[3];
	static float lastorg[3];

	vec3_t delta;
	pparams->onlyClientDraw = false;

	// refresh position
	VectorCopy ( pparams->simorg, v_sim_org );

	// get old values
	VectorCopy ( pparams->cl_viewangles, v_cl_angles );
	VectorCopy ( pparams->viewangles, v_angles );
	VectorCopy ( pparams->vieworg, v_origin );
	v_frametime = pparams->frametime;

	if ( pparams->nextView == 0 )
	{
		// first renderer cycle, full screen

		switch ( g_iUser1 )
		{
			case OBS_CHASE_LOCKED:	V_GetChasePos( g_iUser2, NULL, v_origin, v_angles );
									break;

			case OBS_CHASE_FREE:	V_GetChasePos( g_iUser2, v_cl_angles, v_origin, v_angles );
									break;

			case OBS_ROAMING	:	VectorCopy (v_cl_angles, v_angles);
									VectorCopy (v_sim_org, v_origin);
									break;

			case OBS_IN_EYE		:   V_GetInEyePos( g_iUser2, v_origin, v_angles );
									break;
				
			case OBS_MAP_FREE  :	pparams->onlyClientDraw = true;
									V_GetMapFreePosition( v_cl_angles, v_origin, v_angles );
									break;

			case OBS_MAP_CHASE  :	pparams->onlyClientDraw = true;
									V_GetMapChasePosition( g_iUser2, v_cl_angles, v_origin, v_angles );
									break;
		}

		if ( gHUD.m_Spectator.m_pip->value )
			pparams->nextView = 1;	// force a second renderer view

		gHUD.m_Spectator.m_iDrawCycle = 0;

	}
	else
	{
		// second renderer cycle, inset window

		// set inset parameters
		pparams->viewport[0] = XRES(gHUD.m_Spectator.m_OverviewData.insetWindowX);	// change viewport to inset window
		pparams->viewport[1] = YRES(gHUD.m_Spectator.m_OverviewData.insetWindowY);
		pparams->viewport[2] = XRES(gHUD.m_Spectator.m_OverviewData.insetWindowWidth);
		pparams->viewport[3] = YRES(gHUD.m_Spectator.m_OverviewData.insetWindowHeight);
		pparams->nextView	 = 0;	// on further view
		pparams->onlyClientDraw = false;

		// override some settings in certain modes
		switch ( (int)gHUD.m_Spectator.m_pip->value )
		{
			case INSET_CHASE_FREE : V_GetChasePos( g_iUser2, v_cl_angles, v_origin, v_angles );
									break;	

			case INSET_IN_EYE	 :	V_GetInEyePos( g_iUser2, v_origin, v_angles );
									break;

			case INSET_MAP_FREE  :	pparams->onlyClientDraw = true;
									V_GetMapFreePosition( v_cl_angles, v_origin, v_angles );
									break;

			case INSET_MAP_CHASE  :	pparams->onlyClientDraw = true;

									if ( g_iUser1 == OBS_ROAMING )
										V_GetMapChasePosition( 0, v_cl_angles, v_origin, v_angles );
									else
										V_GetMapChasePosition( g_iUser2, v_cl_angles, v_origin, v_angles );

									break;
		}

		gHUD.m_Spectator.m_iDrawCycle = 1;
	}


	// do the smoothing only once per frame, not in roaming or map mode
	if ( (gHUD.m_Spectator.m_iDrawCycle == 0) && (g_iUser1 == OBS_IN_EYE)  )
	{
		// smooth angles

		VectorSubtract( v_angles, lastang, delta );
		if ( Length( delta ) != 0.0f )
		{
			VectorCopy( v_angles, ViewInterp.Angles[ ViewInterp.CurrentAngle & ORIGIN_MASK ] );
			ViewInterp.AngleTime[ ViewInterp.CurrentAngle & ORIGIN_MASK ] = pparams->time;
			ViewInterp.CurrentAngle++;
			VectorCopy( v_angles, lastang );
		}

		if ( cl_vsmoothing && cl_vsmoothing->value )
		{
			int foundidx;
			int i;
			float t;

			t = pparams->time - cl_vsmoothing->value;

			for ( i = 1; i < ORIGIN_MASK; i++ )
			{
				foundidx = ViewInterp.CurrentAngle - 1 - i;
				if ( ViewInterp.AngleTime[ foundidx & ORIGIN_MASK ] <= t )
					break;
			}
			
			if ( i < ORIGIN_MASK && ViewInterp.AngleTime[ foundidx & ORIGIN_MASK ] != 0.0 )
			{
				// Interpolate
				double dt;
				float  da;
				vec3_t	v1,v2;

				AngleVectors( ViewInterp.Angles[ foundidx & ORIGIN_MASK ], v1, NULL, NULL );
				AngleVectors( ViewInterp.Angles[ (foundidx + 1) & ORIGIN_MASK ], v2, NULL, NULL );
				da = AngleBetweenVectors( v1, v2 );

				dt = ViewInterp.AngleTime[ (foundidx + 1) & ORIGIN_MASK ] - ViewInterp.AngleTime[ foundidx & ORIGIN_MASK ];
					
				if ( dt > 0.0 && ( da < 22.5f) )
				{
					double frac;

					frac = ( t - ViewInterp.AngleTime[ foundidx & ORIGIN_MASK] ) / dt;
					frac = min( 1.0, frac );

					// interpolate angles
					InterpolateAngles( ViewInterp.Angles[ foundidx & ORIGIN_MASK ], ViewInterp.Angles[ (foundidx + 1) & ORIGIN_MASK ], v_angles, frac );
				}
			}
		} 

  		// smooth origin
		
		VectorSubtract( v_origin, lastorg, delta );

		if ( Length( delta ) != 0.0 )
		{
			VectorCopy( v_origin, ViewInterp.Origins[ ViewInterp.CurrentOrigin & ORIGIN_MASK ] );
			ViewInterp.OriginTime[ ViewInterp.CurrentOrigin & ORIGIN_MASK ] = pparams->time;
			ViewInterp.CurrentOrigin++;

			VectorCopy( v_origin, lastorg );
		}

		// don't smooth in roaming (already smoothd), 
		if ( cl_vsmoothing && cl_vsmoothing->value  )
		{
			int foundidx;
			int i;
			float t;

			t = pparams->time - cl_vsmoothing->value;

			for ( i = 1; i < ORIGIN_MASK; i++ )
			{
				foundidx = ViewInterp.CurrentOrigin - 1 - i;
				if ( ViewInterp.OriginTime[ foundidx & ORIGIN_MASK ] <= t )
					break;
			}

			if ( i < ORIGIN_MASK &&  ViewInterp.OriginTime[ foundidx & ORIGIN_MASK ] != 0.0 )
			{
				// Interpolate
				vec3_t delta;
				double frac;
				double dt;
				vec3_t neworg;

				dt = ViewInterp.OriginTime[ (foundidx + 1) & ORIGIN_MASK ] - ViewInterp.OriginTime[ foundidx & ORIGIN_MASK ];
				if ( dt > 0.0 )
				{
					frac = ( t - ViewInterp.OriginTime[ foundidx & ORIGIN_MASK] ) / dt;
					frac = min( 1.0, frac );
					VectorSubtract( ViewInterp.Origins[ ( foundidx + 1 ) & ORIGIN_MASK ], ViewInterp.Origins[ foundidx & ORIGIN_MASK ], delta );
					VectorMA( ViewInterp.Origins[ foundidx & ORIGIN_MASK ], frac, delta, neworg );

					// Dont interpolate large changes
					if ( Length( delta ) < 64 )
					{
						VectorCopy( neworg, v_origin );
					}
				}
			}
		}
	}	

	// Hack in weapon model:


	if ( (g_iUser1 == OBS_IN_EYE || gHUD.m_Spectator.m_pip->value == INSET_IN_EYE) 
		&& ent && g_iUser2 )
	{
		// get position for weapon model
		VectorCopy( v_origin, gunModel->origin);
		VectorCopy( v_angles, gunModel->angles);

		// add idle tremble
		gunModel->angles[PITCH]*=-1;
		
		// calculate player velocity
		float timeDiff = ent->curstate.msg_time - ent->prevstate.msg_time;

		if ( timeDiff > 0 )
		{
			vec3_t distance;
			VectorSubtract(ent->prevstate.origin, ent->curstate.origin, distance);
			VectorScale(distance, 1/timeDiff, distance );

			velocity[0] = velocity[0]*0.66f + distance[0]*0.33f;
			velocity[1] = velocity[1]*0.66f + distance[1]*0.33f;
			velocity[2] = velocity[2]*0.66f + distance[2]*0.33f;
			
			VectorCopy(velocity, pparams->simvel);
			pparams->onground = 1;

			bob = V_CalcBob( pparams );
		}

		vec3_t forward;
		AngleVectors(v_angles, forward, NULL, NULL );

		for ( int i = 0; i < 3; i++ )
		{
			gunModel->origin[ i ] += bob * 0.4 * forward[ i ];
		}
		
		// throw in a little tilt.
		gunModel->angles[YAW]   -= bob * 0.5;
		gunModel->angles[ROLL]  -= bob * 1;
		gunModel->angles[PITCH] -= bob * 0.3;

		VectorCopy( gunModel->angles, gunModel->curstate.angles );
		VectorCopy( gunModel->angles, gunModel->latched.prevangles );

		if ( lastWeaponModelIndex != ent->curstate.weaponmodel )
		{
			// weapon model changed

			lastWeaponModelIndex = ent->curstate.weaponmodel;
			lastViewModelIndex = V_FindViewModelByWeaponModel( lastWeaponModelIndex );
			if ( lastViewModelIndex )
			{
				gEngfuncs.pfnWeaponAnim(0,0);	// reset weapon animation
			}
			else
			{
				// model not found
				gunModel->model = NULL;	// disable weaopn model
				lastWeaponModelIndex = lastViewModelIndex = 0;
			}
		}

		if ( lastViewModelIndex )
		{
			gunModel->model = IEngineStudio.GetModelByIndex( lastViewModelIndex );
			gunModel->curstate.modelindex = lastViewModelIndex;
			gunModel->curstate.frame = 0;
			gunModel->curstate.colormap = 0; 
			gunModel->index = g_iUser2;
		}
		else
		{
			gunModel->model = NULL;	// disable weaopn model
		}
	}
	else
	{
		gunModel->model = NULL;	// disable weaopn model
		lastWeaponModelIndex = lastViewModelIndex = 0;
	}

	lasttime = pparams->time; 

	// write back new values into pparams

	VectorCopy ( v_angles, pparams->viewangles )
	VectorCopy ( v_origin, pparams->vieworg );

}

void DLLEXPORT V_CalcRefdef( struct ref_params_s *pparams )
{
	// intermission / finale rendering
	if ( pparams->intermission )
	{	
		V_CalcIntermissionRefdef ( pparams );	
	}
	else if ( pparams->spectator || g_iUser1 )	// g_iUser true if in spectator mode
	{
		V_CalcSpectatorRefdef ( pparams );	
	}
	else if ( !pparams->paused )
	{
		V_CalcNormalRefdef ( pparams );
	}
}

/*
=============
V_DropPunchAngle

=============
*/
void V_DropPunchAngle ( float frametime, float *ev_punchangle )
{
	float	len;
	
	len = VectorNormalize ( ev_punchangle );
	len -= (10.0 + len * 0.5) * frametime;
	len = max( len, 0.0 );
	VectorScale ( ev_punchangle, len, ev_punchangle );
}

/*
=============
V_PunchAxis

Client side punch effect
=============
*/
void V_PunchAxis( int axis, float punch )
{
	ev_punchangle[ axis ] = punch;
}

/*
=============
V_Init
=============
*/
void V_Init (void)
{
	gEngfuncs.pfnAddCommand ("centerview", V_StartPitchDrift );

	scr_ofsx			= gEngfuncs.pfnRegisterVariable( "scr_ofsx","0", 0 );
	scr_ofsy			= gEngfuncs.pfnRegisterVariable( "scr_ofsy","0", 0 );
	scr_ofsz			= gEngfuncs.pfnRegisterVariable( "scr_ofsz","0", 0 );

	v_centermove		= gEngfuncs.pfnRegisterVariable( "v_centermove", "0.15", 0 );
	v_centerspeed		= gEngfuncs.pfnRegisterVariable( "v_centerspeed","500", 0 );

	cl_bobcycle			= gEngfuncs.pfnRegisterVariable( "cl_bobcycle","0.8", 0 );// best default for my experimental gun wag (sjb)
	cl_bob				= gEngfuncs.pfnRegisterVariable( "cl_bob","0.01", 0 );// best default for my experimental gun wag (sjb)
	cl_bobup			= gEngfuncs.pfnRegisterVariable( "cl_bobup","0.5", 0 );
	cl_waterdist		= gEngfuncs.pfnRegisterVariable( "cl_waterdist","4", 0 );
	cl_chasedist		= gEngfuncs.pfnRegisterVariable( "cl_chasedist","112", 0 );
}


//#define TRACE_TEST
#if defined( TRACE_TEST )

extern float in_fov;
/*
====================
CalcFov
====================
*/
float CalcFov (float fov_x, float width, float height)
{
	float	a;
	float	x;

	if (fov_x < 1 || fov_x > 179)
		fov_x = 90;	// error, set to 90

	x = width/tan(fov_x/360*M_PI);

	a = atan (height/x);

	a = a*360/M_PI;

	return a;
}

int hitent = -1;

void V_Move( int mx, int my )
{
	float fov;
	float fx, fy;
	float dx, dy;
	float c_x, c_y;
	float dX, dY;
	vec3_t forward, up, right;
	vec3_t newangles;

	vec3_t farpoint;
	pmtrace_t tr;

	fov = CalcFov( in_fov, (float)ScreenWidth, (float)ScreenHeight );

	c_x = (float)ScreenWidth / 2.0;
	c_y = (float)ScreenHeight / 2.0;

	dx = (float)mx - c_x;
	dy = (float)my - c_y;

	// Proportion we moved in each direction
	fx = dx / c_x;
	fy = dy / c_y;

	dX = fx * in_fov / 2.0 ;
	dY = fy * fov / 2.0;

	newangles = v_angles;

	newangles[ YAW ] -= dX;
	newangles[ PITCH ] += dY;

	// Now rotate v_forward around that point
	AngleVectors ( newangles, forward, right, up );

	farpoint = v_origin + 8192 * forward;

	// Trace
	tr = *(gEngfuncs.PM_TraceLine( (float *)&v_origin, (float *)&farpoint, PM_TRACELINE_PHYSENTSONLY, 2 /*point sized hull*/, -1 ));

	if ( tr.fraction != 1.0 && tr.ent != 0 )
	{
		hitent = PM_GetInfo( tr.ent );
		PM_ParticleLine( (float *)&v_origin, (float *)&tr.endpos, 5, 1.0, 0.0 );
	}
	else
	{
		hitent = -1;
	}
}

#endif
