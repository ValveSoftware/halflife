/***
*
*	Copyright (c) 1999, 2000 Valve LLC. All rights reserved.
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
#include "hud.h"
#include "cl_util.h"
#include "camera.h"
#include "kbutton.h"
#include "cvardef.h"
#include "usercmd.h"
#include "const.h"
#include "camera.h"
#include "in_defs.h"

#include "SDL2/SDL_mouse.h"
#include "port.h"

float CL_KeyState (kbutton_t *key);

extern "C" 
{
	void EXPORT CAM_Think( void );
	int EXPORT CL_IsThirdPerson( void );
	void EXPORT CL_CameraOffset( float *ofs );
}

extern cl_enginefunc_t gEngfuncs;

//-------------------------------------------------- Constants

#define CAM_DIST_DELTA 1.0
#define CAM_ANGLE_DELTA 2.5
#define CAM_ANGLE_SPEED 2.5
#define CAM_MIN_DIST 30.0
#define CAM_ANGLE_MOVE .5
#define MAX_ANGLE_DIFF 10.0
#define PITCH_MAX 90.0
#define PITCH_MIN 0
#define YAW_MAX  135.0
#define YAW_MIN	 -135.0

enum ECAM_Command
{
	CAM_COMMAND_NONE = 0,
	CAM_COMMAND_TOTHIRDPERSON = 1,
	CAM_COMMAND_TOFIRSTPERSON = 2
};

//-------------------------------------------------- Global Variables

cvar_t	*cam_command;
cvar_t	*cam_snapto;
cvar_t	*cam_idealyaw;
cvar_t	*cam_idealpitch;
cvar_t	*cam_idealdist;
cvar_t	*cam_contain;

cvar_t	*c_maxpitch;
cvar_t	*c_minpitch;
cvar_t	*c_maxyaw;
cvar_t	*c_minyaw;
cvar_t	*c_maxdistance;
cvar_t	*c_mindistance;

// pitch, yaw, dist
vec3_t cam_ofs;


// In third person
int cam_thirdperson;
int cam_mousemove; //true if we are moving the cam with the mouse, False if not
int iMouseInUse=0;
int cam_distancemove;
extern int mouse_x, mouse_y;  //used to determine what the current x and y values are
int cam_old_mouse_x, cam_old_mouse_y; //holds the last ticks mouse movement
POINT		cam_mouse;
//-------------------------------------------------- Local Variables

static kbutton_t cam_pitchup, cam_pitchdown, cam_yawleft, cam_yawright;
static kbutton_t cam_in, cam_out, cam_move;

//-------------------------------------------------- Prototypes

void CAM_ToThirdPerson(void);
void CAM_ToFirstPerson(void);
void CAM_StartDistance(void);
void CAM_EndDistance(void);

void SDL_GetCursorPos( POINT *p )
{
	SDL_GetMouseState( (int *)&p->x, (int *)&p->y );
}

void SDL_SetCursorPos( const int x, const int y )
{
}


//-------------------------------------------------- Local Functions

float MoveToward( float cur, float goal, float maxspeed )
{
	if( cur != goal )
	{
		if( abs( cur - goal ) > 180.0 )
		{
			if( cur < goal )
				cur += 360.0;
			else
				cur -= 360.0;
		}

		if( cur < goal )
		{
			if( cur < goal - 1.0 )
				cur += ( goal - cur ) / 4.0;
			else
				cur = goal;
		}
		else
		{
			if( cur > goal + 1.0 )
				cur -= ( cur - goal ) / 4.0;
			else
				cur = goal;
		}
	}


	// bring cur back into range
	if( cur < 0 )
		cur += 360.0;
	else if( cur >= 360 )
		cur -= 360;

	return cur;
}


//-------------------------------------------------- Gobal Functions

typedef struct
{
	vec3_t		boxmins, boxmaxs;// enclose the test object along entire move
	float		*mins, *maxs;	// size of the moving object
	vec3_t		mins2, maxs2;	// size when clipping against mosnters
	float		*start, *end;
	trace_t		trace;
	int			type;
	edict_t		*passedict;
	qboolean	monsterclip;
} moveclip_t;

extern trace_t SV_ClipMoveToEntity (edict_t *ent, vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end);

void EXPORT CAM_Think( void )
{
	vec3_t origin;
	vec3_t ext, pnt, camForward, camRight, camUp;
	moveclip_t	clip;
	float dist;
	vec3_t camAngles;
	float flSensitivity;
#ifdef LATER
	int i;
#endif
	vec3_t viewangles;

	switch( (int) cam_command->value )
	{
		case CAM_COMMAND_TOTHIRDPERSON:
			CAM_ToThirdPerson();
			break;

		case CAM_COMMAND_TOFIRSTPERSON:
			CAM_ToFirstPerson();
			break;

		case CAM_COMMAND_NONE:
		default:
			break;
	}

	if( !cam_thirdperson )
		return;
	
#ifdef LATER
	if ( cam_contain->value )
	{
		gEngfuncs.GetClientOrigin( origin );
		ext[0] = ext[1] = ext[2] = 0.0;
	}
#endif

	camAngles[ PITCH ] = cam_idealpitch->value;
	camAngles[ YAW ] = cam_idealyaw->value;
	dist = cam_idealdist->value;
	//
	//movement of the camera with the mouse
	//
	if (cam_mousemove)
	{
	    //get windows cursor position
		SDL_GetCursorPos (&cam_mouse);
		//check for X delta values and adjust accordingly
		//eventually adjust YAW based on amount of movement
	  //don't do any movement of the cam using YAW/PITCH if we are zooming in/out the camera	
	  if (!cam_distancemove)
	  {
		
		//keep the camera within certain limits around the player (ie avoid certain bad viewing angles)  
		if (cam_mouse.x>gEngfuncs.GetWindowCenterX())
		{
			//if ((camAngles[YAW]>=225.0)||(camAngles[YAW]<135.0))
			if (camAngles[YAW]<c_maxyaw->value)
			{
				camAngles[ YAW ] += (CAM_ANGLE_MOVE)*((cam_mouse.x-gEngfuncs.GetWindowCenterX())/2);
			}
			if (camAngles[YAW]>c_maxyaw->value)
			{
				
				camAngles[YAW]=c_maxyaw->value;
			}
		}
		else if (cam_mouse.x<gEngfuncs.GetWindowCenterX())
		{
			//if ((camAngles[YAW]<=135.0)||(camAngles[YAW]>225.0))
			if (camAngles[YAW]>c_minyaw->value)
			{
			   camAngles[ YAW ] -= (CAM_ANGLE_MOVE)* ((gEngfuncs.GetWindowCenterX()-cam_mouse.x)/2);
			   	
			}
			if (camAngles[YAW]<c_minyaw->value)
			{
				camAngles[YAW]=c_minyaw->value;
				
			}
		}

		//check for y delta values and adjust accordingly
		//eventually adjust PITCH based on amount of movement
		//also make sure camera is within bounds
		if (cam_mouse.y>gEngfuncs.GetWindowCenterY())
		{
			if(camAngles[PITCH]<c_maxpitch->value)
			{
			    camAngles[PITCH] +=(CAM_ANGLE_MOVE)* ((cam_mouse.y-gEngfuncs.GetWindowCenterY())/2);
			}
			if (camAngles[PITCH]>c_maxpitch->value)
			{
				camAngles[PITCH]=c_maxpitch->value;
			}
		}
		else if (cam_mouse.y<gEngfuncs.GetWindowCenterY())
		{
			if (camAngles[PITCH]>c_minpitch->value)
			{
			   camAngles[PITCH] -= (CAM_ANGLE_MOVE)*((gEngfuncs.GetWindowCenterY()-cam_mouse.y)/2);
			}
			if (camAngles[PITCH]<c_minpitch->value)
			{
				camAngles[PITCH]=c_minpitch->value;
			}
		}

		//set old mouse coordinates to current mouse coordinates
		//since we are done with the mouse

		if ( ( flSensitivity = gHUD.GetSensitivity() ) != 0 )
		{
			cam_old_mouse_x=cam_mouse.x*flSensitivity;
			cam_old_mouse_y=cam_mouse.y*flSensitivity;
		}
		else
		{
			cam_old_mouse_x=cam_mouse.x;
			cam_old_mouse_y=cam_mouse.y;
		}
		SDL_SetCursorPos (gEngfuncs.GetWindowCenterX(), gEngfuncs.GetWindowCenterY());
	  }
	}

	//Nathan code here
	if( CL_KeyState( &cam_pitchup ) )
		camAngles[ PITCH ] += CAM_ANGLE_DELTA;
	else if( CL_KeyState( &cam_pitchdown ) )
		camAngles[ PITCH ] -= CAM_ANGLE_DELTA;

	if( CL_KeyState( &cam_yawleft ) )
		camAngles[ YAW ] -= CAM_ANGLE_DELTA;
	else if( CL_KeyState( &cam_yawright ) )
		camAngles[ YAW ] += CAM_ANGLE_DELTA;

	if( CL_KeyState( &cam_in ) )
	{
		dist -= CAM_DIST_DELTA;
		if( dist < CAM_MIN_DIST )
		{
			// If we go back into first person, reset the angle
			camAngles[ PITCH ] = 0;
			camAngles[ YAW ] = 0;
			dist = CAM_MIN_DIST;
		}

	}
	else if( CL_KeyState( &cam_out ) )
		dist += CAM_DIST_DELTA;

	if (cam_distancemove)
	{
		if (cam_mouse.y>gEngfuncs.GetWindowCenterY())
		{
			if(dist<c_maxdistance->value)
			{
			    dist +=CAM_DIST_DELTA * ((cam_mouse.y-gEngfuncs.GetWindowCenterY())/2);
			}
			if (dist>c_maxdistance->value)
			{
				dist=c_maxdistance->value;
			}
		}
		else if (cam_mouse.y<gEngfuncs.GetWindowCenterY())
		{
			if (dist>c_mindistance->value)
			{
			   dist -= (CAM_DIST_DELTA)*((gEngfuncs.GetWindowCenterY()-cam_mouse.y)/2);
			}
			if (dist<c_mindistance->value)
			{
				dist=c_mindistance->value;
			}
		}
		//set old mouse coordinates to current mouse coordinates
		//since we are done with the mouse
		cam_old_mouse_x=cam_mouse.x*gHUD.GetSensitivity();
		cam_old_mouse_y=cam_mouse.y*gHUD.GetSensitivity();
		SDL_SetCursorPos (gEngfuncs.GetWindowCenterX(), gEngfuncs.GetWindowCenterY());
	}
#ifdef LATER
	if( cam_contain->value )
	{
		// check new ideal
		VectorCopy( origin, pnt );
		AngleVectors( camAngles, camForward, camRight, camUp );
		for (i=0 ; i<3 ; i++)
			pnt[i] += -dist*camForward[i];

		// check line from r_refdef.vieworg to pnt
		memset ( &clip, 0, sizeof ( moveclip_t ) );
		clip.trace = SV_ClipMoveToEntity( sv.edicts, r_refdef.vieworg, ext, ext, pnt );
		if( clip.trace.fraction == 1.0 )
		{
			// update ideal
			cam_idealpitch->value = camAngles[ PITCH ];
			cam_idealyaw->value = camAngles[ YAW ];
			cam_idealdist->value = dist;
		}
	}
	else
#endif
	{
		// update ideal
		cam_idealpitch->value = camAngles[ PITCH ];
		cam_idealyaw->value = camAngles[ YAW ];
		cam_idealdist->value = dist;
	}

	// Move towards ideal
	VectorCopy( cam_ofs, camAngles );

	gEngfuncs.GetViewAngles( (float *)viewangles );

	if( cam_snapto->value )
	{
		camAngles[ YAW ] = cam_idealyaw->value + viewangles[ YAW ];
		camAngles[ PITCH ] = cam_idealpitch->value + viewangles[ PITCH ];
		camAngles[ 2 ] = cam_idealdist->value;
	}
	else
	{
		if( camAngles[ YAW ] - viewangles[ YAW ] != cam_idealyaw->value )
			camAngles[ YAW ] = MoveToward( camAngles[ YAW ], cam_idealyaw->value + viewangles[ YAW ], CAM_ANGLE_SPEED );

		if( camAngles[ PITCH ] - viewangles[ PITCH ] != cam_idealpitch->value )
			camAngles[ PITCH ] = MoveToward( camAngles[ PITCH ], cam_idealpitch->value + viewangles[ PITCH ], CAM_ANGLE_SPEED );

		if( abs( camAngles[ 2 ] - cam_idealdist->value ) < 2.0 )
			camAngles[ 2 ] = cam_idealdist->value;
		else
			camAngles[ 2 ] += ( cam_idealdist->value - camAngles[ 2 ] ) / 4.0;
	}
#ifdef LATER
	if( cam_contain->value )
	{
		// Test new position
		dist = camAngles[ ROLL ];
		camAngles[ ROLL ] = 0;

		VectorCopy( origin, pnt );
		AngleVectors( camAngles, camForward, camRight, camUp );
		for (i=0 ; i<3 ; i++)
			pnt[i] += -dist*camForward[i];

		// check line from r_refdef.vieworg to pnt
		memset ( &clip, 0, sizeof ( moveclip_t ) );
		ext[0] = ext[1] = ext[2] = 0.0;
		clip.trace = SV_ClipMoveToEntity( sv.edicts, r_refdef.vieworg, ext, ext, pnt );
		if( clip.trace.fraction != 1.0 )
			return;
	}
#endif
	cam_ofs[ 0 ] = camAngles[ 0 ];
	cam_ofs[ 1 ] = camAngles[ 1 ];
	cam_ofs[ 2 ] = dist;
}

extern void KeyDown (kbutton_t *b);	// HACK
extern void KeyUp (kbutton_t *b);	// HACK

void CAM_PitchUpDown(void) { KeyDown( &cam_pitchup ); }
void CAM_PitchUpUp(void) { KeyUp( &cam_pitchup ); }
void CAM_PitchDownDown(void) { KeyDown( &cam_pitchdown ); }
void CAM_PitchDownUp(void) { KeyUp( &cam_pitchdown ); }
void CAM_YawLeftDown(void) { KeyDown( &cam_yawleft ); }
void CAM_YawLeftUp(void) { KeyUp( &cam_yawleft ); }
void CAM_YawRightDown(void) { KeyDown( &cam_yawright ); }
void CAM_YawRightUp(void) { KeyUp( &cam_yawright ); }
void CAM_InDown(void) { KeyDown( &cam_in ); }
void CAM_InUp(void) { KeyUp( &cam_in ); }
void CAM_OutDown(void) { KeyDown( &cam_out ); }
void CAM_OutUp(void) { KeyUp( &cam_out ); }

void CAM_ToThirdPerson(void)
{ 
	vec3_t viewangles;

	gEngfuncs.GetViewAngles( (float *)viewangles );

	if( !cam_thirdperson )
	{
		cam_thirdperson = 1; 
		
		cam_ofs[ YAW ] = viewangles[ YAW ]; 
		cam_ofs[ PITCH ] = viewangles[ PITCH ]; 
		cam_ofs[ 2 ] = CAM_MIN_DIST; 
	}

	gEngfuncs.Cvar_SetValue( "cam_command", 0 );
}

void CAM_ToFirstPerson(void) 
{ 
	cam_thirdperson = 0;
	
	gEngfuncs.Cvar_SetValue( "cam_command", 0 );
}

void CAM_ToggleSnapto( void )
{ 
	cam_snapto->value = !cam_snapto->value;
}

void CAM_Init( void )
{
	gEngfuncs.pfnAddCommand( "+campitchup", CAM_PitchUpDown );
	gEngfuncs.pfnAddCommand( "-campitchup", CAM_PitchUpUp );
	gEngfuncs.pfnAddCommand( "+campitchdown", CAM_PitchDownDown );
	gEngfuncs.pfnAddCommand( "-campitchdown", CAM_PitchDownUp );
	gEngfuncs.pfnAddCommand( "+camyawleft", CAM_YawLeftDown );
	gEngfuncs.pfnAddCommand( "-camyawleft", CAM_YawLeftUp );
	gEngfuncs.pfnAddCommand( "+camyawright", CAM_YawRightDown );
	gEngfuncs.pfnAddCommand( "-camyawright", CAM_YawRightUp );
	gEngfuncs.pfnAddCommand( "+camin", CAM_InDown );
	gEngfuncs.pfnAddCommand( "-camin", CAM_InUp );
	gEngfuncs.pfnAddCommand( "+camout", CAM_OutDown );
	gEngfuncs.pfnAddCommand( "-camout", CAM_OutUp );
	gEngfuncs.pfnAddCommand( "thirdperson", CAM_ToThirdPerson );
	gEngfuncs.pfnAddCommand( "firstperson", CAM_ToFirstPerson );
	gEngfuncs.pfnAddCommand( "+cammousemove",CAM_StartMouseMove);
	gEngfuncs.pfnAddCommand( "-cammousemove",CAM_EndMouseMove);
	gEngfuncs.pfnAddCommand( "+camdistance", CAM_StartDistance );
	gEngfuncs.pfnAddCommand( "-camdistance", CAM_EndDistance );
	gEngfuncs.pfnAddCommand( "snapto", CAM_ToggleSnapto );

	cam_command				= gEngfuncs.pfnRegisterVariable ( "cam_command", "0", 0 );	 // tells camera to go to thirdperson
	cam_snapto				= gEngfuncs.pfnRegisterVariable ( "cam_snapto", "0", 0 );	 // snap to thirdperson view
	cam_idealyaw			= gEngfuncs.pfnRegisterVariable ( "cam_idealyaw", "0", 0 );	 // ? yaw
	cam_idealpitch			= gEngfuncs.pfnRegisterVariable ( "cam_idealpitch", "0", 0 );	 // thirperson pitch
	cam_idealdist			= gEngfuncs.pfnRegisterVariable ( "cam_idealdist", "196", 0 );	 // thirdperson distance
	cam_contain				= gEngfuncs.pfnRegisterVariable ( "cam_contain", "0", 0 );	// contain camera to world

	c_maxpitch				= gEngfuncs.pfnRegisterVariable ( "c_maxpitch", "90.0", 0 );
	c_minpitch				= gEngfuncs.pfnRegisterVariable ( "c_minpitch", "0.0", 0 );
	c_maxyaw				= gEngfuncs.pfnRegisterVariable ( "c_maxyaw",   "135.0", 0 );
	c_minyaw				= gEngfuncs.pfnRegisterVariable ( "c_minyaw",   "-135.0", 0 );
	c_maxdistance			= gEngfuncs.pfnRegisterVariable ( "c_maxdistance",   "200.0", 0 );
	c_mindistance			= gEngfuncs.pfnRegisterVariable ( "c_mindistance",   "30.0", 0 );
}

void CAM_ClearStates( void )
{
	vec3_t viewangles;

	gEngfuncs.GetViewAngles( (float *)viewangles );

	cam_pitchup.state = 0;
	cam_pitchdown.state = 0;
	cam_yawleft.state = 0;
	cam_yawright.state = 0;
	cam_in.state = 0;
	cam_out.state = 0;

	cam_thirdperson = 0;
	cam_command->value = 0;
	cam_mousemove=0;

	cam_snapto->value = 0;
	cam_distancemove = 0;

	cam_ofs[ 0 ] = 0.0;
	cam_ofs[ 1 ] = 0.0;
	cam_ofs[ 2 ] = CAM_MIN_DIST;

	cam_idealpitch->value = viewangles[ PITCH ];
	cam_idealyaw->value = viewangles[ YAW ];
	cam_idealdist->value = CAM_MIN_DIST;
}

void CAM_StartMouseMove(void)
{
	float flSensitivity;
		
	//only move the cam with mouse if we are in third person.
	if (cam_thirdperson)
	{
		//set appropriate flags and initialize the old mouse position
		//variables for mouse camera movement
		if (!cam_mousemove)
		{
			cam_mousemove=1;
			iMouseInUse=1;
			SDL_GetCursorPos (&cam_mouse);

			if ( ( flSensitivity = gHUD.GetSensitivity() ) != 0 )
			{
				cam_old_mouse_x=cam_mouse.x*flSensitivity;
				cam_old_mouse_y=cam_mouse.y*flSensitivity;
			}
			else
			{
				cam_old_mouse_x=cam_mouse.x;
				cam_old_mouse_y=cam_mouse.y;
			}
		}
	}
	//we are not in 3rd person view..therefore do not allow camera movement
	else
	{   
		cam_mousemove=0;
		iMouseInUse=0;
	}
}

//the key has been released for camera movement
//tell the engine that mouse camera movement is off
void CAM_EndMouseMove(void)
{
   cam_mousemove=0;
   iMouseInUse=0;
}


//----------------------------------------------------------
//routines to start the process of moving the cam in or out 
//using the mouse
//----------------------------------------------------------
void CAM_StartDistance(void)
{
	//only move the cam with mouse if we are in third person.
	if (cam_thirdperson)
	{
	  //set appropriate flags and initialize the old mouse position
	  //variables for mouse camera movement
	  if (!cam_distancemove)
	  {
		  cam_distancemove=1;
		  cam_mousemove=1;
		  iMouseInUse=1;
		  SDL_GetCursorPos (&cam_mouse);
		  cam_old_mouse_x=cam_mouse.x*gHUD.GetSensitivity();
		  cam_old_mouse_y=cam_mouse.y*gHUD.GetSensitivity();
	  }
	}
	//we are not in 3rd person view..therefore do not allow camera movement
	else
	{   
		cam_distancemove=0;
		cam_mousemove=0;
		iMouseInUse=0;
	}
}

//the key has been released for camera movement
//tell the engine that mouse camera movement is off
void CAM_EndDistance(void)
{
   cam_distancemove=0;
   cam_mousemove=0;
   iMouseInUse=0;
}

int EXPORT CL_IsThirdPerson( void )
{
	return cam_thirdperson ? 1 : 0;
}

void EXPORT CL_CameraOffset( float *ofs )
{
	VectorCopy( cam_ofs, ofs );
}
