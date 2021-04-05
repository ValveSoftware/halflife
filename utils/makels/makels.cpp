/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/

#include "../../public/vstdlib/warnings.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../public/vstdlib/vstdlib.h"


char	**ppszFiles = NULL;
int		nFiles = 0;
int		nMaxFiles = 0;

int
string_comparator( const void *string1, const void *string2 )
{
	char	*s1 = *(char **)string1;
	char	*s2 = *(char **)string2;
	return Q_strcmp( s1, s2 );
}

void PrintUsage(char *pname)
{
	Q_printf("\n\tusage:%s <source directory> <wadfile name> <script name> \n\n",pname);
	Q_printf("\t%s.exe is used to generate a bitmap name sorted 'qlumpy script'.\n",pname);
}

int main(int argc, void **argv)
{
	char *pszdir;
	char *pszWadName;
	char *pszScriptName;
	char szBuf[1024];
	HANDLE hFile, hScriptFile;
	WIN32_FIND_DATA FindData;
	BOOL fWrite;
	BOOL fContinue = TRUE;
	DWORD dwWritten;

	Q_printf("makels Copyright (c) 1998 Valve L.L.C., %s\n", __DATE__ );

	pszdir = (char *)argv[1];

	if ((argc != 4) || (pszdir[0] == '/') || (pszdir[0] == '-'))
	{
		PrintUsage((char *)argv[0]);
		Q_exit(1);
	}

	pszdir = (char *)Q_malloc(Q_strlen((char *)argv[1]) + 7);
	Q_strcpy(pszdir, (char *)argv[1]);
	Q_strcat(pszdir, "\\*.bmp");

	pszWadName = (char *)Q_malloc(Q_strlen((char *)argv[2]) + 5);
	Q_strcpy(pszWadName, (char *)argv[2]);
	Q_strcat(pszWadName, ".WAD");

	pszScriptName = (char *)Q_malloc(Q_strlen((char *)argv[3]));
	Q_strcpy(pszScriptName, (char *)argv[3]);
	hScriptFile = CreateFile(pszScriptName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 
			FILE_ATTRIBUTE_NORMAL, NULL);

	if (hScriptFile == INVALID_HANDLE_VALUE)
	{
		Q_printf("\n---------- ERROR ------------------\n");
		Q_printf(" Could not open the script file: %s\n", pszScriptName);
		Beep(800,500);
		Q_exit(EXIT_FAILURE);
	}

	Q_sprintf(szBuf, "$DEST    \"%s\"\r\n\r\n", pszWadName);
	fWrite = WriteFile(hScriptFile, szBuf, Q_strlen(szBuf), &dwWritten, NULL);
	if (!fWrite || (dwWritten != Q_strlen(szBuf)))
	{
write_error:
		Q_printf("\n---------- ERROR ------------------\n");
		Q_printf(" Could not write to the script file: %s\n", pszScriptName);
		Beep(800,500);
		CloseHandle(hScriptFile);
		Q_exit(EXIT_FAILURE);
	}
	
	
	hFile = FindFirstFile(pszdir, &FindData);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		while (fContinue)
		{
			if (!(FindData.dwFileAttributes &
					(FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_HIDDEN)))
			{
				char szShort[MAX_PATH];

				// ignore N_ and F_ files
				Q_strcpy(szShort, FindData.cFileName);
				Q_strupr(szShort);

				if ((szShort[1] == '_') && ((szShort[0] == 'N') || (szShort[0] == 'F')))
				{

					Q_printf("Skipping %s.\n", FindData.cFileName);

				} else {
				
					if ( nFiles >= nMaxFiles )
					{
						nMaxFiles += 1000;
						ppszFiles = (char **)Q_realloc( ppszFiles, nMaxFiles * sizeof(*ppszFiles) );
						if ( !ppszFiles )
						{
							Q_printf("\n---------- ERROR ------------------\n");
							Q_printf(" Could not realloc more filename pointer storage\n");
							Beep(800,500);
							Q_exit(EXIT_FAILURE);
						}
					}
					ppszFiles[nFiles++] = Q_strdup( szShort );
				}
			}
			fContinue = FindNextFile(hFile, &FindData);
		}	
	}


	if (nFiles > 0)
	{
		Q_qsort( ppszFiles, nFiles, sizeof(char*), string_comparator );

		for( int i = 0; i < nFiles; i++ )
		{
			char *p;
			char szShort[MAX_PATH];
			char szFull[MAX_PATH];

			Q_strcpy(szShort, pszdir);
			p = Q_strchr(szShort, '*');
			*p = '\0';
			Q_strcat(szShort, ppszFiles[i]);
			GetFullPathName(szShort, MAX_PATH, szFull, NULL);

			Q_sprintf(szBuf, "$loadbmp    \"%s\"\r\n", szFull);
			fWrite = WriteFile(hScriptFile, szBuf, Q_strlen(szBuf), &dwWritten, NULL);
			if (!fWrite || (dwWritten != Q_strlen(szBuf)))
				goto write_error;


			p = Q_strchr(ppszFiles[i], '.');
			*p = '\0';

			Q_sprintf(szBuf, "%s  miptex -1 -1 -1 -1\r\n\r\n", ppszFiles[i]);
			fWrite = WriteFile(hScriptFile, szBuf, Q_strlen(szBuf), &dwWritten, NULL);
			if (!fWrite || (dwWritten != Q_strlen(szBuf)))
				goto write_error;

			Q_free( ppszFiles[i] );
		}
	}
	
	Q_printf("Processed %d files specified by %s\n", nFiles, pszdir );

	CloseHandle(hScriptFile);
	Q_free(pszdir);
	Q_exit(0);
	return 0;
}