//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_BUTTONGROUP_H
#define VGUI_BUTTONGROUP_H

#include<VGUI.h>
#include<VGUI_Dar.h>

namespace vgui
{

class Button;

class VGUIAPI ButtonGroup
{
public:
	virtual void addButton(Button* button);
	virtual void setSelected(Button* button);
protected:
	Dar<Button*> _buttonDar;
};

}

#endif