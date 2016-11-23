//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_BITMAP_H
#define VGUI_BITMAP_H

#include<VGUI.h>
#include<VGUI_Image.h>

namespace vgui
{

class Panel;

class VGUIAPI Bitmap : public Image
{
private:
	int         _id;
	bool        _uploaded;
public:
	Bitmap();
protected:
	virtual void setSize(int wide,int tall);
	virtual void setRGBA(int x,int y,uchar r,uchar g,uchar b,uchar a);
public:
	virtual void paint(Panel* panel);
protected:
	uchar* _rgba;
};

}

#endif