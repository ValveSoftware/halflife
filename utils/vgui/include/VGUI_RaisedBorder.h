//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_RAISEDBORDER_H
#define VGUI_RAISEDBORDER_H

#include<VGUI.h>
#include<VGUI_Border.h>

namespace vgui
{

class Panel;

class VGUIAPI RaisedBorder : public Border
{
public:
	RaisedBorder();
protected:
	virtual void paint(Panel* panel);
};

}

#endif