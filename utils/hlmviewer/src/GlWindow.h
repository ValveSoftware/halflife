//
//                 Half-Life Model Viewer (c) 1999 by Mete Ciragan
//
// file:           GlWindow.h
// last modified:  Apr 28 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
// version:        1.2
//
// email:          mete@swissquake.ch
// web:            http://www.swissquake.ch/chumbalum-soft/
//
#ifndef INCLUDED_GLWINDOW
#define INCLUDED_GLWINDOW



#ifndef INCLUDED_MXGLWINDOW
#include <mx/mxGlWindow.h>
#endif

#ifndef INCLUDED_VIEWERSETTINGS
#include "ViewerSettings.h"
#endif



enum // texture names
{
	TEXTURE_GROUND = 1,
	TEXTURE_BACKGROUND = 2
};



class StudioModel;



class GlWindow : public mxGlWindow
{
	int d_textureNames[2];

public:
	// CREATORS
	GlWindow (mxWindow *parent, int x, int y, int w, int h, const char *label, int style);
	~GlWindow ();

	// MANIPULATORS
	virtual int handleEvent (mxEvent *event);
	virtual void draw ();

	int loadTexture (const char *filename, int name);
	void dumpViewport (const char *filename);

	// ACCESSORS
};



extern GlWindow *g_GlWindow;



#endif // INCLUDED_GLWINDOW
