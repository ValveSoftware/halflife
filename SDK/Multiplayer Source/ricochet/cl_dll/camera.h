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
// Camera.h  --  defines and such for a 3rd person camera
// NOTE: must include quakedef.h first

#ifndef _CAMERA_H_
#define _CAMEA_H_

// pitch, yaw, dist
extern vec3_t cam_ofs;
// Using third person camera
extern int cam_thirdperson;

void CAM_Init( void );
void CAM_ClearStates( void );
void CAM_StartMouseMove(void);
void CAM_EndMouseMove(void);

#endif		// _CAMERA_H_
