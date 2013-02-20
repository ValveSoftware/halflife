//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_MENUITEM_H
#define VGUI_MENUITEM_H

#include<VGUI.h>
#include<VGUI_Button.h>

namespace vgui
{

class Menu;

class VGUIAPI MenuItem : public Button
{
public:
	MenuItem(const char* text);
	MenuItem(const char* text,Menu* subMenu);
protected:
	Menu* _subMenu;
};

}

#endif