#ifndef __APIPROXY__
#define __APIPROXY__

#include "archtypes.h"     // DAL
#include "netadr.h"
#include "Sequence.h"

#ifndef _WIN32
#include "enums.h"
#endif

#define	MAX_ALIAS_NAME	32

typedef struct cmdalias_s
{
	struct cmdalias_s	*next;
	char	name[MAX_ALIAS_NAME];
	char	*value;
} cmdalias_t;


// ********************************************************
// Functions exported by the client .dll
// ********************************************************

// Function type declarations for client exports
typedef int (*INITIALIZE_FUNC)	( struct cl_enginefuncs_s*, int );
typedef void (*HUD_INIT_FUNC)		( void );
typedef int (*HUD_VIDINIT_FUNC)	( void );
typedef int (*HUD_REDRAW_FUNC)	( float, int );
typedef int (*HUD_UPDATECLIENTDATA_FUNC) ( struct client_data_s*, float );
typedef void (*HUD_RESET_FUNC)    ( void );
typedef void (*HUD_CLIENTMOVE_FUNC)( struct playermove_s *ppmove, qboolean server );
typedef void (*HUD_CLIENTMOVEINIT_FUNC)( struct playermove_s *ppmove );
typedef char (*HUD_TEXTURETYPE_FUNC)( char *name );
typedef void (*HUD_IN_ACTIVATEMOUSE_FUNC) ( void );
typedef void (*HUD_IN_DEACTIVATEMOUSE_FUNC)		( void );
typedef void (*HUD_IN_MOUSEEVENT_FUNC)		( int mstate );
typedef void (*HUD_IN_CLEARSTATES_FUNC)		( void );
typedef void (*HUD_IN_ACCUMULATE_FUNC ) ( void );
typedef void (*HUD_CL_CREATEMOVE_FUNC)		( float frametime, struct usercmd_s *cmd, int active );
typedef int (*HUD_CL_ISTHIRDPERSON_FUNC) ( void );
typedef void (*HUD_CL_GETCAMERAOFFSETS_FUNC )( float *ofs );
typedef struct kbutton_s * (*HUD_KB_FIND_FUNC) ( const char *name );
typedef void ( *HUD_CAMTHINK_FUNC )( void );
typedef void ( *HUD_CALCREF_FUNC ) ( struct ref_params_s *pparams );
typedef int	 ( *HUD_ADDENTITY_FUNC ) ( int type, struct cl_entity_s *ent, const char *modelname );
typedef void ( *HUD_CREATEENTITIES_FUNC ) ( void );
typedef void ( *HUD_DRAWNORMALTRIS_FUNC ) ( void );
typedef void ( *HUD_DRAWTRANSTRIS_FUNC ) ( void );
typedef void ( *HUD_STUDIOEVENT_FUNC ) ( const struct mstudioevent_s *event, const struct cl_entity_s *entity );
typedef void ( *HUD_POSTRUNCMD_FUNC ) ( struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed );
typedef void ( *HUD_SHUTDOWN_FUNC ) ( void );
typedef void ( *HUD_TXFERLOCALOVERRIDES_FUNC )( struct entity_state_s *state, const struct clientdata_s *client );
typedef void ( *HUD_PROCESSPLAYERSTATE_FUNC )( struct entity_state_s *dst, const struct entity_state_s *src );
typedef void ( *HUD_TXFERPREDICTIONDATA_FUNC ) ( struct entity_state_s *ps, const struct entity_state_s *pps, struct clientdata_s *pcd, const struct clientdata_s *ppcd, struct weapon_data_s *wd, const struct weapon_data_s *pwd );
typedef void ( *HUD_DEMOREAD_FUNC ) ( int size, unsigned char *buffer );
typedef int ( *HUD_CONNECTIONLESS_FUNC )( const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size );
typedef	int	( *HUD_GETHULLBOUNDS_FUNC ) ( int hullnumber, float *mins, float *maxs );
typedef void (*HUD_FRAME_FUNC)		( double );
typedef int (*HUD_KEY_EVENT_FUNC ) ( int eventcode, int keynum, const char *pszCurrentBinding );
typedef void (*HUD_TEMPENTUPDATE_FUNC) ( double frametime, double client_time, double cl_gravity, struct tempent_s **ppTempEntFree, struct tempent_s **ppTempEntActive, 	int ( *Callback_AddVisibleEntity )( struct cl_entity_s *pEntity ),	void ( *Callback_TempEntPlaySound )( struct tempent_s *pTemp, float damp ) );
typedef struct cl_entity_s *(*HUD_GETUSERENTITY_FUNC ) ( int index );
typedef void (*HUD_VOICESTATUS_FUNC)(int entindex, qboolean bTalking);
typedef void (*HUD_DIRECTORMESSAGE_FUNC)( int iSize, void *pbuf );
typedef int ( *HUD_STUDIO_INTERFACE_FUNC )( int version, struct r_studio_interface_s **ppinterface, struct engine_studio_api_s *pstudio );
typedef void (*HUD_CHATINPUTPOSITION_FUNC)( int *x, int *y );
typedef int (*HUD_GETPLAYERTEAM)(int iplayer);
typedef void *(*CLIENTFACTORY)(); // this should be CreateInterfaceFn but that means including interface.h
									// which is a C++ file and some of the client files a C only... 
									// so we return a void * which we then do a typecast on later.


// Pointers to the exported client functions themselves
typedef struct
{
	INITIALIZE_FUNC						pInitFunc;
	HUD_INIT_FUNC						pHudInitFunc;
	HUD_VIDINIT_FUNC					pHudVidInitFunc;
	HUD_REDRAW_FUNC						pHudRedrawFunc;
	HUD_UPDATECLIENTDATA_FUNC			pHudUpdateClientDataFunc;
	HUD_RESET_FUNC						pHudResetFunc;
	HUD_CLIENTMOVE_FUNC					pClientMove;
	HUD_CLIENTMOVEINIT_FUNC				pClientMoveInit;
	HUD_TEXTURETYPE_FUNC				pClientTextureType;
	HUD_IN_ACTIVATEMOUSE_FUNC			pIN_ActivateMouse;
	HUD_IN_DEACTIVATEMOUSE_FUNC			pIN_DeactivateMouse;
	HUD_IN_MOUSEEVENT_FUNC				pIN_MouseEvent;
	HUD_IN_CLEARSTATES_FUNC				pIN_ClearStates;
	HUD_IN_ACCUMULATE_FUNC				pIN_Accumulate;
	HUD_CL_CREATEMOVE_FUNC				pCL_CreateMove;
	HUD_CL_ISTHIRDPERSON_FUNC			pCL_IsThirdPerson;
	HUD_CL_GETCAMERAOFFSETS_FUNC		pCL_GetCameraOffsets;
	HUD_KB_FIND_FUNC					pFindKey;
	HUD_CAMTHINK_FUNC					pCamThink;
	HUD_CALCREF_FUNC					pCalcRefdef;
	HUD_ADDENTITY_FUNC					pAddEntity;
	HUD_CREATEENTITIES_FUNC				pCreateEntities;
	HUD_DRAWNORMALTRIS_FUNC				pDrawNormalTriangles;
	HUD_DRAWTRANSTRIS_FUNC				pDrawTransparentTriangles;
	HUD_STUDIOEVENT_FUNC				pStudioEvent;
	HUD_POSTRUNCMD_FUNC					pPostRunCmd;
	HUD_SHUTDOWN_FUNC					pShutdown;
	HUD_TXFERLOCALOVERRIDES_FUNC		pTxferLocalOverrides;
	HUD_PROCESSPLAYERSTATE_FUNC			pProcessPlayerState;
	HUD_TXFERPREDICTIONDATA_FUNC		pTxferPredictionData;
	HUD_DEMOREAD_FUNC					pReadDemoBuffer;
	HUD_CONNECTIONLESS_FUNC				pConnectionlessPacket;
	HUD_GETHULLBOUNDS_FUNC				pGetHullBounds;
	HUD_FRAME_FUNC						pHudFrame;
	HUD_KEY_EVENT_FUNC					pKeyEvent;
	HUD_TEMPENTUPDATE_FUNC				pTempEntUpdate;
	HUD_GETUSERENTITY_FUNC				pGetUserEntity;
	HUD_VOICESTATUS_FUNC				pVoiceStatus;		// Possibly null on old client dlls.
	HUD_DIRECTORMESSAGE_FUNC			pDirectorMessage;	// Possibly null on old client dlls.
	HUD_STUDIO_INTERFACE_FUNC			pStudioInterface;	// Not used by all clients
	HUD_CHATINPUTPOSITION_FUNC			pChatInputPosition;	// Not used by all clients
	HUD_GETPLAYERTEAM					pGetPlayerTeam; // Not used by all clients
	CLIENTFACTORY						pClientFactory;
} cldll_func_t;

// Function type declarations for client destination functions
typedef void (*DST_INITIALIZE_FUNC)	( struct cl_enginefuncs_s**, int *);
typedef void (*DST_HUD_INIT_FUNC)		( void );
typedef void (*DST_HUD_VIDINIT_FUNC)	( void );
typedef void (*DST_HUD_REDRAW_FUNC)	( float*, int* );
typedef void (*DST_HUD_UPDATECLIENTDATA_FUNC) ( struct client_data_s**, float* );
typedef void (*DST_HUD_RESET_FUNC)    ( void );
typedef void (*DST_HUD_CLIENTMOVE_FUNC)( struct playermove_s **, qboolean * );
typedef void (*DST_HUD_CLIENTMOVEINIT_FUNC)( struct playermove_s ** );
typedef void (*DST_HUD_TEXTURETYPE_FUNC)( char ** );
typedef void (*DST_HUD_IN_ACTIVATEMOUSE_FUNC) ( void );
typedef void (*DST_HUD_IN_DEACTIVATEMOUSE_FUNC)		( void );
typedef void (*DST_HUD_IN_MOUSEEVENT_FUNC)		( int * );
typedef void (*DST_HUD_IN_CLEARSTATES_FUNC)		( void );
typedef void (*DST_HUD_IN_ACCUMULATE_FUNC ) ( void );
typedef void (*DST_HUD_CL_CREATEMOVE_FUNC)		( float *, struct usercmd_s **, int * );
typedef void (*DST_HUD_CL_ISTHIRDPERSON_FUNC) ( void );
typedef void (*DST_HUD_CL_GETCAMERAOFFSETS_FUNC )( float ** );
typedef void (*DST_HUD_KB_FIND_FUNC) ( const char ** );
typedef void (*DST_HUD_CAMTHINK_FUNC )( void );
typedef void (*DST_HUD_CALCREF_FUNC ) ( struct ref_params_s ** );
typedef void (*DST_HUD_ADDENTITY_FUNC ) ( int *, struct cl_entity_s **, const char ** );
typedef void (*DST_HUD_CREATEENTITIES_FUNC ) ( void );
typedef void (*DST_HUD_DRAWNORMALTRIS_FUNC ) ( void );
typedef void (*DST_HUD_DRAWTRANSTRIS_FUNC ) ( void );
typedef void (*DST_HUD_STUDIOEVENT_FUNC ) ( const struct mstudioevent_s **, const struct cl_entity_s ** );
typedef void (*DST_HUD_POSTRUNCMD_FUNC ) ( struct local_state_s **, struct local_state_s **, struct usercmd_s **, int *, double *, unsigned int * );
typedef void (*DST_HUD_SHUTDOWN_FUNC ) ( void );
typedef void (*DST_HUD_TXFERLOCALOVERRIDES_FUNC )( struct entity_state_s **, const struct clientdata_s ** );
typedef void (*DST_HUD_PROCESSPLAYERSTATE_FUNC )( struct entity_state_s **, const struct entity_state_s ** );
typedef void (*DST_HUD_TXFERPREDICTIONDATA_FUNC ) ( struct entity_state_s **, const struct entity_state_s **, struct clientdata_s **, const struct clientdata_s **, struct weapon_data_s **, const struct weapon_data_s ** );
typedef void (*DST_HUD_DEMOREAD_FUNC ) ( int *, unsigned char ** );
typedef void (*DST_HUD_CONNECTIONLESS_FUNC )( const struct netadr_s **, const char **, char **, int ** );
typedef void (*DST_HUD_GETHULLBOUNDS_FUNC ) ( int *, float **, float ** );
typedef void (*DST_HUD_FRAME_FUNC)		( double * );
typedef void (*DST_HUD_KEY_EVENT_FUNC ) ( int *, int *, const char ** );
typedef void (*DST_HUD_TEMPENTUPDATE_FUNC) ( double *, double *, double *, struct tempent_s ***, struct tempent_s ***, int ( **Callback_AddVisibleEntity )( struct cl_entity_s *pEntity ),	void ( **Callback_TempEntPlaySound )( struct tempent_s *pTemp, float damp ) );
typedef void (*DST_HUD_GETUSERENTITY_FUNC ) ( int * );
typedef void (*DST_HUD_VOICESTATUS_FUNC)(int *, qboolean *);
typedef void (*DST_HUD_DIRECTORMESSAGE_FUNC)( int *, void ** );
typedef void (*DST_HUD_STUDIO_INTERFACE_FUNC ) ( int *, struct r_studio_interface_s ***, struct engine_studio_api_s ** );
typedef void (*DST_HUD_CHATINPUTPOSITION_FUNC)( int **, int ** );
typedef void (*DST_HUD_GETPLAYERTEAM)(int);

// Pointers to the client destination functions
typedef struct
{
	DST_INITIALIZE_FUNC						pInitFunc;
	DST_HUD_INIT_FUNC						pHudInitFunc;
	DST_HUD_VIDINIT_FUNC					pHudVidInitFunc;
	DST_HUD_REDRAW_FUNC						pHudRedrawFunc;
	DST_HUD_UPDATECLIENTDATA_FUNC			pHudUpdateClientDataFunc;
	DST_HUD_RESET_FUNC						pHudResetFunc;
	DST_HUD_CLIENTMOVE_FUNC					pClientMove;
	DST_HUD_CLIENTMOVEINIT_FUNC				pClientMoveInit;
	DST_HUD_TEXTURETYPE_FUNC				pClientTextureType;
	DST_HUD_IN_ACTIVATEMOUSE_FUNC			pIN_ActivateMouse;
	DST_HUD_IN_DEACTIVATEMOUSE_FUNC			pIN_DeactivateMouse;
	DST_HUD_IN_MOUSEEVENT_FUNC				pIN_MouseEvent;
	DST_HUD_IN_CLEARSTATES_FUNC				pIN_ClearStates;
	DST_HUD_IN_ACCUMULATE_FUNC				pIN_Accumulate;
	DST_HUD_CL_CREATEMOVE_FUNC				pCL_CreateMove;
	DST_HUD_CL_ISTHIRDPERSON_FUNC			pCL_IsThirdPerson;
	DST_HUD_CL_GETCAMERAOFFSETS_FUNC		pCL_GetCameraOffsets;
	DST_HUD_KB_FIND_FUNC					pFindKey;
	DST_HUD_CAMTHINK_FUNC					pCamThink;
	DST_HUD_CALCREF_FUNC					pCalcRefdef;
	DST_HUD_ADDENTITY_FUNC					pAddEntity;
	DST_HUD_CREATEENTITIES_FUNC				pCreateEntities;
	DST_HUD_DRAWNORMALTRIS_FUNC				pDrawNormalTriangles;
	DST_HUD_DRAWTRANSTRIS_FUNC				pDrawTransparentTriangles;
	DST_HUD_STUDIOEVENT_FUNC				pStudioEvent;
	DST_HUD_POSTRUNCMD_FUNC					pPostRunCmd;
	DST_HUD_SHUTDOWN_FUNC					pShutdown;
	DST_HUD_TXFERLOCALOVERRIDES_FUNC		pTxferLocalOverrides;
	DST_HUD_PROCESSPLAYERSTATE_FUNC			pProcessPlayerState;
	DST_HUD_TXFERPREDICTIONDATA_FUNC		pTxferPredictionData;
	DST_HUD_DEMOREAD_FUNC					pReadDemoBuffer;
	DST_HUD_CONNECTIONLESS_FUNC				pConnectionlessPacket;
	DST_HUD_GETHULLBOUNDS_FUNC				pGetHullBounds;
	DST_HUD_FRAME_FUNC						pHudFrame;
	DST_HUD_KEY_EVENT_FUNC					pKeyEvent;
	DST_HUD_TEMPENTUPDATE_FUNC				pTempEntUpdate;
	DST_HUD_GETUSERENTITY_FUNC				pGetUserEntity;
	DST_HUD_VOICESTATUS_FUNC				pVoiceStatus;	// Possibly null on old client dlls.
	DST_HUD_DIRECTORMESSAGE_FUNC			pDirectorMessage;	// Possibly null on old client dlls.
	DST_HUD_STUDIO_INTERFACE_FUNC			pStudioInterface;  // Not used by all clients
	DST_HUD_CHATINPUTPOSITION_FUNC			pChatInputPosition;  // Not used by all clients
	DST_HUD_GETPLAYERTEAM					pGetPlayerTeam; // Not used by all clients
} cldll_func_dst_t;




// ********************************************************
// Functions exported by the engine
// ********************************************************

// Function type declarations for engine exports
typedef HSPRITE						(*pfnEngSrc_pfnSPR_Load_t )			( const char *szPicName );
typedef int							(*pfnEngSrc_pfnSPR_Frames_t )			( HSPRITE hPic );
typedef int							(*pfnEngSrc_pfnSPR_Height_t )			( HSPRITE hPic, int frame );
typedef int							(*pfnEngSrc_pfnSPR_Width_t )			( HSPRITE hPic, int frame );
typedef void						(*pfnEngSrc_pfnSPR_Set_t )				( HSPRITE hPic, int r, int g, int b );
typedef void						(*pfnEngSrc_pfnSPR_Draw_t )			( int frame, int x, int y, const struct rect_s *prc );
typedef void						(*pfnEngSrc_pfnSPR_DrawHoles_t )		( int frame, int x, int y, const struct rect_s *prc );
typedef void						(*pfnEngSrc_pfnSPR_DrawAdditive_t )	( int frame, int x, int y, const struct rect_s *prc );
typedef void						(*pfnEngSrc_pfnSPR_EnableScissor_t )	( int x, int y, int width, int height );
typedef void						(*pfnEngSrc_pfnSPR_DisableScissor_t )	( void );
typedef struct client_sprite_s	*	(*pfnEngSrc_pfnSPR_GetList_t )			( char *psz, int *piCount );
typedef void						(*pfnEngSrc_pfnFillRGBA_t )			( int x, int y, int width, int height, int r, int g, int b, int a );
typedef int							(*pfnEngSrc_pfnGetScreenInfo_t ) 		( struct SCREENINFO_s *pscrinfo );
typedef void						(*pfnEngSrc_pfnSetCrosshair_t )		( HSPRITE hspr, wrect_t rc, int r, int g, int b );
typedef struct cvar_s *				(*pfnEngSrc_pfnRegisterVariable_t )	( char *szName, char *szValue, int flags );
typedef float						(*pfnEngSrc_pfnGetCvarFloat_t )		( char *szName );
typedef char*						(*pfnEngSrc_pfnGetCvarString_t )		( char *szName );
typedef int							(*pfnEngSrc_pfnAddCommand_t )			( char *cmd_name, void (*pfnEngSrc_function)(void) );
typedef int							(*pfnEngSrc_pfnHookUserMsg_t )			( char *szMsgName, pfnUserMsgHook pfn );
typedef int							(*pfnEngSrc_pfnServerCmd_t )			( char *szCmdString );
typedef int							(*pfnEngSrc_pfnClientCmd_t )			( char *szCmdString );
typedef void						(*pfnEngSrc_pfnPrimeMusicStream_t )	( char *szFilename, int looping );
typedef void						(*pfnEngSrc_pfnGetPlayerInfo_t )		( int ent_num, struct hud_player_info_s *pinfo );
typedef void						(*pfnEngSrc_pfnPlaySoundByName_t )		( char *szSound, float volume );
typedef void						(*pfnEngSrc_pfnPlaySoundByNameAtPitch_t )	( char *szSound, float volume, int pitch );
typedef void						(*pfnEngSrc_pfnPlaySoundVoiceByName_t )		( char *szSound, float volume, int pitch );
typedef void						(*pfnEngSrc_pfnPlaySoundByIndex_t )	( int iSound, float volume );
typedef void						(*pfnEngSrc_pfnAngleVectors_t )		( const float * vecAngles, float * forward, float * right, float * up );
typedef struct client_textmessage_s*(*pfnEngSrc_pfnTextMessageGet_t )		( const char *pName );
typedef int							(*pfnEngSrc_pfnDrawCharacter_t )		( int x, int y, int number, int r, int g, int b );
typedef int							(*pfnEngSrc_pfnDrawConsoleString_t )	( int x, int y, char *string );
typedef void						(*pfnEngSrc_pfnDrawSetTextColor_t )	( float r, float g, float b );
typedef void						(*pfnEngSrc_pfnDrawConsoleStringLen_t )(  const char *string, int *length, int *height );
typedef void						(*pfnEngSrc_pfnConsolePrint_t )		( const char *string );
typedef void						(*pfnEngSrc_pfnCenterPrint_t )			( const char *string );
typedef int							(*pfnEngSrc_GetWindowCenterX_t )		( void );
typedef int							(*pfnEngSrc_GetWindowCenterY_t )		( void );
typedef void						(*pfnEngSrc_GetViewAngles_t )			( float * );
typedef void						(*pfnEngSrc_SetViewAngles_t )			( float * );
typedef int							(*pfnEngSrc_GetMaxClients_t )			( void );
typedef void						(*pfnEngSrc_Cvar_SetValue_t )			( char *cvar, float value );
typedef int       					(*pfnEngSrc_Cmd_Argc_t)					(void);	
typedef char *						(*pfnEngSrc_Cmd_Argv_t )				( int arg );
typedef void						(*pfnEngSrc_Con_Printf_t )				( char *fmt, ... );
typedef void						(*pfnEngSrc_Con_DPrintf_t )			( char *fmt, ... );
typedef void						(*pfnEngSrc_Con_NPrintf_t )			( int pos, char *fmt, ... );
typedef void						(*pfnEngSrc_Con_NXPrintf_t )			( struct con_nprint_s *info, char *fmt, ... );
typedef const char *				(*pfnEngSrc_PhysInfo_ValueForKey_t )	( const char *key );
typedef const char *				(*pfnEngSrc_ServerInfo_ValueForKey_t )( const char *key );
typedef float						(*pfnEngSrc_GetClientMaxspeed_t )		( void );
typedef int							(*pfnEngSrc_CheckParm_t )				( char *parm, char **ppnext );
typedef void						(*pfnEngSrc_Key_Event_t )				( int key, int down );
typedef void						(*pfnEngSrc_GetMousePosition_t )		( int *mx, int *my );
typedef int							(*pfnEngSrc_IsNoClipping_t )			( void );
typedef struct cl_entity_s *		(*pfnEngSrc_GetLocalPlayer_t )		( void );
typedef struct cl_entity_s *		(*pfnEngSrc_GetViewModel_t )			( void );
typedef struct cl_entity_s *		(*pfnEngSrc_GetEntityByIndex_t )		( int idx );
typedef float						(*pfnEngSrc_GetClientTime_t )			( void );
typedef void						(*pfnEngSrc_V_CalcShake_t )			( void );
typedef void						(*pfnEngSrc_V_ApplyShake_t )			( float *origin, float *angles, float factor );
typedef int							(*pfnEngSrc_PM_PointContents_t )		( float *point, int *truecontents );
typedef int							(*pfnEngSrc_PM_WaterEntity_t )			( float *p );
typedef struct pmtrace_s *			(*pfnEngSrc_PM_TraceLine_t )			( float *start, float *end, int flags, int usehull, int ignore_pe );
typedef struct model_s *			(*pfnEngSrc_CL_LoadModel_t )			( const char *modelname, int *index );
typedef int							(*pfnEngSrc_CL_CreateVisibleEntity_t )	( int type, struct cl_entity_s *ent );
typedef const struct model_s *		(*pfnEngSrc_GetSpritePointer_t )		( HSPRITE hSprite );
typedef void						(*pfnEngSrc_pfnPlaySoundByNameAtLocation_t )	( char *szSound, float volume, float *origin );
typedef unsigned short				(*pfnEngSrc_pfnPrecacheEvent_t )		( int type, const char* psz );
typedef void						(*pfnEngSrc_pfnPlaybackEvent_t )		( int flags, const struct edict_s *pInvoker, unsigned short eventindex, float delay, float *origin, float *angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2 );
typedef void						(*pfnEngSrc_pfnWeaponAnim_t )			( int iAnim, int body );
typedef float						(*pfnEngSrc_pfnRandomFloat_t )			( float flLow, float flHigh );
typedef int32						(*pfnEngSrc_pfnRandomLong_t )			( int32 lLow, int32 lHigh );
typedef void						(*pfnEngSrc_pfnHookEvent_t )			( char *name, void ( *pfnEvent )( struct event_args_s *args ) );
typedef int							(*pfnEngSrc_Con_IsVisible_t)			();
typedef const char *				(*pfnEngSrc_pfnGetGameDirectory_t )	( void );
typedef struct cvar_s *				(*pfnEngSrc_pfnGetCvarPointer_t )		( const char *szName );
typedef const char *				(*pfnEngSrc_Key_LookupBinding_t )		( const char *pBinding );
typedef const char *				(*pfnEngSrc_pfnGetLevelName_t )		( void );
typedef void						(*pfnEngSrc_pfnGetScreenFade_t )		( struct screenfade_s *fade );
typedef void						(*pfnEngSrc_pfnSetScreenFade_t )		( struct screenfade_s *fade );
typedef void *						(*pfnEngSrc_VGui_GetPanel_t )         ( );
typedef void                        (*pfnEngSrc_VGui_ViewportPaintBackground_t ) (int extents[4]);
typedef byte*						(*pfnEngSrc_COM_LoadFile_t )				( char *path, int usehunk, int *pLength );
typedef char*						(*pfnEngSrc_COM_ParseFile_t )			( char *data, char *token );
typedef void						(*pfnEngSrc_COM_FreeFile_t)				( void *buffer );
typedef struct triangleapi_s *		pTriAPI;
typedef struct efx_api_s *			pEfxAPI;
typedef struct event_api_s *		pEventAPI;
typedef struct demo_api_s *			pDemoAPI;
typedef struct net_api_s *			pNetAPI;
typedef struct IVoiceTweak_s *		pVoiceTweak;
typedef int							(*pfnEngSrc_IsSpectateOnly_t ) ( void );
typedef struct model_s *			(*pfnEngSrc_LoadMapSprite_t )			( const char *filename );
typedef void						(*pfnEngSrc_COM_AddAppDirectoryToSearchPath_t ) ( const char *pszBaseDir, const char *appName );
typedef int							(*pfnEngSrc_COM_ExpandFilename_t)				 ( const char *fileName, char *nameOutBuffer, int nameOutBufferSize );
typedef const char *				(*pfnEngSrc_PlayerInfo_ValueForKey_t )( int playerNum, const char *key );
typedef void						(*pfnEngSrc_PlayerInfo_SetValueForKey_t )( const char *key, const char *value );
typedef qboolean					(*pfnEngSrc_GetPlayerUniqueID_t)(int iPlayer, char playerID[16]);
typedef int							(*pfnEngSrc_GetTrackerIDForPlayer_t)(int playerSlot);
typedef int							(*pfnEngSrc_GetPlayerForTrackerID_t)(int trackerID);
typedef int							(*pfnEngSrc_pfnServerCmdUnreliable_t )( char *szCmdString );
typedef void						(*pfnEngSrc_GetMousePos_t )(struct tagPOINT *ppt);
typedef void						(*pfnEngSrc_SetMousePos_t )(int x, int y);
typedef void						(*pfnEngSrc_SetMouseEnable_t)(qboolean fEnable);
typedef struct cvar_s *				(*pfnEngSrc_GetFirstCVarPtr_t)();
typedef unsigned int				(*pfnEngSrc_GetFirstCmdFunctionHandle_t)();
typedef unsigned int				(*pfnEngSrc_GetNextCmdFunctionHandle_t)(unsigned int cmdhandle);
typedef const char *				(*pfnEngSrc_GetCmdFunctionName_t)(unsigned int cmdhandle);
typedef float						(*pfnEngSrc_GetClientOldTime_t)();
typedef float						(*pfnEngSrc_GetServerGravityValue_t)();
typedef struct model_s	*			(*pfnEngSrc_GetModelByIndex_t)( int index );
typedef void						(*pfnEngSrc_pfnSetFilterMode_t )( int mode );
typedef void						(*pfnEngSrc_pfnSetFilterColor_t )( float r, float g, float b );
typedef void						(*pfnEngSrc_pfnSetFilterBrightness_t )( float brightness );
typedef sequenceEntry_s*			(*pfnEngSrc_pfnSequenceGet_t )( const char *fileName, const char* entryName );
typedef void						(*pfnEngSrc_pfnSPR_DrawGeneric_t )( int frame, int x, int y, const struct rect_s *prc, int src, int dest, int w, int h );
typedef sentenceEntry_s*			(*pfnEngSrc_pfnSequencePickSentence_t )( const char *sentenceName, int pickMethod, int* entryPicked );
// draw a complete string
typedef int							(*pfnEngSrc_pfnDrawString_t )		( int x, int y, const char *str, int r, int g, int b );
typedef int							(*pfnEngSrc_pfnDrawStringReverse_t )		( int x, int y, const char *str, int r, int g, int b );
typedef const char *				(*pfnEngSrc_LocalPlayerInfo_ValueForKey_t )( const char *key );
typedef int							(*pfnEngSrc_pfnVGUI2DrawCharacter_t )		( int x, int y, int ch, unsigned int font );
typedef int							(*pfnEngSrc_pfnVGUI2DrawCharacterAdd_t )	( int x, int y, int ch, int r, int g, int b, unsigned int font);
typedef unsigned int		(*pfnEngSrc_COM_GetApproxWavePlayLength ) ( const char * filename);
typedef void *						(*pfnEngSrc_pfnGetCareerUI_t)();
typedef void						(*pfnEngSrc_Cvar_Set_t )			( char *cvar, char *value );
typedef int							(*pfnEngSrc_pfnIsPlayingCareerMatch_t)();
typedef double						(*pfnEngSrc_GetAbsoluteTime_t) ( void );
typedef void						(*pfnEngSrc_pfnProcessTutorMessageDecayBuffer_t)(int *buffer, int bufferLength);
typedef void						(*pfnEngSrc_pfnConstructTutorMessageDecayBuffer_t)(int *buffer, int bufferLength);
typedef void						(*pfnEngSrc_pfnResetTutorMessageDecayData_t)();
typedef void						(*pfnEngSrc_pfnFillRGBABlend_t )			( int x, int y, int width, int height, int r, int g, int b, int a );
typedef int						(*pfnEngSrc_pfnGetAppID_t)			( void );
typedef cmdalias_t*				(*pfnEngSrc_pfnGetAliases_t)		( void );
typedef void					(*pfnEngSrc_pfnVguiWrap2_GetMouseDelta_t) ( int *x, int *y );
typedef int							(*pfnEngSrc_pfnFilteredClientCmd_t) 	( char *szCmdString );

// Pointers to the exported engine functions themselves
typedef struct cl_enginefuncs_s
{
	pfnEngSrc_pfnSPR_Load_t					pfnSPR_Load;
	pfnEngSrc_pfnSPR_Frames_t				pfnSPR_Frames;
	pfnEngSrc_pfnSPR_Height_t				pfnSPR_Height;
	pfnEngSrc_pfnSPR_Width_t				pfnSPR_Width;
	pfnEngSrc_pfnSPR_Set_t					pfnSPR_Set;
	pfnEngSrc_pfnSPR_Draw_t					pfnSPR_Draw;
	pfnEngSrc_pfnSPR_DrawHoles_t			pfnSPR_DrawHoles;
	pfnEngSrc_pfnSPR_DrawAdditive_t			pfnSPR_DrawAdditive;
	pfnEngSrc_pfnSPR_EnableScissor_t		pfnSPR_EnableScissor;
	pfnEngSrc_pfnSPR_DisableScissor_t		pfnSPR_DisableScissor;
	pfnEngSrc_pfnSPR_GetList_t				pfnSPR_GetList;
	pfnEngSrc_pfnFillRGBA_t					pfnFillRGBA;
	pfnEngSrc_pfnGetScreenInfo_t			pfnGetScreenInfo;
	pfnEngSrc_pfnSetCrosshair_t				pfnSetCrosshair;
	pfnEngSrc_pfnRegisterVariable_t			pfnRegisterVariable;
	pfnEngSrc_pfnGetCvarFloat_t				pfnGetCvarFloat;
	pfnEngSrc_pfnGetCvarString_t			pfnGetCvarString;
	pfnEngSrc_pfnAddCommand_t				pfnAddCommand;
	pfnEngSrc_pfnHookUserMsg_t				pfnHookUserMsg;
	pfnEngSrc_pfnServerCmd_t				pfnServerCmd;
	pfnEngSrc_pfnClientCmd_t				pfnClientCmd;
	pfnEngSrc_pfnGetPlayerInfo_t			pfnGetPlayerInfo;
	pfnEngSrc_pfnPlaySoundByName_t			pfnPlaySoundByName;
	pfnEngSrc_pfnPlaySoundByIndex_t			pfnPlaySoundByIndex;
	pfnEngSrc_pfnAngleVectors_t				pfnAngleVectors;
	pfnEngSrc_pfnTextMessageGet_t			pfnTextMessageGet;
	pfnEngSrc_pfnDrawCharacter_t			pfnDrawCharacter;
	pfnEngSrc_pfnDrawConsoleString_t		pfnDrawConsoleString;
	pfnEngSrc_pfnDrawSetTextColor_t			pfnDrawSetTextColor;
	pfnEngSrc_pfnDrawConsoleStringLen_t		pfnDrawConsoleStringLen;
	pfnEngSrc_pfnConsolePrint_t				pfnConsolePrint;
	pfnEngSrc_pfnCenterPrint_t				pfnCenterPrint;
	pfnEngSrc_GetWindowCenterX_t			GetWindowCenterX;
	pfnEngSrc_GetWindowCenterY_t			GetWindowCenterY;
	pfnEngSrc_GetViewAngles_t				GetViewAngles;
	pfnEngSrc_SetViewAngles_t				SetViewAngles;
	pfnEngSrc_GetMaxClients_t				GetMaxClients;
	pfnEngSrc_Cvar_SetValue_t				Cvar_SetValue;
	pfnEngSrc_Cmd_Argc_t					Cmd_Argc;
	pfnEngSrc_Cmd_Argv_t					Cmd_Argv;
	pfnEngSrc_Con_Printf_t					Con_Printf;
	pfnEngSrc_Con_DPrintf_t					Con_DPrintf;
	pfnEngSrc_Con_NPrintf_t					Con_NPrintf;
	pfnEngSrc_Con_NXPrintf_t				Con_NXPrintf;
	pfnEngSrc_PhysInfo_ValueForKey_t		PhysInfo_ValueForKey;
	pfnEngSrc_ServerInfo_ValueForKey_t		ServerInfo_ValueForKey;
	pfnEngSrc_GetClientMaxspeed_t			GetClientMaxspeed;
	pfnEngSrc_CheckParm_t					CheckParm;
	pfnEngSrc_Key_Event_t					Key_Event;
	pfnEngSrc_GetMousePosition_t			GetMousePosition;
	pfnEngSrc_IsNoClipping_t				IsNoClipping;
	pfnEngSrc_GetLocalPlayer_t				GetLocalPlayer;
	pfnEngSrc_GetViewModel_t				GetViewModel;
	pfnEngSrc_GetEntityByIndex_t			GetEntityByIndex;
	pfnEngSrc_GetClientTime_t				GetClientTime;
	pfnEngSrc_V_CalcShake_t					V_CalcShake;
	pfnEngSrc_V_ApplyShake_t				V_ApplyShake;
	pfnEngSrc_PM_PointContents_t			PM_PointContents;
	pfnEngSrc_PM_WaterEntity_t				PM_WaterEntity;
	pfnEngSrc_PM_TraceLine_t				PM_TraceLine;
	pfnEngSrc_CL_LoadModel_t				CL_LoadModel;
	pfnEngSrc_CL_CreateVisibleEntity_t		CL_CreateVisibleEntity;
	pfnEngSrc_GetSpritePointer_t			GetSpritePointer;
	pfnEngSrc_pfnPlaySoundByNameAtLocation_t	pfnPlaySoundByNameAtLocation;
	pfnEngSrc_pfnPrecacheEvent_t			pfnPrecacheEvent;
	pfnEngSrc_pfnPlaybackEvent_t			pfnPlaybackEvent;
	pfnEngSrc_pfnWeaponAnim_t				pfnWeaponAnim;
	pfnEngSrc_pfnRandomFloat_t				pfnRandomFloat;
	pfnEngSrc_pfnRandomLong_t				pfnRandomLong;
	pfnEngSrc_pfnHookEvent_t				pfnHookEvent;
	pfnEngSrc_Con_IsVisible_t				Con_IsVisible;
	pfnEngSrc_pfnGetGameDirectory_t			pfnGetGameDirectory;
	pfnEngSrc_pfnGetCvarPointer_t			pfnGetCvarPointer;
	pfnEngSrc_Key_LookupBinding_t			Key_LookupBinding;
	pfnEngSrc_pfnGetLevelName_t				pfnGetLevelName;
	pfnEngSrc_pfnGetScreenFade_t			pfnGetScreenFade;
	pfnEngSrc_pfnSetScreenFade_t			pfnSetScreenFade;
	pfnEngSrc_VGui_GetPanel_t				VGui_GetPanel;
	pfnEngSrc_VGui_ViewportPaintBackground_t	VGui_ViewportPaintBackground;
	pfnEngSrc_COM_LoadFile_t				COM_LoadFile;
	pfnEngSrc_COM_ParseFile_t				COM_ParseFile;
	pfnEngSrc_COM_FreeFile_t				COM_FreeFile;
	struct triangleapi_s		*pTriAPI;
	struct efx_api_s			*pEfxAPI;
	struct event_api_s			*pEventAPI;
	struct demo_api_s			*pDemoAPI;
	struct net_api_s			*pNetAPI;
	struct IVoiceTweak_s		*pVoiceTweak;
	pfnEngSrc_IsSpectateOnly_t				IsSpectateOnly;
	pfnEngSrc_LoadMapSprite_t				LoadMapSprite;
	pfnEngSrc_COM_AddAppDirectoryToSearchPath_t		COM_AddAppDirectoryToSearchPath;
	pfnEngSrc_COM_ExpandFilename_t			COM_ExpandFilename;
	pfnEngSrc_PlayerInfo_ValueForKey_t		PlayerInfo_ValueForKey;
	pfnEngSrc_PlayerInfo_SetValueForKey_t	PlayerInfo_SetValueForKey;
	pfnEngSrc_GetPlayerUniqueID_t			GetPlayerUniqueID;
	pfnEngSrc_GetTrackerIDForPlayer_t		GetTrackerIDForPlayer;
	pfnEngSrc_GetPlayerForTrackerID_t		GetPlayerForTrackerID;
	pfnEngSrc_pfnServerCmdUnreliable_t		pfnServerCmdUnreliable;
	pfnEngSrc_GetMousePos_t					pfnGetMousePos;
	pfnEngSrc_SetMousePos_t					pfnSetMousePos;
	pfnEngSrc_SetMouseEnable_t				pfnSetMouseEnable;
	pfnEngSrc_GetFirstCVarPtr_t				GetFirstCvarPtr;
	pfnEngSrc_GetFirstCmdFunctionHandle_t	GetFirstCmdFunctionHandle;
	pfnEngSrc_GetNextCmdFunctionHandle_t	GetNextCmdFunctionHandle;
	pfnEngSrc_GetCmdFunctionName_t			GetCmdFunctionName;
	pfnEngSrc_GetClientOldTime_t			hudGetClientOldTime;
	pfnEngSrc_GetServerGravityValue_t		hudGetServerGravityValue;
	pfnEngSrc_GetModelByIndex_t				hudGetModelByIndex;
	pfnEngSrc_pfnSetFilterMode_t			pfnSetFilterMode;
	pfnEngSrc_pfnSetFilterColor_t			pfnSetFilterColor;
	pfnEngSrc_pfnSetFilterBrightness_t		pfnSetFilterBrightness;
	pfnEngSrc_pfnSequenceGet_t				pfnSequenceGet;
	pfnEngSrc_pfnSPR_DrawGeneric_t			pfnSPR_DrawGeneric;
	pfnEngSrc_pfnSequencePickSentence_t		pfnSequencePickSentence;
	pfnEngSrc_pfnDrawString_t				pfnDrawString;
	pfnEngSrc_pfnDrawStringReverse_t				pfnDrawStringReverse;
	pfnEngSrc_LocalPlayerInfo_ValueForKey_t		LocalPlayerInfo_ValueForKey;
	pfnEngSrc_pfnVGUI2DrawCharacter_t		pfnVGUI2DrawCharacter;
	pfnEngSrc_pfnVGUI2DrawCharacterAdd_t	pfnVGUI2DrawCharacterAdd;
	pfnEngSrc_COM_GetApproxWavePlayLength	COM_GetApproxWavePlayLength;
	pfnEngSrc_pfnGetCareerUI_t				pfnGetCareerUI;
	pfnEngSrc_Cvar_Set_t					Cvar_Set;
	pfnEngSrc_pfnIsPlayingCareerMatch_t		pfnIsCareerMatch;
	pfnEngSrc_pfnPlaySoundVoiceByName_t	pfnPlaySoundVoiceByName;
	pfnEngSrc_pfnPrimeMusicStream_t		pfnPrimeMusicStream;
	pfnEngSrc_GetAbsoluteTime_t				GetAbsoluteTime;
	pfnEngSrc_pfnProcessTutorMessageDecayBuffer_t		pfnProcessTutorMessageDecayBuffer;
	pfnEngSrc_pfnConstructTutorMessageDecayBuffer_t		pfnConstructTutorMessageDecayBuffer;
	pfnEngSrc_pfnResetTutorMessageDecayData_t		pfnResetTutorMessageDecayData;
	pfnEngSrc_pfnPlaySoundByNameAtPitch_t	pfnPlaySoundByNameAtPitch;
	pfnEngSrc_pfnFillRGBABlend_t					pfnFillRGBABlend;
	pfnEngSrc_pfnGetAppID_t					pfnGetAppID;
	pfnEngSrc_pfnGetAliases_t				pfnGetAliasList;
	pfnEngSrc_pfnVguiWrap2_GetMouseDelta_t pfnVguiWrap2_GetMouseDelta;
	pfnEngSrc_pfnFilteredClientCmd_t		pfnFilteredClientCmd;
} cl_enginefunc_t;

// Function type declarations for engine destination functions
typedef void	(*pfnEngDst_pfnSPR_Load_t )				( const char ** );
typedef void	(*pfnEngDst_pfnSPR_Frames_t )			( HSPRITE * );
typedef void	(*pfnEngDst_pfnSPR_Height_t )			( HSPRITE *, int * );
typedef void	(*pfnEngDst_pfnSPR_Width_t )			( HSPRITE *, int * );
typedef void	(*pfnEngDst_pfnSPR_Set_t )				( HSPRITE *, int *, int *, int * );
typedef void	(*pfnEngDst_pfnSPR_Draw_t )				( int *, int *, int *, const struct rect_s ** );
typedef void	(*pfnEngDst_pfnSPR_DrawHoles_t )		( int *, int *, int *, const struct rect_s ** );
typedef void	(*pfnEngDst_pfnSPR_DrawAdditive_t )		( int *, int *, int *, const struct rect_s ** );
typedef void	(*pfnEngDst_pfnSPR_EnableScissor_t )	( int *, int *, int *, int * );
typedef void	(*pfnEngDst_pfnSPR_DisableScissor_t )	( void );
typedef void	(*pfnEngDst_pfnSPR_GetList_t )			( char **, int ** );
typedef void	(*pfnEngDst_pfnFillRGBA_t )				( int *, int *, int *, int *, int *, int *, int *, int * );
typedef void	(*pfnEngDst_pfnGetScreenInfo_t ) 		( struct SCREENINFO_s ** );
typedef void	(*pfnEngDst_pfnSetCrosshair_t )			( HSPRITE *, struct rect_s *, int *, int *, int * );
typedef void	(*pfnEngDst_pfnRegisterVariable_t )		( char **, char **, int * );
typedef void	(*pfnEngDst_pfnGetCvarFloat_t )			( char ** );
typedef void	(*pfnEngDst_pfnGetCvarString_t )		( char ** );
typedef void	(*pfnEngDst_pfnAddCommand_t )			( char **, void (**pfnEngDst_function)(void) );
typedef void	(*pfnEngDst_pfnHookUserMsg_t )			( char **, pfnUserMsgHook * );
typedef void	(*pfnEngDst_pfnServerCmd_t )			( char ** );
typedef void	(*pfnEngDst_pfnClientCmd_t )			( char ** );
typedef void	(*pfnEngDst_pfnPrimeMusicStream_t )	( char **, int *);
typedef void	(*pfnEngDst_pfnGetPlayerInfo_t )		( int *, struct hud_player_info_s ** );
typedef void	(*pfnEngDst_pfnPlaySoundByName_t )		( char **, float * );
typedef void	(*pfnEngDst_pfnPlaySoundByNameAtPitch_t )	( char **, float *, int * );
typedef void	(*pfnEngDst_pfnPlaySoundVoiceByName_t )	(char **, float * );
typedef void	(*pfnEngDst_pfnPlaySoundByIndex_t )		( int *, float * );
typedef void	(*pfnEngDst_pfnAngleVectors_t )			( const float * *, float * *, float * *, float * * );
typedef void	(*pfnEngDst_pfnTextMessageGet_t )		( const char ** );
typedef void	(*pfnEngDst_pfnDrawCharacter_t )		( int *, int *, int *, int *, int *, int * );
typedef void	(*pfnEngDst_pfnDrawConsoleString_t )	( int *, int *, char ** );
typedef void	(*pfnEngDst_pfnDrawSetTextColor_t )		( float *, float *, float * );
typedef void	(*pfnEngDst_pfnDrawConsoleStringLen_t )	(  const char **, int **, int ** );
typedef void	(*pfnEngDst_pfnConsolePrint_t )			( const char ** );
typedef void	(*pfnEngDst_pfnCenterPrint_t )			( const char ** );
typedef void	(*pfnEngDst_GetWindowCenterX_t )		( void );
typedef void	(*pfnEngDst_GetWindowCenterY_t )		( void );
typedef void	(*pfnEngDst_GetViewAngles_t )			( float ** );
typedef void	(*pfnEngDst_SetViewAngles_t )			( float ** );
typedef void	(*pfnEngDst_GetMaxClients_t )			( void );
typedef void	(*pfnEngDst_Cvar_SetValue_t )			( char **, float * );
typedef void    (*pfnEngDst_Cmd_Argc_t)					(void);	
typedef void	(*pfnEngDst_Cmd_Argv_t )				( int * );
typedef void	(*pfnEngDst_Con_Printf_t )				( char **);
typedef void	(*pfnEngDst_Con_DPrintf_t )				( char **);
typedef void	(*pfnEngDst_Con_NPrintf_t )				( int *, char ** );
typedef void	(*pfnEngDst_Con_NXPrintf_t )			( struct con_nprint_s **, char **);
typedef void	(*pfnEngDst_PhysInfo_ValueForKey_t )	( const char ** );
typedef void	(*pfnEngDst_ServerInfo_ValueForKey_t )	( const char ** );
typedef void	(*pfnEngDst_GetClientMaxspeed_t )		( void );
typedef void	(*pfnEngDst_CheckParm_t )				( char **, char *** );
typedef void	(*pfnEngDst_Key_Event_t )				( int *, int * );
typedef void	(*pfnEngDst_GetMousePosition_t )		( int **, int ** );
typedef void	(*pfnEngDst_IsNoClipping_t )			( void );
typedef void	(*pfnEngDst_GetLocalPlayer_t )			( void );
typedef void	(*pfnEngDst_GetViewModel_t )			( void );
typedef void	(*pfnEngDst_GetEntityByIndex_t )		( int * );
typedef void	(*pfnEngDst_GetClientTime_t )			( void );
typedef void	(*pfnEngDst_V_CalcShake_t )				( void );
typedef void	(*pfnEngDst_V_ApplyShake_t )			( float **, float **, float * );
typedef void	(*pfnEngDst_PM_PointContents_t )		( float **, int ** );
typedef void	(*pfnEngDst_PM_WaterEntity_t )			( float ** );
typedef void	(*pfnEngDst_PM_TraceLine_t )			( float **, float **, int *, int *, int * );
typedef void	(*pfnEngDst_CL_LoadModel_t )			( const char **, int ** );
typedef void	(*pfnEngDst_CL_CreateVisibleEntity_t )	( int *, struct cl_entity_s ** );
typedef void	(*pfnEngDst_GetSpritePointer_t )		( HSPRITE * );
typedef void	(*pfnEngDst_pfnPlaySoundByNameAtLocation_t )	( char **, float *, float ** );
typedef void	(*pfnEngDst_pfnPrecacheEvent_t )		( int *, const char* * );
typedef void	(*pfnEngDst_pfnPlaybackEvent_t )		( int *, const struct edict_s **, unsigned short *, float *, float **, float **, float *, float *, int *, int *, int *, int * );
typedef void	(*pfnEngDst_pfnWeaponAnim_t )			( int *, int * );
typedef void	(*pfnEngDst_pfnRandomFloat_t )			( float *, float * );
typedef void	(*pfnEngDst_pfnRandomLong_t )			( int32 *, int32 * );
typedef void	(*pfnEngDst_pfnHookEvent_t )			( char **, void ( **pfnEvent )( struct event_args_s *args ) );
typedef void	(*pfnEngDst_Con_IsVisible_t)			();
typedef void	(*pfnEngDst_pfnGetGameDirectory_t )		( void );
typedef void	(*pfnEngDst_pfnGetCvarPointer_t )		( const char ** );
typedef void	(*pfnEngDst_Key_LookupBinding_t )		( const char ** );
typedef void	(*pfnEngDst_pfnGetLevelName_t )			( void );
typedef void	(*pfnEngDst_pfnGetScreenFade_t )		( struct screenfade_s ** );
typedef void	(*pfnEngDst_pfnSetScreenFade_t )		( struct screenfade_s ** );
typedef void	(*pfnEngDst_VGui_GetPanel_t )			( );
typedef void	(*pfnEngDst_VGui_ViewportPaintBackground_t ) (int **);
typedef void	(*pfnEngDst_COM_LoadFile_t )			( char **, int *, int ** );
typedef void	(*pfnEngDst_COM_ParseFile_t )			( char **, char ** );
typedef void	(*pfnEngDst_COM_FreeFile_t)				( void ** );
typedef void	(*pfnEngDst_IsSpectateOnly_t )			( void );
typedef void	(*pfnEngDst_LoadMapSprite_t )			( const char ** );
typedef void	(*pfnEngDst_COM_AddAppDirectoryToSearchPath_t ) ( const char **, const char ** );
typedef void	(*pfnEngDst_COM_ExpandFilename_t)		( const char **, char **, int * );
typedef void	(*pfnEngDst_PlayerInfo_ValueForKey_t )	( int *, const char ** );
typedef void	(*pfnEngDst_PlayerInfo_SetValueForKey_t )( const char **, const char ** );
typedef void	(*pfnEngDst_GetPlayerUniqueID_t)		(int *, char **);
typedef void	(*pfnEngDst_GetTrackerIDForPlayer_t)	(int *);
typedef void	(*pfnEngDst_GetPlayerForTrackerID_t)	(int *);
typedef void	(*pfnEngDst_pfnServerCmdUnreliable_t )	( char ** );
typedef void	(*pfnEngDst_GetMousePos_t )				(struct tagPOINT **);
typedef void	(*pfnEngDst_SetMousePos_t )				(int *, int *);
typedef void	(*pfnEngDst_SetMouseEnable_t )			(qboolean *);
typedef void	(*pfnEngDst_pfnSetFilterMode_t)			( int * );
typedef void	(*pfnEngDst_pfnSetFilterColor_t)		( float *, float *, float * );
typedef void	(*pfnEngDst_pfnSetFilterBrightness_t)	( float * );
typedef void	(*pfnEngDst_pfnSequenceGet_t )			( const char**, const char** );
typedef void	(*pfnEngDst_pfnSPR_DrawGeneric_t )		( int *, int *, int *, const struct rect_s **, int *, int *, int *, int * );
typedef void	(*pfnEngDst_pfnSequencePickSentence_t )	( const char**, int *, int ** );
typedef void	(*pfnEngDst_pfnDrawString_t )			( int *, int *, const char *, int *, int *, int * );
typedef void	(*pfnEngDst_pfnDrawStringReverse_t )			( int *, int *, const char *, int *, int *, int * );
typedef void	(*pfnEngDst_LocalPlayerInfo_ValueForKey_t )( const char **);
typedef void	(*pfnEngDst_pfnVGUI2DrawCharacter_t )		( int *, int *, int *, unsigned int * );
typedef void	(*pfnEngDst_pfnVGUI2DrawCharacterAdd_t )	( int *, int *, int *, int *, int *, int *, unsigned int *);
typedef void	(*pfnEngDst_pfnProcessTutorMessageDecayBuffer_t )(int **, int *);
typedef void	(*pfnEngDst_pfnConstructTutorMessageDecayBuffer_t )(int **, int *);
typedef void	(*pfnEngDst_pfnResetTutorMessageDecayData_t)();
typedef void	(*pfnEngDst_pfnFillRGBABlend_t )				( int *, int *, int *, int *, int *, int *, int *, int * );
typedef void	(*pfnEngDst_pfnGetAppID_t )				( void );
typedef void	(*pfnEngDst_pfnGetAliases_t )				( void );
typedef void	(*pfnEngDst_pfnVguiWrap2_GetMouseDelta_t) ( int *x, int *y );
typedef void	(*pfnEngDst_pfnFilteredClientCmd_t )	( char ** );


// Pointers to the engine destination functions
typedef struct
{
	pfnEngDst_pfnSPR_Load_t					pfnSPR_Load;
	pfnEngDst_pfnSPR_Frames_t				pfnSPR_Frames;
	pfnEngDst_pfnSPR_Height_t				pfnSPR_Height;
	pfnEngDst_pfnSPR_Width_t				pfnSPR_Width;
	pfnEngDst_pfnSPR_Set_t					pfnSPR_Set;
	pfnEngDst_pfnSPR_Draw_t					pfnSPR_Draw;
	pfnEngDst_pfnSPR_DrawHoles_t			pfnSPR_DrawHoles;
	pfnEngDst_pfnSPR_DrawAdditive_t			pfnSPR_DrawAdditive;
	pfnEngDst_pfnSPR_EnableScissor_t		pfnSPR_EnableScissor;
	pfnEngDst_pfnSPR_DisableScissor_t		pfnSPR_DisableScissor;
	pfnEngDst_pfnSPR_GetList_t				pfnSPR_GetList;
	pfnEngDst_pfnFillRGBA_t					pfnFillRGBA;
	pfnEngDst_pfnGetScreenInfo_t			pfnGetScreenInfo;
	pfnEngDst_pfnSetCrosshair_t				pfnSetCrosshair;
	pfnEngDst_pfnRegisterVariable_t			pfnRegisterVariable;
	pfnEngDst_pfnGetCvarFloat_t				pfnGetCvarFloat;
	pfnEngDst_pfnGetCvarString_t			pfnGetCvarString;
	pfnEngDst_pfnAddCommand_t				pfnAddCommand;
	pfnEngDst_pfnHookUserMsg_t				pfnHookUserMsg;
	pfnEngDst_pfnServerCmd_t				pfnServerCmd;
	pfnEngDst_pfnClientCmd_t				pfnClientCmd;
	pfnEngDst_pfnGetPlayerInfo_t			pfnGetPlayerInfo;
	pfnEngDst_pfnPlaySoundByName_t			pfnPlaySoundByName;
	pfnEngDst_pfnPlaySoundByIndex_t			pfnPlaySoundByIndex;
	pfnEngDst_pfnAngleVectors_t				pfnAngleVectors;
	pfnEngDst_pfnTextMessageGet_t			pfnTextMessageGet;
	pfnEngDst_pfnDrawCharacter_t			pfnDrawCharacter;
	pfnEngDst_pfnDrawConsoleString_t		pfnDrawConsoleString;
	pfnEngDst_pfnDrawSetTextColor_t			pfnDrawSetTextColor;
	pfnEngDst_pfnDrawConsoleStringLen_t		pfnDrawConsoleStringLen;
	pfnEngDst_pfnConsolePrint_t				pfnConsolePrint;
	pfnEngDst_pfnCenterPrint_t				pfnCenterPrint;
	pfnEngDst_GetWindowCenterX_t			GetWindowCenterX;
	pfnEngDst_GetWindowCenterY_t			GetWindowCenterY;
	pfnEngDst_GetViewAngles_t				GetViewAngles;
	pfnEngDst_SetViewAngles_t				SetViewAngles;
	pfnEngDst_GetMaxClients_t				GetMaxClients;
	pfnEngDst_Cvar_SetValue_t				Cvar_SetValue;
	pfnEngDst_Cmd_Argc_t					Cmd_Argc;
	pfnEngDst_Cmd_Argv_t					Cmd_Argv;
	pfnEngDst_Con_Printf_t					Con_Printf;
	pfnEngDst_Con_DPrintf_t					Con_DPrintf;
	pfnEngDst_Con_NPrintf_t					Con_NPrintf;
	pfnEngDst_Con_NXPrintf_t				Con_NXPrintf;
	pfnEngDst_PhysInfo_ValueForKey_t		PhysInfo_ValueForKey;
	pfnEngDst_ServerInfo_ValueForKey_t		ServerInfo_ValueForKey;
	pfnEngDst_GetClientMaxspeed_t			GetClientMaxspeed;
	pfnEngDst_CheckParm_t					CheckParm;
	pfnEngDst_Key_Event_t					Key_Event;
	pfnEngDst_GetMousePosition_t			GetMousePosition;
	pfnEngDst_IsNoClipping_t				IsNoClipping;
	pfnEngDst_GetLocalPlayer_t				GetLocalPlayer;
	pfnEngDst_GetViewModel_t				GetViewModel;
	pfnEngDst_GetEntityByIndex_t			GetEntityByIndex;
	pfnEngDst_GetClientTime_t				GetClientTime;
	pfnEngDst_V_CalcShake_t					V_CalcShake;
	pfnEngDst_V_ApplyShake_t				V_ApplyShake;
	pfnEngDst_PM_PointContents_t			PM_PointContents;
	pfnEngDst_PM_WaterEntity_t				PM_WaterEntity;
	pfnEngDst_PM_TraceLine_t				PM_TraceLine;
	pfnEngDst_CL_LoadModel_t				CL_LoadModel;
	pfnEngDst_CL_CreateVisibleEntity_t		CL_CreateVisibleEntity;
	pfnEngDst_GetSpritePointer_t			GetSpritePointer;
	pfnEngDst_pfnPlaySoundByNameAtLocation_t	pfnPlaySoundByNameAtLocation;
	pfnEngDst_pfnPrecacheEvent_t			pfnPrecacheEvent;
	pfnEngDst_pfnPlaybackEvent_t			pfnPlaybackEvent;
	pfnEngDst_pfnWeaponAnim_t				pfnWeaponAnim;
	pfnEngDst_pfnRandomFloat_t				pfnRandomFloat;
	pfnEngDst_pfnRandomLong_t				pfnRandomLong;
	pfnEngDst_pfnHookEvent_t				pfnHookEvent;
	pfnEngDst_Con_IsVisible_t				Con_IsVisible;
	pfnEngDst_pfnGetGameDirectory_t			pfnGetGameDirectory;
	pfnEngDst_pfnGetCvarPointer_t			pfnGetCvarPointer;
	pfnEngDst_Key_LookupBinding_t			Key_LookupBinding;
	pfnEngDst_pfnGetLevelName_t				pfnGetLevelName;
	pfnEngDst_pfnGetScreenFade_t			pfnGetScreenFade;
	pfnEngDst_pfnSetScreenFade_t			pfnSetScreenFade;
	pfnEngDst_VGui_GetPanel_t				VGui_GetPanel;
	pfnEngDst_VGui_ViewportPaintBackground_t	VGui_ViewportPaintBackground;
	pfnEngDst_COM_LoadFile_t				COM_LoadFile;
	pfnEngDst_COM_ParseFile_t				COM_ParseFile;
	pfnEngDst_COM_FreeFile_t				COM_FreeFile;
	struct triangleapi_s		*pTriAPI;
	struct efx_api_s			*pEfxAPI;
	struct event_api_s			*pEventAPI;
	struct demo_api_s			*pDemoAPI;
	struct net_api_s			*pNetAPI;
	struct IVoiceTweak_s		*pVoiceTweak;
	pfnEngDst_IsSpectateOnly_t				IsSpectateOnly;
	pfnEngDst_LoadMapSprite_t				LoadMapSprite;
	pfnEngDst_COM_AddAppDirectoryToSearchPath_t		COM_AddAppDirectoryToSearchPath;
	pfnEngDst_COM_ExpandFilename_t			COM_ExpandFilename;
	pfnEngDst_PlayerInfo_ValueForKey_t		PlayerInfo_ValueForKey;
	pfnEngDst_PlayerInfo_SetValueForKey_t	PlayerInfo_SetValueForKey;
	pfnEngDst_GetPlayerUniqueID_t			GetPlayerUniqueID;
	pfnEngDst_GetTrackerIDForPlayer_t		GetTrackerIDForPlayer;
	pfnEngDst_GetPlayerForTrackerID_t		GetPlayerForTrackerID;
	pfnEngDst_pfnServerCmdUnreliable_t		pfnServerCmdUnreliable;
	pfnEngDst_GetMousePos_t					pfnGetMousePos;
	pfnEngDst_SetMousePos_t					pfnSetMousePos;
	pfnEngDst_SetMouseEnable_t				pfnSetMouseEnable;
	pfnEngDst_pfnSetFilterMode_t			pfnSetFilterMode ;
	pfnEngDst_pfnSetFilterColor_t			pfnSetFilterColor ;
	pfnEngDst_pfnSetFilterBrightness_t		pfnSetFilterBrightness ;
	pfnEngDst_pfnSequenceGet_t				pfnSequenceGet;
	pfnEngDst_pfnSPR_DrawGeneric_t			pfnSPR_DrawGeneric;
	pfnEngDst_pfnSequencePickSentence_t		pfnSequencePickSentence;
	pfnEngDst_pfnDrawString_t				pfnDrawString;
	pfnEngDst_pfnDrawString_t				pfnDrawStringReverse;
	pfnEngDst_LocalPlayerInfo_ValueForKey_t	LocalPlayerInfo_ValueForKey;
	pfnEngDst_pfnVGUI2DrawCharacter_t		pfnVGUI2DrawCharacter;
	pfnEngDst_pfnVGUI2DrawCharacterAdd_t	pfnVGUI2DrawCharacterAdd;
	pfnEngDst_pfnPlaySoundVoiceByName_t	pfnPlaySoundVoiceByName;
	pfnEngDst_pfnPrimeMusicStream_t			pfnPrimeMusicStream;
	pfnEngDst_pfnProcessTutorMessageDecayBuffer_t		pfnProcessTutorMessageDecayBuffer;
	pfnEngDst_pfnConstructTutorMessageDecayBuffer_t		pfnConstructTutorMessageDecayBuffer;
	pfnEngDst_pfnResetTutorMessageDecayData_t		pfnResetTutorMessageDecayData;
	pfnEngDst_pfnPlaySoundByNameAtPitch_t	pfnPlaySoundByNameAtPitch;
	pfnEngDst_pfnFillRGBABlend_t					pfnFillRGBABlend;
	pfnEngDst_pfnGetAppID_t							pfnGetAppID;
	pfnEngDst_pfnGetAliases_t				pfnGetAliasList;
	pfnEngDst_pfnVguiWrap2_GetMouseDelta_t	pfnVguiWrap2_GetMouseDelta;
	pfnEngDst_pfnFilteredClientCmd_t		pfnFilteredClientCmd;
} cl_enginefunc_dst_t;


// ********************************************************
// Functions exposed by the engine to the module
// ********************************************************

// Functions for ModuleS
typedef void (*PFN_KICKPLAYER)(int nPlayerSlot, int nReason);

typedef struct modshelpers_s
{
	PFN_KICKPLAYER m_pfnKickPlayer;

	// reserved for future expansion
	int m_nVoid1;
	int m_nVoid2;
	int m_nVoid3;
	int m_nVoid4;
	int m_nVoid5;
	int m_nVoid6;
	int m_nVoid7;
	int m_nVoid8;
	int m_nVoid9;
} modshelpers_t;

// Functions for moduleC
typedef struct modchelpers_s
{
	// reserved for future expansion
	int m_nVoid0;
	int m_nVoid1;
	int m_nVoid2;
	int m_nVoid3;
	int m_nVoid4;
	int m_nVoid5;
	int m_nVoid6;
	int m_nVoid7;
	int m_nVoid8;
	int m_nVoid9;
} modchelpers_t;


// ********************************************************
// Information about the engine
// ********************************************************
typedef struct engdata_s
{
	cl_enginefunc_t	*pcl_enginefuncs;		// functions exported by the engine
	cl_enginefunc_dst_t *pg_engdstAddrs;	// destination handlers for engine exports
	cldll_func_t *pcl_funcs;				// client exports
	cldll_func_dst_t *pg_cldstAddrs;		// client export destination handlers
	struct modfuncs_s *pg_modfuncs;			// engine's pointer to module functions
	struct cmd_function_s **pcmd_functions;	// list of all registered commands
	void *pkeybindings;						// all key bindings (not really a void *, but easier this way)
	void (*pfnConPrintf)(char *, ...);		// dump to console
	struct cvar_s **pcvar_vars;				// pointer to head of cvar list
	struct glwstate_t *pglwstate;			// OpenGl information
	void *(*pfnSZ_GetSpace)(struct sizebuf_s *, int); // pointer to SZ_GetSpace
	struct modfuncs_s *pmodfuncs;			// &g_modfuncs
	void *pfnGetProcAddress;				// &GetProcAddress
	void *pfnGetModuleHandle;				// &GetModuleHandle
	struct server_static_s *psvs;			// &svs
	struct client_static_s *pcls;			// &cls
	void (*pfnSV_DropClient)(struct client_s *, qboolean, char *, ...);	// pointer to SV_DropClient
	void (*pfnNetchan_Transmit)(struct netchan_s *, int, byte *);		// pointer to Netchan_Transmit
	void (*pfnNET_SendPacket)(enum netsrc_s sock, int length, void *data, netadr_t to); // &NET_SendPacket
	struct cvar_s *(*pfnCvarFindVar)(const char *pchName);				// pointer to Cvar_FindVar
	int *phinstOpenGlEarly;					// &g_hinstOpenGlEarly

	// Reserved for future expansion
	void *pVoid0;							// reserved for future expan
	void *pVoid1;							// reserved for future expan
	void *pVoid2;							// reserved for future expan
	void *pVoid3;							// reserved for future expan
	void *pVoid4;							// reserved for future expan
	void *pVoid5;							// reserved for future expan
	void *pVoid6;							// reserved for future expan
	void *pVoid7;							// reserved for future expan
	void *pVoid8;							// reserved for future expan
	void *pVoid9;							// reserved for future expan
} engdata_t;


// ********************************************************
// Functions exposed by the security module
// ********************************************************
typedef void (*PFN_LOADMOD)(char *pchModule);
typedef void (*PFN_CLOSEMOD)(void);
typedef int (*PFN_NCALL)(int ijump, int cnArg, ...);

typedef void (*PFN_GETCLDSTADDRS)(cldll_func_dst_t *pcldstAddrs);
typedef void (*PFN_GETENGDSTADDRS)(cl_enginefunc_dst_t *pengdstAddrs);
typedef void (*PFN_MODULELOADED)(void);

typedef void (*PFN_PROCESSOUTGOINGNET)(struct netchan_s *pchan, struct sizebuf_s *psizebuf);
typedef qboolean (*PFN_PROCESSINCOMINGNET)(struct netchan_s *pchan, struct sizebuf_s *psizebuf);

typedef void (*PFN_TEXTURELOAD)(char *pszName, int dxWidth, int dyHeight, char *pbData);
typedef void (*PFN_MODELLOAD)(struct model_s *pmodel, void *pvBuf);

typedef void (*PFN_FRAMEBEGIN)(void);
typedef void (*PFN_FRAMERENDER1)(void);
typedef void (*PFN_FRAMERENDER2)(void);

typedef void (*PFN_SETMODSHELPERS)(modshelpers_t *pmodshelpers);
typedef void (*PFN_SETMODCHELPERS)(modchelpers_t *pmodchelpers);
typedef void (*PFN_SETENGDATA)(engdata_t *pengdata);

typedef void (*PFN_CONNECTCLIENT)(int iPlayer);
typedef void (*PFN_RECORDIP)(unsigned int pnIP);
typedef void (*PFN_PLAYERSTATUS)(unsigned char *pbData, int cbData);

typedef void (*PFN_SETENGINEVERSION)(int nVersion);

// typedef class CMachine *(*PFN_PCMACHINE)(void);
typedef int (*PFN_PCMACHINE)(void);
typedef void (*PFN_SETIP)(int ijump);
typedef void (*PFN_EXECUTE)(void);

typedef struct modfuncs_s
{
	// Functions for the pcode interpreter
	PFN_LOADMOD m_pfnLoadMod;
	PFN_CLOSEMOD m_pfnCloseMod;
	PFN_NCALL m_pfnNCall;

	// API destination functions
	PFN_GETCLDSTADDRS m_pfnGetClDstAddrs;
	PFN_GETENGDSTADDRS m_pfnGetEngDstAddrs;

	// Miscellaneous functions
	PFN_MODULELOADED m_pfnModuleLoaded;     // Called right after the module is loaded

	// Functions for processing network traffic
	PFN_PROCESSOUTGOINGNET m_pfnProcessOutgoingNet;   // Every outgoing packet gets run through this
	PFN_PROCESSINCOMINGNET m_pfnProcessIncomingNet;   // Every incoming packet gets run through this

	// Resource functions
	PFN_TEXTURELOAD m_pfnTextureLoad;     // Called as each texture is loaded
	PFN_MODELLOAD m_pfnModelLoad;         // Called as each model is loaded

	// Functions called every frame
	PFN_FRAMEBEGIN m_pfnFrameBegin;       // Called at the beginning of each frame cycle
	PFN_FRAMERENDER1 m_pfnFrameRender1;   // Called at the beginning of the render loop
	PFN_FRAMERENDER2 m_pfnFrameRender2;   // Called at the end of the render loop

	// Module helper transfer
	PFN_SETMODSHELPERS m_pfnSetModSHelpers;
	PFN_SETMODCHELPERS m_pfnSetModCHelpers;
	PFN_SETENGDATA m_pfnSetEngData;

	// Which version of the module is this?
	int m_nVersion;

	// Miscellaneous game stuff
	PFN_CONNECTCLIENT m_pfnConnectClient;	// Called whenever a new client connects
	PFN_RECORDIP m_pfnRecordIP;				// Secure master has reported a new IP for us
	PFN_PLAYERSTATUS m_pfnPlayerStatus;		// Called whenever we receive a PlayerStatus packet

	// Recent additions
	PFN_SETENGINEVERSION m_pfnSetEngineVersion;	// 1 = patched engine

	// reserved for future expansion
	int m_nVoid2;
	int m_nVoid3;
	int m_nVoid4;
	int m_nVoid5;
	int m_nVoid6;
	int m_nVoid7;
	int m_nVoid8;
	int m_nVoid9;
} modfuncs_t;


#define k_nEngineVersion15Base		0
#define k_nEngineVersion15Patch		1
#define k_nEngineVersion16Base		2
#define k_nEngineVersion16Validated	3		// 1.6 engine with built-in validation


typedef struct validator_s
{
	int m_nRandomizer;			// Random number to be XOR'd into all subsequent fields
	int m_nSignature1;			// First signature that identifies this structure
	int m_nSignature2;			// Second signature
	int m_pbCode;				// Beginning of the code block
	int m_cbCode;				// Size of the code block
	int m_nChecksum;			// Checksum of the code block
	int m_nSpecial;				// For engine, 1 if hw.dll, 0 if sw.dll.  For client, pclfuncs checksum
	int m_nCompensator;			// Keeps the checksum correct
} validator_t;


#define k_nChecksumCompensator 0x36a8f09c	// Don't change this value: it's hardcorded in cdll_int.cpp, 

#define k_nModuleVersionCur 0x43210004


#endif // __APIPROXY__
