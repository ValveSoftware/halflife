//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_TREEFOLDER_H
#define VGUI_TREEFOLDER_H

#include<VGUI.h>
#include<VGUI_Panel.h>

namespace vgui
{

class VGUIAPI TreeFolder : public Panel
{
public:
	TreeFolder(const char* name);
	TreeFolder(const char* name,int x,int y);
protected:
	virtual void init(const char* name);
public:
	virtual void setOpenedTraverse(bool state);
	virtual void setOpened(bool state);
	virtual bool isOpened();
protected:
	virtual void paintBackground();
protected:
	bool _opened;
};

}

#endif