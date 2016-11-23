//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_LISTPANEL_H
#define VGUI_LISTPANEL_H

#include<VGUI.h>
#include<VGUI_Panel.h>

namespace vgui
{

class ScrollBar;

//TODO: make a ScrollPanel and use a constrained one for _vpanel in ListPanel
class VGUIAPI ListPanel : public Panel
{
public:
	ListPanel(int x,int y,int wide,int tall);
public:
	virtual void setSize(int wide,int tall);
	virtual void addString(const char* str);
	virtual void addItem(Panel* panel);
	virtual void setPixelScroll(int value);
	virtual void translatePixelScroll(int delta);
protected:
	virtual void performLayout();
	virtual void paintBackground();
protected: 
	Panel*     _vpanel;
	ScrollBar* _scroll;
};

}

#endif