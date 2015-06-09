//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxToggleButton.h
// implementation: all
// last modified:  Apr 28 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#ifndef INCLUDED_MXTOGGLEBUTTON
#define INCLUDED_MXTOGGLEBUTTON



#ifndef INCLUDED_MXWIDEGT
#include <mx/mxWidget.h>
#endif



class mxWindow;



class mxToggleButton_i;
class mxToggleButton : public mxWidget
{
	mxToggleButton_i *d_this;

public:
	// CREATORS
	mxToggleButton (mxWindow *parent, int x, int y, int w, int h, const char *label = 0, int id = 0);
	virtual ~mxToggleButton ();

	// MANIPULATORS
	void setChecked (bool b);

	// ACCESSORS
	bool isChecked () const;

private:
	// NOT IMPLEMENTED
	mxToggleButton (const mxToggleButton&);
	mxToggleButton& operator= (const mxToggleButton&);
};



#endif // INCLUDED_MXTOGGLEBUTTON
