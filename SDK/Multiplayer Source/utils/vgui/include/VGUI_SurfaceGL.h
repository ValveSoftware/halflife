//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_SURFACEGL_H
#define VGUI_SURFACEGL_H

//macros borrowed from GLUT to get rid of win32 dependent junk in gl headers
#ifdef _WIN32
# ifndef APIENTRY
#  define VGUI_APIENTRY_DEFINED
#  if (_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED)
#   define APIENTRY    __stdcall
#  else
#   define APIENTRY
#  endif
# endif
# ifndef CALLBACK
#  define VGUI_CALLBACK_DEFINED
#  if (defined(_M_MRX000) || defined(_M_IX86) || defined(_M_ALPHA) || defined(_M_PPC)) && !defined(MIDL_PASS)
#   define CALLBACK __stdcall
#  else
#   define CALLBACK
#  endif
# endif
# ifndef WINGDIAPI
#  define VGUI_WINGDIAPI_DEFINED
#  define WINGDIAPI __declspec(dllimport)
# endif
# ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#  define _WCHAR_T_DEFINED
# endif
# pragma comment(lib,"opengl32.lib")
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#ifdef VGUI_APIENTRY_DEFINED
# undef VGUI_APIENTRY_DEFINED
# undef APIENTRY
#endif

#ifdef VGUI_CALLBACK_DEFINED
# undef VGUI_CALLBACK_DEFINED
# undef CALLBACK
#endif

#ifdef VGUI_WINGDIAPI_DEFINED
# undef VGUI_WINGDIAPI_DEFINED
# undef WINGDIAPI
#endif



#include<VGUI.h>
#include<VGUI_Surface.h>
#include<VGUI_Panel.h>

namespace vgui
{

class VGUIAPI SurfaceGL : public Surface
{
public:
	SurfaceGL(Panel* embeddedPanel);
public:
	virtual void createPopup(Panel* embeddedPanel);
protected:
	virtual bool recreateContext();
	virtual void pushMakeCurrent(Panel* panel,bool useInsets);
	virtual void popMakeCurrent(Panel* panel);
	virtual void makeCurrent();
	virtual void swapBuffers();
	virtual void setColor(int r,int g,int b);
	virtual void filledRect(int x0,int y0,int x1,int y1);
	virtual void outlinedRect(int x0,int y0,int x1,int y1);
	virtual void setTextFont(Font* font);
	virtual void setTextColor(int r,int g,int b);
	virtual void setDrawPos(int x,int y);
	virtual void printText(const char* str,int strlen);
	virtual void setTextureRGBA(int id,const char* rgba,int wide,int tall);
	virtual void setTexture(int id);
	virtual void texturedRect(int x0,int y0,int x1,int y1);
protected:
	int   _drawPos[2];
	uchar _drawColor[3];
	uchar _drawTextColor[3];
};

}

#endif

