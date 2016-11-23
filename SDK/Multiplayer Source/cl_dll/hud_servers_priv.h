//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#if !defined( HUD_SERVERS_PRIVH )
#define HUD_SERVERS_PRIVH
#pragma once

#include "netadr.h"

class CHudServers
{
public:
	typedef struct request_s
	{
		struct request_s	*next;
		netadr_t					remote_address;
		int							context;
	} request_t;

	typedef struct server_s
	{
		struct server_s	*next;
		netadr_t				remote_address;
		char					*info;
		int						ping;
	} server_t;

	CHudServers();
	~CHudServers();

	void	Think( double time );
	void	QueryThink( void );
	int		isQuerying( void );

	int		LoadMasterAddresses( int maxservers, int *count, netadr_t *padr );

	void	RequestList( void );
	void	RequestBroadcastList( int clearpending );

	void	ServerPing( int server );
	void	ServerRules( int server );
	void	ServerPlayers( int server );

	void	CancelRequest( void );

	int		CompareServers( server_t *p1, server_t *p2 );

	void	ClearServerList( server_t **ppList );
	void	ClearRequestList( request_t **ppList );

	void	AddServer( server_t **ppList, server_t *p );

	void	RemoveServerFromList( request_t **ppList, request_t *item );

	request_t *FindRequest( int context, request_t *pList );

	int		ServerListSize( void );
	char	*GetServerInfo( int server );
	int		GetServerCount( void );
	void	SortServers( const char *fieldname );

	void	ListResponse( struct net_response_s *response );
	void	ServerResponse( struct net_response_s *response );
	void	PingResponse( struct net_response_s *response );
	void	RulesResponse( struct net_response_s *response );
	void	PlayersResponse( struct net_response_s *response );
private:
	
	server_t *GetServer( int server );

	//
	char				m_szToken[ 1024 ];
	int					m_nRequesting;
	int					m_nDone;

	double				m_dStarted;

	request_t	*m_pServerList;
	request_t	*m_pActiveList;
	
	server_t		*m_pServers;

	int					m_nServerCount;

	int					m_nActiveQueries;
	int					m_nQuerying;
	double				m_fElapsed;

	request_t			*m_pPingRequest;
	request_t			*m_pRulesRequest;
	request_t			*m_pPlayersRequest;
};

#endif // HUD_SERVERS_PRIVH