//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_MINIAPP_H
#define VGUI_MINIAPP_H

#include<VGUI.h>

namespace vgui
{

class Frame;

class VGUIAPI MiniApp
{
public:
	MiniApp();
public:
	virtual void   getName(char* buf,int bufLen);
	virtual Frame* createInstance()=0;
protected:
	virtual void setName(const char* name);
protected:
	char* _name;
};

}

#endif