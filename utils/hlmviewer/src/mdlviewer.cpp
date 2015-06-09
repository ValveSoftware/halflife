//
//                 Half-Life Model Viewer (c) 1999 by Mete Ciragan
//
// file:           mdlviewer.cpp
// last modified:  Jun 03 1999, Mete Ciragan
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
#include <mx/gl.h>
#include <mx/mxTga.h>
#include "mdlviewer.h"
#include "GlWindow.h"
#include "ControlPanel.h"
#include "StudioModel.h"
#include "pakviewer.h"
#include "FileAssociation.h"



extern bool bFilterTextures;



MDLViewer *g_MDLViewer = 0;
char g_appTitle[] = "Half-Life Model Viewer v1.25";
static char recentFiles[8][256] = { "", "", "", "", "", "", "", "" };



void
MDLViewer::initRecentFiles ()
{
	for (int i = 0; i < 8; i++)
	{
		if (strlen (recentFiles[i]))
		{
			mb->modify (IDC_FILE_RECENTMODELS1 + i, IDC_FILE_RECENTMODELS1 + i, recentFiles[i]);
		}
		else
		{
			mb->modify (IDC_FILE_RECENTMODELS1 + i, IDC_FILE_RECENTMODELS1 + i, "(empty)");
			mb->setEnabled (IDC_FILE_RECENTMODELS1 + i, false);
		}
	}
}



void
MDLViewer::loadRecentFiles ()
{
	char path[256];
	strcpy (path, mx::getApplicationPath ());
	strcat (path, "/hlmv.rf");
	FILE *file = fopen (path, "rb");
	if (file)
	{
		fread (recentFiles, sizeof recentFiles, 1, file);
		fclose (file);
	}
}



void
MDLViewer::saveRecentFiles ()
{
	char path[256];

	strcpy (path, mx::getApplicationPath ());
	strcat (path, "/hlmv.rf");

	FILE *file = fopen (path, "wb");
	if (file)
	{
		fwrite (recentFiles, sizeof recentFiles, 1, file);
		fclose (file);
	}
}



bool
swap3dfxgl (bool b)
{
#ifdef WIN32__
	//
	// rename opengl drivers to 3dfxgl
	//
	char szName[256], szPath[256];

	strcpy (szName, mx::getApplicationPath ());
	GetCurrentDirectory (256, szPath);
	SetCurrentDirectory (szName);

	BOOL ret = TRUE;
	if (b)
	{
		ret &= CopyFile ("3dfxgl.dll", "opengl32.dll", FALSE);
		ret &= DeleteFile ("3dfxgl.dll");
	}
	else
	{
		ret &= CopyFile ("opengl32.dll", "3dfxgl.dll", FALSE);
		ret &= DeleteFile ("opengl32.dll");
	}

	SetCurrentDirectory (szPath);

	return (ret == TRUE);
#else
	return TRUE;
#endif
}



MDLViewer::MDLViewer ()
: mxWindow (0, 0, 0, 0, 0, g_appTitle, mxWindow::Normal)
{
	// create menu stuff
	mb = new mxMenuBar (this);
	mxMenu *menuFile = new mxMenu ();
	mxMenu *menuOptions = new mxMenu ();
	mxMenu *menuView = new mxMenu ();
	mxMenu *menuHelp = new mxMenu ();

	mb->addMenu ("File", menuFile);
	mb->addMenu ("Options", menuOptions);
	mb->addMenu ("View", menuView);
	mb->addMenu ("Help", menuHelp);

	mxMenu *menuRecentModels = new mxMenu ();
	menuRecentModels->add ("(empty)", IDC_FILE_RECENTMODELS1);
	menuRecentModels->add ("(empty)", IDC_FILE_RECENTMODELS2);
	menuRecentModels->add ("(empty)", IDC_FILE_RECENTMODELS3);
	menuRecentModels->add ("(empty)", IDC_FILE_RECENTMODELS4);

	mxMenu *menuRecentPakFiles = new mxMenu ();
	menuRecentPakFiles->add ("(empty)", IDC_FILE_RECENTPAKFILES1);
	menuRecentPakFiles->add ("(empty)", IDC_FILE_RECENTPAKFILES2);
	menuRecentPakFiles->add ("(empty)", IDC_FILE_RECENTPAKFILES3);
	menuRecentPakFiles->add ("(empty)", IDC_FILE_RECENTPAKFILES4);

	menuFile->add ("Load Model...", IDC_FILE_LOADMODEL);
	menuFile->addSeparator ();
	menuFile->add ("Load Background Texture...", IDC_FILE_LOADBACKGROUNDTEX);
	menuFile->add ("Load Ground Texture...", IDC_FILE_LOADGROUNDTEX);
	menuFile->addSeparator ();
	menuFile->add ("Unload Ground Texture", IDC_FILE_UNLOADGROUNDTEX);
	menuFile->addSeparator ();
	menuFile->add ("Open PAK file...", IDC_FILE_OPENPAKFILE);
	menuFile->add ("Close PAK file", IDC_FILE_CLOSEPAKFILE);
	menuFile->addSeparator ();
	menuFile->addMenu ("Recent Models", menuRecentModels);
	menuFile->addMenu ("Recent PAK files", menuRecentPakFiles);
	menuFile->addSeparator ();
	menuFile->add ("Exit", IDC_FILE_EXIT);

	menuOptions->add ("Background Color...", IDC_OPTIONS_COLORBACKGROUND);
	menuOptions->add ("Ground Color...", IDC_OPTIONS_COLORGROUND);
	menuOptions->add ("Light Color...", IDC_OPTIONS_COLORLIGHT);
	menuOptions->addSeparator ();
	menuOptions->add ("Filter Textures", IDC_OPTIONS_FILTERTEXTURES);
	menuOptions->addSeparator ();
	menuOptions->add ("Center View", IDC_OPTIONS_CENTERVIEW);
#ifdef WIN32
	menuOptions->addSeparator ();
	menuOptions->add ("Make Screenshot...", IDC_OPTIONS_MAKESCREENSHOT);
	//menuOptions->add ("Dump Model Info", IDC_OPTIONS_DUMP);
#endif

	menuView->add ("File Associations...", IDC_VIEW_FILEASSOCIATIONS);

#ifdef WIN32
	menuHelp->add ("Goto Homepage...", IDC_HELP_GOTOHOMEPAGE);
	menuHelp->addSeparator ();
#endif
	menuHelp->add ("About...", IDC_HELP_ABOUT);

	mb->setChecked (IDC_OPTIONS_FILTERTEXTURES, bFilterTextures);

	// create the OpenGL window
	d_GlWindow = new GlWindow (this, 0, 0, 0, 0, "", mxWindow::Normal);
#ifdef WIN32
	SetWindowLong ((HWND) d_GlWindow->getHandle (), GWL_EXSTYLE, WS_EX_CLIENTEDGE);
#endif

	d_cpl = new ControlPanel (this);
	d_cpl->setGlWindow (d_GlWindow);
	g_GlWindow = d_GlWindow;

	// finally create the pakviewer window
	d_PAKViewer = new PAKViewer (this);
	g_FileAssociation = new FileAssociation ();

	loadRecentFiles ();
	initRecentFiles ();

	setBounds (20, 20, 640, 540);
	setVisible (true);
}



MDLViewer::~MDLViewer ()
{
	saveRecentFiles ();
	//SaveViewerSettings ("hlmv.cfg");

	swap3dfxgl (false);

#ifdef WIN32
	//DeleteFile ("hlmv.cfg");
	DeleteFile ("midump.txt");
#endif
}



int
MDLViewer::handleEvent (mxEvent *event)
{
	switch (event->event)
	{
	case mxEvent::Action:
	{
		switch (event->action)
		{
		case IDC_FILE_LOADMODEL:
		{
			const char *ptr = mxGetOpenFileName (this, 0, "*.mdl");
			if (ptr)
			{
				int i;
				d_cpl->loadModel (ptr);

				for (i = 0; i < 4; i++)
				{
					if (!mx_strcasecmp (recentFiles[i], ptr))
						break;
				}

				// swap existing recent file
				if (i < 4)
				{
					char tmp[256];
					strcpy (tmp, recentFiles[0]);
					strcpy (recentFiles[0], recentFiles[i]);
					strcpy (recentFiles[i], tmp);
				}

				// insert recent file
				else
				{
					for (i = 3; i > 0; i--)
						strcpy (recentFiles[i], recentFiles[i - 1]);

					strcpy (recentFiles[0], ptr);
				}

				initRecentFiles ();
			}
		}
		break;

		case IDC_FILE_LOADBACKGROUNDTEX:
		case IDC_FILE_LOADGROUNDTEX:
		{
			const char *ptr = mxGetOpenFileName (this, 0, "*.*");
			if (ptr)
			{
				if (d_GlWindow->loadTexture (ptr, event->action - IDC_FILE_LOADBACKGROUNDTEX))
				{
					if (event->action == IDC_FILE_LOADBACKGROUNDTEX)
						d_cpl->setShowBackground (true);
					else
						d_cpl->setShowGround (true);

				}
				else
					mxMessageBox (this, "Error loading texture.", g_appTitle, MX_MB_OK | MX_MB_ERROR);
			}
		}
		break;

		case IDC_FILE_UNLOADGROUNDTEX:
		{
			d_GlWindow->loadTexture (0, 1);
			d_cpl->setShowGround (false);
		}
		break;

		case IDC_FILE_OPENPAKFILE:
		{
			const char *ptr = mxGetOpenFileName (this, "\\sierra\\half-life\\valve", "*.pak");
			if (ptr)
			{
				int i;

				d_PAKViewer->openPAKFile (ptr);

				for (i = 4; i < 8; i++)
				{
					if (!mx_strcasecmp (recentFiles[i], ptr))
						break;
				}

				// swap existing recent file
				if (i < 8)
				{
					char tmp[256];
					strcpy (tmp, recentFiles[4]);
					strcpy (recentFiles[4], recentFiles[i]);
					strcpy (recentFiles[i], tmp);
				}

				// insert recent file
				else
				{
					for (i = 7; i > 4; i--)
						strcpy (recentFiles[i], recentFiles[i - 1]);

					strcpy (recentFiles[4], ptr);
				}

				initRecentFiles ();

				redraw ();
			}
		}
		break;

		case IDC_FILE_CLOSEPAKFILE:
		{
			d_PAKViewer->closePAKFile ();
			redraw ();
		}
		break;

		case IDC_FILE_RECENTMODELS1:
		case IDC_FILE_RECENTMODELS2:
		case IDC_FILE_RECENTMODELS3:
		case IDC_FILE_RECENTMODELS4:
		{
			int i = event->action - IDC_FILE_RECENTMODELS1;
			d_cpl->loadModel (recentFiles[i]);

			char tmp[256];			
			strcpy (tmp, recentFiles[0]);
			strcpy (recentFiles[0], recentFiles[i]);
			strcpy (recentFiles[i], tmp);

			initRecentFiles ();

			redraw ();
		}
		break;

		case IDC_FILE_RECENTPAKFILES1:
		case IDC_FILE_RECENTPAKFILES2:
		case IDC_FILE_RECENTPAKFILES3:
		case IDC_FILE_RECENTPAKFILES4:
		{
			int i = event->action - IDC_FILE_RECENTPAKFILES1 + 4;
			d_PAKViewer->openPAKFile (recentFiles[i]);

			char tmp[256];			
			strcpy (tmp, recentFiles[4]);
			strcpy (recentFiles[4], recentFiles[i]);
			strcpy (recentFiles[i], tmp);

			initRecentFiles ();

			redraw ();
		}
		break;

		case IDC_FILE_EXIT:
		{
			d_PAKViewer->closePAKFile ();
			redraw ();
			mx::quit ();
		}
		break;

		case IDC_OPTIONS_COLORBACKGROUND:
		case IDC_OPTIONS_COLORGROUND:
		case IDC_OPTIONS_COLORLIGHT:
		{
			float *cols[3] = { g_viewerSettings.bgColor, g_viewerSettings.gColor, g_viewerSettings.lColor };
			float *col = cols[event->action - IDC_OPTIONS_COLORBACKGROUND];
			int r = (int) (col[0] * 255.0f);
			int g = (int) (col[1] * 255.0f);
			int b = (int) (col[2] * 255.0f);
			if (mxChooseColor (this, &r, &g, &b))
			{
				col[0] = (float) r / 255.0f;
				col[1] = (float) g / 255.0f;
				col[2] = (float) b / 255.0f;
			}
		}
		break;

		case IDC_OPTIONS_CENTERVIEW:
			d_cpl->centerView ();
			break;

		case IDC_OPTIONS_MAKESCREENSHOT:
		{
			char *ptr = (char *) mxGetSaveFileName (this, "", "*.tga");
			if (ptr)
			{
				if (!strstr (ptr, ".tga"))
					strcat (ptr, ".tga");
				d_GlWindow->dumpViewport (ptr);
			}
		}
		break;

		case IDC_OPTIONS_FILTERTEXTURES:
			bFilterTextures = !mb->isChecked (IDC_OPTIONS_FILTERTEXTURES);
			mb->setChecked (IDC_OPTIONS_FILTERTEXTURES, bFilterTextures);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, bFilterTextures ? GL_LINEAR:GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, bFilterTextures ? GL_LINEAR:GL_NEAREST);
			break;

		case IDC_OPTIONS_DUMP:
			d_cpl->dumpModelInfo ();
			break;

		case IDC_VIEW_FILEASSOCIATIONS:
			g_FileAssociation->setAssociation (0);
			g_FileAssociation->setVisible (true);
			break;

#ifdef WIN32
		case IDC_HELP_GOTOHOMEPAGE:
			ShellExecute (0, "open", "http://www.swissquake.ch/chumbalum-soft/index.html", 0, 0, SW_SHOW);
			break;
#endif

		case IDC_HELP_ABOUT:
			mxMessageBox (this,
				"Half-Life Model Viewer v1.25 (c) 2002 by Mete Ciragan\n\n"
				"Left-drag to rotate.\n"
				"Right-drag to zoom.\n"
				"Shift-left-drag to x-y-pan.\n\n"
				"Build:\t" __DATE__ ".\n"
				"Email:\tmete@swissquake.ch\n"
				"Web:\thttp://www.swissquake.ch/chumbalum-soft/", "About Half-Life Model Viewer",
				MX_MB_OK | MX_MB_INFORMATION);
			break;
		
		} //switch (event->action)

	} // mxEvent::Action
	break;

	case mxEvent::Size:
	{
		int w = event->width;
		int h = event->height;
		int y = mb->getHeight ();
#ifdef WIN32
#define HEIGHT 120
#else
#define HEIGHT 140
		h -= 40;
#endif

		if (d_PAKViewer->isVisible ())
		{
			w -= 170;
			d_PAKViewer->setBounds (w, y, 170, h);
		}

		d_GlWindow->setBounds (0, y, w, h - HEIGHT);
		d_cpl->setBounds (0, y + h - HEIGHT, w, HEIGHT);
	}
	break;
	} // event->event

	return 1;
}



void
MDLViewer::redraw ()
{
	mxEvent event;
	event.event = mxEvent::Size;
	event.width = w2 ();
	event.height = h2 ();
	handleEvent (&event);
}



int
main (int argc, char *argv[])
{
	//
	// make sure, we start in the right directory
	//
	char szName[256];

	strcpy (szName, mx::getApplicationPath ());
	mx_setcwd (szName);

	char cmdline[1024] = "";
	if (argc > 1)
	{
		strcpy (cmdline, argv[1]);
		for (int i = 2; i < argc; i++)
		{
			strcat (cmdline, " ");
			strcat (cmdline, argv[i]);
		}
	}

	if (!strcmp (cmdline, "-fullscreen"))
	{
#ifdef WIN32
		mxGlWindow::setFormat (mxGlWindow::FormatDouble, 32, 24);
		mx::init (argc, argv);

		if (!LoadViewerSettings ("hlmv.cfg"))
		{
			mxMessageBox (0, "Error loading configuration.", g_appTitle, MX_MB_OK | MX_MB_ERROR);
			return 0;
		}

		if (g_viewerSettings.cds)
			mx::setDisplayMode (g_viewerSettings.width, g_viewerSettings.height, 16);

		mxGlWindow::setFormat (mxGlWindow::FormatDouble, 16, 16);
		GlWindow *glw = new GlWindow (0, 0, 0, g_viewerSettings.width, g_viewerSettings.height, "", mxWindow::Popup);

		glw->loadTexture (g_viewerSettings.backgroundTexFile, 0);
		glw->loadTexture (g_viewerSettings.groundTexFile, 1);

		g_studioModel.FreeModel ();
		if (!g_studioModel.LoadModel (g_viewerSettings.modelFile))
		{
			if (g_viewerSettings.cds)
				mx::setDisplayMode (0, 0, 0);
			mxMessageBox (0, "Error loading model.", g_appTitle, MX_MB_OK | MX_MB_ERROR);
			exit (-1);
		}

		if (!g_studioModel.PostLoadModel (g_viewerSettings.modelFile))
		{
			if (g_viewerSettings.cds)
				mx::setDisplayMode (0, 0, 0);
			mxMessageBox (0, "Error post-loading model.", g_appTitle, MX_MB_OK | MX_MB_ERROR);
			exit (-1);
		}

		g_studioModel.SetSequence (g_viewerSettings.sequence);

		int i;
		studiohdr_t *hdr = g_studioModel.getStudioHeader ();
		for (i = 0; i < hdr->numbodyparts; i++)
			g_studioModel.SetBodygroup (i, g_viewerSettings.submodels[i]);

		for (i = 0; i < hdr->numbonecontrollers; i++)
		{
			mstudiobonecontroller_t *pbonecontrollers = (mstudiobonecontroller_t *) ((byte *) hdr + hdr->bonecontrollerindex);
			if (pbonecontrollers[i].index == 4)
				g_studioModel.SetMouth (g_viewerSettings.controllers[i]);
			else
				g_studioModel.SetController (pbonecontrollers[i].index, g_viewerSettings.controllers[i]);
		}

		g_studioModel.SetSkin (g_viewerSettings.skin);

		if (g_viewerSettings.speedScale == 0.0f)
			g_viewerSettings.speedScale = 1.0f;

		if (g_viewerSettings.use3dfx)
			g_viewerSettings.useStencil = false;

		mx::setIdleWindow (glw);
		int ret = mx::run ();

		g_studioModel.FreeModel ();

		if (g_viewerSettings.cds)
			mx::setDisplayMode (0, 0, 0);

		return ret;
#endif
	}

	//mx::setDisplayMode (0, 0, 0);
	mx::init (argc, argv);
	g_MDLViewer = new MDLViewer ();
	g_MDLViewer->setMenuBar (g_MDLViewer->getMenuBar ());
	g_MDLViewer->setBounds (20, 20, 640, 540);
	g_MDLViewer->setVisible (true);

	//LoadViewerSettings ("hlmv.cfg");

	if (strstr (cmdline, ".mdl"))
	{
		g_ControlPanel->loadModel (cmdline);
	}

	int ret = mx::run ();

	mx::cleanup ();

	return ret;
}
