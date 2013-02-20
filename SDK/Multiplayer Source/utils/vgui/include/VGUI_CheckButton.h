//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_CHECKBUTTON_H
#define VGUI_CHECKBUTTON_H

#include<VGUI.h>
#include<VGUI_ToggleButton.h>

namespace vgui
{

class VGUIAPI CheckButton : public ToggleButton
{
public:
	CheckButton(const char* text,int x,int y,int wide,int tall);
	CheckButton(const char* text,int x,int y);
protected:
	virtual void paintBackground();
};

}

#endif