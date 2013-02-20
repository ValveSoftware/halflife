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
#ifndef CLIENT_H
#define CLIENT_H

extern void respawn( entvars_t* pev, BOOL fCopyCorpse );
extern BOOL ClientConnect( edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[ 128 ] );
extern void ClientDisconnect( edict_t *pEntity );
extern void ClientKill( edict_t *pEntity );
extern void ClientPutInServer( edict_t *pEntity );
extern void ClientCommand( edict_t *pEntity );
extern void ClientUserInfoChanged( edict_t *pEntity, char *infobuffer );
extern void ServerActivate( edict_t *pEdictList, int edictCount, int clientMax );
extern void ServerDeactivate( void );
extern void StartFrame( void );
extern void PlayerPostThink( edict_t *pEntity );
extern void PlayerPreThink( edict_t *pEntity );
extern void ParmsNewLevel( void );
extern void ParmsChangeLevel( void );

extern void ClientPrecache( void );

extern const char *GetGameDescription( void );
extern void PlayerCustomization( edict_t *pEntity, customization_t *pCust );

extern void SpectatorConnect ( edict_t *pEntity );
extern void SpectatorDisconnect ( edict_t *pEntity );
extern void SpectatorThink ( edict_t *pEntity );

extern void Sys_Error( const char *error_string );

extern void SetupVisibility( edict_t *pViewEntity, edict_t *pClient, unsigned char **pvs, unsigned char **pas );
extern void	UpdateClientData ( const struct edict_s *ent, int sendweapons, struct clientdata_s *cd );
extern int AddToFullPack( struct entity_state_s *state, int e, edict_t *ent, edict_t *host, int hostflags, int player, unsigned char *pSet );
extern void CreateBaseline( int player, int eindex, struct entity_state_s *baseline, struct edict_s *entity, int playermodelindex, vec3_t player_mins, vec3_t player_maxs );
extern void RegisterEncoders( void );

extern int GetWeaponData( struct edict_s *player, struct weapon_data_s *info );

extern void	CmdStart( const edict_t *player, const struct usercmd_s *cmd, unsigned int random_seed );
extern void	CmdEnd ( const edict_t *player );

extern int	ConnectionlessPacket( const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size );

extern int GetHullBounds( int hullnumber, float *mins, float *maxs );

extern void	CreateInstancedBaselines ( void );

extern int	InconsistentFile( const edict_t *player, const char *filename, char *disconnect_message );

extern int AllowLagCompensation( void );

extern int ShouldCollide( edict_t *pentTouched, edict_t *pentOther );

#endif		// CLIENT_H
