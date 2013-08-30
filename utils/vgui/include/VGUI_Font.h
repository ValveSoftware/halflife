//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_FONT_H
#define VGUI_FONT_H

#include<VGUI.h>

namespace vgui
{

class BaseFontPlat;

//TODO: cursors and fonts should work like gl binds
class VGUIAPI Font
 {
 public:
  Font(const char* name,int tall,int wide,float rotation,int weight,bool italic,bool underline,bool strikeout,bool symbol);
  // If pFileData is non-NULL, then it will try to load the 32-bit (RLE) TGA file. If that fails,
  // it will create the font using the specified parameters.
  // pUniqueName should be set if pFileData and fileDataLen are set so it can determine if a font is already loaded.
  Font(const char* name,void *pFileData,int fileDataLen, int tall,int wide,float rotation,int weight,bool italic,bool underline,bool strikeout,bool symbol);
 private:
  virtual void init(const char* name,void *pFileData,int fileDataLen, int tall,int wide,float rotation,int weight,bool italic,bool underline,bool strikeout,bool symbol);
 public:
  BaseFontPlat* getPlat();
  virtual void getCharRGBA(int ch,int rgbaX,int rgbaY,int rgbaWide,int rgbaTall,uchar* rgba);
  virtual void getCharABCwide(int ch,int& a,int& b,int& c);
  virtual void getTextSize(const char* text,int& wide,int& tall);
  virtual int  getTall();
#ifndef _WIN32
  virtual int getWide();
#endif
  virtual int  getId();
 protected:
  char*			_name;
  BaseFontPlat*	_plat;
  int			_id;
 friend class Surface;
 };


void Font_Reset();

}

#endif