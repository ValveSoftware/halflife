//
//                 Half-Life Model Viewer (c) 1999 by Mete Ciragan
//
// file:           FileAssociation.h
// last modified:  Apr 28 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
// version:        1.2
//
// email:          mete@swissquake.ch
// web:            http://www.swissquake.ch/chumbalum-soft/
//
#ifndef INCLUDED_FILEASSOCIATION
#define INCLUDED_FILEASSOCIATION



#ifndef INCLUDED_MXWINDOW
#include <mx/mxWindow.h>
#endif



#define IDC_EXTENSION			1001
#define IDC_ADD					1002
#define IDC_REMOVE				1003
#define IDC_ACTION1				1004
#define IDC_ACTION2				1005
#define IDC_ACTION3				1006
#define IDC_ACTION4				1007
#define IDC_PROGRAM				1008
#define IDC_CHOOSEPROGRAM		1009
#define IDC_OK					1010
#define IDC_CANCEL				1011



typedef struct
{
	char extension[16];
	char program[256];
	int association;
} association_t;




class mxChoice;
class mxRadioButton;
class mxLineEdit;
class mxButton;



class FileAssociation : public mxWindow
{
	mxChoice *cExtension;
	mxRadioButton *rbAction[4];
	mxLineEdit *leProgram;
	mxButton *bChooseProgram;
	association_t d_associations[16];

	void initAssociations ();
	void saveAssociations ();

public:
	// CREATORS
	FileAssociation ();
	virtual ~FileAssociation ();

	// MANIPULATORS
	int handleEvent (mxEvent *event);
	void setAssociation (int index);

	// ACCESSORS
	int getMode (char *extension);
	char *getProgram (char *extension);
};



extern FileAssociation *g_FileAssociation;



#endif // INCLUDED_FILEASSOCIATION