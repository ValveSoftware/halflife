//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_TEXTIMAGE_H
#define VGUI_TEXTIMAGE_H

#include<VGUI.h>
#include<VGUI_Image.h>
#include<VGUI_Scheme.h>


//TODO: need to add wrapping flag instead of being arbitrary about when wrapping and auto-resizing actually happens
//		This is probably why you are having problems if you had text in a constructor and then changed the font

namespace vgui
{

class Panel;
class Font;
class App;

class VGUIAPI TextImage : public Image
{
private:
	char*                     _text;
	int                       _textBufferLen;
	vgui::Scheme::SchemeFont  _schemeFont;
	vgui::Font*               _font;
	int                       _textColor[4];
	vgui::Scheme::SchemeColor _textSchemeColor;
public:	
	TextImage(int textBufferLen,const char* text);
	TextImage(const char* text);
private:
	virtual void  init(int textBufferLen,const char* text);
public:
	virtual void  getTextSize(int& wide,int& tall);
	virtual void  getTextSizeWrapped(int& wide,int& tall);
	virtual Font* getFont();
	virtual void  setText(int textBufferLen,const char* text);
	virtual void  setText(const char* text);
	virtual void  setFont(vgui::Scheme::SchemeFont schemeFont);
	virtual void  setFont(vgui::Font* font);
	virtual void  setSize(int wide,int tall);
protected:
	virtual void paint(Panel* panel);
};

}

#endif



