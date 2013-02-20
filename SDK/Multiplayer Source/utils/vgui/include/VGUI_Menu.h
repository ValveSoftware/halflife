//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_MENU_H
#define VGUI_MENU_H

#include<VGUI.h>
#include<VGUI_Panel.h>

namespace vgui
{

class Panel;

class VGUIAPI Menu : public Panel
{
public:
	Menu(int x,int y,int wide,int tall);
	Menu(int wide,int tall);
public:
	virtual void addMenuItem(Panel* panel);
};

}

#endif