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

#ifndef	ACTIVITY_H
#define	ACTIVITY_H


typedef enum {
	ACT_RESET = 0,		// Set m_Activity to this invalid value to force a reset to m_IdealActivity
	ACT_IDLE,
	ACT_HOP,
	ACT_HOP_LEFT_FOOT,
	ACT_LEAP,

	ACT_TURN_LEFT,
	ACT_TURN_RIGHT,

	ACT_BASE_STAND,
	ACT_BASE_STAND_THROW,
	ACT_FREEZE_STAND,
	ACT_FREEZE_STAND_THROW,
	ACT_HARD_STAND,
	ACT_HARD_STAND_THROW,
	ACT_TRIPLE_STAND,
	ACT_TRIPLE_STAND_THROW,

	ACT_UNARMED_WALK,
	ACT_UNARMED_RUN,
	ACT_UNARMED_BACKPEDAL,

	ACT_BASE_WALK,
	ACT_BASE_RUN,
	ACT_BASE_THROW,
	ACT_BASE_BACKUP,
	ACT_BASE_BACKUP_THROW,

	ACT_BASE_REVERSE,
	ACT_BASE_REVERSE_THROW,

	ACT_FALL,
	ACT_FALL_FORWARD,
	ACT_FALL_BACKWARD,
	ACT_FALL_LEFT,
	ACT_FALL_RIGHT,

	ACT_FLINCH_CLOCKWISE,
	ACT_FLINCH_COUNTERCLOCKWISE,
	ACT_FLINCH_BACK,
	ACT_FLINCH_LEFT,
	ACT_FLINCH_RIGHT,
	ACT_FLINCH_FORWARD,

	ACT_DIE_HEADSHOT,
	ACT_DIEFORWARD,
	ACT_DIEBACKWARD,
} Activity;


typedef struct {
	int	type;
	char *name;
} activity_map_t;

extern activity_map_t activity_map[];


#endif	//ACTIVITY_H
