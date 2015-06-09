//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxMessageBox.h
// implementation: all
// last modified:  Mar 14 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#ifndef INCLUDED_MXMESSAGEBOX
#define INCLUDED_MXMESSAGEBOX



class mxWindow;



enum {
	MX_MB_OK = 0,
	MX_MB_YESNO = 1,
	MX_MB_YESNOCANCEL = 2,
	MX_MB_INFORMATION = 4,
	MX_MB_ERROR = 8,
	MX_MB_WARNING = 16,
	MX_MB_QUESTION = 32
};



#ifdef __cplusplus
extern "C" {
#endif

int mxMessageBox (mxWindow *parent, const char *msg, const char *title, int style = 0);

#ifdef __cplusplus
}
#endif



#endif // INCLUDED_MXMESSAGEBOX
