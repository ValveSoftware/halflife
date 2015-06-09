//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxInit.h
// implementation: all
// last modified:  Apr 28 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#ifndef INCLUDED_MXINIT
#define INCLUDED_MXINIT



#ifdef WIN32
#include <windows.h>
#endif



class mxWindow;



class mx  
{
public:
	// NO CREATORS
	mx() {}
	virtual ~mx () {}

	// MANIPULATORS
	static int init (int argc, char *argv[]);
	static int run ();
	static int check ();
	static void quit ();
	static void cleanup ();
	static void setEventWindow (mxWindow *window);
	static int setDisplayMode (int w, int h, int bpp);
	static void setIdleWindow (mxWindow *window);

	// ACCESSORS
	static int getDisplayWidth ();
	static int getDisplayHeight ();
	static mxWindow *getMainWindow ();
	static const char *getApplicationPath ();
	static int getTickCount ();

private:
	// NOT IMPLEMENTED
	mx (const mx&);
	mx& operator= (const mx&);
};




#endif // INCLUDED_MXINIT
