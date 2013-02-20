//==========================================================================;
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (c) 1992 - 1997  Microsoft Corporation.  All Rights Reserved.
//
//--------------------------------------------------------------------------;


// For every module and executable we store a debugging level and flags
// for the types of output that are desired. Constants for the types are
// defined in WXDEBUG.H and more can be added.
// The keys are stored in the registry under the
// HKEY_LOCAL_MACHINE\SOFTWARE\Debug\<Module Name>\Type and
// HKEY_LOCAL_MACHINE\SOFTWARE\Debug\<Module Name>\Level key values
//
// There are also global values under SOFTWARE\Debug\Global which are loaded
// after the module-specific values. The Types specified there are OR'ed with
// the module specific types and m_dwLevel is set to the greater of the global
// and the module specific settings.

#include <stdarg.h>
#include <stdio.h>

#include "extdll.h"
#include "util.h"
#include "wxdebug.h"

#include <tchar.h>

#ifdef _DEBUG

void WINAPI DbgInitModuleName(void);
void WINAPI DbgInitModuleSettings(void);
void WINAPI DbgInitGlobalSettings(void);
void WINAPI DbgInitLogTo(HKEY hKey);
void WINAPI DbgInitKeyLevels(HKEY hKey, DWORD *pdwTypes, DWORD *pdwLevel);



const INT iDEBUGINFO = 512;                 // Used to format strings

HINSTANCE m_hInst;                          // Module instance handle
TCHAR m_ModuleName[iDEBUGINFO];             // Cut down module name
//CRITICAL_SECTION m_CSDebug;                 // Controls access to list
BOOL m_bInit = FALSE;                       // Have we been initialised
HANDLE m_hOutput = INVALID_HANDLE_VALUE;    // Optional output written here
DWORD m_dwTypes = 0;
DWORD m_dwLevel = 0;

const TCHAR *m_pBaseKey = TEXT("SOFTWARE\\Debug");
const TCHAR *m_pGlobalKey = TEXT("GLOBAL");
TCHAR *pKeyNames[] =
{
    TEXT("Types"),
    TEXT("Level")
};


// DbgInitialize
// This sets the instance handle that the debug library uses to find
// the module's file name from the Win32 GetModuleFileName function
void WINAPI DbgInitialise(HINSTANCE hInst)
{
    if (!m_bInit)
    {
        //InitializeCriticalSection(&m_CSDebug);
        m_bInit = TRUE;
        m_hInst = hInst;
        DbgInitModuleName();
        DbgInitModuleSettings();
        DbgInitGlobalSettings();
    }
}


// DbgTerminate
// This is called to clear up any resources the debug library uses - at the
// moment we delete our critical section and the handle of the output file.
void WINAPI DbgTerminate()
{
    if (m_bInit)
    {
        if (m_hOutput != INVALID_HANDLE_VALUE)
        {
            DBGASSERTEXECUTE(CloseHandle(m_hOutput));
            m_hOutput = INVALID_HANDLE_VALUE;
        }
        //DeleteCriticalSection(&m_CSDebug);
        m_bInit = FALSE;
    }
}


// DbgInitModuleName
// Initialise the module file name
void WINAPI DbgInitModuleName()
{
    TCHAR FullName[iDEBUGINFO];     // Load the full path and module name
    TCHAR *pName;                   // Searches from the end for a backslash

    GetModuleFileName(m_hInst,FullName,iDEBUGINFO);
    pName = _tcsrchr(FullName,'\\');
    if (pName == NULL)
    {
        pName = FullName;
    }
    else
    {
        pName++;
    }
    lstrcpy(m_ModuleName,pName);
}


// DbgInitModuleSettings
// Retrieve the module-specific settings
void WINAPI DbgInitModuleSettings()
{
    LONG lReturn;               // Create key return value
    TCHAR szInfo[iDEBUGINFO];   // Constructs key names
    HKEY hModuleKey;            // Module key handle

    // Construct the base key name
    wsprintf(szInfo,TEXT("%s\\%s"),m_pBaseKey,m_ModuleName);

    // Create or open the key for this module
    lReturn = RegCreateKeyEx(HKEY_LOCAL_MACHINE,   // Handle of an open key
                             szInfo,               // Address of subkey name
                             (DWORD)0,             // Reserved value
                             NULL,                 // Address of class name
                             (DWORD)0,             // Special options flags
                             KEY_ALL_ACCESS,       // Desired security access
                             NULL,                 // Key security descriptor
                             &hModuleKey,          // Opened handle buffer
                             NULL);                // What really happened

    if (lReturn != ERROR_SUCCESS)
    {
        DbgLogInfo(LOG_ERROR, 0, TEXT("Could not access module key"));
        return;
    }

    DbgInitLogTo(hModuleKey);
    DbgInitKeyLevels(hModuleKey, &m_dwTypes, &m_dwLevel);
    RegCloseKey(hModuleKey);
}


// DbgInitGlobalSettings
// This is called by DbgInitialize to read the global debug settings for
// Level and Type from the registry. The Types are OR'ed together and m_dwLevel
// is set to the greater of the global and module-specific values.
void WINAPI DbgInitGlobalSettings()
{
    LONG lReturn;               // Create key return value
    TCHAR szInfo[iDEBUGINFO];   // Constructs key names
    HKEY hGlobalKey;            // Global override key
    DWORD dwTypes = 0;
    DWORD dwLevel = 0;

    // Construct the global base key name
    wsprintf(szInfo,TEXT("%s\\%s"),m_pBaseKey,m_pGlobalKey);

    // Create or open the key for this module
    lReturn = RegCreateKeyEx(HKEY_LOCAL_MACHINE,   // Handle of an open key
                             szInfo,               // Address of subkey name
                             (DWORD)0,             // Reserved value
                             NULL,                 // Address of class name
                             (DWORD)0,             // Special options flags
                             KEY_ALL_ACCESS,       // Desired security access
                             NULL,                 // Key security descriptor
                             &hGlobalKey,          // Opened handle buffer
                             NULL);                // What really happened

    if (lReturn != ERROR_SUCCESS)
    {
        DbgLogInfo(LOG_ERROR, 0, TEXT("Could not access GLOBAL module key"));
        return;
    }

    DbgInitKeyLevels(hGlobalKey, &dwTypes, &dwLevel);
    RegCloseKey(hGlobalKey);

    m_dwTypes |= dwTypes;
    if (dwLevel > m_dwLevel)
        m_dwLevel = dwLevel;
}


// DbgInitLogTo
// Called by DbgInitModuleSettings to setup alternate logging destinations
void WINAPI DbgInitLogTo(HKEY hKey)
{
    LONG  lReturn;
    DWORD dwKeyType;
    DWORD dwKeySize;
    TCHAR szFile[MAX_PATH] = {0};
    static const TCHAR cszKey[] = TEXT("LogToFile");

    dwKeySize = MAX_PATH;
    lReturn = RegQueryValueEx(
        hKey,                       // Handle to an open key
        cszKey,                     // Subkey name derivation
        NULL,                       // Reserved field
        &dwKeyType,                 // Returns the field type
        (LPBYTE) szFile,            // Returns the field's value
        &dwKeySize);                // Number of bytes transferred

    // create an empty key if it does not already exist
    if (lReturn != ERROR_SUCCESS || dwKeyType != REG_SZ)
       {
       dwKeySize = 1;
       lReturn = RegSetValueEx(
            hKey,                   // Handle of an open key
            cszKey,                 // Address of subkey name
            (DWORD) 0,              // Reserved field
            REG_SZ,                 // Type of the key field
            (PBYTE)szFile,          // Value for the field
            dwKeySize);            // Size of the field buffer
       }

    // if an output-to was specified.  try to open it.
    if (m_hOutput != INVALID_HANDLE_VALUE)
    {
       DBGASSERTEXECUTE(CloseHandle(m_hOutput));
       m_hOutput = INVALID_HANDLE_VALUE;
    }
    if (szFile[0] != 0)
    {
        if (!lstrcmpi(szFile, TEXT("Console")))
        {
            m_hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
            if (m_hOutput == INVALID_HANDLE_VALUE)
            {
                AllocConsole();
                m_hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
            }
            SetConsoleTitle (TEXT("Valve Debug Output"));
        } else if (szFile[0] &&
            lstrcmpi(szFile, TEXT("Debug")) &&
            lstrcmpi(szFile, TEXT("Debugger")) &&
            lstrcmpi(szFile, TEXT("Deb")))
        {
            m_hOutput = CreateFile(szFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (INVALID_HANDLE_VALUE != m_hOutput)
            {
                static const TCHAR cszBar[] = TEXT("\r\n\r\n=====DbgInitialize()=====\r\n\r\n");
                SetFilePointer (m_hOutput, 0, NULL, FILE_END);
                DbgOutString (cszBar);
            }
        }
    }
}


// DbgInitKeyLevels
// This is called by DbgInitModuleSettings and DbgInitGlobalSettings to read 
// settings for Types and Level from the registry
void WINAPI DbgInitKeyLevels(HKEY hKey, DWORD *pdwTypes, DWORD *pdwLevel)
{
    LONG lReturn;               // Create key return value
    DWORD dwKeySize;            // Size of the key value
    DWORD dwKeyType;            // Receives it's type

    // Get the Types value
    dwKeySize = sizeof(DWORD);
    lReturn = RegQueryValueEx(
        hKey,                   // Handle to an open key
        pKeyNames[0],           // Subkey name derivation
        NULL,                   // Reserved field
        &dwKeyType,             // Returns the field type
        (LPBYTE)pdwTypes,       // Returns the field's value
        &dwKeySize );           // Number of bytes transferred

    // If either the key was not available or it was not a DWORD value
    // then we ensure only the high priority debug logging is output
    //  but we try and update the field to a zero filled DWORD value

    if (lReturn != ERROR_SUCCESS || dwKeyType != REG_DWORD)
    {
        *pdwTypes = 0;
        lReturn = RegSetValueEx(
            hKey,               // Handle of an open key
            pKeyNames[0],       // Address of subkey name
            (DWORD)0,           // Reserved field
            REG_DWORD,          // Type of the key field
            (PBYTE)pdwTypes,    // Value for the field
            sizeof(DWORD));     // Size of the field buffer

        if (lReturn != ERROR_SUCCESS)
        {
            DbgLogInfo(LOG_ERROR, 0, TEXT("Could not create subkey %s"),pKeyNames[0]);
            *pdwTypes = 0;
        }
    }

    // Get the Level value
    dwKeySize = sizeof(DWORD);
    lReturn = RegQueryValueEx(
        hKey,                   // Handle to an open key
        pKeyNames[1],           // Subkey name derivation
        NULL,                   // Reserved field
        &dwKeyType,             // Returns the field type
        (LPBYTE)pdwLevel,       // Returns the field's value
        &dwKeySize );           // Number of bytes transferred

    // If either the key was not available or it was not a DWORD value
    // then we ensure only the high priority debug logging is output
    //  but we try and update the field to a zero filled DWORD value

    if (lReturn != ERROR_SUCCESS || dwKeyType != REG_DWORD)
    {
        *pdwLevel = 0;
        lReturn = RegSetValueEx(
            hKey,               // Handle of an open key
            pKeyNames[1],       // Address of subkey name
            (DWORD)0,           // Reserved field
            REG_DWORD,          // Type of the key field
            (PBYTE)pdwLevel,   // Value for the field
            sizeof(DWORD));     // Size of the field buffer

        if (lReturn != ERROR_SUCCESS)
        {
            DbgLogInfo(LOG_ERROR, 0, TEXT("Could not create subkey %s"),pKeyNames[1]);
            *pdwLevel = 0;
        }
    }
}


// DbgOutString
void WINAPI DbgOutString(LPCTSTR psz)
{
    if (!m_bInit)
        return;
    if (m_hOutput != INVALID_HANDLE_VALUE) {
        UINT  cb = lstrlen(psz);
        DWORD dw;
        WriteFile (m_hOutput, psz, cb, &dw, NULL);
    } else {
        OutputDebugString (psz);
    }
}


// DbgLogInfo
// Print a formatted string to the debugger prefixed with this module's name
// Because the debug code is linked statically every module loaded will
// have its own copy of this code. It therefore helps if the module name is
// included on the output so that the offending code can be easily found
void WINAPI DbgLogInfo(DWORD Type, DWORD Level, const TCHAR *pFormat,...)
{
    if (!m_bInit)
        return;
    // Check the current level for this type combination */
    if (((Type & m_dwTypes) == 0) || (m_dwLevel < Level))
        return;

    TCHAR szInfo[2000];

    // Format the variable length parameter list

    va_list va;
    va_start(va, pFormat);

    //lstrcpy(szInfo, m_ModuleName);
    //lstrcat(szInfo, TEXT(": "));
    wvsprintf(szInfo /* + lstrlen(szInfo) */, pFormat, va);
    //lstrcat(szInfo, TEXT("\r\n"));
    DbgOutString(szInfo);

    va_end(va);
}


// DbgKernelAssert
// If we are executing as a pure kernel filter we cannot display message
// boxes to the user, this provides an alternative which puts the error
// condition on the debugger output with a suitable eye catching message
void WINAPI DbgKernelAssert(const TCHAR *pCondition, const TCHAR *pFileName, INT iLine)
{
    if (!m_bInit)
        return;
	DbgLogInfo(LOG_ERROR, 0, TEXT(m_ModuleName));
    DbgLogInfo(LOG_ERROR, 0, TEXT(": Assertion FAILED (%s) at line %d in file %s\r\n"), pCondition, iLine, pFileName);
    DebugBreak();
}

#endif // _DEBUG


