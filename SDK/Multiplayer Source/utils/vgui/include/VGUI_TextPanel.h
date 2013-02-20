//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_TEXTPANEL_H
#define VGUI_TEXTPANEL_H

#include<VGUI.h>
#include<VGUI_Panel.h>
#include<VGUI_Scheme.h>

//NOTE : If you are having trouble with this class, your problem is probably in TextImage

namespace vgui
{

class TextImage;
class Font;

class VGUIAPI TextPanel : public Panel
{
private:
	TextImage* _textImage;
public:
	TextPanel(const char* text,int x,int y,int wide,int tall);
public:
	virtual void setText(const char* text);
	virtual void setFont(vgui::Scheme::SchemeFont schemeFont);
	virtual void setFont(vgui::Font* font);
	virtual void setSize(int wide,int tall);
	virtual void setFgColor(int r,int g,int b,int a);
	virtual void setFgColor(Scheme::SchemeColor sc);
	virtual TextImage* getTextImage();
protected:
	virtual void paint();
};

}

#endif



