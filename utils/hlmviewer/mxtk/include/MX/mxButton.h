//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxButton.h
// implementation: all
// last modified:  Mar 14 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#ifndef INCLUDED_MXBUTTON
#define INCLUDED_MXBUTTON



#ifndef INCLUDED_MXWIDGET
#include <mx/mxWidget.h>
#endif



class mxWindow;



class mxButton_i;
class mxButton : public mxWidget
{
	mxButton_i *d_this;

public:
	// CREATORS

	mxButton (mxWindow *parent, int x, int y, int w, int h, const char *label = 0, int id = 0);

	virtual ~mxButton ();

private:
	// NOT IMPLEMENTED
	mxButton (const mxButton&);
	mxButton& operator= (const mxButton&);
};



#endif // INCLUDED_MXBUTTON

