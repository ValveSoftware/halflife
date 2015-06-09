//
//                 Half-Life Model Viewer (c) 1999 by Mete Ciragan
//
// file:           ViewerSettings.h
// last modified:  May 29 1999, Mete Ciragan
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
#ifndef INCLUDED_VIEWERSETTINGS
#define INCLUDED_VIEWERSETTINGS



enum // render modes
{
	RM_WIREFRAME,
	RM_FLATSHADED,
	RM_SMOOTHSHADED,
	RM_TEXTURED
};



typedef struct
{
	// model 
	float rot[3];
	float trans[3];

	// render
	int renderMode;
	float transparency;
	bool showBackground;
	bool showGround;
	bool showHitBoxes;
	bool showBones;
	bool showTexture;
	bool showAttachments;
	int texture;
	float textureScale;
	int skin;
	bool mirror;
	bool useStencil;	// if 3dfx fullscreen set false

	// animation
	int sequence;
	float speedScale;

	// bodyparts and bonecontrollers
	int submodels[32];
	float controllers[8];

	// fullscreen
	int width, height;
	bool use3dfx;
	bool cds;

	// colors
	float bgColor[4];
	float lColor[4];
	float gColor[4];

	// misc
	int textureLimit;
	bool pause;

	// only used for fullscreen mode
	char modelFile[256];
	char backgroundTexFile[256];
	char groundTexFile[256];
} ViewerSettings;



extern ViewerSettings g_viewerSettings;



#ifdef __cplusplus
extern "C" {
#endif

void InitViewerSettings (void);
int LoadViewerSettings (const char *filename);
int SaveViewerSettings (const char *filename);

#ifdef __cplusplus
}
#endif



#endif // INCLUDED_VIEWERSETTINGS