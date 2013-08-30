//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef ServerBrowser_H
#define ServerBrowser_H

#include<VGUI_Panel.h>

namespace vgui
{
class Button;
class TablePanel;
class HeaderPanel;
}

class CTransparentPanel;
class CommandButton;

// Scoreboard positions
#define SB_X_INDENT				(20 * ((float)ScreenHeight / 640))
#define SB_Y_INDENT				(20 * ((float)ScreenHeight / 480))

class ServerBrowser : public CTransparentPanel
{
private:
	HeaderPanel * _headerPanel;
	TablePanel*  _tablePanel;

	CommandButton*	   _connectButton;
	CommandButton*	   _refreshButton;
	CommandButton*	   _broadcastRefreshButton;
	CommandButton*	   _stopButton;
	CommandButton*	   _sortButton;
	CommandButton*	   _cancelButton;

	CommandButton*		_pingButton;

public:
	ServerBrowser(int x,int y,int wide,int tall);
public:
	virtual void setSize(int wide,int tall);
};



#endif