//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxTab.h
// implementation: all
// last modified:  Apr 18 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#ifndef INCLUDED_MXTAB
#define INCLUDED_MXTAB



#ifndef INCLUDED_MXWIDGET
#include <mx/mxWidget.h>
#endif



class mxWindow;



class mxTab_i;
class mxTab : public mxWidget
{
	mxTab_i *d_this;

public:
	// CREATORS
	mxTab (mxWindow *parent, int x, int y, int w, int h, int id = 0);
	virtual ~mxTab ();

	// MANIPULATORS
	void add (mxWidget *widget, const char *text);
	void remove (int index);
	void select (int index);

	// ACCESSORS
	int getSelectedIndex () const;

private:
	// NOT IMPLEMENTED
	mxTab (const mxTab&);
	mxTab& operator= (const mxTab&);
};



#endif // INCLUDED_MXTAB
