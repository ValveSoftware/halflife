//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_BORDERPAIR_H
#define VGUI_BORDERPAIR_H

#include<VGUI.h>
#include<VGUI_Border.h>

namespace vgui
{

class Panel;

class VGUIAPI BorderPair : public Border
{
public:
	BorderPair(Border* border0,Border* border1);
public:
	virtual void doPaint(Panel* panel);
protected:
	virtual void paint(Panel* panel);
protected:
	Border* _border[2];
};

}

#endif