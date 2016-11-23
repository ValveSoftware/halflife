//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_BITMAPTGA_H
#define VGUI_BITMAPTGA_H

#include<VGUI_Bitmap.h>

namespace vgui
{

class Panel;
class InputStream;

class VGUIAPI BitmapTGA : public Bitmap
{
public:
	BitmapTGA(InputStream* is,bool invertAlpha);
private:
	virtual bool loadTGA(InputStream* is,bool invertAlpha);
};

}

#endif