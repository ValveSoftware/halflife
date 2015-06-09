//
//                 Half-Life Model Viewer (c) 1999 by Mete Ciragan
//
// file:           ControlPanel.cpp
// last modified:  Oct 20 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
// version:        1.24
//
// email:          mete@swissquake.ch
// web:            http://www.swissquake.ch/chumbalum-soft/
//
#include "ControlPanel.h"
#include "ViewerSettings.h"
#include "StudioModel.h"
#include "GlWindow.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mx/mx.h>
#include <mx/mxBmp.h>



extern char g_appTitle[];



bool swap3dfxgl (bool b);



ControlPanel *g_ControlPanel = 0;
bool g_bStopPlaying = false;
static int g_nCurrFrame = 0;



ControlPanel::ControlPanel (mxWindow *parent)
: mxWindow (parent, 0, 0, 0, 0, "Control Panel", mxWindow::Normal)
{
	InitViewerSettings ();

	// create tabcontrol with subdialog windows
	tab = new mxTab (this, 0, 0, 0, 0, IDC_TAB);
#ifdef WIN32
	SetWindowLong ((HWND) tab->getHandle (), GWL_EXSTYLE, WS_EX_CLIENTEDGE);
#endif

	mxWindow *wRender = new mxWindow (this, 0, 0, 0, 0);
	tab->add (wRender, "Render");
	cRenderMode = new mxChoice (wRender, 5, 5, 100, 22, IDC_RENDERMODE);
	cRenderMode->add ("Wireframe");
	cRenderMode->add ("Flatshaded");
	cRenderMode->add ("Smoothshaded");
	cRenderMode->add ("Textured");
	cRenderMode->select (3);
	mxToolTip::add (cRenderMode, "Select Render Mode");
	slTransparency = new mxSlider (wRender, 5, 28, 100, 18, IDC_TRANSPARENCY);
	slTransparency->setValue (100);
	mxToolTip::add (slTransparency, "Model Transparency");
	cbGround = new mxCheckBox (wRender, 110, 5, 150, 20, "Ground", IDC_GROUND);
	cbMirror = new mxCheckBox (wRender, 110, 25, 150, 20, "Mirror Model On Ground", IDC_MIRROR);
	cbBackground = new mxCheckBox (wRender, 110, 45, 150, 20, "Background", IDC_BACKGROUND);
	mxCheckBox *cbHitBoxes = new mxCheckBox (wRender, 110, 65, 150, 20, "Hit Boxes", IDC_HITBOXES);
	mxCheckBox *cbBones = new mxCheckBox (wRender, 5, 65, 100, 20, "Bones", IDC_BONES);
	mxCheckBox *cbAttachments = new mxCheckBox (wRender, 5, 45, 100, 20, "Attachments", IDC_ATTACHMENTS);

#ifdef HAVE_SCALE
	leMeshScale = new mxLineEdit (wRender, 270, 5, 50, 18, "1.0");
	mxToolTip::add (leMeshScale, "Mesh Scale");
	leBoneScale = new mxLineEdit (wRender, 270, 25, 50, 18, "1.0");
	mxToolTip::add (leBoneScale, "Bone Scale");
	mxButton *bMeshScale = new mxButton (wRender, 325, 5, 50, 18, "Scale", 10001);
	mxButton *bBoneScale = new mxButton (wRender, 325, 25, 50, 18, "Scale", 10002);
#endif

	mxWindow *wSequence = new mxWindow (this, 0, 0, 0, 0);
	tab->add (wSequence, "Sequence");
	cSequence = new mxChoice (wSequence, 5, 5, 200, 22, IDC_SEQUENCE);	
	mxToolTip::add (cSequence, "Select Sequence");
	slSpeedScale = new mxSlider (wSequence, 5, 32, 200, 18, IDC_SPEEDSCALE);
	slSpeedScale->setRange (0, 200);
	slSpeedScale->setValue (40);
	mxToolTip::add (slSpeedScale, "Speed Scale");
	tbStop = new mxToggleButton (wSequence, 5, 55, 60, 22, "Stop", IDC_STOP);
	mxToolTip::add (tbStop, "Stop Playing");
	bPrevFrame = new mxButton (wSequence, 70, 55, 30, 22, "<<", IDC_PREVFRAME);
	bPrevFrame->setEnabled (false);
	mxToolTip::add (bPrevFrame, "Prev Frame");
	leFrame = new mxLineEdit (wSequence, 105, 55, 50, 22, "", IDC_FRAME); 
	leFrame->setEnabled (false);
	mxToolTip::add (leFrame, "Set Frame");
	bNextFrame = new mxButton (wSequence, 160, 55, 30, 22, ">>", IDC_NEXTFRAME);
	bNextFrame->setEnabled (false);
	mxToolTip::add (bNextFrame, "Next Frame");


	mxWindow *wBody = new mxWindow (this, 0, 0, 0, 0);
	tab->add (wBody, "Body");
	cBodypart = new mxChoice (wBody, 5, 5, 100, 22, IDC_BODYPART);
	mxToolTip::add (cBodypart, "Choose a bodypart");
	cSubmodel = new mxChoice (wBody, 110, 5, 100, 22, IDC_SUBMODEL);
	mxToolTip::add (cSubmodel, "Choose a submodel of current bodypart");
	cController = new mxChoice (wBody, 5, 30, 100, 22, IDC_CONTROLLER);	
	mxToolTip::add (cController, "Choose a bone controller");
	slController = new mxSlider (wBody, 105, 32, 100, 18, IDC_CONTROLLERVALUE);
	slController->setRange (0, 45);
	mxToolTip::add (slController, "Change current bone controller value");
	lModelInfo1 = new mxLabel (wBody, 220, 5, 120, 100, "No Model.");
	lModelInfo2 = new mxLabel (wBody, 340, 5, 120, 100, "");
	cSkin = new mxChoice (wBody, 5, 55, 100, 22, IDC_SKINS);
	mxToolTip::add (cSkin, "Choose a skin family");

	mxWindow *wTexture = new mxWindow (this, 0, 0, 0, 0);
	tab->add (wTexture, "Texture");
	cTextures = new mxChoice (wTexture, 5, 5, 150, 22, IDC_TEXTURES);
	mxToolTip::add (cTextures, "Choose a texture");
	
	new mxButton (wTexture, 160, 5, 75, 18, "Export", IDC_EXPORTTEXTURE);
	new mxButton (wTexture, 160, 25, 75, 18, "Import", IDC_IMPORTTEXTURE);
	new mxButton (wTexture, 160, 45, 75, 18, "Save Model", IDC_SAVEMODEL);
	lTexSize = new mxLabel (wTexture, 162, 70, 150, 18, "Width x Height");
	cbChrome = new mxCheckBox (wTexture, 5, 30, 150, 22, "Chrome Effect", IDC_CHROME);
	mxToolTip::add (new mxSlider (wTexture, 5, 57, 150, 18, IDC_TEXTURESCALE), "Scale texture size");
	
#ifdef WIN32
	mxWindow *wFullscreen = new mxWindow (this, 0, 0, 0, 0);
	tab->add (wFullscreen, "Fullscreen");

	// Create widgets for the Fullscreen Tab
	mxLabel *lResolution = new mxLabel (wFullscreen, 5, 7, 50, 18, "Resolution");
	leWidth = new mxLineEdit (wFullscreen, 5, 5, 50, 22, "800");
	mxLabel *lX = new mxLabel (wFullscreen, 65, 7, 22, 22, "x");
	leHeight = new mxLineEdit (wFullscreen, 82, 5, 50, 22, "600");
	//cb3dfxOpenGL = new mxCheckBox (wFullscreen, 5, 30, 130, 22, "3Dfx OpenGL");
	mxButton *bView = new mxButton (wFullscreen, 140, 5, 75, 22, "Fullscreen!", IDC_FULLSCREEN);
#endif

	g_ControlPanel = this;
}



ControlPanel::~ControlPanel ()
{
}



int
ControlPanel::handleEvent (mxEvent *event)
{
	static char str[128];

	if (event->event == mxEvent::Size)
	{
		tab->setBounds (0, 0, event->width, event->height);
		return 1;
	}

	switch (event->action)
	{
		case IDC_TAB:
		{
			g_viewerSettings.showTexture = (tab->getSelectedIndex () == 3);
		}
		break;

		case IDC_RENDERMODE:
		{
			int index = cRenderMode->getSelectedIndex ();
			if (index >= 0)
			{
				setRenderMode (index);
			}
		}
		break;

		case IDC_TRANSPARENCY:
		{
			int value = slTransparency->getValue ();
			g_viewerSettings.transparency = (float) value / 100.0f; 
		}
		break;

		case IDC_GROUND:
			setShowGround (((mxCheckBox *) event->widget)->isChecked ());
			break;

		case IDC_MIRROR:
			setMirror (((mxCheckBox *) event->widget)->isChecked ());
			break;

		case IDC_BACKGROUND:
			setShowBackground (((mxCheckBox *) event->widget)->isChecked ());
			break;

		case IDC_HITBOXES:
			g_viewerSettings.showHitBoxes = ((mxCheckBox *) event->widget)->isChecked ();
			break;

		case IDC_BONES:
			g_viewerSettings.showBones = ((mxCheckBox *) event->widget)->isChecked ();
			break;

		case IDC_ATTACHMENTS:
			g_viewerSettings.showAttachments = ((mxCheckBox *) event->widget)->isChecked ();
			break;

		case IDC_SEQUENCE:
		{
			int index = cSequence->getSelectedIndex ();
			if (index >= 0)
			{
				setSequence (index);
			}
		}
		break;

		case IDC_SPEEDSCALE:
		{
			int v = ((mxSlider *) event->widget)->getValue ();
			g_viewerSettings.speedScale = (float) (v * 5) / 200.0f;
		}
		break;

		case IDC_STOP:
		{
			if (tbStop->isChecked ())
			{
				tbStop->setLabel ("Play");
				g_bStopPlaying = true;
				g_nCurrFrame = g_studioModel.SetFrame (-1);
				sprintf (str, "%d", g_nCurrFrame);
				leFrame->setLabel (str);
				bPrevFrame->setEnabled (true);
				leFrame->setEnabled (true);
				bNextFrame->setEnabled (true);
			}
			else
			{
				tbStop->setLabel ("Stop");
				g_bStopPlaying = false;
				bPrevFrame->setEnabled (false);
				leFrame->setEnabled (false);
				bNextFrame->setEnabled (false);
			}
		}
		break;

		case IDC_PREVFRAME:
		{
			g_nCurrFrame = g_studioModel.SetFrame (g_nCurrFrame - 1);
			sprintf (str, "%d", g_nCurrFrame);
			leFrame->setLabel (str);
		}
		break;

		case IDC_FRAME:
		{
			g_nCurrFrame = atoi (leFrame->getLabel ());
			g_nCurrFrame = g_studioModel.SetFrame (g_nCurrFrame);
		}
		break;

		case IDC_NEXTFRAME:
		{
			g_nCurrFrame = g_studioModel.SetFrame (g_nCurrFrame + 1);
			sprintf (str, "%d", g_nCurrFrame);
			leFrame->setLabel (str);
		}
		break;

		case IDC_BODYPART:
		{
			int index = cBodypart->getSelectedIndex ();
			if (index >= 0)
			{
				setBodypart (index);

			}
		}
		break;

		case IDC_SUBMODEL:
		{
			int index = cSubmodel->getSelectedIndex ();
			if (index >= 0)
			{
				setSubmodel (index);

			}
		}
		break;

		case IDC_CONTROLLER:
		{
			int index = cController->getSelectedIndex ();
			if (index >= 0)
				setBoneController (index);
		}
		break;

		case IDC_CONTROLLERVALUE:
		{
			int index = cController->getSelectedIndex ();
			if (index >= 0)
				setBoneControllerValue (index, (float) slController->getValue ());
		}
		break;

		case IDC_SKINS:
		{
			int index = cSkin->getSelectedIndex ();
			if (index >= 0)
			{
				g_studioModel.SetSkin (index);
				g_viewerSettings.skin = index;
				d_GlWindow->redraw ();
			}
		}
		break;

		case IDC_TEXTURES:
		{
			int index = cTextures->getSelectedIndex ();
			if (index >= 0)
			{
				g_viewerSettings.texture = index;
				studiohdr_t *hdr = g_studioModel.getTextureHeader ();
				if (hdr)
				{
					mstudiotexture_t *ptexture = (mstudiotexture_t *) ((byte *) hdr + hdr->textureindex) + index;
					char str[32];
					sprintf (str, "Size: %d x %d", ptexture->width, ptexture->height);
					lTexSize->setLabel (str);
					cbChrome->setChecked ((ptexture->flags & STUDIO_NF_CHROME) == STUDIO_NF_CHROME);
				}
				d_GlWindow->redraw ();
			}
		}
		break;

		case IDC_CHROME:
		{
			studiohdr_t *hdr = g_studioModel.getTextureHeader ();
			if (hdr)
			{
				mstudiotexture_t *ptexture = (mstudiotexture_t *) ((byte *) hdr + hdr->textureindex) + g_viewerSettings.texture;
				if (cbChrome->isChecked ())
					ptexture->flags |= STUDIO_NF_CHROME;
				else
					ptexture->flags &= ~STUDIO_NF_CHROME;
			}
		}
		break;

		case IDC_EXPORTTEXTURE:
		{
			char *ptr = (char *) mxGetSaveFileName (this, "", "*.bmp");
			if (!ptr)
				break;

			char filename[256];
			char ext[16];

			strcpy (filename, ptr);
			strcpy (ext, mx_getextension (filename));
			if (mx_strcasecmp (ext, ".bmp"))
				strcat (filename, ".bmp");

			studiohdr_t *phdr = g_studioModel.getTextureHeader ();
			if (phdr)
			{
				mxImage image;
				mstudiotexture_t *ptexture = (mstudiotexture_t *) ((byte *) phdr + phdr->textureindex) + g_viewerSettings.texture;
				image.width = ptexture->width;
				image.height = ptexture->height;
				image.bpp = 8;
				image.data = (void *) ((byte *) phdr + ptexture->index);
				image.palette = (void *) ((byte *) phdr + ptexture->width * ptexture->height + ptexture->index);
				if (!mxBmpWrite (filename, &image))
					mxMessageBox (this, "Error writing .BMP texture.", g_appTitle, MX_MB_OK | MX_MB_ERROR);
				image.data = 0;
				image.palette = 0;
			}
		}
		break;

		case IDC_IMPORTTEXTURE:
		{
			char *ptr = (char *) mxGetOpenFileName (this, "", "*.bmp");
			if (!ptr)
				break;

			char filename[256];
			char ext[16];

			strcpy (filename, ptr);
			strcpy (ext, mx_getextension (filename));
			if (mx_strcasecmp (ext, ".bmp"))
				strcat (filename, ".bmp");

			mxImage *image = mxBmpRead (filename);
			if (!image)
			{
				mxMessageBox (this, "Error loading .BMP texture.", g_appTitle, MX_MB_OK | MX_MB_ERROR);
				return 1;
			}

			if (!image->palette)
			{
				delete image;
				mxMessageBox (this, "Error loading .BMP texture.  Must be 8-bit!", g_appTitle, MX_MB_OK | MX_MB_ERROR);
				return 1;
			}

			studiohdr_t *phdr = g_studioModel.getTextureHeader ();
			if (phdr)
			{
				mstudiotexture_t *ptexture = (mstudiotexture_t *) ((byte *) phdr + phdr->textureindex) + g_viewerSettings.texture;
				if (image->width == ptexture->width && image->height == ptexture->height)
				{
					memcpy ((byte *) phdr + ptexture->index, image->data, image->width * image->height);
					memcpy ((byte *) phdr + ptexture->index + image->width * image->height, image->palette, 768);

					g_studioModel.UploadTexture (ptexture, (byte *) phdr + ptexture->index, (byte *) phdr + ptexture->index + image->width * image->height, g_viewerSettings.texture + 3);
				}
				else
					mxMessageBox (this, "Texture must be of same size.", g_appTitle, MX_MB_OK | MX_MB_ERROR);
			}

			delete image;
			d_GlWindow->redraw ();
		}
		break;

		case IDC_SAVEMODEL:
		{
			char *ptr = (char *) mxGetSaveFileName (this, "", "*.mdl");
			if (!ptr)
				break;

			char filename[256];
			char ext[16];

			strcpy (filename, ptr);
			strcpy (ext, mx_getextension (filename));
			if (mx_strcasecmp (ext, ".mdl"))
				strcat (filename, ".mdl");

			if (!g_studioModel.SaveModel (filename))
				mxMessageBox (this, "Error saving model.", g_appTitle, MX_MB_OK | MX_MB_ERROR);
			else
				strcpy (g_viewerSettings.modelFile, filename);
		}
		break;

		case IDC_TEXTURESCALE:
		{
			g_viewerSettings.textureScale =  1.0f + (float) ((mxSlider *) event->widget)->getValue () * 4.0f / 100.0f;
			d_GlWindow->redraw ();
		}
		break;

#ifdef HAVE_SCALE
		case 10001:
		{
			float scale = (float) atof (leMeshScale->getLabel ());
			if (scale > 0.0f)
			{
				g_studioModel.scaleMeshes (scale);
			}
		}
		break;

		case 10002:
		{
			float scale = (float) atof (leBoneScale->getLabel ());
			if (scale > 0.0f)
			{
				g_studioModel.scaleBones (scale);
			}
		}
		break;
#endif

#ifdef WIN32
		case IDC_FULLSCREEN:
			fullscreen ();
			break;
#endif
	}

	return 1;
}



void
ControlPanel::dumpModelInfo ()
{
#if 0
	studiohdr_t *hdr = g_studioModel.getStudioHeader ();
	if (hdr)
	{
		DeleteFile ("midump.txt");
		FILE *file = fopen ("midump.txt", "wt");
		if (file)
		{
			byte *phdr = (byte *) hdr;
			int i;

			fprintf (file, "id: %c%c%c%c\n", phdr[0], phdr[1], phdr[2], phdr[3]);
			fprintf (file, "version: %d\n", hdr->version);
			fprintf (file, "name: \"%s\"\n", hdr->name);
			fprintf (file, "length: %d\n\n", hdr->length);

			fprintf (file, "eyeposition: %f %f %f\n", hdr->eyeposition[0], hdr->eyeposition[1], hdr->eyeposition[2]);
			fprintf (file, "min: %f %f %f\n", hdr->min[0], hdr->min[1], hdr->min[2]);
			fprintf (file, "max: %f %f %f\n", hdr->max[0], hdr->max[1], hdr->max[2]);
			fprintf (file, "bbmin: %f %f %f\n", hdr->bbmin[0], hdr->bbmin[1], hdr->bbmin[2]);
			fprintf (file, "bbmax: %f %f %f\n", hdr->bbmax[0], hdr->bbmax[1], hdr->bbmax[2]);
			
			fprintf (file, "flags: %d\n\n", hdr->flags);

			fprintf (file, "numbones: %d\n", hdr->numbones);
			for (i = 0; i < hdr->numbones; i++)
			{
				mstudiobone_t *pbones = (mstudiobone_t *) (phdr + hdr->boneindex);
				fprintf (file, "\nbone %d.name: \"%s\"\n", i + 1, pbones[i].name);
				fprintf (file, "bone %d.parent: %d\n", i + 1, pbones[i].parent);
				fprintf (file, "bone %d.flags: %d\n", i + 1, pbones[i].flags);
				fprintf (file, "bone %d.bonecontroller: %d %d %d %d %d %d\n", i + 1, pbones[i].bonecontroller[0], pbones[i].bonecontroller[1], pbones[i].bonecontroller[2], pbones[i].bonecontroller[3], pbones[i].bonecontroller[4], pbones[i].bonecontroller[5]);
				fprintf (file, "bone %d.value: %f %f %f %f %f %f\n", i + 1, pbones[i].value[0], pbones[i].value[1], pbones[i].value[2], pbones[i].value[3], pbones[i].value[4], pbones[i].value[5]);
				fprintf (file, "bone %d.scale: %f %f %f %f %f %f\n", i + 1, pbones[i].scale[0], pbones[i].scale[1], pbones[i].scale[2], pbones[i].scale[3], pbones[i].scale[4], pbones[i].scale[5]);
			}

			fprintf (file, "\nnumbonecontrollers: %d\n", hdr->numbonecontrollers);
			for (i = 0; i < hdr->numbonecontrollers; i++)
			{
				mstudiobonecontroller_t *pbonecontrollers = (mstudiobonecontroller_t *) (phdr + hdr->bonecontrollerindex);
				fprintf (file, "\nbonecontroller %d.bone: %d\n", i + 1, pbonecontrollers[i].bone);
				fprintf (file, "bonecontroller %d.type: %d\n", i + 1, pbonecontrollers[i].type);
				fprintf (file, "bonecontroller %d.start: %f\n", i + 1, pbonecontrollers[i].start);
				fprintf (file, "bonecontroller %d.end: %f\n", i + 1, pbonecontrollers[i].end);
				fprintf (file, "bonecontroller %d.rest: %d\n", i + 1, pbonecontrollers[i].rest);
				fprintf (file, "bonecontroller %d.index: %d\n", i + 1, pbonecontrollers[i].index);
			}

			fprintf (file, "\nnumhitboxes: %d\n", hdr->numhitboxes);
			for (i = 0; i < hdr->numhitboxes; i++)
			{
				mstudiobbox_t *pbboxes = (mstudiobbox_t *) (phdr + hdr->hitboxindex);
				fprintf (file, "\nhitbox %d.bone: %d\n", i + 1, pbboxes[i].bone);
				fprintf (file, "hitbox %d.group: %d\n", i + 1, pbboxes[i].group);
				fprintf (file, "hitbox %d.bbmin: %f %f %f\n", i + 1, pbboxes[i].bbmin[0], pbboxes[i].bbmin[1], pbboxes[i].bbmin[2]);
				fprintf (file, "hitbox %d.bbmax: %f %f %f\n", i + 1, pbboxes[i].bbmax[0], pbboxes[i].bbmax[1], pbboxes[i].bbmax[2]);
			}

			fprintf (file, "\nnumseq: %d\n", hdr->numseq);
			for (i = 0; i < hdr->numseq; i++)
			{
				mstudioseqdesc_t *pseqdescs = (mstudioseqdesc_t *) (phdr + hdr->seqindex);
				fprintf (file, "\nseqdesc %d.label: \"%s\"\n", i + 1, pseqdescs[i].label);
				fprintf (file, "seqdesc %d.fps: %f\n", i + 1, pseqdescs[i].fps);
				fprintf (file, "seqdesc %d.flags: %d\n", i + 1, pseqdescs[i].flags);
				fprintf (file, "<...>\n");
			}
/*
			fprintf (file, "\nnumseqgroups: %d\n", hdr->numseqgroups);
			for (i = 0; i < hdr->numseqgroups; i++)
			{
				mstudioseqgroup_t *pseqgroups = (mstudioseqgroup_t *) (phdr + hdr->seqgroupindex);
				fprintf (file, "\nseqgroup %d.label: \"%s\"\n", i + 1, pseqgroups[i].label);
				fprintf (file, "\nseqgroup %d.namel: \"%s\"\n", i + 1, pseqgroups[i].name);
				fprintf (file, "\nseqgroup %d.data: %d\n", i + 1, pseqgroups[i].data);
			}
*/
			hdr = g_studioModel.getTextureHeader ();
			fprintf (file, "\nnumtextures: %d\n", hdr->numtextures);
			fprintf (file, "textureindex: %d\n", hdr->textureindex);
			fprintf (file, "texturedataindex: %d\n", hdr->texturedataindex);
			for (i = 0; i < hdr->numtextures; i++)
			{
				mstudiotexture_t *ptextures = (mstudiotexture_t *) ((byte *) hdr + hdr->textureindex);
				fprintf (file, "\ntexture %d.name: \"%s\"\n", i + 1, ptextures[i].name);
				fprintf (file, "texture %d.flags: %d\n", i + 1, ptextures[i].flags);
				fprintf (file, "texture %d.width: %d\n", i + 1, ptextures[i].width);
				fprintf (file, "texture %d.height: %d\n", i + 1, ptextures[i].height);
				fprintf (file, "texture %d.index: %d\n", i + 1, ptextures[i].index);
			}

			hdr = g_studioModel.getStudioHeader ();
			fprintf (file, "\nnumskinref: %d\n", hdr->numskinref);
			fprintf (file, "numskinfamilies: %d\n", hdr->numskinfamilies);

			fprintf (file, "\nnumbodyparts: %d\n", hdr->numbodyparts);
			for (i = 0; i < hdr->numbodyparts; i++)
			{
				mstudiobodyparts_t *pbodyparts = (mstudiobodyparts_t *) ((byte *) hdr + hdr->bodypartindex);
				fprintf (file, "\nbodypart %d.name: \"%s\"\n", i + 1, pbodyparts[i].name);
				fprintf (file, "bodypart %d.nummodels: %d\n", i + 1, pbodyparts[i].nummodels);
				fprintf (file, "bodypart %d.base: %d\n", i + 1, pbodyparts[i].base);
				fprintf (file, "bodypart %d.modelindex: %d\n", i + 1, pbodyparts[i].modelindex);
			}

			fprintf (file, "\nnumattachments: %d\n", hdr->numattachments);
			for (i = 0; i < hdr->numattachments; i++)
			{
				mstudioattachment_t *pattachments = (mstudioattachment_t *) ((byte *) hdr + hdr->attachmentindex);
				fprintf (file, "attachment %d.name: \"%s\"\n", i + 1, pattachments[i].name);
			}

			fclose (file);

			ShellExecute ((HWND) getHandle (), "open", "midump.txt", 0, 0, SW_SHOW);
		}
	}
#endif
}



void
ControlPanel::loadModel (const char *filename)
{
	g_studioModel.FreeModel ();
	if (g_studioModel.LoadModel ((char *) filename))
	{
		if (g_studioModel.PostLoadModel ((char *) filename))
		{
			initSequences ();
			initBodyparts ();
			initBoneControllers ();
			initSkins ();
			initTextures ();
			centerView ();
			strcpy (g_viewerSettings.modelFile, filename);
			setModelInfo ();
			g_viewerSettings.sequence = 0;
			g_viewerSettings.speedScale = 1.0f;
			slSpeedScale->setValue (40);
			int i;
			for (i = 0; i < 32; i++)
				g_viewerSettings.submodels[i] = 0;
			for (i = 0; i < 8; i++)
				g_viewerSettings.controllers[i] = 0;

			mx_setcwd (mx_getpath (filename));
		}
		else
			mxMessageBox (this, "Error post-loading model.", g_appTitle, MX_MB_ERROR | MX_MB_OK);
	}
	else
		mxMessageBox (this, "Error loading model.", g_appTitle, MX_MB_ERROR | MX_MB_OK);
}



void
ControlPanel::setRenderMode (int mode)
{
	g_viewerSettings.renderMode = mode;
	d_GlWindow->redraw ();
}



void
ControlPanel::setShowGround (bool b)
{
	g_viewerSettings.showGround = b;
	cbGround->setChecked (b);
	if (!b)
	{
		cbMirror->setChecked (b);
		g_viewerSettings.mirror = b;
	}
}



void
ControlPanel::setMirror (bool b)
{
	g_viewerSettings.useStencil = (!g_viewerSettings.use3dfx && b);
	g_viewerSettings.mirror = b;
	cbMirror->setChecked (b);
	if (b)
	{
		cbGround->setChecked (b);
		g_viewerSettings.showGround = b;
	}
}



void
ControlPanel::setShowBackground (bool b)
{
	g_viewerSettings.showBackground = b;
	cbBackground->setChecked (b);
}



void
ControlPanel::initSequences ()
{
	studiohdr_t *hdr = g_studioModel.getStudioHeader ();
	if (hdr)
	{
		cSequence->removeAll ();
		for (int i = 0; i < hdr->numseq; i++)
		{
			mstudioseqdesc_t *pseqdescs = (mstudioseqdesc_t *) ((byte *) hdr + hdr->seqindex);
			cSequence->add (pseqdescs[i].label);
		}

		cSequence->select (0);
	}
}



void
ControlPanel::setSequence (int index)
{
	cSequence->select (index);
	g_studioModel.SetSequence(index);
	g_viewerSettings.sequence = index;
}



void
ControlPanel::initBodyparts ()
{
	studiohdr_t *hdr = g_studioModel.getStudioHeader ();
	if (hdr)
	{
		int i;
		mstudiobodyparts_t *pbodyparts = (mstudiobodyparts_t *) ((byte *) hdr + hdr->bodypartindex);

		cBodypart->removeAll ();
		if (hdr->numbodyparts > 0)
		{
			for (i = 0; i < hdr->numbodyparts; i++)
				cBodypart->add (pbodyparts[i].name);

			cBodypart->select (0);

			cSubmodel->removeAll ();
			for (i = 0; i < pbodyparts[0].nummodels; i++)
			{
				char str[64];
				sprintf (str, "Submodel %d", i + 1);
				cSubmodel->add (str);
			}
			cSubmodel->select (0);
		}
	}
}



void
ControlPanel::setBodypart (int index)
{
	studiohdr_t *hdr = g_studioModel.getStudioHeader ();
	if (hdr)
	{
		//cBodypart->setEn
		cBodypart->select (index);
		if (index < hdr->numbodyparts)
		{
			mstudiobodyparts_t *pbodyparts = (mstudiobodyparts_t *) ((byte *) hdr + hdr->bodypartindex);
			cSubmodel->removeAll ();
		
			for (int i = 0; i < pbodyparts[index].nummodels; i++)
			{
				char str[64];
				sprintf (str, "Submodel %d", i + 1);
				cSubmodel->add (str);
			}
			cSubmodel->select (0);
			//g_studioModel.SetBodygroup (index, 0);
		}
	}
}



void
ControlPanel::setSubmodel (int index)
{
	g_studioModel.SetBodygroup (cBodypart->getSelectedIndex (), index);
	g_viewerSettings.submodels[cBodypart->getSelectedIndex ()] = index;
}



void
ControlPanel::initBoneControllers ()
{
	studiohdr_t *hdr = g_studioModel.getStudioHeader ();
	if (hdr)
	{
		cController->setEnabled (hdr->numbonecontrollers > 0);
		slController->setEnabled (hdr->numbonecontrollers > 0);
		cController->removeAll ();

		mstudiobonecontroller_t *pbonecontrollers = (mstudiobonecontroller_t *) ((byte *) hdr + hdr->bonecontrollerindex);
		for (int i = 0; i < hdr->numbonecontrollers; i++)
		{
			char str[32];
			if (pbonecontrollers[i].index == 4)
				sprintf (str, "Mouth");
			else
				sprintf (str, "Controller %d", pbonecontrollers[i].index);
			cController->add (str);
		}

		if (hdr->numbonecontrollers > 0)
		{
			cController->select (0);
			slController->setRange ((int) pbonecontrollers[0].start, (int) pbonecontrollers[0].end);
			slController->setValue (0);
		}

	}
}



void
ControlPanel::setBoneController (int index)
{
	studiohdr_t *hdr = g_studioModel.getStudioHeader ();
	if (hdr)
	{
		mstudiobonecontroller_t *pbonecontrollers = (mstudiobonecontroller_t *) ((byte *) hdr + hdr->bonecontrollerindex);
		slController->setRange ((int) pbonecontrollers[index].start, (int) pbonecontrollers[index].end);
		slController->setValue (0);
	}
}



void
ControlPanel::setBoneControllerValue (int index, float value)
{
	studiohdr_t *hdr = g_studioModel.getStudioHeader ();
	if (hdr)
	{
		mstudiobonecontroller_t *pbonecontrollers = (mstudiobonecontroller_t *) ((byte *) hdr + hdr->bonecontrollerindex);
		if (pbonecontrollers[index].index == 4)
			g_studioModel.SetMouth (value);
		else
			g_studioModel.SetController (pbonecontrollers[index].index, value);
	
		g_viewerSettings.controllers[index] = value;
	}
}



void
ControlPanel::initSkins ()
{
	studiohdr_t *hdr = g_studioModel.getStudioHeader ();
	if (hdr)
	{
		cSkin->setEnabled (hdr->numskinfamilies > 0);
		cSkin->removeAll ();

		for (int i = 0; i < hdr->numskinfamilies; i++)
		{
			char str[32];
			sprintf (str, "Skin %d", i + 1);
			cSkin->add (str);
		}

		cSkin->select (0);
		g_studioModel.SetSkin (0);
		g_viewerSettings.skin = 0;
	}
}



void
ControlPanel::setModelInfo ()
{
	static char str[2048];
	studiohdr_t *hdr = g_studioModel.getStudioHeader ();

	if (!hdr)
		return;

	sprintf (str,
		"Bones: %d\n"
		"Bone Controllers: %d\n"
		"Hit Boxes: %d\n"
		"Sequences: %d\n"
		"Sequence Groups: %d\n",
		hdr->numbones,
		hdr->numbonecontrollers,
		hdr->numhitboxes,
		hdr->numseq,
		hdr->numseqgroups
		);

	lModelInfo1->setLabel (str);

	sprintf (str,
		"Textures: %d\n"
		"Skin Families: %d\n"
		"Bodyparts: %d\n"
		"Attachments: %d\n"
		"Transitions: %d\n",
		hdr->numtextures,
		hdr->numskinfamilies,
		hdr->numbodyparts,
		hdr->numattachments,
		hdr->numtransitions);

	lModelInfo2->setLabel (str);
}



void
ControlPanel::initTextures ()
{
	studiohdr_t *hdr = g_studioModel.getTextureHeader ();
	if (hdr)
	{
		cTextures->removeAll ();
		mstudiotexture_t *ptextures = (mstudiotexture_t *) ((byte *) hdr + hdr->textureindex);
		for (int i = 0; i < hdr->numtextures; i++)
			cTextures->add (ptextures[i].name);
		cTextures->select (0);
		g_viewerSettings.texture = 0;
		if (hdr->numtextures > 0)
			cbChrome->setChecked ((ptextures[0].flags & STUDIO_NF_CHROME) == STUDIO_NF_CHROME);
	}
}



void
ControlPanel::centerView ()
{
	float min[3], max[3];
	g_studioModel.ExtractBbox (min, max);

	float dx = max[0] - min[0];
	float dy = max[1] - min[1];
	float dz = max[2] - min[2];
	float d = dx;
	if (dy > d)
		d = dy;
	if (dz > d)
		d = dz;
	g_viewerSettings.trans[0] = 0;
	g_viewerSettings.trans[1] = min[2] + dz / 2;
	g_viewerSettings.trans[2] = d * 1.0f;
	g_viewerSettings.rot[0] = -90.0f;
	g_viewerSettings.rot[1] = -90.0f;
	g_viewerSettings.rot[2] = 0.0f;
	d_GlWindow->redraw ();
}



#ifdef WIN32
void
ControlPanel::fullscreen ()
{
	//g_viewerSettings.use3dfx = cb3dfxOpenGL->isChecked ();
	swap3dfxgl (g_viewerSettings.use3dfx);

	char szName[256];

	GetModuleFileName (NULL, szName, 256);
	char *ptr = strrchr (szName, '\\');
	*ptr = '\0';
	SetCurrentDirectory (szName);

	g_viewerSettings.width = atoi (leWidth->getLabel ());
	g_viewerSettings.height = atoi (leHeight->getLabel ());
	g_viewerSettings.cds = true;
	//g_viewerSettings.use3dfx = cb3dfxOpenGL->isChecked ();

	if (SaveViewerSettings ("hlmv.cfg"))
	{
		g_viewerSettings.pause = true;
		g_viewerSettings.use3dfx = false;
		WinExec ("hlmv.exe -fullscreen", SW_SHOW);
	}
}
#endif
