//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include <VGUI_Font.h>


// handle to an individual scheme
typedef int SchemeHandle_t;


// Register console variables, etc..
void Scheme_Init();


//-----------------------------------------------------------------------------
// Purpose: Handles the loading of text scheme description from disk
//			supports different font/color/size schemes at different resolutions 
//-----------------------------------------------------------------------------
class CSchemeManager
{
public:
	// initialization
	CSchemeManager( int xRes, int yRes );
	virtual ~CSchemeManager();

	// scheme handling
	SchemeHandle_t getSchemeHandle( const char *schemeName );

	// getting info from schemes
	vgui::Font *getFont( SchemeHandle_t schemeHandle );
	void getFgColor( SchemeHandle_t schemeHandle, int &r, int &g, int &b, int &a );
	void getBgColor( SchemeHandle_t schemeHandle, int &r, int &g, int &b, int &a );
	void getFgArmedColor( SchemeHandle_t schemeHandle, int &r, int &g, int &b, int &a );
	void getBgArmedColor( SchemeHandle_t schemeHandle, int &r, int &g, int &b, int &a );
	void getFgMousedownColor( SchemeHandle_t schemeHandle, int &r, int &g, int &b, int &a );
	void getBgMousedownColor( SchemeHandle_t schemeHandle, int &r, int &g, int &b, int &a );
	void getBorderColor( SchemeHandle_t schemeHandle, int &r, int &g, int &b, int &a );

private:
	class CScheme;
	CScheme *m_pSchemeList;
	int m_iNumSchemes;

	// Resolution we were initted at.
	int		m_xRes;

	CScheme *getSafeScheme( SchemeHandle_t schemeHandle );
};


