//
//                 Half-Life Model Viewer (c) 1999 by Mete Ciragan
//
// file:           FileAssociation.cpp
// last modified:  May 04 1999, Mete Ciragan
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
#include "FileAssociation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mx/mx.h>



FileAssociation *g_FileAssociation = 0;



FileAssociation::FileAssociation ()
: mxWindow (0, 100, 100, 400, 210, "File Associations", mxWindow::Dialog)
{
	cExtension = new mxChoice (this, 5, 5, 220, 22, IDC_EXTENSION);

	//new mxButton (this, 230, 5, 75, 22, "Add", IDC_ADD);
	//new mxButton (this, 310, 5, 75, 22, "Remove", IDC_REMOVE);

	new mxGroupBox (this, 5, 30, 380, 115, "Assocations");
	rbAction[0] = new mxRadioButton (this, 10, 50, 120, 22, "program", IDC_ACTION1, true);
	rbAction[1] = new mxRadioButton (this, 10, 72, 120, 22, "associated program", IDC_ACTION2);
	rbAction[2] = new mxRadioButton (this, 10, 94, 120, 22, "HLMV default", IDC_ACTION3);
	rbAction[3] = new mxRadioButton (this, 10, 116, 120, 22, "none", IDC_ACTION4);
	leProgram = new mxLineEdit (this, 130, 50, 220, 22, "", IDC_PROGRAM);
	leProgram->setEnabled (false);
	bChooseProgram = new mxButton (this, 352, 50, 22, 22, ">>", IDC_CHOOSEPROGRAM);
	bChooseProgram->setEnabled (false);

	rbAction[0]->setChecked (false);
	rbAction[1]->setChecked (true);

	new mxButton (this, 110, 155, 75, 22, "Ok", IDC_OK);
	new mxButton (this, 215, 155, 75, 22, "Cancel", IDC_CANCEL);

	initAssociations ();
}



FileAssociation::~FileAssociation ()
{
}



int
FileAssociation::handleEvent (mxEvent *event)
{
	if (event->event != mxEvent::Action)
		return 0;

	switch (event->action)
	{
	case IDC_EXTENSION:
	{
		int index = cExtension->getSelectedIndex ();
		if (index >= 0)
			setAssociation (index);
	}
	break;

	case IDC_ACTION1:
	case IDC_ACTION2:
	case IDC_ACTION3:
	case IDC_ACTION4:
	{
		leProgram->setEnabled (rbAction[0]->isChecked ());
		bChooseProgram->setEnabled (rbAction[0]->isChecked ());

		int index = cExtension->getSelectedIndex ();
		if (index >= 0)
			d_associations[index].association = event->action - IDC_ACTION1;
		
	}
	break;

	case IDC_PROGRAM:
	{
		int index = cExtension->getSelectedIndex ();
		if (index >= 0)
			strcpy (d_associations[index].program, leProgram->getLabel ());
	}
	break;

	case IDC_CHOOSEPROGRAM:
	{
		const char *ptr = mxGetOpenFileName (this, 0, "*.exe");
		if (ptr)
		{
			leProgram->setLabel (ptr);

			int index = cExtension->getSelectedIndex ();
			if (index >= 0)
				strcpy (d_associations[index].program, leProgram->getLabel ());
		}
	}
	break;

	case IDC_OK:
		saveAssociations ();

	case IDC_CANCEL:
		setVisible (false);
		break;
	}

	return 1;
}



void
FileAssociation::initAssociations ()
{
	int i;

	cExtension->removeAll ();

	for (i = 0; i < 16; i++)
		d_associations[i].association = -1;

	char path[256];
	strcpy (path, mx::getApplicationPath ());
	strcat (path, "/hlmv.fa");
	FILE *file = fopen (path, "rt");
	if (!file)
		return;

	i = 0;
	char line[256];
	while (i < 16 && fgets (line, 256, file))
	{
		int j = 0;
		while (line[++j] != '\"');
		line[j] = '\0';
		strcpy (d_associations[i].extension, &line[1]);

		while (line[++j] != '\"');
		int k = j + 1;
		while (line[++j] != '\"');
		line[j] = '\0';
		strcpy (d_associations[i].program, &line[k]);

		d_associations[i].association = atoi (&line[++j]);

		cExtension->add (d_associations[i].extension);
		++i;
	}

	fclose (file);

	setAssociation (0);
}



void
FileAssociation::setAssociation (int index)
{
	cExtension->select (index);
	leProgram->setLabel (d_associations[index].program);

	for (int i = 0; i < 4; i++)
		rbAction[i]->setChecked (i == d_associations[index].association);

	leProgram->setEnabled (d_associations[index].association == 0);
	bChooseProgram->setEnabled (d_associations[index].association == 0);

	// TODO: check for valid associtaion
#ifdef WIN32__
	char path[256];

	strcpy (path, mx_gettemppath ());
	strcat (path, "/hlmvtemp.");
	strcat (path, d_associations[index].extension);

	FILE *file = fopen (path, "wb");
	if (file)
		fclose (file);

	int val = (int) ShellExecute ((HWND) getHandle (), "open", path, 0, 0, SW_HIDE);
	char str[32];
	sprintf (str, "%d", val);
	setLabel (str);
	rbAction[1]->setEnabled (val != 31);
/*
	WORD dw = 0;
	HICON hIcon = ExtractAssociatedIcon ((HINSTANCE) GetWindowLong ((HWND) getHandle (), GWL_HINSTANCE), path, &dw);
	SendMessage ((HWND) getHandle (), WM_SETICON, (WPARAM) ICON_SMALL, (LPARAM) hIcon);
	char str[32];
	sprintf (str, "%d", (int) hIcon);
	setLabel (str);
*/
	DeleteFile (path);

	//DestroyIcon (hIcon);
#endif

	rbAction[2]->setEnabled (
		!mx_strcasecmp (d_associations[index].extension, "mdl") ||
		!mx_strcasecmp (d_associations[index].extension, "tga") ||
		!mx_strcasecmp (d_associations[index].extension, "wav")
	);
}



void
FileAssociation::saveAssociations ()
{
	char path[256];

	strcpy (path, mx::getApplicationPath ());
	strcat (path, "/hlmv.fa");

	FILE *file = fopen (path, "wt");
	if (!file)
		return;

	for (int i = 0; i < 16; i++)
	{
		if (d_associations[i].association == -1)
			break;

		fprintf (file, "\"%s\" \"%s\" %d\n",
			d_associations[i].extension,
			d_associations[i].program,
			d_associations[i].association);
	}

	fclose (file);
}



int
FileAssociation::getMode (char *extension)
{
	for (int i = 0; i < 16; i++)
	{
		//if (!strcmp (d_associations[i].extension, mx_strlower (extension)))
		if (!strcmp (d_associations[i].extension, extension))
			return d_associations[i].association;
	}

	return -1;
}



char *
FileAssociation::getProgram (char *extension)
{
	for (int i = 0; i < 16; i++)
	{
		//if (!strcmp (d_associations[i].extension, mx_strlower (extension)))
		if (!strcmp (d_associations[i].extension, extension))
			return d_associations[i].program;
	}

	return 0;
}
