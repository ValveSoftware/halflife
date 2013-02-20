//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_TABPANEL_H
#define VGUI_TABPANEL_H

#include<VGUI.h>
#include<VGUI_Panel.h>

namespace vgui
{

class ButtonGroup;

class VGUIAPI TabPanel : public Panel
{
public:
	enum TabPlacement
	{
		tp_top=0,
		tp_bottom,
		tp_left,
		tp_right,
	};
public:
	TabPanel(int x,int y,int wide,int tall);
public:
	virtual Panel* addTab(const char* text);
	virtual void   setSelectedTab(Panel* tab);
	virtual void   setSize(int wide,int tall);
protected:
	virtual void recomputeLayoutTop();
	virtual void recomputeLayout();
protected:
	TabPlacement _tabPlacement;
	Panel*       _tabArea;
	Panel*       _clientArea;
	Panel*       _selectedTab;
	Panel*       _selectedPanel;
	ButtonGroup* _buttonGroup;
};

}

#endif