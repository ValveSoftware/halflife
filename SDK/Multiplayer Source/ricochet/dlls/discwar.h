//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: Header for Discwar
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================

#ifndef DISCWAR_H
#define DISCWAR_H
#pragma once

#define WEAPON_DISC					1
#define MAX_DISCS					3				// Max number of discs a player can carry
#define STARTING_DISCS				MAX_DISCS		// Number of discs a player starts with
#define NUM_FASTSHOT_DISCS			3				// Number of discs a player gets with the fastshot powerup per normal disc

#define DISC_VELOCITY				1000			// Velocity multiplier for discs when thrown
#define DISC_PUSH_MULTIPLIER		1200			// Velocity multiplier used to push a player when hit by a disc

//#define DISC_POWERUP_TIME			5				// Time (in seconds) a powerup lasts for
#define DISC_POWERUP_RESPAWN_TIME	10				// Time (in seconds) it takes after a powerup is picked up before the next one appears

#define MAX_SCORE_TIME_AFTER_HIT	4.0				// Time (in seconds) in which a player gets a point if the enemy dies within this time
													// after being hit by a disc.

// Powerups
#define	POW_TRIPLE					(1<<0)
#define	POW_FAST					(1<<1)
#define	POW_HARD					(1<<2)
#define	POW_FREEZE					(1<<3)

#define POW_VISUALIZE_REBOUNDS		(1<<4)			// Removing this one for now

#define	NUM_POWERUPS				4				// 4, not 5, because VISUALIZE_REBOUNDS is removed.

#define FREEZE_TIME					7
#define	FREEZE_SPEED				50

// Rewards
#define	REWARD_BOUNCE_NONE			(1<<1)
#define	REWARD_BOUNCE_ONE			(1<<2)
#define	REWARD_BOUNCE_TWO			(1<<3)
#define	REWARD_BOUNCE_THREE			(1<<4)
#define	REWARD_DECAPITATE			(1<<5)
#define	REWARD_TELEPORT				(1<<6)
#define	REWARD_DOUBLEKILL			(1<<7)

#endif // DISCWAR_H

