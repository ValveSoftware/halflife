//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxListBox.h
// implementation: all
// last modified:  Mar 14 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#ifndef INCLUDED_MXLIST
#define INCLUDED_MXLIST



#ifndef INCLUDED_MxWIDGET
#include <mx/mxWidget.h>
#endif



class mxWindow;



class mxListBox_i;
class mxListBox : public mxWidget
{
	mxListBox_i *d_this;

public:
	// ENUMS
	enum { Normal, MultiSelection };

	// CREATORS
	mxListBox (mxWindow *parent, int x, int y, int w, int h, int id = 0, int style = 0);
	virtual ~mxListBox ();

	// MANIPULATORS
	void add (const char *item);
	void select (int index);
	void deselect (int index);
	void remove (int index);
	void removeAll ();
	void setItemText (int index, const char *text);
	void setItemData (int index, void *data);

	// ACCESSORS
	int getItemCount () const;
	int getSelectedIndex () const;
	bool isSelected (int index) const;
	const char *getItemText (int index) const;
	void *getItemData (int index) const;

private:
	// NOT IMPLEMENTED
	mxListBox (const mxListBox&);
	mxListBox& operator= (const mxListBox&);
};



#endif // INCLUDED_MXLISTBOX
