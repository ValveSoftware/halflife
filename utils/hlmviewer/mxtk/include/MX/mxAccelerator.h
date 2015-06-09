//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxAccelerator.h
// implementation: all
// last modified:  Jun 13 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#ifndef INCLUDED_MXACCELERATOR
#define INCLUDED_MXACCELERATOR



class mxAccelerator  
{
public:
	// NO CREATORS
	mxAccelerator() {}
	virtual ~mxAccelerator () {}

	// MANIPULATORS
	static void add (int key, int flags, int cmd);
	static void loadTable ();
	static void removeAll ();

private:
	// NOT IMPLEMENTED
	mxAccelerator (const mxAccelerator&);
	mxAccelerator& operator= (const mxAccelerator&);
};



#endif // INCLUDED_MXACCELERATOR
