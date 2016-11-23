/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
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
/*

===== monsters.cpp ========================================================

  Monster-related utility code

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "nodes.h"
#include "monsters.h"
 
CBaseEntity* CBaseMonster :: CheckTraceHullAttack( float flDist, int iDamage, int iDmgType ) { return NULL; }
void CBaseMonster :: Eat ( float flFullDuration ) { }
BOOL CBaseMonster :: FShouldEat ( void ) { return TRUE; }
void CBaseMonster :: BarnacleVictimBitten ( entvars_t *pevBarnacle ) { }
void CBaseMonster :: BarnacleVictimReleased ( void ) { }
void CBaseMonster :: Listen ( void ) { }
float CBaseMonster :: FLSoundVolume ( CSound *pSound ) { return 0.0; }
BOOL CBaseMonster :: FValidateHintType ( short sHint ) { return FALSE; }
void CBaseMonster :: Look ( int iDistance ) { }
int CBaseMonster :: ISoundMask ( void ) { return 0; }
CSound* CBaseMonster :: PBestSound ( void ) { return NULL; }
CSound* CBaseMonster :: PBestScent ( void ) { return NULL; } 
void CBaseMonster :: MonsterThink ( void ) { }
void CBaseMonster :: MonsterUse ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value ) { }
int CBaseMonster :: IgnoreConditions ( void ) { return 0; }
void CBaseMonster :: RouteClear ( void ) { }
void CBaseMonster :: RouteNew ( void ) { }
BOOL CBaseMonster :: FRouteClear ( void ) { return FALSE; }
BOOL CBaseMonster :: FRefreshRoute ( void ) { return 0; }
BOOL CBaseMonster::MoveToEnemy( Activity movementAct, float waitTime ) { return FALSE; }
BOOL CBaseMonster::MoveToLocation( Activity movementAct, float waitTime, const Vector &goal ) { return FALSE; }
BOOL CBaseMonster::MoveToTarget( Activity movementAct, float waitTime ) { return FALSE; }
BOOL CBaseMonster::MoveToNode( Activity movementAct, float waitTime, const Vector &goal ) { return FALSE; }
int ShouldSimplify( int routeType ) { return TRUE; }
void CBaseMonster :: RouteSimplify( CBaseEntity *pTargetEnt ) { }
BOOL CBaseMonster :: FBecomeProne ( void ) { return TRUE; }
BOOL CBaseMonster :: CheckRangeAttack1 ( float flDot, float flDist ) { return FALSE; }
BOOL CBaseMonster :: CheckRangeAttack2 ( float flDot, float flDist ) { return FALSE; }
BOOL CBaseMonster :: CheckMeleeAttack1 ( float flDot, float flDist ) { return FALSE; }
BOOL CBaseMonster :: CheckMeleeAttack2 ( float flDot, float flDist ) { return FALSE; }
void CBaseMonster :: CheckAttacks ( CBaseEntity *pTarget, float flDist ) { }
BOOL CBaseMonster :: FCanCheckAttacks ( void ) { return FALSE; }
int CBaseMonster :: CheckEnemy ( CBaseEntity *pEnemy ) { return 0; }
void CBaseMonster :: PushEnemy( CBaseEntity *pEnemy, Vector &vecLastKnownPos ) { }
BOOL CBaseMonster :: PopEnemy( ) { return FALSE; }
void CBaseMonster :: SetActivity ( Activity NewActivity ) { }
void CBaseMonster :: SetSequenceByName ( char *szSequence ) { }
int CBaseMonster :: CheckLocalMove ( const Vector &vecStart, const Vector &vecEnd, CBaseEntity *pTarget, float *pflDist ) { return 0; }
float CBaseMonster :: OpenDoorAndWait( entvars_t *pevDoor ) { return 0.0; }
void CBaseMonster :: AdvanceRoute ( float distance ) { }
int CBaseMonster :: RouteClassify( int iMoveFlag ) { return 0; }
BOOL CBaseMonster :: BuildRoute ( const Vector &vecGoal, int iMoveFlag, CBaseEntity *pTarget ) { return FALSE; }
void CBaseMonster :: InsertWaypoint ( Vector vecLocation, int afMoveFlags ) { }
BOOL CBaseMonster :: FTriangulate ( const Vector &vecStart , const Vector &vecEnd, float flDist, CBaseEntity *pTargetEnt, Vector *pApex ) { return FALSE; }
void CBaseMonster :: Move ( float flInterval ) { }
BOOL CBaseMonster:: ShouldAdvanceRoute( float flWaypointDist ) { return FALSE; }
void CBaseMonster::MoveExecute( CBaseEntity *pTargetEnt, const Vector &vecDir, float flInterval ) { }
void CBaseMonster :: MonsterInit ( void ) { }
void CBaseMonster :: MonsterInitThink ( void ) { }
void CBaseMonster :: StartMonster ( void ) { }
void CBaseMonster :: MovementComplete( void ) { }
int CBaseMonster::TaskIsRunning( void ) { return 0; }
int CBaseMonster::IRelationship ( CBaseEntity *pTarget ) { return 0; }
BOOL CBaseMonster :: FindCover ( Vector vecThreat, Vector vecViewOffset, float flMinDist, float flMaxDist ) { return FALSE; }
BOOL CBaseMonster :: BuildNearestRoute ( Vector vecThreat, Vector vecViewOffset, float flMinDist, float flMaxDist ) { return FALSE; }
CBaseEntity *CBaseMonster :: BestVisibleEnemy ( void ) { return NULL; }
BOOL CBaseMonster :: FInViewCone ( CBaseEntity *pEntity ) { return FALSE; }
BOOL CBaseMonster :: FInViewCone ( Vector *pOrigin ) { return FALSE; }
BOOL CBaseEntity :: FVisible ( CBaseEntity *pEntity ) { return FALSE; }
BOOL CBaseEntity :: FVisible ( const Vector &vecOrigin ) { return FALSE; }
void CBaseMonster :: MakeIdealYaw( Vector vecTarget ) { }
float	CBaseMonster::FlYawDiff ( void ) { return 0.0; }
float CBaseMonster::ChangeYaw ( int yawSpeed ) { return 0; }
float	CBaseMonster::VecToYaw ( Vector vecDir ) { return 0.0; }
void CBaseMonster :: SetEyePosition ( void ) { }
void CBaseMonster :: HandleAnimEvent( MonsterEvent_t *pEvent ) { }
Vector CBaseMonster :: GetGunPosition( void ) { return g_vecZero; }
void CBaseEntity::TraceAttack(entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType) { }
void CBaseEntity::FireBullets(ULONG cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t *pevAttacker ) { }
BOOL CBaseMonster :: FGetNodeRoute ( Vector vecDest ) { return TRUE; }
int CBaseMonster :: FindHintNode ( void ) { return NO_NODE; }
void CBaseMonster::ReportAIState( void ) { }
void CBaseMonster :: KeyValue( KeyValueData *pkvd ) { }
BOOL CBaseMonster :: FCheckAITrigger ( void ) { return FALSE; }
int CBaseMonster :: CanPlaySequence( BOOL fDisregardMonsterState, int interruptLevel ) { return FALSE; }
BOOL CBaseMonster :: FindLateralCover ( const Vector &vecThreat, const Vector &vecViewOffset ) { return FALSE; }
Vector CBaseMonster :: ShootAtEnemy( const Vector &shootOrigin ) { return g_vecZero; }
BOOL CBaseMonster :: FacingIdeal( void ) { return FALSE; }
BOOL CBaseMonster :: FCanActiveIdle ( void ) { return FALSE; }
void CBaseMonster::PlaySentence( const char *pszSentence, float duration, float volume, float attenuation ) { }
void CBaseMonster::PlayScriptedSentence( const char *pszSentence, float duration, float volume, float attenuation, BOOL bConcurrent, CBaseEntity *pListener ) { }
void CBaseMonster::SentenceStop( void ) { }
void CBaseMonster::CorpseFallThink( void ) { }
void CBaseMonster :: MonsterInitDead( void ) { }
BOOL CBaseMonster :: BBoxFlat ( void ) { return TRUE; }
BOOL CBaseMonster :: GetEnemy ( void ) { return FALSE; }
void CBaseMonster :: TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType) { }
CBaseEntity* CBaseMonster :: DropItem ( char *pszItemName, const Vector &vecPos, const Vector &vecAng ) { return NULL; }
BOOL CBaseMonster :: ShouldFadeOnDeath( void ) { return FALSE; }
void CBaseMonster::FadeMonster( void ) { }
BOOL CBaseMonster :: HasHumanGibs( void ) { return TRUE; }
BOOL CBaseMonster :: HasAlienGibs( void ) { return FALSE; }
MONSTERSTATE CBaseMonster :: GetIdealState ( void ) { return MONSTERSTATE_ALERT; }
Schedule_t* CBaseMonster :: GetScheduleOfType ( int Type ) { return NULL; }
Schedule_t *CBaseMonster :: GetSchedule ( void ) { return NULL; }
void CBaseMonster :: RunTask ( Task_t *pTask ) { }
void CBaseMonster :: StartTask ( Task_t *pTask ) { }
Schedule_t *CBaseMonster::ScheduleFromName( const char *pName ) { return NULL;}
void CBaseMonster::BecomeDead( void ) {}
void CBaseMonster :: RunAI ( void ) {}
void CBaseMonster :: Killed( entvars_t *pevAttacker, int iGib ) {}
int CBaseMonster :: TakeHealth (float flHealth, int bitsDamageType) { return 0; }
int CBaseMonster::Restore( class CRestore & ) { return 1; }
int CBaseMonster::Save( class CSave & ) { return 1; }


