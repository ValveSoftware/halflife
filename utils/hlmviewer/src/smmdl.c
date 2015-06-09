//
//                 Half-Life Model Viewer (c) 1999 by Mete Ciragan
//
// file:           smmdl.c
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
// this one cleans up driver names
//
#include <windows.h>
#include <string.h>



int
main (int argc, char *argv[])
{
	char szName[256], szPath[256], *ptr;

	Sleep (2000);

	GetModuleFileName (NULL, szName, 256);
	ptr = strrchr (szName, '\\');
	*ptr = '\0';
	GetCurrentDirectory (256, szPath);
	SetCurrentDirectory (szName);

	CopyFile ("opengl32.dll", "3dfxgl.dll", FALSE);
	DeleteFile ("opengl32.dll");

	DeleteFile ("hlmv.cfg");

	Sleep (2000);

	WinExec ("hlmv.exe", SW_SHOW);

	SetCurrentDirectory (szPath);

	return 0;
}
