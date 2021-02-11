//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

// Camera.h  --  defines and such for a 3rd person camera
// NOTE: must include quakedef.h first

#ifndef _CAMERA_H_
#define _CAMERA_H_

// pitch, yaw, dist
extern vec3_t cam_ofs;
// Using third person camera
extern int cam_thirdperson;

void CAM_Init( void );
void CAM_ClearStates( void );
void CAM_StartMouseMove(void);
void CAM_EndMouseMove(void);

#endif		// _CAMERA_H_
