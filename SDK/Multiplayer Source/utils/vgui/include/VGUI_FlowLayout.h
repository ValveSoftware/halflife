//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_FLOWLAYOUT_H
#define VGUI_FLOWLAYOUT_H

#include<VGUI.h>
#include<VGUI_Layout.h>

namespace vgui
{

class VGUIAPI FlowLayout : public Layout
{
private:
	int _hgap;
public:
	FlowLayout(int hgap);
public:
	virtual void performLayout(Panel* panel);
};

}

#endif