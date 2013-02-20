/***
*
*	Copyright (c) 1999, Valve LLC. All rights reserved.
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

/*
==========================
This file contains "stubs" of class member implementations so that we can predict certain
 weapons client side.  From time to time you might find that you need to implement part of the
 these functions.  If so, cut it from here, paste it in hl_weapons.cpp or somewhere else and
 add in the functionality you need.
==========================
*/
#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"player.h"
#include	"weapons.h"
#include	"nodes.h"

// Globals used by game logic
const Vector g_vecZero = Vector( 0, 0, 0 );
int gmsgWeapPickup = 0;
enginefuncs_t g_engfuncs;
globalvars_t  *gpGlobals;

ItemInfo CBasePlayerItem::ItemInfoArray[MAX_WEAPONS];

void EMIT_SOUND_DYN(edict_t *entity, int channel, const char *sample, float volume, float attenuation, int flags, int pitch) { }

// CBaseEntity Stubs
int CBaseEntity :: TakeHealth( float flHealth, int bitsDamageType ) { return 1; }
int CBaseEntity :: TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType ) { return 1; }
CBaseEntity *CBaseEntity::GetNextTarget( void ) { return NULL; }
int CBaseEntity::Save( CSave &save ) { return 1; }
int CBaseEntity::Restore( CRestore &restore ) { return 1; }
void CBaseEntity::SetObjectCollisionBox( void ) { }
int	CBaseEntity :: Intersects( CBaseEntity *pOther ) { return 0; }
void CBaseEntity :: MakeDormant( void ) { }
int CBaseEntity :: IsDormant( void ) { return 0; }
BOOL CBaseEntity :: IsInWorld( void ) { return TRUE; }
int CBaseEntity::ShouldToggle( USE_TYPE useType, BOOL currentState ) { return 0; }
int	CBaseEntity :: DamageDecal( int bitsDamageType ) { return -1; }
CBaseEntity * CBaseEntity::Create( char *szName, const Vector &vecOrigin, const Vector &vecAngles, edict_t *pentOwner ) { return NULL; }
void CBaseEntity::SUB_Remove( void ) { }

// CBaseDelay Stubs
void CBaseDelay :: KeyValue( struct KeyValueData_s * ) { }
int CBaseDelay::Restore( class CRestore & ) { return 1; }
int CBaseDelay::Save( class CSave & ) { return 1; }

// CBaseAnimating Stubs
int CBaseAnimating::Restore( class CRestore & ) { return 1; }
int CBaseAnimating::Save( class CSave & ) { return 1; }

// DEBUG Stubs
edict_t *DBG_EntOfVars( const entvars_t *pev ) { return NULL; }
void DBG_AssertFunction(BOOL fExpr,	const char*	szExpr,	const char*	szFile,	int szLine,	const char*	szMessage) { }

// UTIL_* Stubs
void UTIL_PrecacheOther( const char *szClassname ) { }
void UTIL_BloodDrips( const Vector &origin, const Vector &direction, int color, int amount ) { }
void UTIL_DecalTrace( TraceResult *pTrace, int decalNumber ) { }
void UTIL_GunshotDecalTrace( TraceResult *pTrace, int decalNumber ) { }
BOOL UTIL_IsValidEntity( edict_t *pent ) { return TRUE; }
void UTIL_SetOrigin( entvars_t *, const Vector &org ) { }
BOOL UTIL_GetNextBestWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon ) { return TRUE; }
void UTIL_LogPrintf(char *,...) { }
void UTIL_ClientPrintAll( int,char const *,char const *,char const *,char const *,char const *) { }
void ClientPrint( entvars_t *client, int msg_dest, const char *msg_name, const char *param1, const char *param2, const char *param3, const char *param4 ) { }

// CBaseToggle Stubs
int CBaseToggle::Restore( class CRestore & ) { return 1; }
int CBaseToggle::Save( class CSave & ) { return 1; }
void CBaseToggle :: KeyValue( struct KeyValueData_s * ) { }

// CGrenade Stubs
void CGrenade::BounceSound( void ) { }
void CGrenade::Explode( Vector, Vector ) { }
void CGrenade::Explode( TraceResult *, int ) { }
void CGrenade::Killed( entvars_t *, int ) { }
void CGrenade::Spawn( void ) { }

CBaseEntity* CBaseMonster :: CheckTraceHullAttack( float flDist, int iDamage, int iDmgType ) { return NULL; }
void CBaseMonster :: Look ( int iDistance ) { }
float CBaseAnimating :: StudioFrameAdvance ( float flInterval ) { return 0.0; }
int CBaseMonster::IRelationship ( CBaseEntity *pTarget ) { return 0; }
CBaseEntity *CBaseMonster :: BestVisibleEnemy ( void ) { return NULL; }
BOOL CBaseMonster :: FInViewCone ( CBaseEntity *pEntity ) { return FALSE; }
BOOL CBaseMonster :: FInViewCone ( Vector *pOrigin ) { return FALSE; }
BOOL CBaseEntity :: FVisible ( CBaseEntity *pEntity ) { return FALSE; }
BOOL CBaseEntity :: FVisible ( const Vector &vecOrigin ) { return FALSE; }
void CBaseMonster :: MakeIdealYaw( Vector vecTarget ) { }
float CBaseMonster::ChangeYaw ( int yawSpeed ) { return 0; }
int CBaseAnimating :: LookupActivity ( int activity ) { return 0; }
int CBaseAnimating :: LookupActivityHeaviest ( int activity ) { return 0; }

BOOL CBaseAnimating :: GetSequenceFlags( ) { return FALSE; }
void CBaseAnimating :: DispatchAnimEvents ( float flInterval ) { }
float CBaseAnimating :: SetBoneController ( int iController, float flValue ) { return 0.0; }
void CBaseAnimating :: InitBoneControllers ( void ) { }
float CBaseAnimating :: SetBlending ( int iBlender, float flValue ) { return 0; }
void CBaseAnimating :: GetBonePosition ( int iBone, Vector &origin, Vector &angles ) { }
void CBaseAnimating :: GetAttachment ( int iAttachment, Vector &origin, Vector &angles ) { }
int CBaseAnimating :: FindTransition( int iEndingSequence, int iGoalSequence, int *piDir ) { return -1; }
void CBaseAnimating :: GetAutomovement( Vector &origin, Vector &angles, float flInterval ) { }
void CBaseAnimating :: SetBodygroup( int iGroup, int iValue ) { }
int CBaseAnimating :: GetBodygroup( int iGroup ) { return 0; }
void CBaseEntity::TraceAttack(entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType) { }
void CBaseEntity::FireBullets(ULONG cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t *pevAttacker ) { }
void CBaseEntity :: TraceBleed( float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType ) { }
void CBaseMonster :: MakeDamageBloodDecal ( int cCount, float flNoise, TraceResult *ptr, const Vector &vecDir ) { }
void CBaseMonster::ReportAIState( void ) { }
void CBaseMonster :: KeyValue( KeyValueData *pkvd ) { }
BOOL CBaseMonster :: FCheckAITrigger ( void ) { return FALSE; }
void CBaseMonster::CorpseFallThink( void ) { }
void CBaseMonster :: MonsterInitDead( void ) { }
void CBaseMonster :: TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType) { }
BOOL CBaseMonster :: ShouldFadeOnDeath( void ) { return FALSE; }
void CBaseMonster :: RadiusDamage(entvars_t* pevInflictor, entvars_t*	pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType ) { }
void CBaseMonster :: RadiusDamage( Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType ) { }
void CBaseMonster::FadeMonster( void ) { }
void CBaseMonster :: GibMonster( void ) { }
BOOL CBaseMonster :: HasHumanGibs( void ) { return FALSE; }
BOOL CBaseMonster :: HasAlienGibs( void ) { return FALSE; }
Activity CBaseMonster :: GetDeathActivity ( void ) { return (Activity)0; }
void CBaseMonster::BecomeDead( void ) {}
void CBaseMonster :: Killed( entvars_t *pevAttacker, int iGib ) {}
int CBaseMonster :: TakeHealth (float flHealth, int bitsDamageType) { return 0; }
int CBaseMonster :: TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType ) { return 0; }

int TrainSpeed(int iSpeed, int iMax) { 	return 0; }
void CBasePlayer :: DeathSound( void ) { }
int CBasePlayer :: TakeHealth( float flHealth, int bitsDamageType ) { return 0; }
void CBasePlayer :: TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType) { }
int CBasePlayer :: TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType ) { return 0; }
void CBasePlayer::RemoveAllItems( BOOL removeSuit ) { }
void CBasePlayer::WaterMove() { }
BOOL CBasePlayer::IsOnLadder( void ) { return FALSE; }
void CBasePlayer::PlayerDeathThink(void) { }
void CBasePlayer::StartDeathCam( void ) { }
void CBasePlayer::StartObserver( Vector vecPosition, Vector vecViewAngle ) { }
void CBasePlayer::PlayerUse ( void ) { }
void CBasePlayer::Jump() { }
void CBasePlayer::Duck( ) { }
int  CBasePlayer::Classify ( void ) { return 0; }
void CBasePlayer :: PlayStepSound(int step, float fvol) { }	
void CBasePlayer :: UpdateStepSound( void ) { }
void CBasePlayer::PreThink(void) { }
void CBasePlayer::CheckTimeBasedDamage()  { }
void CBasePlayer :: UpdateGeigerCounter( void ) { }
void CBasePlayer::CheckSuitUpdate() { }
void CBasePlayer::SetSuitUpdate(char *name, int fgroup, int iNoRepeatTime) { }
void CBasePlayer :: UpdatePlayerSound ( void ) { }
void CBasePlayer :: Precache( void ) { }
int CBasePlayer::Save( CSave &save ) { return 0; }
void CBasePlayer::RenewItems(void) { }
int CBasePlayer::Restore( CRestore &restore ) { return 0; }
void CBasePlayer::SelectNextItem( int iItem ) { }
BOOL CBasePlayer::HasWeapons( void ) { return FALSE; }
void CBasePlayer::SelectPrevItem( int iItem ) { }
CBaseEntity *FindEntityForward( CBaseEntity *pMe ) { return NULL; }
BOOL CBasePlayer :: FlashlightIsOn( void ) { return FALSE; }
void CBasePlayer :: FlashlightTurnOn( void ) { }
void CBasePlayer :: FlashlightTurnOff( void ) { }
void CBasePlayer :: ForceClientDllUpdate( void ) { }
void CBasePlayer::ImpulseCommands( ) { }
void CBasePlayer::CheatImpulseCommands( int iImpulse ) { }
int CBasePlayer::AddPlayerItem( CBasePlayerItem *pItem ) { return FALSE; }
int CBasePlayer::RemovePlayerItem( CBasePlayerItem *pItem ) { return FALSE; }
void CBasePlayer::ItemPreFrame() { }
void CBasePlayer::ItemPostFrame() { }
int CBasePlayer::AmmoInventory( int iAmmoIndex ) { return -1; }
int CBasePlayer::GetAmmoIndex(const char *psz) { return -1; }
void CBasePlayer::SendAmmoUpdate(void) { }
void CBasePlayer :: UpdateClientData( void ) { }
BOOL CBasePlayer :: FBecomeProne ( void ) { return TRUE; }
void CBasePlayer :: BarnacleVictimBitten ( entvars_t *pevBarnacle ) { }
void CBasePlayer :: BarnacleVictimReleased ( void ) { }
int CBasePlayer :: Illumination( void ) { return 0; }
void CBasePlayer :: EnableControl(BOOL fControl) { }
Vector CBasePlayer :: GetAutoaimVector( float flDelta ) { return g_vecZero; }
Vector CBasePlayer :: AutoaimDeflection( Vector &vecSrc, float flDist, float flDelta  ) { return g_vecZero; }
void CBasePlayer :: ResetAutoaim( ) { }
void CBasePlayer :: SetCustomDecalFrames( int nFrames ) { }
int CBasePlayer :: GetCustomDecalFrames( void ) { return -1; }
void CBasePlayer::DropPlayerItem ( char *pszItemName ) { }
BOOL CBasePlayer::HasPlayerItem( CBasePlayerItem *pCheckItem ) { return FALSE; }
BOOL CBasePlayer :: SwitchWeapon( CBasePlayerItem *pWeapon )  { return FALSE; }
Vector CBasePlayer :: GetGunPosition( void ) { return g_vecZero; }
const char *CBasePlayer::TeamID( void ) { return ""; }
int CBasePlayer :: GiveAmmo( int iCount, char *szName, int iMax ) { return 0; }
void CBasePlayer::AddPoints( int score, BOOL bAllowNegativeScore ) { } 
void CBasePlayer::AddPointsToTeam( int score, BOOL bAllowNegativeScore ) { } 
void CBasePlayer::RemoveAllPowerups( void ) {}
bool CBasePlayer::HasPowerup(int) { return 0; }

void ClearMultiDamage(void) { }
void ApplyMultiDamage(entvars_t *pevInflictor, entvars_t *pevAttacker ) { }
void AddMultiDamage( entvars_t *pevInflictor, CBaseEntity *pEntity, float flDamage, int bitsDamageType) { }
void SpawnBlood(Vector vecSpot, int bloodColor, float flDamage) { }
int DamageDecal( CBaseEntity *pEntity, int bitsDamageType ) { return 0; }
void DecalGunshot( TraceResult *pTrace, int iBulletType ) { }
void EjectBrass ( const Vector &vecOrigin, const Vector &vecVelocity, float rotation, int model, int soundtype ) { }
void AddAmmoNameToAmmoRegistry( const char *szAmmoname ) { }
int CBasePlayerItem::Restore( class CRestore & ) { return 1; }
int CBasePlayerItem::Save( class CSave & ) { return 1; }
int CBasePlayerWeapon::Restore( class CRestore & ) { return 1; }
int CBasePlayerWeapon::Save( class CSave & ) { return 1; }
void CBasePlayerItem :: SetObjectCollisionBox( void ) { }
void CBasePlayerItem :: FallInit( void ) { }
void CBasePlayerItem::FallThink ( void ) { }
void CBasePlayerItem::Materialize( void ) { }
void CBasePlayerItem::AttemptToMaterialize( void ) { }
void CBasePlayerItem :: CheckRespawn ( void ) { }
CBaseEntity* CBasePlayerItem::Respawn( void ) { return NULL; }
void CBasePlayerItem::DefaultTouch( CBaseEntity *pOther ) { }
void CBasePlayerItem::DestroyItem( void ) { }
int CBasePlayerItem::AddToPlayer( CBasePlayer *pPlayer ) { return TRUE; }
void CBasePlayerItem::Drop( void ) { }
void CBasePlayerItem::Kill( void ) { }
void CBasePlayerItem::Holster( int skiplocal ) { }
void CBasePlayerItem::AttachToPlayer ( CBasePlayer *pPlayer ) { }
int CBasePlayerWeapon::AddDuplicate( CBasePlayerItem *pOriginal ) { return 0; }
int CBasePlayerWeapon::AddToPlayer( CBasePlayer *pPlayer ) { return FALSE; }
int CBasePlayerWeapon::UpdateClientData( CBasePlayer *pPlayer ) { return 0; }
BOOL CBasePlayerWeapon :: AddPrimaryAmmo( int iCount, char *szName, int iMaxClip, int iMaxCarry ) { return TRUE; }
BOOL CBasePlayerWeapon :: AddSecondaryAmmo( int iCount, char *szName, int iMax ) { return TRUE; }
BOOL CBasePlayerWeapon :: IsUseable( void ) { return TRUE; }
int CBasePlayerWeapon::PrimaryAmmoIndex( void ) { return -1; }
int CBasePlayerWeapon::SecondaryAmmoIndex( void ) {	return -1; }
void CBasePlayerAmmo::Spawn( void ) { }
CBaseEntity* CBasePlayerAmmo::Respawn( void ) { return this; }
void CBasePlayerAmmo::Materialize( void ) { }
void CBasePlayerAmmo :: DefaultTouch( CBaseEntity *pOther ) { }
int CBasePlayerWeapon::ExtractAmmo( CBasePlayerWeapon *pWeapon ) { return 0; }
int CBasePlayerWeapon::ExtractClipAmmo( CBasePlayerWeapon *pWeapon ) { return 0; }	
void CBasePlayerWeapon::RetireWeapon( void ) { }