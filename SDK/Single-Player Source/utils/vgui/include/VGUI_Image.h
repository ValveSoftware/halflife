//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_IMAGE_H
#define VGUI_IMAGE_H

#include<VGUI.h>
#include<VGUI_Scheme.h>
#include<VGUI_Color.h>

//TODO:: needs concept of insets

namespace vgui
{

class Panel;

class VGUIAPI Image
{
friend class Panel;
private:
	int                 _pos[2];
	int                 _size[2];
	Panel*              _panel;
	Color               _color;
public:
	Image();
public:
	virtual void setPos(int x,int y);
	virtual void getPos(int& x,int& y);
	virtual void getSize(int& wide,int& tall);
	virtual void setColor(Color color);
	virtual void getColor(Color& color);
protected:
	virtual void setSize(int wide,int tall);
	virtual void drawSetColor(Scheme::SchemeColor sc);
	virtual void drawSetColor(int r,int g,int b,int a);
	virtual void drawFilledRect(int x0,int y0,int x1,int y1);
	virtual void drawOutlinedRect(int x0,int y0,int x1,int y1);
	virtual void drawSetTextFont(Scheme::SchemeFont sf);
	virtual void drawSetTextFont(Font* font);
	virtual void drawSetTextColor(Scheme::SchemeColor sc);
	virtual void drawSetTextColor(int r,int g,int b,int a);
	virtual void drawSetTextPos(int x,int y);
	virtual void drawPrintText(const char* str,int strlen);
	virtual void drawPrintText(int x,int y,const char* str,int strlen);
	virtual void drawPrintChar(char ch);
	virtual void drawPrintChar(int x,int y,char ch);
	virtual void drawSetTextureRGBA(int id,const char* rgba,int wide,int tall);
	virtual void drawSetTexture(int id);
	virtual void drawTexturedRect(int x0,int y0,int x1,int y1);
	virtual void paint(Panel* panel);
public:
	virtual void doPaint(Panel* panel);
};
}

#endif