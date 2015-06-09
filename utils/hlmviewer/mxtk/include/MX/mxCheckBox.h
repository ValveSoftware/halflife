//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxCheckBox.h
// implementation: all
// last modified:  Mar 14 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#ifndef INCLUDED_MXCHECKBOX
#define INCLUDED_MXCHECKBOX



#ifndef INCLUDED_MXWIDGET
#include <mx/mxWidget.h>
#endif



class mxWindow;



class mxCheckBox_i;
class mxCheckBox : public mxWidget
{
	mxCheckBox_i *d_this;

public:
	// CREATORS
	mxCheckBox (mxWindow *parent, int x, int y, int w, int h, const char *label = 0, int id = 0);
	virtual ~mxCheckBox ();

	// MANIPULATORS
	void setChecked (bool b);

	// ACCESSORS
	bool isChecked () const;

private:
	// NOT IMPLEMENTED
	mxCheckBox (const mxCheckBox&);
	mxCheckBox& operator= (const mxCheckBox&);
};



#endif // INCLUDED_MXCHECKBOX
