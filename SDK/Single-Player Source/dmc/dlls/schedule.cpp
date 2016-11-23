/***
*
*	Copyright (c) 1996-2002,, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
//=========================================================
// schedule.cpp - functions and data pertaining to the 
// monsters' AI scheduling system.
//=========================================================
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "animation.h"
#include "scripted.h"
#include "nodes.h"
#include "defaultai.h"

extern CGraph WorldGraph;

BOOL CBaseMonster :: FHaveSchedule( void ) { return FALSE; };
void CBaseMonster :: ClearSchedule( void ) { };
BOOL CBaseMonster :: FScheduleDone ( void ) { return FALSE; };
void CBaseMonster :: ChangeSchedule ( Schedule_t *pNewSchedule ) { };
void CBaseMonster :: NextScheduledTask ( void ) { };
int CBaseMonster :: IScheduleFlags ( void ) { return 0; };
BOOL CBaseMonster :: FScheduleValid ( void ) { return FALSE; };
void CBaseMonster :: MaintainSchedule ( void ) { };
void CBaseMonster :: SetTurnActivity ( void ) { };
Task_t	*CBaseMonster :: GetTask ( void ) { return NULL; };
