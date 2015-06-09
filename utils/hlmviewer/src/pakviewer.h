//
//                 Half-Life Model Viewer (c) 1999 by Mete Ciragan
//
// file:           pakviewer.h
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
#ifndef INCLUDED_PAKVIEWER
#define INCLUDED_PAKVIEWER



#ifndef INCLUDED_MXWINDOW
#include <mx/mxWindow.h>
#endif



#define IDC_PAKVIEWER		1001



typedef struct
{
	char name[56];
	int filepos;
	int filelen;
} lump_t;



#ifdef __cpluspus
extern "C" {
#endif

int pak_ExtractFile (const char *pakFile, const char *lumpName, char *outFile);

#ifdef __cpluspus
}
#endif



class mxTreeView;
class mxButton;
class mxPopupMenu;
class GlWindow;



class PAKViewer : public mxWindow
{
	char d_pakFile[256];
	char d_currLumpName[256];
	bool d_loadEntirePAK;
	mxTreeView *tvPAK;
	mxPopupMenu *pmMenu;

public:
	// CREATORS
	PAKViewer (mxWindow *window);
	~PAKViewer ();

	// MANIPULATORS
	virtual int handleEvent (mxEvent *event);
	int OnPAKViewer ();
	int OnLoadModel ();
	int OnLoadTexture (int pos);
	int OnPlaySound ();
	int OnExtract ();

	bool openPAKFile (const char *pakFile);
	void closePAKFile ();
	void setLoadEntirePAK (bool b) { d_loadEntirePAK = b; }

	// ACCESSORS
	bool getLoadEntirePAK () const { return d_loadEntirePAK; }
};



#endif // INCLUDED_PAKVIEWER