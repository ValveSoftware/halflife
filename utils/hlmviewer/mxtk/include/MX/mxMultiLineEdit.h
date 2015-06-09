//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxMultiMultiLineEdit.h
// implementation: all
// last modified:  May 24 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#ifndef INCLUDED_MXMULTILINEEDIT
#define INCLUDED_MXMULTILINEEDIT



#ifndef INCLUDED_MXWIDGET
#include <mx/mxWidget.h>
#endif



class mxWindow;



class mxMultiLineEdit_i;
class mxMultiLineEdit : public mxWidget
{
	mxMultiLineEdit_i *d_this;

public:
	// ENUMS
	enum { Normal, ReadOnly, Password };

	// CREATORS
	mxMultiLineEdit (mxWindow *parent, int x, int y, int w, int h, const char *label = 0, int id = 0, int style = 0);
	virtual ~mxMultiLineEdit ();

	// MANIPULATORS
	void setMaxLength (int max);
	void appendText (const char *text);

	// ACCESSORS
	int getMaxLength () const;

private:
	// NOT IMPLEMENTED
	mxMultiLineEdit (const mxMultiLineEdit&);
	mxMultiLineEdit& operator= (const mxMultiLineEdit&);
};



#endif // INCLUDED_MXMULTILINEEDIT

