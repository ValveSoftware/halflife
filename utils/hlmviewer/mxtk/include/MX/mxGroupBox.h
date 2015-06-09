//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxGroupBox.h
// implementation: all
// last modified:  Mar 14 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#ifndef INCLUDED_MXGROUPBOX
#define INCLUDED_MXGROUPBOX



#ifndef INCLUDED_MXWIDGET
#include <mx/mxWidget.h>
#endif



class mxWindow;



class mxGroupBox_i;
class mxGroupBox : public mxWidget
{
	mxGroupBox_i *d_this;

public:
	// CREATORS
	mxGroupBox (mxWindow *parent, int x, int y, int w, int h, const char *label = 0);
	virtual ~mxGroupBox ();

private:
	// NOT IMPLEMENTED
	mxGroupBox (const mxGroupBox&);
	mxGroupBox& operator= (const mxGroupBox&);
};



#endif // INCLUDED_MXGROUPBOX
