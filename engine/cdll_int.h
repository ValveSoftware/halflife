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
//
//  cdll_int.h
//
// 4-23-98  
// JOHN:  client dll interface declarations
//

#ifndef CDLL_INT_H
#define CDLL_INT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "const.h"
#include "steam/steamtypes.h"
#include "ref_params.h"
#include "r_efx.h"
#include "studio_event.h"

// this file is included by both the engine and the client-dll,
// so make sure engine declarations aren't done twice

typedef int HSPRITE;	// handle to a graphic

#define SCRINFO_SCREENFLASH 1
#define SCRINFO_STRETCHED	2

typedef struct SCREENINFO_s
{
	int		iSize;
	int		iWidth;
	int		iHeight;
	int		iFlags;
	int		iCharHeight;
	short	charWidths[256];
} SCREENINFO;


typedef struct client_data_s
{
	// fields that cannot be modified  (ie. have no effect if changed)
	vec3_t origin;

	// fields that can be changed by the cldll
	vec3_t viewangles;
	int		iWeaponBits;
//	int		iAccessoryBits;
	float	fov;	// field of view
} client_data_t;

typedef struct client_sprite_s
{
	char szName[64];
	char szSprite[64];
	int hspr;
	int iRes;
	wrect_t rc;
} client_sprite_t;



typedef struct hud_player_info_s
{
	char *name;
	short ping;
	byte thisplayer;  // TRUE if this is the calling player

	byte spectator;
	byte packetloss;

	char *model;
	short topcolor;
	short bottomcolor;

	uint64 m_nSteamID;
} hud_player_info_t;



typedef struct module_s
{
	unsigned char				ucMD5Hash[16];	// hash over code
	qboolean					fLoaded;		// true if successfully loaded
} module_t;



	
		

#ifndef IN_BUTTONS_H
#include "in_buttons.h"
#endif

#define CLDLL_INTERFACE_VERSION		7

extern void LoadSecurityModuleFromDisk(char * pszDllName);
extern void LoadSecurityModuleFromMemory( unsigned char * pCode, int nSize );
extern void CloseSecurityModule();


extern void ClientDLL_Init( void ); // from cdll_int.c
extern void ClientDLL_Shutdown( void );
extern void ClientDLL_HudInit( void );
extern void ClientDLL_HudVidInit( void );
extern void	ClientDLL_UpdateClientData( void );
extern void ClientDLL_Frame( double time );
extern void ClientDLL_HudRedraw( int intermission );
extern void ClientDLL_MoveClient( struct playermove_s *ppmove );
extern void ClientDLL_ClientMoveInit( struct playermove_s *ppmove );
extern char ClientDLL_ClientTextureType( char *name );

extern void ClientDLL_CreateMove( float frametime, struct usercmd_s *cmd, int active );
extern void ClientDLL_ActivateMouse( void );
extern void ClientDLL_DeactivateMouse( void );
extern void ClientDLL_MouseEvent( int mstate );
extern void ClientDLL_ClearStates( void );
extern int ClientDLL_IsThirdPerson( void );
extern void ClientDLL_GetCameraOffsets( float *ofs );
extern int ClientDLL_GraphKeyDown( void );
extern struct kbutton_s *ClientDLL_FindKey( const char *name );
extern void ClientDLL_CAM_Think( void );
extern void ClientDLL_IN_Accumulate( void );
extern void ClientDLL_CalcRefdef( struct ref_params_s *pparams );
extern int ClientDLL_AddEntity( int type, struct cl_entity_s *ent );
extern void ClientDLL_CreateEntities( void );

extern void ClientDLL_DrawNormalTriangles( void );
extern void ClientDLL_DrawTransparentTriangles( void );
extern void ClientDLL_StudioEvent( const struct mstudioevent_s *event, const struct cl_entity_s *entity );
extern void ClientDLL_PostRunCmd( struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed );
extern void ClientDLL_TxferLocalOverrides( struct entity_state_s *state, const struct clientdata_s *client );
extern void ClientDLL_ProcessPlayerState( struct entity_state_s *dst, const struct entity_state_s *src );
extern void ClientDLL_TxferPredictionData ( struct entity_state_s *ps, const struct entity_state_s *pps, struct clientdata_s *pcd, const struct clientdata_s *ppcd, struct weapon_data_s *wd, const struct weapon_data_s *pwd );
extern void ClientDLL_ReadDemoBuffer( int size, unsigned char *buffer );
extern int ClientDLL_ConnectionlessPacket( const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size );
extern int ClientDLL_GetHullBounds( int hullnumber, float *mins, float *maxs );

extern void ClientDLL_VGui_ConsolePrint(const char* text);

extern int ClientDLL_Key_Event( int down, int keynum, const char *pszCurrentBinding );
extern void ClientDLL_TempEntUpdate( double ft, double ct, double grav, struct tempent_s **ppFreeTE, struct tempent_s **ppActiveTE, int ( *addTEntity )( struct cl_entity_s *pEntity ), void ( *playTESound )( struct tempent_s *pTemp, float damp ) );
extern struct cl_entity_s *ClientDLL_GetUserEntity( int index );
extern void ClientDLL_VoiceStatus(int entindex, qboolean bTalking);
extern void ClientDLL_DirectorMessage( int iSize, void *pbuf );
extern void ClientDLL_ChatInputPosition( int *x, int *y );

//#include "server.h" // server_static_t define for apiproxy
#include "APIProxy.h"

extern cldll_func_t	cl_funcs;
extern cl_enginefunc_t cl_engsrcProxies;
extern cl_enginefunc_dst_t g_engdstAddrs;

// Module exports
extern modfuncs_t g_modfuncs;
extern module_t	g_module;

// Macros for exported engine funcs
#define RecEngSPR_Load(a)					(g_engdstAddrs.pfnSPR_Load(&a))
#define RecEngSPR_Frames(a)					(g_engdstAddrs.pfnSPR_Frames(&a))
#define RecEngSPR_Height(a, b)				(g_engdstAddrs.pfnSPR_Height(&a, &b))
#define RecEngSPR_Width(a, b)				(g_engdstAddrs.pfnSPR_Width(&a, &b))
#define RecEngSPR_Set(a, b, c, d)			(g_engdstAddrs.pfnSPR_Set(&a, &b, &c, &d))
#define RecEngSPR_Draw(a, b, c, d)			(g_engdstAddrs.pfnSPR_Draw(&a, &b, &c, &d))
#define RecEngSPR_DrawHoles(a, b, c, d)		(g_engdstAddrs.pfnSPR_DrawHoles(&a, &b, &c, &d))
#define RecEngSPR_DrawAdditive(a, b, c, d)	(g_engdstAddrs.pfnSPR_DrawAdditive(&a, &b, &c, &d))
#define RecEngSPR_EnableScissor(a, b, c, d)	(g_engdstAddrs.pfnSPR_EnableScissor(&a, &b, &c, &d))
#define RecEngSPR_DisableScissor()			(g_engdstAddrs.pfnSPR_DisableScissor())
#define RecEngSPR_GetList(a, b)				(g_engdstAddrs.pfnSPR_GetList(&a, &b))
#define RecEngDraw_FillRGBA(a, b, c, d, e, f, g, h)		(g_engdstAddrs.pfnFillRGBA(&a, &b, &c, &d, &e, &f, &g, &h))
#define RecEnghudGetScreenInfo(a)			(g_engdstAddrs.pfnGetScreenInfo(&a))
#define RecEngSetCrosshair(a, b, c, d, e)	(g_engdstAddrs.pfnSetCrosshair(&a, &b, &c, &d, &e))
#define RecEnghudRegisterVariable(a, b, c)	(g_engdstAddrs.pfnRegisterVariable(&a, &b, &c))
#define RecEnghudGetCvarFloat(a)			(g_engdstAddrs.pfnGetCvarFloat(&a))
#define RecEnghudGetCvarString(a)			(g_engdstAddrs.pfnGetCvarString(&a))
#define RecEnghudAddCommand(a, b)			(g_engdstAddrs.pfnAddCommand(&a, &b))
#define RecEnghudHookUserMsg(a, b)			(g_engdstAddrs.pfnHookUserMsg(&a, &b))
#define RecEnghudServerCmd(a)				(g_engdstAddrs.pfnServerCmd(&a))
#define RecEnghudClientCmd(a)				(g_engdstAddrs.pfnClientCmd(&a))
#define RecEngPrimeMusicStream(a, b)	(g_engdstAddrs.pfnPrimeMusicStream(&a, &b))
#define RecEnghudGetPlayerInfo(a, b)		(g_engdstAddrs.pfnGetPlayerInfo(&a, &b))
#define RecEnghudPlaySoundByName(a, b)		(g_engdstAddrs.pfnPlaySoundByName(&a, &b))
#define RecEnghudPlaySoundByNameAtPitch(a, b, c)	(g_engdstAddrs.pfnPlaySoundByNameAtPitch(&a, &b, &c))
#define RecEnghudPlaySoundVoiceByName(a, b)	(g_engdstAddrs.pfnPlaySoundVoiceByName(&a, &b))
#define RecEnghudPlaySoundByIndex(a, b)		(g_engdstAddrs.pfnPlaySoundByIndex(&a, &b))
#define RecEngAngleVectors(a, b, c, d)		(g_engdstAddrs.pfnAngleVectors(&a, &b, &c, &d))
#define RecEngTextMessageGet(a)				(g_engdstAddrs.pfnTextMessageGet(&a))
#define RecEngTextMessageDrawCharacter(a, b, c, d, e, f)	(g_engdstAddrs.pfnDrawCharacter(&a, &b, &c, &d, &e, &f))
#define RecEngDrawConsoleString(a, b, c)	(g_engdstAddrs.pfnDrawConsoleString(&a, &b, &c))
#define RecEngDrawSetTextColor(a, b, c)		(g_engdstAddrs.pfnDrawSetTextColor(&a, &b, &c))
#define RecEnghudDrawConsoleStringLen(a, b, c)	(g_engdstAddrs.pfnDrawConsoleStringLen(&a, &b, &c))
#define RecEnghudConsolePrint(a)			(g_engdstAddrs.pfnConsolePrint(&a))
#define RecEnghudCenterPrint(a)				(g_engdstAddrs.pfnCenterPrint(&a))
#define RecEnghudCenterX()					(g_engdstAddrs.GetWindowCenterX())
#define RecEnghudCenterY()					(g_engdstAddrs.GetWindowCenterY())
#define RecEnghudGetViewAngles(a)			(g_engdstAddrs.GetViewAngles(&a))
#define RecEnghudSetViewAngles(a)			(g_engdstAddrs.SetViewAngles(&a))
#define RecEnghudGetMaxClients()			(g_engdstAddrs.GetMaxClients())
#define RecEngCvar_SetValue(a, b)			(g_engdstAddrs.Cvar_SetValue(&a, &b))
#define RecEngCmd_Argc()					(g_engdstAddrs.Cmd_Argc())
#define RecEngCmd_Argv(a)					(g_engdstAddrs.Cmd_Argv(&a))
#define RecEngCon_Printf(a)					(g_engdstAddrs.Con_Printf(&a))
#define RecEngCon_DPrintf(a)				(g_engdstAddrs.Con_DPrintf(&a))
#define RecEngCon_NPrintf(a, b)				(g_engdstAddrs.Con_NPrintf(&a, &b))
#define RecEngCon_NXPrintf(a, b)			(g_engdstAddrs.Con_NXPrintf(&a, &b))
#define RecEnghudPhysInfo_ValueForKey(a)	(g_engdstAddrs.PhysInfo_ValueForKey(&a))
#define RecEnghudServerInfo_ValueForKey(a)	(g_engdstAddrs.ServerInfo_ValueForKey(&a))
#define RecEnghudGetClientMaxspeed()		(g_engdstAddrs.GetClientMaxspeed())
#define RecEnghudCheckParm(a, b)			(g_engdstAddrs.CheckParm(&a, &b))
#define RecEngKey_Event(a, b)				(g_engdstAddrs.Key_Event(&a, &b))
#define RecEnghudGetMousePosition(a, b)		(g_engdstAddrs.GetMousePosition(&a, &b))
#define RecEnghudIsNoClipping()				(g_engdstAddrs.IsNoClipping())
#define RecEnghudGetLocalPlayer()			(g_engdstAddrs.GetLocalPlayer())
#define RecEnghudGetViewModel()				(g_engdstAddrs.GetViewModel())
#define RecEnghudGetEntityByIndex(a)		(g_engdstAddrs.GetEntityByIndex(&a))
#define RecEnghudGetClientTime()			(g_engdstAddrs.GetClientTime())
#define RecEngV_CalcShake()					(g_engdstAddrs.V_CalcShake())
#define RecEngV_ApplyShake(a, b, c)			(g_engdstAddrs.V_ApplyShake(&a, &b, &c))
#define RecEngPM_PointContents(a, b)		(g_engdstAddrs.PM_PointContents(&a, &b))
#define RecEngPM_WaterEntity(a)				(g_engdstAddrs.PM_WaterEntity(&a))
#define RecEngPM_TraceLine(a, b, c, d, e)	(g_engdstAddrs.PM_TraceLine(&a, &b, &c, &d, &e))
#define RecEngCL_LoadModel(a, b)			(g_engdstAddrs.CL_LoadModel(&a, &b))
#define RecEngCL_CreateVisibleEntity(a, b)	(g_engdstAddrs.CL_CreateVisibleEntity(&a, &b))
#define RecEnghudGetSpritePointer(a)		(g_engdstAddrs.GetSpritePointer(&a))
#define RecEnghudPlaySoundByNameAtLocation(a, b, c)		(g_engdstAddrs.pfnPlaySoundByNameAtLocation(&a, &b, &c))
#define RecEnghudPrecacheEvent(a, b)		(g_engdstAddrs.pfnPrecacheEvent(&a, &b))
#define RecEnghudPlaybackEvent(a, b, c, d, e, f, g, h, i, j, k, l)	(g_engdstAddrs.pfnPlaybackEvent(&a, &b, &c, &d, &e, &f, &g, &h, &i, &j, &k, &l))
#define RecEnghudWeaponAnim(a, b)			(g_engdstAddrs.pfnWeaponAnim(&a, &b))
#define RecEngRandomFloat(a, b)				(g_engdstAddrs.pfnRandomFloat(&a, &b))
#define RecEngRandomLong(a, b)				(g_engdstAddrs.pfnRandomLong(&a, &b))
#define RecEngCL_HookEvent(a, b)			(g_engdstAddrs.pfnHookEvent(&a, &b))
#define RecEngCon_IsVisible()				(g_engdstAddrs.Con_IsVisible())
#define RecEnghudGetGameDir()				(g_engdstAddrs.pfnGetGameDirectory())
#define RecEngCvar_FindVar(a)				(g_engdstAddrs.pfnGetCvarPointer(&a))
#define RecEngKey_NameForBinding(a)			(g_engdstAddrs.Key_LookupBinding(&a))
#define RecEnghudGetLevelName()				(g_engdstAddrs.pfnGetLevelName())
#define RecEnghudGetScreenFade(a)			(g_engdstAddrs.pfnGetScreenFade(&a))
#define RecEnghudSetScreenFade(a)			(g_engdstAddrs.pfnSetScreenFade(&a))
#define RecEngVGuiWrap_GetPanel()			(g_engdstAddrs.VGui_GetPanel())
#define RecEngVGui_ViewportPaintBackground(a)	(g_engdstAddrs.VGui_ViewportPaintBackground(&a))
#define RecEngCOM_LoadFile(a, b, c)			(g_engdstAddrs.COM_LoadFile(&a, &b, &c))
#define RecEngCOM_ParseFile(a, b)			(g_engdstAddrs.COM_ParseFile(&a, &b))
#define RecEngCOM_FreeFile(a)				(g_engdstAddrs.COM_FreeFile(&a))
#define RecEngCL_IsSpectateOnly()			(g_engdstAddrs.IsSpectateOnly())
#define RecEngR_LoadMapSprite(a)			(g_engdstAddrs.LoadMapSprite(&a))
#define RecEngCOM_AddAppDirectoryToSearchPath(a, b)		(g_engdstAddrs.COM_AddAppDirectoryToSearchPath(&a, &b))
#define RecEngClientDLL_ExpandFileName(a, b, c)		(g_engdstAddrs.COM_ExpandFilename(&a, &b, &c))
#define RecEngPlayerInfo_ValueForKey(a, b)	(g_engdstAddrs.PlayerInfo_ValueForKey(&a, &b))
#define RecEngPlayerInfo_SetValueForKey(a, b)		(g_engdstAddrs.PlayerInfo_SetValueForKey(&a, &b))
#define RecEngGetPlayerUniqueID(a, b)		(g_engdstAddrs.GetPlayerUniqueID(&a, &b))
#define RecEngGetTrackerIDForPlayer(a)		(g_engdstAddrs.GetTrackerIDForPlayer(&a))
#define RecEngGetPlayerForTrackerID(a)		(g_engdstAddrs.GetPlayerForTrackerID(&a))
#define RecEnghudServerCmdUnreliable(a)		(g_engdstAddrs.pfnServerCmdUnreliable(&a))
#define RecEngGetMousePos(a)				(g_engdstAddrs.pfnGetMousePos(&a))
#define RecEngSetMousePos(a, b)				(g_engdstAddrs.pfnSetMousePos(&a, &b))
#define RecEngSetMouseEnable(a)				(g_engdstAddrs.pfnSetMouseEnable(&a))
#define RecEngSetFilterMode(a)				(g_engdstAddrs.pfnSetFilterMode(&a))
#define RecEngSetFilterColor(a,b,c)			(g_engdstAddrs.pfnSetFilterColor(&a,&b,&c))
#define RecEngSetFilterBrightness(a)		(g_engdstAddrs.pfnSetFilterBrightness(&a))
#define RecEngSequenceGet(a,b)				(g_engdstAddrs.pfnSequenceGet(&a,&b))
#define RecEngSPR_DrawGeneric(a,b,c,d,e,f,g,h)	(g_engdstAddrs.pfnSPR_DrawGeneric(&a, &b, &c, &d, &e, &f, &g, &h))
#define RecEngSequencePickSentence(a,b,c)	(g_engdstAddrs.pfnSequencePickSentence(&a, &b, &c))
#define RecEngLocalPlayerInfo_ValueForKey(a)	(g_engdstAddrs.LocalPlayerInfo_ValueForKey(&a))
#define RecEngProcessTutorMessageDecayBuffer(a, b)		(g_engdstAddrs.pfnProcessTutorMessageDecayBuffer(&a, &b))
#define RecEngConstructTutorMessageDecayBuffer(a, b)	(g_engdstAddrs.pfnConstructTutorMessageDecayBuffer(&a, &b))
#define RecEngResetTutorMessageDecayBuffer()		(g_engdstAddrs.pfnResetTutorMessageDecayBuffer())
#define RecEngDraw_FillRGBABlend(a, b, c, d, e, f, g, h)		(g_engdstAddrs.pfnFillRGBABlend(&a, &b, &c, &d, &e, &f, &g, &h))

// Dummy destination function for use when there's no security module
extern void NullDst(void);

// Use this to init an engdst structure to point to NullDst
#define k_engdstNull \
{ \
	(pfnEngDst_pfnSPR_Load_t)						NullDst, \
	(pfnEngDst_pfnSPR_Frames_t)						NullDst, \
	(pfnEngDst_pfnSPR_Height_t)						NullDst, \
	(pfnEngDst_pfnSPR_Width_t)						NullDst, \
	(pfnEngDst_pfnSPR_Set_t)						NullDst, \
	(pfnEngDst_pfnSPR_Draw_t)						NullDst, \
	(pfnEngDst_pfnSPR_DrawHoles_t)					NullDst, \
	(pfnEngDst_pfnSPR_DrawAdditive_t)				NullDst, \
	(pfnEngDst_pfnSPR_EnableScissor_t)				NullDst, \
	(pfnEngDst_pfnSPR_DisableScissor_t)				NullDst, \
	(pfnEngDst_pfnSPR_GetList_t)					NullDst, \
	(pfnEngDst_pfnFillRGBA_t)						NullDst, \
	(pfnEngDst_pfnGetScreenInfo_t)					NullDst, \
	(pfnEngDst_pfnSetCrosshair_t)					NullDst, \
	(pfnEngDst_pfnRegisterVariable_t)				NullDst, \
	(pfnEngDst_pfnGetCvarFloat_t)					NullDst, \
	(pfnEngDst_pfnGetCvarString_t)					NullDst, \
	(pfnEngDst_pfnAddCommand_t)						NullDst, \
	(pfnEngDst_pfnHookUserMsg_t)					NullDst, \
	(pfnEngDst_pfnServerCmd_t)						NullDst, \
	(pfnEngDst_pfnClientCmd_t)						NullDst, \
	(pfnEngDst_pfnGetPlayerInfo_t)					NullDst, \
	(pfnEngDst_pfnPlaySoundByName_t)				NullDst, \
	(pfnEngDst_pfnPlaySoundByIndex_t)				NullDst, \
	(pfnEngDst_pfnAngleVectors_t)					NullDst, \
	(pfnEngDst_pfnTextMessageGet_t)					NullDst, \
	(pfnEngDst_pfnDrawCharacter_t)					NullDst, \
	(pfnEngDst_pfnDrawConsoleString_t)				NullDst, \
	(pfnEngDst_pfnDrawSetTextColor_t)				NullDst, \
	(pfnEngDst_pfnDrawConsoleStringLen_t)			NullDst, \
	(pfnEngDst_pfnConsolePrint_t)					NullDst, \
	(pfnEngDst_pfnCenterPrint_t)					NullDst, \
	(pfnEngDst_GetWindowCenterX_t)					NullDst, \
	(pfnEngDst_GetWindowCenterY_t)					NullDst, \
	(pfnEngDst_GetViewAngles_t)						NullDst, \
	(pfnEngDst_SetViewAngles_t)						NullDst, \
	(pfnEngDst_GetMaxClients_t)						NullDst, \
	(pfnEngDst_Cvar_SetValue_t)						NullDst, \
	(pfnEngDst_Cmd_Argc_t)							NullDst, \
	(pfnEngDst_Cmd_Argv_t)							NullDst, \
	(pfnEngDst_Con_Printf_t)						NullDst, \
	(pfnEngDst_Con_DPrintf_t)						NullDst, \
	(pfnEngDst_Con_NPrintf_t)						NullDst, \
	(pfnEngDst_Con_NXPrintf_t)						NullDst, \
	(pfnEngDst_PhysInfo_ValueForKey_t)				NullDst, \
	(pfnEngDst_ServerInfo_ValueForKey_t)			NullDst, \
	(pfnEngDst_GetClientMaxspeed_t)					NullDst, \
	(pfnEngDst_CheckParm_t)							NullDst, \
	(pfnEngDst_Key_Event_t)							NullDst, \
	(pfnEngDst_GetMousePosition_t)					NullDst, \
	(pfnEngDst_IsNoClipping_t)						NullDst, \
	(pfnEngDst_GetLocalPlayer_t)					NullDst, \
	(pfnEngDst_GetViewModel_t)						NullDst, \
	(pfnEngDst_GetEntityByIndex_t)					NullDst, \
	(pfnEngDst_GetClientTime_t)						NullDst, \
	(pfnEngDst_V_CalcShake_t)						NullDst, \
	(pfnEngDst_V_ApplyShake_t)						NullDst, \
	(pfnEngDst_PM_PointContents_t)					NullDst, \
	(pfnEngDst_PM_WaterEntity_t)					NullDst, \
	(pfnEngDst_PM_TraceLine_t)						NullDst, \
	(pfnEngDst_CL_LoadModel_t)						NullDst, \
	(pfnEngDst_CL_CreateVisibleEntity_t)			NullDst, \
	(pfnEngDst_GetSpritePointer_t)					NullDst, \
	(pfnEngDst_pfnPlaySoundByNameAtLocation_t)		NullDst, \
	(pfnEngDst_pfnPrecacheEvent_t)					NullDst, \
	(pfnEngDst_pfnPlaybackEvent_t)					NullDst, \
	(pfnEngDst_pfnWeaponAnim_t)						NullDst, \
	(pfnEngDst_pfnRandomFloat_t)					NullDst, \
	(pfnEngDst_pfnRandomLong_t)						NullDst, \
	(pfnEngDst_pfnHookEvent_t)						NullDst, \
	(pfnEngDst_Con_IsVisible_t)						NullDst, \
	(pfnEngDst_pfnGetGameDirectory_t)				NullDst, \
	(pfnEngDst_pfnGetCvarPointer_t)					NullDst, \
	(pfnEngDst_Key_LookupBinding_t)					NullDst, \
	(pfnEngDst_pfnGetLevelName_t)					NullDst, \
	(pfnEngDst_pfnGetScreenFade_t)					NullDst, \
	(pfnEngDst_pfnSetScreenFade_t)					NullDst, \
	(pfnEngDst_VGui_GetPanel_t)						NullDst, \
	(pfnEngDst_VGui_ViewportPaintBackground_t)		NullDst, \
	(pfnEngDst_COM_LoadFile_t)						NullDst, \
	(pfnEngDst_COM_ParseFile_t)						NullDst, \
	(pfnEngDst_COM_FreeFile_t)						NullDst, \
	NULL, \
	NULL, \
	NULL, \
	NULL, \
	NULL, \
	NULL, \
	(pfnEngDst_IsSpectateOnly_t)					NullDst, \
	(pfnEngDst_LoadMapSprite_t)						NullDst, \
	(pfnEngDst_COM_AddAppDirectoryToSearchPath_t)	NullDst, \
	(pfnEngDst_COM_ExpandFilename_t)				NullDst, \
	(pfnEngDst_PlayerInfo_ValueForKey_t)			NullDst, \
	(pfnEngDst_PlayerInfo_SetValueForKey_t)			NullDst, \
	(pfnEngDst_GetPlayerUniqueID_t)					NullDst, \
	(pfnEngDst_GetTrackerIDForPlayer_t)				NullDst, \
	(pfnEngDst_GetPlayerForTrackerID_t)				NullDst, \
	(pfnEngDst_pfnServerCmdUnreliable_t)			NullDst, \
	(pfnEngDst_GetMousePos_t)						NullDst, \
	(pfnEngDst_SetMousePos_t)						NullDst, \
	(pfnEngDst_SetMouseEnable_t)					NullDst, \
	(pfnEngDst_pfnSetFilterMode_t)					NullDst, \
	(pfnEngDst_pfnSetFilterColor_t)					NullDst, \
	(pfnEngDst_pfnSetFilterBrightness_t)			NullDst, \
	(pfnEngDst_pfnSequenceGet_t)					NullDst, \
	(pfnEngDst_pfnSPR_DrawGeneric_t)				NullDst, \
	(pfnEngDst_pfnSequencePickSentence_t)			NullDst, \
	(pfnEngDst_pfnDrawString_t)						NullDst, \
	(pfnEngDst_pfnDrawStringReverse_t)				NullDst, \
	(pfnEngDst_LocalPlayerInfo_ValueForKey_t)		NullDst, \
	(pfnEngDst_pfnVGUI2DrawCharacter_t)			NullDst, \
	(pfnEngDst_pfnVGUI2DrawCharacterAdd_t)	NullDst, \
	(pfnEngDst_pfnPlaySoundVoiceByName_t)		NullDst, \
	(pfnEngDst_pfnPrimeMusicStream_t)				NullDst, \
	(pfnEngDst_pfnProcessTutorMessageDecayBuffer_t)	NullDst, \
	(pfnEngDst_pfnConstructTutorMessageDecayBuffer_t)	NullDst, \
	(pfnEngDst_pfnResetTutorMessageDecayData_t) NullDst, \
	(pfnEngDst_pfnPlaySoundByNameAtPitch_t)			NullDst, \
	(pfnEngDst_pfnFillRGBABlend_t)						NullDst, \
	(pfnEngDst_pfnGetAppID_t)						NullDst, \
	(pfnEngDst_pfnGetAliases_t)						NullDst, \
	(pfnEngDst_pfnVguiWrap2_GetMouseDelta_t)		NullDst, \
};

// Use this to init a cldll_func_dst structure to point to NullDst
#define k_cldstNull \
{ \
	(DST_INITIALIZE_FUNC)				NullDst, \
	(DST_HUD_INIT_FUNC)					NullDst, \
	(DST_HUD_VIDINIT_FUNC)				NullDst, \
	(DST_HUD_REDRAW_FUNC)				NullDst, \
	(DST_HUD_UPDATECLIENTDATA_FUNC)		NullDst, \
	(DST_HUD_RESET_FUNC)				NullDst, \
	(DST_HUD_CLIENTMOVE_FUNC)			NullDst, \
	(DST_HUD_CLIENTMOVEINIT_FUNC)		NullDst, \
	(DST_HUD_TEXTURETYPE_FUNC)			NullDst, \
	(DST_HUD_IN_ACTIVATEMOUSE_FUNC)		NullDst, \
	(DST_HUD_IN_DEACTIVATEMOUSE_FUNC)	NullDst, \
	(DST_HUD_IN_MOUSEEVENT_FUNC)		NullDst, \
	(DST_HUD_IN_CLEARSTATES_FUNC)		NullDst, \
	(DST_HUD_IN_ACCUMULATE_FUNC)		NullDst, \
	(DST_HUD_CL_CREATEMOVE_FUNC)		NullDst, \
	(DST_HUD_CL_ISTHIRDPERSON_FUNC)		NullDst, \
	(DST_HUD_CL_GETCAMERAOFFSETS_FUNC)	NullDst, \
	(DST_HUD_KB_FIND_FUNC)				NullDst, \
	(DST_HUD_CAMTHINK_FUNC)				NullDst, \
	(DST_HUD_CALCREF_FUNC)				NullDst, \
	(DST_HUD_ADDENTITY_FUNC)			NullDst, \
	(DST_HUD_CREATEENTITIES_FUNC)		NullDst, \
	(DST_HUD_DRAWNORMALTRIS_FUNC)		NullDst, \
	(DST_HUD_DRAWTRANSTRIS_FUNC)		NullDst, \
	(DST_HUD_STUDIOEVENT_FUNC)			NullDst, \
	(DST_HUD_POSTRUNCMD_FUNC)			NullDst, \
	(DST_HUD_SHUTDOWN_FUNC)				NullDst, \
	(DST_HUD_TXFERLOCALOVERRIDES_FUNC)	NullDst, \
	(DST_HUD_PROCESSPLAYERSTATE_FUNC)	NullDst, \
	(DST_HUD_TXFERPREDICTIONDATA_FUNC)	NullDst, \
	(DST_HUD_DEMOREAD_FUNC)				NullDst, \
	(DST_HUD_CONNECTIONLESS_FUNC)		NullDst, \
	(DST_HUD_GETHULLBOUNDS_FUNC)		NullDst, \
	(DST_HUD_FRAME_FUNC)				NullDst, \
	(DST_HUD_KEY_EVENT_FUNC)			NullDst, \
	(DST_HUD_TEMPENTUPDATE_FUNC)		NullDst, \
	(DST_HUD_GETUSERENTITY_FUNC)		NullDst, \
	(DST_HUD_VOICESTATUS_FUNC)			NullDst, \
	(DST_HUD_DIRECTORMESSAGE_FUNC)		NullDst, \
	(DST_HUD_STUDIO_INTERFACE_FUNC)		NullDst, \
	(DST_HUD_CHATINPUTPOSITION_FUNC)	NullDst, \
	(DST_HUD_GETPLAYERTEAM)				NullDst, \
}

#ifdef __cplusplus
}
#endif

#endif // CDLL_INT_H
	