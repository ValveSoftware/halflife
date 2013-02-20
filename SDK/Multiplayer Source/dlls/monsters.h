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
#ifndef MONSTERS_H
#include "skill.h"
#define MONSTERS_H

/*

===== monsters.h ========================================================

  Header file for monster-related utility code

*/

// Hit Group standards
#define	HITGROUP_GENERIC	0
#define	HITGROUP_HEAD		1
#define	HITGROUP_CHEST		2
#define	HITGROUP_STOMACH	3
#define HITGROUP_LEFTARM	4	
#define HITGROUP_RIGHTARM	5
#define HITGROUP_LEFTLEG	6
#define HITGROUP_RIGHTLEG	7


// spawn flags 256 and above are already taken by the engine
extern void UTIL_MoveToOrigin( edict_t* pent, const Vector &vecGoal, float flDist, int iMoveType ); 

Vector VecCheckToss ( entvars_t *pev, const Vector &vecSpot1, Vector vecSpot2, float flGravityAdj = 1.0 );
Vector VecCheckThrow ( entvars_t *pev, const Vector &vecSpot1, Vector vecSpot2, float flSpeed, float flGravityAdj = 1.0 );
extern DLL_GLOBAL Vector		g_vecAttackDir;
extern DLL_GLOBAL CONSTANT float g_flMeleeRange;
extern DLL_GLOBAL CONSTANT float g_flMediumRange;
extern DLL_GLOBAL CONSTANT float g_flLongRange;
extern void EjectBrass (const Vector &vecOrigin, const Vector &vecVelocity, float rotation, int model, int soundtype );
extern void ExplodeModel( const Vector &vecOrigin, float speed, int model, int count );

BOOL FBoxVisible ( entvars_t *pevLooker, entvars_t *pevTarget );
BOOL FBoxVisible ( entvars_t *pevLooker, entvars_t *pevTarget, Vector &vecTargetOrigin, float flSize = 0.0 );

// monster to monster relationship types
#define R_AL	-2 // (ALLY) pals. Good alternative to R_NO when applicable.
#define R_FR	-1// (FEAR)will run
#define	R_NO	0// (NO RELATIONSHIP) disregard
#define R_DL	1// (DISLIKE) will attack
#define R_HT	2// (HATE)will attack this character instead of any visible DISLIKEd characters
#define R_NM	3// (NEMESIS)  A monster Will ALWAYS attack its nemsis, no matter what


#define bits_MEMORY_KILLED				( 1 << 7 )// HACKHACK -- remember that I've already called my Killed()

//
// A gib is a chunk of a body, or a piece of wood/metal/rocks/etc.
//
class CGib : public CBaseEntity
{
public:
	void Spawn( const char *szGibModel );
	void EXPORT BounceGibTouch ( CBaseEntity *pOther );
	void EXPORT StickyGibTouch ( CBaseEntity *pOther );
	void EXPORT WaitTillLand( void );
	void		LimitVelocity( void );

	virtual int	ObjectCaps( void ) { return (CBaseEntity :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_DONT_SAVE; }
	static	void SpawnHeadGib( entvars_t *pevVictim );
	static	void SpawnRandomGibs( entvars_t *pevVictim, int cGibs, int human );
	static  void SpawnStickyGibs( entvars_t *pevVictim, Vector vecOrigin, int cGibs );

	int		m_bloodColor;
	int		m_cBloodDecals;
	int		m_material;
	float	m_lifeTime;
};


#endif	//MONSTERS_H
