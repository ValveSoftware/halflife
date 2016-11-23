//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_ETCHEDBORDER_H
#define VGUI_ETCHEDBORDER_H

#include<VGUI.h>
#include<VGUI_Border.h>

namespace vgui
{

class Panel;

class VGUIAPI EtchedBorder : public Border
{
public:
	EtchedBorder();
protected:
	virtual void paint(Panel* panel);
};

}

#endif