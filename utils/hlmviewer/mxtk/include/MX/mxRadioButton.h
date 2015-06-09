//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxRadioButton.h
// implementation: all
// last modified:  Mar 14 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#ifndef INCLUDED_MXRADIOBUTTON
#define INCLUDED_MXRADIOBUTTON



#ifndef INCLUDED_MXWIDGET
#include <mx/mxWidget.h>
#endif



class mxWindow;



class mxRadioButton_i;
class mxRadioButton : public mxWidget
{
	mxRadioButton_i *d_this;

public:
	// CREATORS
	mxRadioButton (mxWindow *parent, int x, int y, int w, int h, const char *label = 0, int id = 0, bool newGroup = 0);
	virtual ~mxRadioButton ();

	// MANIPULATORS
	void setChecked (bool b);

	// ACCESSORS
	bool isChecked () const;

private:
	// NOT IMPLEMENTED
	mxRadioButton (const mxRadioButton&);
	mxRadioButton& operator= (const mxRadioButton&);
};



#endif // INCLUDED_MXRADIOBUTTON
