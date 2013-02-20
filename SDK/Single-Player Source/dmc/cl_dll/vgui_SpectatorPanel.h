
//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

// vgui_SpectatorPanel.h: interface for the SpectatorPanel class.
//
//////////////////////////////////////////////////////////////////////

#ifndef SPECTATORPANEL_H
#define SPECTATORPANEL_H

#include <VGUI_Panel.h>
#include <VGUI_Label.h>
#include <VGUI_Button.h>

using namespace vgui;

#define SPECTATOR_PANEL_CMD_NONE				0

#define SPECTATOR_PANEL_CMD_OPTIONS				1
#define	SPECTATOR_PANEL_CMD_PREVPLAYER			2
#define SPECTATOR_PANEL_CMD_NEXTPLAYER			3
#define	SPECTATOR_PANEL_CMD_HIDEMENU			4
#define	SPECTATOR_PANEL_CMD_TOGGLE_INSET		5


class SpectatorPanel : public Panel //, public vgui::CDefaultInputSignal
{

public:
	SpectatorPanel(int x,int y,int wide,int tall);
	virtual ~SpectatorPanel();

	void			ActionSignal(int cmd);

	// InputSignal overrides.
public:
	void Initialize();

	


public:

	void EnableInsetView(bool isEnabled);
	void ShowMenu(bool isVisible);

	
	CommandButton     *	m_OptionButton;
	CommandButton     *	m_HideButton;
	CommandButton	  *	m_PrevPlayerButton;
	CommandButton	  *	m_NextPlayerButton;
		
	CTransparentPanel *			m_TopBorder;
	CTransparentPanel *			m_BottomBorder;

	CommandButton *	m_InsetViewButton;
	
	Label *			m_TopMainLabel;
	Label *			m_BottomMainLabel;


	bool			m_menuVisible;
};



class CSpectatorHandler_Command : public ActionSignal
{

private:
	SpectatorPanel * m_pFather;
	int				 m_cmd;

public:
	CSpectatorHandler_Command( SpectatorPanel * panel, int cmd )
	{
		m_pFather = panel;
		m_cmd = cmd;
	}

	virtual void actionPerformed( Panel * panel )
	{
		m_pFather->ActionSignal(m_cmd);
	}
};


#endif // !defined SPECTATORPANEL_H
