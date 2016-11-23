//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_INTLABEL_H
#define VGUI_INTLABEL_H

#include<VGUI.h>
#include<VGUI_Label.h>
#include<VGUI_IntChangeSignal.h>

namespace vgui
{

class Panel;

class VGUIAPI IntLabel : public Label , public IntChangeSignal
{
public:
	IntLabel(int value,int x,int y,int wide,int tall);
public:
	virtual void setValue(int value);
	virtual void intChanged(int value,Panel* panel);
protected:
	virtual void paintBackground();
protected:
	int _value;
};

}

#endif
