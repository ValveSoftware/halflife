//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxChoice.h
// implementation: all
// last modified:  Mar 14 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#ifndef INCLUDED_MXCHOICE
#define INCLUDED_MXCHOICE



#ifndef INCLUDED_MXWIDGET
#include <mx/mxWidget.h>
#endif



class mxWindow;



class mxChoice_i;
class mxChoice : public mxWidget
{
	mxChoice_i *d_this;

public:
	// CREATORS
	mxChoice (mxWindow *parent, int x, int y, int w, int h, int id = 0);
	virtual ~mxChoice ();

	// MANIPULATORS
	void add (const char *item);
	void select (int index);
	void remove (int index);
	void removeAll ();

	// ACCESSORS
	int getItemCount () const;
	int getSelectedIndex () const;

private:
	// NOT IMPLEMENTED
	mxChoice (const mxChoice&);
	mxChoice& operator= (const mxChoice&);
};



#endif // INCLUDED_MXCHOICE
