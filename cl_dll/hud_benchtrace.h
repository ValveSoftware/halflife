#if !defined( HUD_BENCHTRACEH )
#define HUD_BENCHTRACEH
#pragma once

void Trace_StartTrace( int *results, int *finished, const char *pszServer );
void Trace_Think( void );

#endif // !HUD_BENCHTRACEH