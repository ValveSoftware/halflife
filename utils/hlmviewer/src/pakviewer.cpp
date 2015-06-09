//
//                 Half-Life Model Viewer (c) 1999 by Mete Ciragan
//
// file:           pakviewer.cpp
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mx/mx.h>
#include "pakviewer.h"
#include "mdlviewer.h"
#include "GlWindow.h"
#include "StudioModel.h"
#include "ControlPanel.h"
#include "FileAssociation.h"



int
pak_ExtractFile (const char *pakFile, const char *lumpName, char *outFile)
{
	FILE *file = fopen (pakFile, "rb");
	if (!file)
		return 0;

	int ident, dirofs, dirlen;

	fread (&ident, sizeof (int), 1, file);
	if (ident != (int) (('K' << 24) + ('C' << 16) + ('A' << 8) + 'P'))
	{
		fclose (file);
		return 0;
	}

	fread (&dirofs, sizeof (int), 1, file);
	fread (&dirlen, sizeof (int), 1, file);

	fseek (file, dirofs, SEEK_SET);
	int numLumps = dirlen / 64;

	for (int i = 0; i < numLumps; i++)
	{
		char name[56];
		int filepos, filelen;

		fread (name, 56, 1, file);
		fread (&filepos, sizeof (int), 1, file);
		fread (&filelen, sizeof (int), 1, file);

		if (!mx_strcasecmp (name, lumpName))
		{
			FILE *out = fopen (outFile, "wb");
			if (!out)
			{
				fclose (file);
				return 0;
			}

			fseek (file, filepos, SEEK_SET);

			while (filelen--)
				fputc (fgetc (file), out);

			fclose (out);
			fclose (file);

			return 1;
		}
	}

	fclose (file);

	return 0;
}



PAKViewer::PAKViewer (mxWindow *window)
: mxWindow (window, 0, 0, 0, 0, "", mxWindow::Normal)
{
	strcpy (d_pakFile, "");
	strcpy (d_currLumpName, "");

	tvPAK = new mxTreeView (this, 0, 0, 0, 0, IDC_PAKVIEWER);
	pmMenu = new mxPopupMenu ();
	pmMenu->add ("Load Model", 1);
	pmMenu->addSeparator ();
	pmMenu->add ("Load Background", 2);
	pmMenu->add ("Load Ground", 3);
	pmMenu->addSeparator ();
	pmMenu->add ("Play Sound", 4);
	pmMenu->addSeparator ();
	pmMenu->add ("Extract File...", 5);
	setLoadEntirePAK (true);

	setVisible (false);
}



PAKViewer::~PAKViewer ()
{
	tvPAK->remove (0);
	closePAKFile ();
}



void
_makeTempFileName (char *str, const char *suffix)
{
	strcpy (str, mx_gettemppath ());

	strcat (str, "/hltempmodel");
	strcat (str, suffix);
}



int
PAKViewer::handleEvent (mxEvent *event)
{
	switch (event->event)
	{
	case mxEvent::Action:
	{
		switch (event->action)
		{
		case IDC_PAKVIEWER: // tvPAK
			if (event->flags & mxEvent::RightClicked)
			{
				pmMenu->setEnabled (1, strstr (d_currLumpName, ".mdl") != 0);
				pmMenu->setEnabled (2, strstr (d_currLumpName, ".tga") != 0);
				pmMenu->setEnabled (3, strstr (d_currLumpName, ".tga") != 0);
				pmMenu->setEnabled (4, strstr (d_currLumpName, ".wav") != 0);
				int ret = pmMenu->popup (tvPAK, event->x, event->y);
				switch (ret)
				{
				case 1:
					OnLoadModel ();
					break;

				case 2:
					OnLoadTexture (0);
					break;

				case 3:
					OnLoadTexture (1);
					break;

				case 4:
					OnPlaySound ();
					break;

				case 5:
					OnExtract ();
					break;
				}
			}
			else if (event->flags & mxEvent::DoubleClicked)
			{
				OnPAKViewer ();
				char e[16];

				strncpy (e, mx_getextension (d_currLumpName), 16);
				int mode = g_FileAssociation->getMode (&e[1]);
				if (mode == -1)
					return 1;

				char *program = g_FileAssociation->getProgram (&e[1]);

#ifdef WIN32
				if (mode == 0)
				{
					char str[256];
					_makeTempFileName (str, e);
					if (!pak_ExtractFile (d_pakFile, d_currLumpName, str))
						mxMessageBox (this, "Error extracting from PAK file.", g_appTitle, MX_MB_OK | MX_MB_ERROR);
					else
					{
						if (program)
						{
							char path[256];
							strcpy (path, program);
							strcat (path, " ");
							strcat (path, str);
							if ((int) WinExec (path, SW_SHOW) <= 32)
								mxMessageBox (this, "Error executing specified program.", g_appTitle, MX_MB_OK | MX_MB_ERROR);
						}
					}
				}

				// associated program
				else if (mode == 1)
				{
					char str[256];
					_makeTempFileName (str, e);
					if (!pak_ExtractFile (d_pakFile, d_currLumpName, str))
						mxMessageBox (this, "Error extracting from PAK file.", g_appTitle, MX_MB_OK | MX_MB_ERROR);
					else
						if ((int) ShellExecute ((HWND) getHandle (), "open", str, 0, 0, SW_SHOW) <= 32)
							mxMessageBox (this, "Error executing document with associated program.", g_appTitle, MX_MB_OK | MX_MB_ERROR);
				}

				// HLMV default
				else	
#endif
				if (mode == 2)
				{
					if (!strcmp (e, ".mdl"))
						OnLoadModel ();

					else if (!strcmp (e, ".tga"))
						OnLoadTexture (0);

					else if (!strcmp (e, ".wav"))
						OnPlaySound ();

					return 1;
				}
			}
			
			return OnPAKViewer ();
		} // event->action
	} // mxEvent::Action
	break;

	case mxEvent::Size:
	{
		tvPAK->setBounds (0, 0, event->width, event->height);
	} // mxEvent::Size
	break;

	} // event->event

	return 1;
}



int
PAKViewer::OnPAKViewer ()
{
	mxTreeViewItem *tvi = tvPAK->getSelectedItem ();
	if (tvi)
	{
		strcpy (d_currLumpName, tvPAK->getLabel (tvi));

		// find the full lump name
		mxTreeViewItem *tviParent = tvPAK->getParent (tvi);
		char tmp[128];
		while (tviParent)
		{
			strcpy (tmp, d_currLumpName);
			strcpy (d_currLumpName, tvPAK->getLabel (tviParent));
			strcat (d_currLumpName, "/");
			strcat (d_currLumpName, tmp);
			tviParent = tvPAK->getParent (tviParent);
		}

		if (!d_loadEntirePAK)
		{
			// finally insert "models/"
			strcpy (tmp, d_currLumpName);
			strcpy (d_currLumpName, "models/");
			strcat (d_currLumpName, tmp);
		}
	}

	return 1;
}



int
PAKViewer::OnLoadModel ()
{
	static char str2[256];
	char suffix[16];

	strcpy (suffix, ".mdl");
	_makeTempFileName (str2, suffix);

	if (!pak_ExtractFile (d_pakFile, d_currLumpName, str2))
	{
		mxMessageBox (this, "Error extracting from PAK file.", g_appTitle, MX_MB_OK | MX_MB_ERROR);
		return 1;
	}

	g_studioModel.FreeModel ();
	studiohdr_t *hdr = g_studioModel.LoadModel (str2);
	if (!hdr)
	{
		mxMessageBox (this, "Error reading model header.", g_appTitle, MX_MB_OK | MX_MB_ERROR);
		return 1;
	}

	if (hdr->numtextures == 0)
	{
		char texturename[256];

		strcpy( texturename, d_currLumpName );
		strcpy( &texturename[strlen(texturename) - 4], "T.mdl" );

		strcpy (suffix, "T.mdl");
		_makeTempFileName (str2, suffix);

		if (!pak_ExtractFile (d_pakFile, texturename, str2))
		{
			g_studioModel.FreeModel ();
			mxMessageBox (this, "Error extracting from PAK file 1.", g_appTitle, MX_MB_OK | MX_MB_ERROR);
			return 1;
		}
	}

	if (hdr->numseqgroups > 1)
	{
		for (int i = 1; i < hdr->numseqgroups; i++)
		{
			char seqgroupname[256];

			strcpy( seqgroupname, d_currLumpName );
			sprintf( &seqgroupname[strlen(seqgroupname) - 4], "%02d.mdl", i );

			sprintf (suffix, "%02d.mdl", i);
			_makeTempFileName (str2, suffix);

			if (!pak_ExtractFile (d_pakFile, seqgroupname, str2))
			{
				g_studioModel.FreeModel ();
				mxMessageBox (this, "Error extracting from PAK file 2.", g_appTitle, MX_MB_OK | MX_MB_ERROR);
				return 1;
			}
		}
	}

	g_studioModel.FreeModel ();

	strcpy (suffix, ".mdl");
	_makeTempFileName (str2, suffix);
	g_ControlPanel->loadModel (str2);

	return 1;
}



int
PAKViewer::OnLoadTexture (int pos)
{
	static char str2[256];
	char suffix[16] = "";

	if (strstr (d_currLumpName, ".tga"))
		sprintf (suffix, "%d%s", pos, ".tga");

	_makeTempFileName (str2, suffix);

	if (!pak_ExtractFile (d_pakFile, d_currLumpName, str2))
	{
		mxMessageBox (this, "Error extracting from PAK file.", g_appTitle, MX_MB_OK | MX_MB_ERROR);
		return 1;
	}

	if (g_MDLViewer->getGlWindow ()->loadTexture (str2, pos))
	{
		if (pos == 0)
			g_ControlPanel->setShowBackground (true);
		else
			g_ControlPanel->setShowGround (true);
	}
	else
		mxMessageBox (this, "Error loading texture.",  g_appTitle, MX_MB_OK | MX_MB_ERROR);

	return 1;
}



int
PAKViewer::OnPlaySound ()
{
#ifdef WIN32
	static char str2[256];
	char suffix[16] = "";

	// stop any playing sound
	PlaySound (0, 0, SND_FILENAME | SND_ASYNC);

	if (strstr (d_currLumpName, ".wav"))
		sprintf (suffix, "%d%s", 44, ".wav");

	_makeTempFileName (str2, suffix);

	if (!pak_ExtractFile (d_pakFile, d_currLumpName, str2))
	{
		mxMessageBox (this, "Error extracting from PAK file.", g_appTitle, MX_MB_OK | MX_MB_ERROR);
		return 1;
	}

	PlaySound (str2, 0, SND_FILENAME | SND_ASYNC);

#endif
	return 1;
}



int
PAKViewer::OnExtract ()
{
	char *ptr = (char *) mxGetSaveFileName (this, "", "*.*");
	if (ptr)
	{
		if (!pak_ExtractFile (d_pakFile, d_currLumpName, ptr))
			mxMessageBox (this, "Error extracting from PAK file.", g_appTitle, MX_MB_OK | MX_MB_ERROR);
	}

	return 1;
}



int
_compare(const void *arg1, const void *arg2)
{
	if (strchr ((char *) arg1, '/') && !strchr ((char *) arg2, '/'))
		return -1;

	else if (!strchr ((char *) arg1, '/') && strchr ((char *) arg2, '/'))
		return 1;

	else
		return strcmp ((char *) arg1, (char *) arg2);
}



bool
PAKViewer::openPAKFile (const char *pakFile)
{
	FILE *file = fopen (pakFile, "rb");
	if (!file)
		return false;

	int ident, dirofs, dirlen;

	// check for id
	fread (&ident, sizeof (int), 1, file);
	if (ident != (int) (('K' << 24) + ('C' << 16) + ('A' << 8) + 'P'))
	{
		fclose (file);
		return false;
	}

	// load lumps
	fread (&dirofs, sizeof (int), 1, file);
	fread (&dirlen, sizeof (int), 1, file);
	int numLumps = dirlen / 64;

	fseek (file, dirofs, SEEK_SET);
	lump_t *lumps = new lump_t[numLumps];
	if (!lumps)
	{
		fclose (file);
		return false;
	}

	fread (lumps, sizeof (lump_t), numLumps, file);
	fclose (file);

	qsort (lumps, numLumps, sizeof (lump_t), _compare);

	// save pakFile for later
	strcpy (d_pakFile, pakFile);

	tvPAK->remove (0);

	char namestack[32][32];
	mxTreeViewItem *tvistack[32];
	for (int k = 0; k < 32; k++)
	{
		strcpy (namestack[k], "");
		tvistack[k] = 0;
	}

	for (int i = 0; i < numLumps; i++)
	{
		if (d_loadEntirePAK || !strncmp (lumps[i].name, "models", 6))
		{
			char *tok;
			if (d_loadEntirePAK)
				tok = &lumps[i].name[0];
			else
				tok = &lumps[i].name[7];

			int i = 1;
			while (tok)
			{
				char *end = strchr (tok, '/');
				if (end)
					*end = '\0';

				if (strcmp (namestack[i], tok))
				{
					strcpy (namestack[i], tok);
/*
					if (i == 0)
						tvistack[i] = tvPAK->add (0, tok);
					else*/
						tvistack[i] = tvPAK->add (tvistack[i - 1], tok);

					for (int j = i + 1; j < 32; j++)
					{
						strcpy (namestack[j], "");
						tvistack[j] = 0;
					}
				}

				++i;

				if (end)
					tok = end + 1;
				else
					tok = 0;
			}
		}
	}

	delete[] lumps;

	setVisible (true);

	return true;
}



void
PAKViewer::closePAKFile ()
{
	strcpy (d_pakFile, "");
	setVisible (false);
}
