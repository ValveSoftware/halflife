// hud_servers.cpp
#include "hud.h"
#include "cl_util.h"
#include "hud_servers_priv.h"
#include "hud_servers.h"
#include "net_api.h"
#include <string.h>
#ifdef _WIN32
#include "winsani_in.h"
#include <winsock.h>
#include "winsani_out.h"
#else
#include <arpa/inet.h>
#endif
static int	context_id;

// Default master server address in case we can't read any from valvecomm.lst file
#define VALVE_MASTER_ADDRESS "half-life.east.won.net"
#define PORT_MASTER	 27010
#define PORT_SERVER  27015

// File where we really should look for master servers
#define MASTER_PARSE_FILE "valvecomm.lst"

#define MAX_QUERIES 20

#define NET_API gEngfuncs.pNetAPI

static CHudServers *g_pServers = NULL;

/*
===================
ListResponse

Callback from engine
===================
*/
void NET_CALLBACK ListResponse( struct net_response_s *response )
{
	if ( g_pServers )
	{
		g_pServers->ListResponse( response );
	}
}

/*
===================
ServerResponse

Callback from engine
===================
*/
void NET_CALLBACK ServerResponse( struct net_response_s *response )
{
	if ( g_pServers )
	{
		g_pServers->ServerResponse( response );
	}
}

/*
===================
PingResponse

Callback from engine
===================
*/
void NET_CALLBACK PingResponse( struct net_response_s *response )
{
	if ( g_pServers )
	{
		g_pServers->PingResponse( response );
	}
}

/*
===================
RulesResponse

Callback from engine
===================
*/
void NET_CALLBACK RulesResponse( struct net_response_s *response )
{
	if ( g_pServers )
	{
		g_pServers->RulesResponse( response );
	}
}
/*
===================
PlayersResponse

Callback from engine
===================
*/
void NET_CALLBACK PlayersResponse( struct net_response_s *response )
{
	if ( g_pServers )
	{
		g_pServers->PlayersResponse( response );
	}
}
/*
===================
ListResponse

===================
*/
void CHudServers::ListResponse( struct net_response_s *response )
{
	request_t *list;
	request_t *p;
	int c = 0;

	if ( !( response->error == NET_SUCCESS ) )
		return;

	if ( response->type != NETAPI_REQUEST_SERVERLIST )
		return;

	if ( response->response )
	{
		list = ( request_t * ) response->response;
		while ( list )
		{
			c++;

			//if ( c < 40 )
			{
				// Copy from parsed stuff
				p = new request_t;
				p->context = -1;
				p->remote_address = list->remote_address;
				p->next = m_pServerList;
				m_pServerList = p;
			}

			// Move on
			list = list->next;
		}
	}

	gEngfuncs.Con_Printf( "got list\n" );

	m_nQuerying = 1;
	m_nActiveQueries = 0;
}

/*
===================
ServerResponse

===================
*/
void CHudServers::ServerResponse( struct net_response_s *response )
{
	char *szresponse;
	request_t *p;
	server_t *browser;
	int	len;
	char sz[ 32 ];

	// Remove from active list
	p = FindRequest( response->context, m_pActiveList );
	if ( p )
	{
		static int first = 0;

		RemoveServerFromList( &m_pActiveList, p );
		m_nActiveQueries--;

		if ( !first )
		{
			gEngfuncs.Con_Printf( "recv first %f\n", gEngfuncs.GetClientTime() );
			first = 1;
		}
	}

	
	if ( response->error != NET_SUCCESS )
		return;
	
	switch ( response->type )
	{
	case NETAPI_REQUEST_DETAILS:
		if ( response->response )
		{
			szresponse = (char *)response->response;
			len = strlen( szresponse ) + 100 + 1;
			sprintf( sz, "%i", (int)( 1000.0 * response->ping ) );

			browser = new server_t;
			browser->remote_address = response->remote_address;
			browser->info = new char[ len ];
			browser->ping = (int)( 1000.0 * response->ping );
			strcpy( browser->info, szresponse );

			NET_API->SetValueForKey( browser->info, "address", gEngfuncs.pNetAPI->AdrToString( &response->remote_address ), len );
			NET_API->SetValueForKey( browser->info, "ping", sz, len );
			
			AddServer( &m_pServers, browser );
		}
		break;
	default:
		break;
	}
}

/*
===================
PingResponse

===================
*/
void CHudServers::PingResponse( struct net_response_s *response )
{
	char sz[ 32 ];

	if ( response->error != NET_SUCCESS )
		return;

	switch ( response->type )
	{
	case NETAPI_REQUEST_PING:
		sprintf( sz, "%.2f", 1000.0 * response->ping );

		gEngfuncs.Con_Printf( "ping == %s\n", sz );
		break;
	default:
		break;
	}
}

/*
===================
RulesResponse

===================
*/
void CHudServers::RulesResponse( struct net_response_s *response )
{
	char *szresponse;
	
	if ( response->error != NET_SUCCESS )
		return;
	
	switch ( response->type )
	{
	case NETAPI_REQUEST_RULES:
		if ( response->response )
		{
			szresponse = (char *)response->response;

			gEngfuncs.Con_Printf( "rules %s\n", szresponse );
		}
		break;
	default:
		break;
	}
}

/*
===================
PlayersResponse

===================
*/
void CHudServers::PlayersResponse( struct net_response_s *response )
{
	char *szresponse;

	if ( response->error != NET_SUCCESS )
		return;
	
	switch ( response->type )
	{
	case NETAPI_REQUEST_PLAYERS:
		if ( response->response )
		{
			szresponse = (char *)response->response;

			gEngfuncs.Con_Printf( "players %s\n", szresponse );
		}
		break;
	default:
		break;
	}
}

/*
===================
CompareServers

Return 1 if p1 is "less than" p2, 0 otherwise
===================
*/
int	CHudServers::CompareServers( server_t *p1, server_t *p2 )
{
	const char *n1, *n2;

	if ( p1->ping < p2->ping )
		return 1;

	if ( p1->ping == p2->ping )
	{
		// Pings equal, sort by second key:  hostname
		if ( p1->info && p2->info )
		{
			n1 = NET_API->ValueForKey( p1->info, "hostname" );
			n2 = NET_API->ValueForKey( p2->info, "hostname" );

			if ( n1 && n2 )
			{
				if ( stricmp( n1, n2 ) < 0 )
					return 1;
			}
		}
	}

	return 0;
}

/*
===================
AddServer

===================
*/
void CHudServers::AddServer( server_t **ppList, server_t *p )
{
server_t *list;

	if ( !ppList || ! p )
		return;

	m_nServerCount++;

	// What sort key?  Ping?
	list = *ppList;

	// Head of list?
	if ( !list )
	{
		p->next = NULL;
		*ppList = p;
		return;
	}

	// Put on head of list
	if ( CompareServers( p, list ) )
	{
		p->next = *ppList;
		*ppList = p;
	}
	else
	{
		while ( list->next )
		{
			// Insert before list next
			if ( CompareServers( p, list->next ) )
			{
				p->next = list->next->next;
				list->next = p;
				return;
			}

			list = list->next;
		}

		// Just add at end
		p->next = NULL;
		list->next = p;
	}
}

/*
===================
Think

===================
*/
void CHudServers::Think( double time )
{
	m_fElapsed += time;

	if ( !m_nRequesting )
		return;

	if ( !m_nQuerying )
		return;

	QueryThink();

	if ( ServerListSize() > 0 )
		return;

	m_dStarted = 0.0;
	m_nRequesting = 0;
	m_nDone = 0;
	m_nQuerying = 0;
	m_nActiveQueries = 0;
}

/*
===================
QueryThink

===================
*/
void CHudServers::QueryThink( void )
{
	request_t *p;

	if ( !m_nRequesting || m_nDone )
		return;

	if ( !m_nQuerying )
		return;

	if ( m_nActiveQueries > MAX_QUERIES )
		return;

	// Nothing left
	if ( !m_pServerList )
		return;

	while ( 1 )
	{
		static int first = 0;
		p = m_pServerList;

		// No more in list?
		if ( !p )
			break;

		// Move to next
		m_pServerList = m_pServerList->next;

		// Setup context_id
		p->context = context_id;

		// Make sure networking system has started.
		// NET_API->InitNetworking();

		if ( !first )
		{
			gEngfuncs.Con_Printf( "send first %f\n", gEngfuncs.GetClientTime() );
			first = 1;
		}

		// Start up query on this one
		NET_API->SendRequest( context_id++, NETAPI_REQUEST_DETAILS, 0, 2.0, &p->remote_address, ::ServerResponse );

		// Increment active list
		m_nActiveQueries++;

		// Add to active list
		p->next = m_pActiveList;
		m_pActiveList = p;

		// Too many active?
		if ( m_nActiveQueries > MAX_QUERIES )
			break;
	}
}

/*
==================
ServerListSize

# of servers in active query and in pending to be queried lists
==================
*/
int	CHudServers::ServerListSize( void )
{
	int c = 0;
	request_t *p;

	p = m_pServerList;
	while ( p )
	{
		c++;
		p = p->next;
	}

	p = m_pActiveList;
	while ( p )
	{
		c++;
		p = p->next;
	}

	return c;
}

/*
===================
FindRequest

Look up a request by context id
===================
*/
CHudServers::request_t *CHudServers::FindRequest( int context, request_t *pList )
{
	request_t *p;
	p = pList;
	while ( p )
	{
		if ( context == p->context )
			return p;

		p = p->next;
	}
	return NULL;
}

/*
===================
RemoveServerFromList

Remote, but don't delete, item from *ppList
===================
*/
void CHudServers::RemoveServerFromList( request_t **ppList, request_t *item )
{
	request_t *p, *n;
	request_t *newlist = NULL;

	if ( !ppList )
		return;

	p = *ppList;
	while ( p )
	{
		n = p->next;
		if ( p != item )
		{
			p->next = newlist;
			newlist = p;
		}
		p = n;
	}
	*ppList = newlist;
}

/*
===================
ClearRequestList

===================
*/
void CHudServers::ClearRequestList( request_t **ppList )
{
	request_t *p, *n;

	if ( !ppList )
		return;

	p = *ppList;
	while ( p )
	{
		n = p->next;
		delete p;
		p = n;
	}
	*ppList = NULL;
}

/*
===================
ClearServerList

===================
*/
void CHudServers::ClearServerList( server_t **ppList )
{
	server_t *p, *n;

	if ( !ppList )
		return;

	p = *ppList;
	while ( p )
	{
		n = p->next;
		delete[] p->info;
		delete p;
		p = n;
	}
	*ppList = NULL;
}

int CompareField( CHudServers::server_t *p1, CHudServers::server_t *p2, const char *fieldname, int iSortOrder )
{
	const char *sz1, *sz2;
	float fv1, fv2;

	sz1 = NET_API->ValueForKey( p1->info, fieldname );
	sz2 = NET_API->ValueForKey( p2->info, fieldname );

	fv1 = atof( sz1 );
	fv2 = atof( sz2 );

	if ( fv1 && fv2 )
	{
		if ( fv1 > fv2 )
			return iSortOrder;
		else if ( fv1 < fv2 )
			return -iSortOrder;
		else
			return 0;
	}

	// String compare
	return stricmp( sz1, sz2 );
}

int ServerListCompareFunc( CHudServers::server_t *p1, CHudServers::server_t *p2, const char *fieldname )
{
	if (!p1 || !p2)  // No meaningful comparison
		return 0;  

	int iSortOrder = 1;

	int retval = 0;

	retval = CompareField( p1, p2, fieldname, iSortOrder );

	return retval;
}
#ifndef _WIN32
#define __cdecl
#endif
static char g_fieldname[ 256 ];
int __cdecl FnServerCompare(const void *elem1, const void *elem2 )
{
	CHudServers::server_t *list1, *list2;

	list1 = *(CHudServers::server_t **)elem1;
	list2 = *(CHudServers::server_t **)elem2;

	return ServerListCompareFunc( list1, list2, g_fieldname );
}

void CHudServers::SortServers( const char *fieldname )
{
	server_t *p;
	// Create a list
	if ( !m_pServers )
		return;

	strcpy( g_fieldname, fieldname );

	int i;
	int c = 0;

	p = m_pServers;
	while ( p )
	{
		c++;
		p = p->next;
	}

	server_t **pSortArray;
	
	pSortArray = new server_t *[ c ];
	memset( pSortArray, 0, c  * sizeof( server_t * ) );

	// Now copy the list into the pSortArray:
	p = m_pServers;
	i = 0;
	while ( p )
	{
		pSortArray[ i++ ] = p;
		p = p->next;
	}

	// Now do that actual sorting.
	size_t nCount = c;
	size_t nSize  = sizeof( server_t * );

	qsort(
		pSortArray,
		(size_t)nCount,
		(size_t)nSize,
		FnServerCompare
	);

	// Now rebuild the list.
	m_pServers = pSortArray[0];
	for ( i = 0; i < c - 1; i++ )
	{
		pSortArray[ i ]->next = pSortArray[ i + 1 ];
	}
	pSortArray[ c - 1 ]->next = NULL;

	// Clean Up.
	delete[] pSortArray;
}

/*
===================
GetServer

Return particular server
===================
*/
CHudServers::server_t *CHudServers::GetServer( int server )
{
	int c = 0;
	server_t *p;

	p = m_pServers;
	while ( p )
	{
		if ( c == server )
			return p;

		c++;
		p = p->next;
	}
	return NULL;
}

/*
===================
GetServerInfo

Return info ( key/value ) string for particular server
===================
*/
char *CHudServers::GetServerInfo( int server )
{
	server_t *p = GetServer( server );
	if ( p )
	{
		return p->info;
	}
	return NULL;
}

/*
===================
CancelRequest

Kill all pending requests in engine
===================
*/
void CHudServers::CancelRequest( void )
{
	m_nRequesting = 0;
	m_nQuerying   = 0;
	m_nDone		  = 1;

	NET_API->CancelAllRequests();
}

/*
==================
LoadMasterAddresses

Loads the master server addresses from file and into the passed in array
==================
*/
int CHudServers::LoadMasterAddresses( int maxservers, int *count, netadr_t *padr )
{
	int			i;
	char		szMaster[ 256 ];
	char		szMasterFile[256];
	char		*pbuffer = NULL;
	char		*pstart = NULL ;
	netadr_t	adr;
	char		szAdr[64];
	int			nPort;
	int			nCount = 0;
	bool		bIgnore;
	int			nDefaultPort;

	// Assume default master and master file
	strcpy( szMaster, VALVE_MASTER_ADDRESS );    // IP:PORT string
	strcpy( szMasterFile, MASTER_PARSE_FILE );

	// See if there is a command line override
	i = gEngfuncs.CheckParm( "-comm", &pstart );
	if ( i && pstart )
	{
		strcpy (szMasterFile, pstart );
	}

	// Read them in from proper file
	pbuffer = (char *)gEngfuncs.COM_LoadFile( szMasterFile, 5, NULL ); // Use malloc
	if ( !pbuffer )
	{
		goto finish_master;
	}

	pstart = pbuffer;

	while ( nCount < maxservers )
	{
		pstart = gEngfuncs.COM_ParseFile( pstart, m_szToken );

		if ( strlen(m_szToken) <= 0)
			break;

		bIgnore = true;

		if ( !stricmp( m_szToken, "Master" ) )
		{
			nDefaultPort = PORT_MASTER;
			bIgnore = FALSE;
		}

		// Now parse all addresses between { }
		pstart = gEngfuncs.COM_ParseFile( pstart, m_szToken );
		if ( strlen(m_szToken) <= 0 )
			break;

		if ( stricmp ( m_szToken, "{" ) )
			break;

		// Parse addresses until we get to "}"
		while ( nCount < maxservers )
		{
			char base[256];

			// Now parse all addresses between { }
			pstart = gEngfuncs.COM_ParseFile( pstart, m_szToken );
			
			if (strlen(m_szToken) <= 0)
				break;

			if ( !stricmp ( m_szToken, "}" ) )
				break;
			
			sprintf( base, "%s", m_szToken );
				
			pstart = gEngfuncs.COM_ParseFile( pstart, m_szToken );
			
			if (strlen(m_szToken) <= 0)
				break;

			if ( stricmp( m_szToken, ":" ) )
				break;

			pstart = gEngfuncs.COM_ParseFile( pstart, m_szToken );
			
			if (strlen(m_szToken) <= 0)
				break;

			nPort = atoi ( m_szToken );
			if ( !nPort )
				nPort = nDefaultPort;

			sprintf( szAdr, "%s:%i", base, nPort );

			// Can we resolve it any better
			if ( !NET_API->StringToAdr( szAdr, &adr ) )
				bIgnore = true;

			if ( !bIgnore )
			{
				padr[ nCount++ ] = adr;
			}
		}
	}

finish_master:
	if ( !nCount )
	{
		sprintf( szMaster, VALVE_MASTER_ADDRESS );    // IP:PORT string

		// Convert to netadr_t
		if ( NET_API->StringToAdr ( szMaster, &adr ) )
		{
		
			padr[ nCount++ ] = adr;
		}
	}

	*count = nCount;

	if ( pbuffer )
	{
		gEngfuncs.COM_FreeFile( pbuffer );
	}

	return ( nCount > 0 ) ? 1 : 0;
}

/*
===================
RequestList

Request list of game servers from master
===================
*/
void CHudServers::RequestList( void )
{
	m_nRequesting	= 1;
	m_nDone			= 0;
	m_dStarted		= m_fElapsed;

	int	count = 0;
	netadr_t adr;

	if ( !LoadMasterAddresses( 1, &count, &adr ) )
	{
		gEngfuncs.Con_DPrintf( "SendRequest:  Unable to read master server addresses\n" );
		return;
	}

	ClearRequestList( &m_pActiveList );
	ClearRequestList( &m_pServerList );
	ClearServerList( &m_pServers );

	m_nServerCount = 0;

	// Make sure networking system has started.
	NET_API->InitNetworking();

	// Kill off left overs if any
	NET_API->CancelAllRequests();

	// Request Server List from master
	NET_API->SendRequest( context_id++, NETAPI_REQUEST_SERVERLIST, 0, 5.0, &adr, ::ListResponse );
}

void CHudServers::RequestBroadcastList( int clearpending )
{
	m_nRequesting	= 1;
	m_nDone			= 0;
	m_dStarted		= m_fElapsed;

	netadr_t		adr;
	memset( &adr, 0, sizeof( adr ) );

	if ( clearpending )
	{
		ClearRequestList( &m_pActiveList );
		ClearRequestList( &m_pServerList );
		ClearServerList( &m_pServers );

		m_nServerCount = 0;
	}

	// Make sure to byte swap server if necessary ( using "host" to "net" conversion
	adr.port = htons( PORT_SERVER );

	// Make sure networking system has started.
	NET_API->InitNetworking();

	if ( clearpending )
	{
		// Kill off left overs if any
		NET_API->CancelAllRequests();
	}

	adr.type = NA_BROADCAST;

	// Request Servers from LAN via IP
	NET_API->SendRequest( context_id++, NETAPI_REQUEST_DETAILS, FNETAPI_MULTIPLE_RESPONSE, 5.0, &adr, ::ServerResponse );

	adr.type = NA_BROADCAST_IPX;

	// Request Servers from LAN via IPX ( if supported )
	NET_API->SendRequest( context_id++, NETAPI_REQUEST_DETAILS, FNETAPI_MULTIPLE_RESPONSE, 5.0, &adr, ::ServerResponse );
}

void CHudServers::ServerPing( int server )
{
	server_t *p;
	
	p = GetServer( server );
	if ( !p )
		return;

	// Make sure networking system has started.
	NET_API->InitNetworking();

	// Request Server List from master
	NET_API->SendRequest( context_id++, NETAPI_REQUEST_PING, 0, 5.0, &p->remote_address, ::PingResponse );
}

void CHudServers::ServerRules( int server )
{
	server_t *p;
	
	p = GetServer( server );
	if ( !p )
		return;

	// Make sure networking system has started.
	NET_API->InitNetworking();

	// Request Server List from master
	NET_API->SendRequest( context_id++, NETAPI_REQUEST_RULES, 0, 5.0, &p->remote_address, ::RulesResponse );
}

void CHudServers::ServerPlayers( int server )
{
	server_t *p;
	
	p = GetServer( server );
	if ( !p )
		return;

	// Make sure networking system has started.
	NET_API->InitNetworking();

	// Request Server List from master
	NET_API->SendRequest( context_id++, NETAPI_REQUEST_PLAYERS, 0, 5.0, &p->remote_address, ::PlayersResponse );
}

int CHudServers::isQuerying()
{
	return m_nRequesting ? 1 : 0;
}


/*
===================
GetServerCount

Return number of servers in browser list
===================
*/
int CHudServers::GetServerCount( void )
{
	return m_nServerCount;
}

/*
===================
CHudServers

===================
*/
CHudServers::CHudServers( void )
{
	m_nRequesting		= 0;
	m_dStarted			= 0.0;
	m_nDone				= 0;
	m_pServerList		= NULL;
	m_pServers			= NULL;
	m_pActiveList		= NULL;
	m_nQuerying			= 0;
	m_nActiveQueries	= 0;
	
	m_fElapsed			= 0.0;


	m_pPingRequest		= NULL;
	m_pRulesRequest		= NULL;
	m_pPlayersRequest	= NULL;
}

/*
===================
~CHudServers

===================
*/
CHudServers::~CHudServers( void )
{
	ClearRequestList( &m_pActiveList );
	ClearRequestList( &m_pServerList );
	ClearServerList( &m_pServers );

	m_nServerCount = 0;

	if ( m_pPingRequest )
	{
		delete m_pPingRequest;
		m_pPingRequest = NULL;

	}
	
	if ( m_pRulesRequest )
	{
		delete m_pRulesRequest;
		m_pRulesRequest = NULL;
	}

	if ( m_pPlayersRequest )
	{
		delete m_pPlayersRequest;
		m_pPlayersRequest = NULL;
	}
}

///////////////////////////////
//
// PUBLIC APIs
//
///////////////////////////////

/*
===================
ServersGetCount

===================
*/
int	ServersGetCount( void )
{
	if ( g_pServers )
	{
		return g_pServers->GetServerCount();
	}
	return 0;
}

int	ServersIsQuerying( void )
{
	if ( g_pServers )
	{
		return g_pServers->isQuerying();
	}
	return 0;
}

/*
===================
ServersGetInfo

===================
*/
const char *ServersGetInfo( int server )
{
	if ( g_pServers )
	{
		return g_pServers->GetServerInfo( server );
	}
	
	return NULL;
}

void SortServers( const char *fieldname )
{
	if ( g_pServers )
	{
		g_pServers->SortServers( fieldname );
	}
}

/*
===================
ServersShutdown

===================
*/
void ServersShutdown( void )
{
	if ( g_pServers )
	{
		delete g_pServers;
		g_pServers = NULL;
	}
}

/*
===================
ServersInit

===================
*/
void ServersInit( void )
{
	// Kill any previous instance
	ServersShutdown();
	
	g_pServers = new CHudServers();
}

/*
===================
ServersThink

===================
*/
void ServersThink( double time )
{
	if ( g_pServers )
	{
		g_pServers->Think( time );
	}
}

/*
===================
ServersCancel

===================
*/
void ServersCancel( void )
{
	if ( g_pServers )
	{
		g_pServers->CancelRequest();
	}
}

// Requests
/*
===================
ServersList

===================
*/
void ServersList( void )
{
	if ( g_pServers )
	{
		g_pServers->RequestList();
	}
}

void BroadcastServersList( int clearpending )
{
	if ( g_pServers )
	{
		g_pServers->RequestBroadcastList( clearpending );
	}
}

void ServerPing( int server )
{
	if ( g_pServers )
	{
		g_pServers->ServerPing( server );
	}
}

void ServerRules( int server )
{
	if ( g_pServers )
	{
		g_pServers->ServerRules( server );
	}
}

void ServerPlayers( int server )
{
	if ( g_pServers )
	{
		g_pServers->ServerPlayers( server );
	}
}
