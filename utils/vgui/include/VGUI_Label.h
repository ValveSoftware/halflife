//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_LABEL_H
#define VGUI_LABEL_H

#include<VGUI.h>
#include<VGUI_Panel.h>
#include<VGUI_Scheme.h>
#include<VGUI_Image.h>

//TODO: this should use a TextImage for the text

namespace vgui
{

class Panel;
class TextImage;

class VGUIAPI Label : public Panel
{
public:
	enum Alignment
	{
		a_northwest=0,
		a_north,
		a_northeast,
		a_west,
		a_center,
		a_east,
		a_southwest,
		a_south,
		a_southeast,
	};
public:	
	Label(int textBufferLen,const char* text,int x,int y,int wide,int tall);
	Label(const char* text,int x,int y,int wide,int tall);
	Label(const char* text,int x,int y);
	Label(const char* text);
	
	inline Label() : Panel(0,0,10,10)
	{
		init(1,"",true);
	}
private:
	void init(int textBufferLen,const char* text,bool textFitted);
public:
	virtual void setImage(Image* image);
	virtual void setText(int textBufferLen,const char* text);
	virtual void setText(const char* format,...);
	virtual void setFont(Scheme::SchemeFont schemeFont);
	virtual void setFont(Font* font);
	virtual void getTextSize(int& wide,int& tall);
	virtual void getContentSize(int& wide,int& tall);
	virtual void setTextAlignment(Alignment alignment);
	virtual void setContentAlignment(Alignment alignment);
	virtual Panel* createPropertyPanel();
	virtual void setFgColor(int r,int g,int b,int a);
	virtual void setFgColor(vgui::Scheme::SchemeColor sc);
	virtual void setContentFitted(bool state);
protected:
	virtual void computeAlignment(int& tx0,int& ty0,int& tx1,int& ty1,int& ix0,int& iy0,int& ix1,int& iy1,int& minX,int& minY,int& maxX,int& maxY);
	virtual void paint();
	virtual void recomputeMinimumSize();
protected:
	bool       _textEnabled;
	bool       _imageEnabled;
	bool       _contentFitted;
	Alignment  _textAlignment;
	Alignment  _contentAlignment;
	TextImage* _textImage;
	Image*     _image;
};

}

#endif