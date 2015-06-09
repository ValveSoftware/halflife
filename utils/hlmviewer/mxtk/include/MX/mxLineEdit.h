//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxLineEdit.h
// implementation: all
// last modified:  Mar 14 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#ifndef INCLUDED_MXLINEEDIT
#define INCLUDED_MXLINEEDIT



#ifndef INCLUDED_MXWIDGET
#include <mx/mxWidget.h>
#endif



class mxWindow;



class mxLineEdit_i;
class mxLineEdit : public mxWidget
{
	mxLineEdit_i *d_this;

public:
	// ENUMS
	enum { Normal, ReadOnly, Password };

	// CREATORS
	mxLineEdit (mxWindow *parent, int x, int y, int w, int h, const char *label = 0, int id = 0, int style = 0);
	virtual ~mxLineEdit ();

	// MANIPULATORS
	void setMaxLength (int max);

	// ACCESSORS
	int getMaxLength () const;

private:
	// NOT IMPLEMENTED
	mxLineEdit (const mxLineEdit&);
	mxLineEdit& operator= (const mxLineEdit&);
};



#endif // INCLUDED_MXLINEEDIT

