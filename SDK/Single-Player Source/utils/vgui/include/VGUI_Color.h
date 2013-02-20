//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_COLOR_H
#define VGUI_COLOR_H

#include<VGUI.h>
#include<VGUI_Scheme.h>

//TODO: rename getColor(r,g,b,a) to getRGBA(r,g,b,a)
//TODO: rename setColor(r,g,b,a) to setRGBA(r,g,b,a)
//TODO: rename getColor(sc) to getSchemeColor(sc)
//TODO: rename setColor(sc) to setSchemeColor(sc)

namespace vgui
{

class VGUIAPI Color
{
private:
	uchar               _color[4];
	Scheme::SchemeColor _schemeColor;
public:
	Color();
	Color(int r,int g,int b,int a);
	Color(Scheme::SchemeColor sc);
private:
	virtual void init();
public:
	virtual void setColor(int r,int g,int b,int a);
	virtual void setColor(Scheme::SchemeColor sc);
	virtual void getColor(int& r,int& g,int& b,int& a);
	virtual void getColor(Scheme::SchemeColor& sc);
	virtual int  operator[](int index);
};

}


#endif
