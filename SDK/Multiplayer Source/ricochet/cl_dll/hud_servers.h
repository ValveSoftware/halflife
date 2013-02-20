#if !defined( HUD_SERVERSH )
#define HUD_SERVERSH
#pragma once

#define NET_CALLBACK /* */

// Dispatchers
void NET_CALLBACK ListResponse( struct net_response_s *response );
void NET_CALLBACK ServerResponse( struct net_response_s *response );
void NET_CALLBACK PingResponse( struct net_response_s *response );
void NET_CALLBACK RulesResponse( struct net_response_s *response );
void NET_CALLBACK PlayersResponse( struct net_response_s *response );

void ServersInit( void );
void ServersShutdown( void );
void ServersThink( double time );
void ServersCancel( void );

// Get list and get server info from each
void ServersList( void );

// Query for IP / IPX LAN servers
void BroadcastServersList( int clearpending );

void ServerPing( int server );
void ServerRules( int server );
void ServerPlayers( int server );

int	ServersGetCount( void );
const char *ServersGetInfo( int server );
int	ServersIsQuerying( void );
void SortServers( const char *fieldname );

#endif // HUD_SERVERSH