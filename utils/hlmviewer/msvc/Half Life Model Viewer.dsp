# Microsoft Developer Studio Project File - Name="Half Life Model Viewer" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Half Life Model Viewer - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "Half Life Model Viewer.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "Half Life Model Viewer.mak" CFG="Half Life Model Viewer - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "Half Life Model Viewer - Win32 Release" (basierend auf  "Win32 (x86) Application")
!MESSAGE "Half Life Model Viewer - Win32 Debug" (basierend auf  "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Half Life Model Viewer - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\mxtk\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "HAVE_SCALE" /FR /YX /FD /I /mxtk/include" " /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x807 /d "NDEBUG"
# ADD RSC /l 0x807 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 mxtk.lib kernel32.lib user32.lib gdi32.lib winspool.lib comctl32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib opengl32.lib glu32.lib winmm.lib /nologo /version:1.2 /entry:"mainCRTStartup" /subsystem:windows /machine:I386 /out:"../bin/hlmv.exe" /libpath:"..\mxtk\lib" /libpath:"/mxtk/lib" /release
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "Half Life Model Viewer - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\mxtk\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /I /mxtk/include" " /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x807 /d "_DEBUG"
# ADD RSC /l 0x807 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 mxtkd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comctl32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib opengl32.lib glu32.lib winmm.lib /nologo /version:1.1 /entry:"mainCRTStartup" /subsystem:windows /debug /machine:I386 /out:"../bin/hlmv.exe" /pdbtype:sept /libpath:"..\mxtk\lib" /libpath:"/mxtk/lib/"

!ENDIF 

# Begin Target

# Name "Half Life Model Viewer - Win32 Release"
# Name "Half Life Model Viewer - Win32 Debug"
# Begin Source File

SOURCE=..\src\anorms.h
# End Source File
# Begin Source File

SOURCE=..\src\ControlPanel.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ControlPanel.h
# End Source File
# Begin Source File

SOURCE=..\src\FileAssociation.cpp
# End Source File
# Begin Source File

SOURCE=..\src\FileAssociation.h
# End Source File
# Begin Source File

SOURCE=..\src\GlWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\src\GlWindow.h
# End Source File
# Begin Source File

SOURCE=..\src\hlmviewer.rc
# End Source File
# Begin Source File

SOURCE=..\src\icon1.ico
# End Source File
# Begin Source File

SOURCE=..\src\mathlib.c
# End Source File
# Begin Source File

SOURCE=..\src\mathlib.h
# End Source File
# Begin Source File

SOURCE=..\src\mdlviewer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\mdlviewer.h
# End Source File
# Begin Source File

SOURCE=..\src\pakviewer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\pakviewer.h
# End Source File
# Begin Source File

SOURCE=..\src\resource.h
# End Source File
# Begin Source File

SOURCE=..\src\studio.h
# End Source File
# Begin Source File

SOURCE=..\src\studio_render.cpp
# End Source File
# Begin Source File

SOURCE=..\src\studio_utils.cpp
# End Source File
# Begin Source File

SOURCE=..\src\StudioModel.h
# End Source File
# Begin Source File

SOURCE=..\src\ViewerSettings.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ViewerSettings.h
# End Source File
# End Target
# End Project
