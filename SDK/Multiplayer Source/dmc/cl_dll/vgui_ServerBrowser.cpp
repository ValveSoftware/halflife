//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include<VGUI_HeaderPanel.h>
#include<VGUI_TablePanel.h>
#include<VGUI_LineBorder.h>
#include<VGUI_Label.h>
#include<VGUI_Button.h>
#include<VGUI_ActionSignal.h>

#include "hud.h"
#include "cl_util.h"
#include "hud_servers.h"
#include "net_api.h"

#include "vgui_viewport.h"
#include "vgui_ServerBrowser.h"

using namespace vgui;

namespace
{

#define MAX_SB_ROWS 24

#define NUM_COLUMNS 5

#define HEADER_SIZE_Y			YRES(18)

// Column sizes
#define CSIZE_ADDRESS			XRES(200)
#define CSIZE_SERVER			XRES(400) 
#define CSIZE_MAP				XRES(500)
#define CSIZE_CURRENT			XRES(570)
#define CSIZE_PING				XRES(640)

#define CELL_HEIGHT				YRES(15)

class ServerBrowserTablePanel;

class CBrowser_InputSignal : public InputSignal
{
private:
	ServerBrowserTablePanel *m_pBrowser;
public:
	CBrowser_InputSignal( ServerBrowserTablePanel *pBrowser )
	{
		m_pBrowser = pBrowser;
	}

	virtual void cursorMoved(int x,int y,Panel* panel) {};
	virtual void cursorEntered(Panel* panel){};
	virtual void cursorExited(Panel* Panel) {};

	virtual void mousePressed(MouseCode code,Panel* panel);

	virtual void mouseDoublePressed(MouseCode code,Panel* panel);
	virtual void mouseReleased(MouseCode code,Panel* panel) {};
	virtual void mouseWheeled(int delta,Panel* panel) {};
	virtual void keyPressed(KeyCode code,Panel* panel) {};
	virtual void keyTyped(KeyCode code,Panel* panel) {};
	virtual void keyReleased(KeyCode code,Panel* panel) {};
	virtual void keyFocusTicked(Panel* panel) {};
};

class ServerBrowserTablePanel : public TablePanel
{
private:
	Label				*m_pLabel;
	int					m_nMouseOverRow;

public:
	
	ServerBrowserTablePanel( int x,int y,int wide,int tall,int columnCount) : TablePanel( x,y,wide,tall,columnCount)
	{
		m_pLabel = new Label( "", 0, 0 /*,wide, tall*/ );
		
		m_nMouseOverRow = 0;
	}

public:
	void setMouseOverRow( int row )
	{
		m_nMouseOverRow	= row;
	}

	void DoSort( char *sortkey )
	{
		// Request server list and refresh servers...
		SortServers( sortkey );
	}

	void DoRefresh( void )
	{
		// Request server list and refresh servers...
		ServersList();
		BroadcastServersList( 0 );
	}
	
	void DoBroadcastRefresh( void )
	{
		// Request server list and refresh servers...
		BroadcastServersList( 1 );
	}

	void DoStop( void )
	{
		// Stop requesting
		ServersCancel();
	}

	void DoCancel( void )
	{
		ClientCmd( "togglebrowser\n" );
	}

	void DoConnect( void )
	{
		const char *info;
		const char *address;
		char sz[ 256 ];

		info = ServersGetInfo( m_nMouseOverRow );
		if ( !info )
			return;

		address = gEngfuncs.pNetAPI->ValueForKey( info, "address" );
		//gEngfuncs.Con_Printf( "Connecting to %s\n", address );

		sprintf( sz, "connect %s\n", address );

		ClientCmd( sz );

		DoCancel();
	}

	void DoPing( void )
	{
		ServerPing( 0 );
		ServerRules( 0 );
		ServerPlayers( 0 );
	}

	virtual int getRowCount()
	{
		int rowcount;
		int height, width;

		getSize( width, height );

		// Space for buttons
		height -= YRES(20);
		height = max( 0, height );

		rowcount = height / CELL_HEIGHT;

		return rowcount;
	}

	virtual int getCellTall(int row)
	{
		return CELL_HEIGHT - 2;
	}
	
	virtual Panel* getCellRenderer(int column,int row,bool columnSelected,bool rowSelected,bool cellSelected)
	{
		const char *info;
		const char *val, *val2;
		char sz[ 32 ];

		info = ServersGetInfo( row );

		if ( row == m_nMouseOverRow )
		{
			m_pLabel->setFgColor( 200, 240, 63, 100 );
		}
		else
		{
			m_pLabel->setFgColor( 255, 255, 255, 0 );
		}
		m_pLabel->setBgColor( 0, 0, 0, 200 );
		m_pLabel->setContentAlignment( vgui::Label::a_west );
		m_pLabel->setFont( Scheme::sf_primary2 );

		if ( info )
		{
			// Fill out with the correct data
			switch ( column )
			{
			case 0:
				val = gEngfuncs.pNetAPI->ValueForKey( info, "address" );
				if ( val )
				{
					strncpy( sz, val, 31 );
					sz[ 31 ] = '\0';
					// Server Name;
					m_pLabel->setText( sz );
				}
				break;
			case 1:
				val = gEngfuncs.pNetAPI->ValueForKey( info, "hostname" );
				if ( val )
				{
					strncpy( sz, val, 31 );
					sz[ 31 ] = '\0';
					// Server Map;
					m_pLabel->setText( sz );
				}
				break;
			case 2:
				val = gEngfuncs.pNetAPI->ValueForKey( info, "map" );
				if ( val )
				{
					strncpy( sz, val, 31 );
					sz[ 31 ] = '\0';
					// Server Name;
					m_pLabel->setText( sz );
				}
				break;
			case 3:
				val = gEngfuncs.pNetAPI->ValueForKey( info, "current" );
				val2 = gEngfuncs.pNetAPI->ValueForKey( info, "max" );
				if ( val && val2 )
				{
					sprintf( sz, "%s/%s", val, val2 );
					sz[ 31 ] = '\0';
					// Server Map;
					m_pLabel->setText( sz );
				}
				break;
			case 4:
				val = gEngfuncs.pNetAPI->ValueForKey( info, "ping" );
				if ( val )
				{
					strncpy( sz, val, 31 );
					sz[ 31 ] = '\0';
					// Server Name;
					m_pLabel->setText( sz );
				}
				break;
			default:
				break;
			}
		}
		else
		{
			if ( !row && !column )
			{
				if ( ServersIsQuerying() )
				{
					m_pLabel->setText( "Waiting for servers to respond..." );
				}
				else
				{
					m_pLabel->setText( "Press 'Refresh' to search for servers..." );
				}
			}
			else
			{
				m_pLabel->setText( "" );
			}
		}
		
		return m_pLabel;
	}

	virtual Panel* startCellEditing(int column,int row)
	{
		return null;
	}

};

class ConnectHandler : public ActionSignal
{
private:
	ServerBrowserTablePanel *m_pBrowser;

public:
	ConnectHandler( ServerBrowserTablePanel *browser )
	{
		m_pBrowser = browser;	
	}

	virtual void actionPerformed( Panel *panel )
	{
		m_pBrowser->DoConnect();
	}
};

class RefreshHandler : public ActionSignal
{
private:
	ServerBrowserTablePanel *m_pBrowser;

public:
	RefreshHandler( ServerBrowserTablePanel *browser )
	{
		m_pBrowser = browser;	
	}

	virtual void actionPerformed( Panel *panel )
	{
		m_pBrowser->DoRefresh();
	}
};

class BroadcastRefreshHandler : public ActionSignal
{
private:
	ServerBrowserTablePanel *m_pBrowser;

public:
	BroadcastRefreshHandler( ServerBrowserTablePanel *browser )
	{
		m_pBrowser = browser;	
	}

	virtual void actionPerformed( Panel *panel )
	{
		m_pBrowser->DoBroadcastRefresh();
	}
};

class StopHandler : public ActionSignal
{
private:
	ServerBrowserTablePanel *m_pBrowser;

public:
	StopHandler( ServerBrowserTablePanel *browser )
	{
		m_pBrowser = browser;	
	}

	virtual void actionPerformed( Panel *panel )
	{
		m_pBrowser->DoStop();
	}
};

class CancelHandler : public ActionSignal
{
private:
	ServerBrowserTablePanel *m_pBrowser;

public:
	CancelHandler( ServerBrowserTablePanel *browser )
	{
		m_pBrowser = browser;	
	}

	virtual void actionPerformed( Panel *panel )
	{
		m_pBrowser->DoCancel();
	}
};

class PingHandler : public ActionSignal
{
private:
	ServerBrowserTablePanel *m_pBrowser;

public:
	PingHandler( ServerBrowserTablePanel *browser )
	{
		m_pBrowser = browser;	
	}

	virtual void actionPerformed( Panel *panel )
	{
		m_pBrowser->DoPing();
	}
};

class SortHandler : public ActionSignal
{
private:
	ServerBrowserTablePanel *m_pBrowser;

public:
	SortHandler( ServerBrowserTablePanel *browser )
	{
		m_pBrowser = browser;	
	}

	virtual void actionPerformed( Panel *panel )
	{
		m_pBrowser->DoSort( "map" );
	}
};

}

class LabelSortInputHandler : public InputSignal
{
private:
	ServerBrowserTablePanel *m_pBrowser;
	char m_szSortKey[ 64 ];

public:
	LabelSortInputHandler( ServerBrowserTablePanel *pBrowser, char *name )
	{
		m_pBrowser = pBrowser;
		strcpy( m_szSortKey, name );
	}

	virtual void cursorMoved(int x,int y,Panel* panel) {};
	virtual void cursorEntered(Panel* panel){};
	virtual void cursorExited(Panel* Panel) {};

	virtual void mousePressed(MouseCode code,Panel* panel)
	{
		m_pBrowser->DoSort( m_szSortKey );
	}

	virtual void mouseDoublePressed(MouseCode code,Panel* panel)
	{
		m_pBrowser->DoSort( m_szSortKey );
	}

	virtual void mouseReleased(MouseCode code,Panel* panel) {};
	virtual void mouseWheeled(int delta,Panel* panel) {};
	virtual void keyPressed(KeyCode code,Panel* panel) {};
	virtual void keyTyped(KeyCode code,Panel* panel) {};
	virtual void keyReleased(KeyCode code,Panel* panel) {};
	virtual void keyFocusTicked(Panel* panel) {};
};

class CSBLabel : public Label
{

private:
	char m_szSortKey[ 64 ];
	ServerBrowserTablePanel *m_pBrowser;

public:
	CSBLabel( char *name, char *sortkey ) : Label( name )
	{
		m_pBrowser = NULL;

		strcpy( m_szSortKey, sortkey );

		int label_bg_r = 120,
			label_bg_g = 75,
			label_bg_b = 32,
			label_bg_a = 200;

		int label_fg_r = 255,
			label_fg_g = 0,
			label_fg_b = 0,
			label_fg_a = 0;

		setContentAlignment( vgui::Label::a_west );
		setFgColor( label_fg_r, label_fg_g, label_fg_b, label_fg_a );
		setBgColor( label_bg_r, label_bg_g, label_bg_b, label_bg_a );
		setFont( Scheme::sf_primary2 );

	}

	void setTable( ServerBrowserTablePanel *browser )
	{
		m_pBrowser = browser;

		addInputSignal( new LabelSortInputHandler( (ServerBrowserTablePanel * )m_pBrowser, m_szSortKey ) );
	}
};

ServerBrowser::ServerBrowser(int x,int y,int wide,int tall) : CTransparentPanel( 100, x,y,wide,tall )
{
	int i;

	_headerPanel = new HeaderPanel(0,0,wide,HEADER_SIZE_Y);
	_headerPanel->setParent(this);
	_headerPanel->setFgColor( 100,100,100, 100 );
	_headerPanel->setBgColor( 0, 0, 0, 100 );

	CSBLabel *pLabel[5];
	
	pLabel[0] = new CSBLabel( "Address", "address" );
	pLabel[1] = new CSBLabel( "Server", "hostname" );
	pLabel[2] = new CSBLabel( "Map", "map" );
	pLabel[3] = new CSBLabel( "Current", "current" );
	pLabel[4] = new CSBLabel( "Latency", "ping" );

	for ( i = 0; i < 5; i++ )
	{
		_headerPanel->addSectionPanel( pLabel[i] );
	}

	// _headerPanel->setFont( Scheme::sf_primary1 );

	_headerPanel->setSliderPos( 0, CSIZE_ADDRESS );
	_headerPanel->setSliderPos( 1, CSIZE_SERVER );
	_headerPanel->setSliderPos( 2, CSIZE_MAP );
	_headerPanel->setSliderPos( 3, CSIZE_CURRENT );
	_headerPanel->setSliderPos( 4, CSIZE_PING );

	_tablePanel = new ServerBrowserTablePanel( 0, HEADER_SIZE_Y, wide, tall - HEADER_SIZE_Y, NUM_COLUMNS );
	_tablePanel->setParent(this);
	_tablePanel->setHeaderPanel(_headerPanel);
	_tablePanel->setFgColor( 100,100,100, 100 );
	_tablePanel->setBgColor( 0, 0, 0, 100 );

	_tablePanel->addInputSignal( new CBrowser_InputSignal( (ServerBrowserTablePanel *)_tablePanel ) );

	for ( i = 0; i < 5; i++ )
	{
		pLabel[i]->setTable( (ServerBrowserTablePanel * )_tablePanel );
	}

	int bw = 80, bh = 15;
	int by = tall - HEADER_SIZE_Y;

	int btnx = 10;

	_connectButton = new CommandButton( "Connect", btnx, by, bw, bh );
	_connectButton->setParent( this );
	_connectButton->addActionSignal( new ConnectHandler(  (ServerBrowserTablePanel * )_tablePanel ) );

	btnx += bw;

	_refreshButton = new CommandButton( "Refresh", btnx, by, bw, bh );
	_refreshButton->setParent( this );
	_refreshButton->addActionSignal( new RefreshHandler(  (ServerBrowserTablePanel * )_tablePanel ) );

	/*
	btnx += bw;

	_broadcastRefreshButton = new CommandButton( "LAN", btnx, by, bw, bh );
	_broadcastRefreshButton->setParent( this );
	_broadcastRefreshButton->addActionSignal( new BroadcastRefreshHandler(  (ServerBrowserTablePanel * )_tablePanel ) );
	*/

	btnx += bw;

	_stopButton = new CommandButton( "Stop", btnx, by, bw, bh );
	_stopButton->setParent( this );
	_stopButton->addActionSignal( new StopHandler(  (ServerBrowserTablePanel * )_tablePanel ) );

	/*
	btnx += bw;

	_pingButton = new CommandButton( "Test", btnx, by, bw, bh );
	_pingButton->setParent( this );
	_pingButton->addActionSignal( new PingHandler(  (ServerBrowserTablePanel * )_tablePanel ) );
	
	btnx += bw;

	_sortButton = new CommandButton( "Sort", btnx, by, bw, bh );
	_sortButton->setParent( this );
	_sortButton->addActionSignal( new SortHandler(  (ServerBrowserTablePanel * )_tablePanel ) );
	*/

	btnx += bw;

	_cancelButton = new CommandButton( "Close", btnx, by, bw, bh );
	_cancelButton->setParent( this );
	_cancelButton->addActionSignal( new CancelHandler(  (ServerBrowserTablePanel * )_tablePanel ) );

	setPaintBorderEnabled(false);
	setPaintBackgroundEnabled(false);
	setPaintEnabled(false);

}

void ServerBrowser::setSize(int wide,int tall)
{
	Panel::setSize(wide,tall);

	_headerPanel->setBounds(0,0,wide,HEADER_SIZE_Y);
	_tablePanel->setBounds(0,HEADER_SIZE_Y,wide,tall - HEADER_SIZE_Y);
	
	_connectButton->setBounds( 5, tall - HEADER_SIZE_Y, 75, 15 );
	_refreshButton->setBounds( 85, tall - HEADER_SIZE_Y, 75, 15 );
	/*
	_broadcastRefreshButton->setBounds( 165, tall - HEADER_SIZE_Y, 75, 15 );
	*/
	_stopButton->setBounds( 165, tall - HEADER_SIZE_Y, 75, 15 );
	/*
	_pingButton->setBounds( 325, tall - HEADER_SIZE_Y, 75, 15 );
	*/
	_cancelButton->setBounds( 245, tall - HEADER_SIZE_Y, 75, 15 );
}

void CBrowser_InputSignal::mousePressed(MouseCode code,Panel* panel)
{
	int x, y;
	int therow = 2;

	if ( code != MOUSE_LEFT )
		return;

	panel->getApp()->getCursorPos(x,y);
	panel->screenToLocal( x, y );

	therow = y / CELL_HEIGHT;
	
	// Figure out which row it's on
	m_pBrowser->setMouseOverRow( therow );
}

void CBrowser_InputSignal::mouseDoublePressed(MouseCode code,Panel* panel)
{
	int x, y;
	int therow = 2;

	if ( code != MOUSE_LEFT )
		return;

	panel->getApp()->getCursorPos(x,y);
	panel->screenToLocal( x, y );

	therow = y / CELL_HEIGHT;
	
	// Figure out which row it's on
	m_pBrowser->setMouseOverRow( therow );
	m_pBrowser->DoConnect();
}
