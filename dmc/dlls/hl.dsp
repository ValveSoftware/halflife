# Microsoft Developer Studio Project File - Name="hl" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=hl - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "hl.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "hl.mak" CFG="hl - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "hl - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "hl - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/GoldSrc/dmc/dlls", TTQCAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "hl - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Releasehl"
# PROP Intermediate_Dir ".\Releasehl"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G5 /MT /W3 /Zi /O2 /I "..\engine" /I "..\common" /I "..\..\public" /I "." /I "..\..\game_shared" /I "..\dlls" /I "..\..\engine" /I "..\..\common" /I "..\pm_shared" /I "..\\" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "QUIVER" /D "VOXEL" /D "QUAKE2" /D "VALVE_DLL" /Fr /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /map /debug /machine:I386 /def:".\hl.def" /out:".\Releasehl/dmc.dll"
# SUBTRACT LINK32 /profile
# Begin Custom Build - Copying...
InputDir=.\Releasehl
ProjDir=.
InputPath=.\Releasehl\dmc.dll
InputName=dmc
SOURCE="$(InputPath)"

BuildCmds= \
	call ..\..\filecopy.bat $(InputPath) $(ProjDir)\..\..\..\game\mod\dlls\$(InputName).dll \
	call ..\..\filecopy.bat $(InputDir)\$(InputName).pdb $(ProjDir)\..\..\..\game\mod\dlls\$(InputName).pdb \
	

"$(ProjDir)\..\..\..\game\mod\dlls\$(InputName).dll" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(ProjDir)\..\..\..\game\mod\dlls\$(InputName).pdb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\hl___Win"
# PROP BASE Intermediate_Dir ".\hl___Win"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\debughl"
# PROP Intermediate_Dir ".\debughl"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G5 /MTd /W3 /Gm /ZI /Od /I "..\public" /I "." /I "..\..\game_shared" /I "..\dlls" /I "..\..\engine" /I "..\..\common" /I "..\pm_shared" /I "..\\" /I "..\..\public" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "QUIVER" /D "VOXEL" /D "QUAKE2" /D "VALVE_DLL" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "..\engine" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 user32.lib advapi32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /def:".\hl.def" /implib:".\Debug\hl.lib"
# SUBTRACT LINK32 /profile
# Begin Custom Build - Copying...
ProjDir=.
InputPath=.\debughl\hl.dll
InputName=hl
SOURCE="$(InputPath)"

"$(ProjDir)\..\..\..\game\mod\dlls\$(InputName).dll" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	call ..\..\filecopy.bat $(InputPath) $(ProjDir)\..\..\..\game\mod\dlls\$(InputName).dll

# End Custom Build

!ENDIF 

# Begin Target

# Name "hl - Win32 Release"
# Name "hl - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;for;f90"
# Begin Group "Shared Weapons"

# PROP Default_Filter "*.cpp"
# Begin Source File

SOURCE=.\quake_gun.cpp
# End Source File
# Begin Source File

SOURCE=.\quake_weapons_all.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\animating.cpp
# End Source File
# Begin Source File

SOURCE=.\animation.cpp
# End Source File
# Begin Source File

SOURCE=.\bmodels.cpp
# End Source File
# Begin Source File

SOURCE=.\buttons.cpp
# End Source File
# Begin Source File

SOURCE=.\cbase.cpp
# End Source File
# Begin Source File

SOURCE=.\client.cpp
# End Source File
# Begin Source File

SOURCE=.\combat.cpp
# End Source File
# Begin Source File

SOURCE=.\doors.cpp
# End Source File
# Begin Source File

SOURCE=.\effects.cpp
# End Source File
# Begin Source File

SOURCE=.\explode.cpp
# End Source File
# Begin Source File

SOURCE=.\func_break.cpp
# End Source File
# Begin Source File

SOURCE=.\func_tank.cpp
# End Source File
# Begin Source File

SOURCE=.\game.cpp
# End Source File
# Begin Source File

SOURCE=.\gamerules.cpp
# End Source File
# Begin Source File

SOURCE=.\globals.cpp
# End Source File
# Begin Source File

SOURCE=.\h_ai.cpp
# End Source File
# Begin Source File

SOURCE=.\h_export.cpp
# End Source File
# Begin Source File

SOURCE=.\lights.cpp
# End Source File
# Begin Source File

SOURCE=.\maprules.cpp
# End Source File
# Begin Source File

SOURCE=.\monsters.cpp
# End Source File
# Begin Source File

SOURCE=.\monsterstate.cpp
# End Source File
# Begin Source File

SOURCE=.\multiplay_gamerules.cpp
# End Source File
# Begin Source File

SOURCE=.\nodes.cpp
# End Source File
# Begin Source File

SOURCE=.\observer.cpp
# End Source File
# Begin Source File

SOURCE=.\pathcorner.cpp
# End Source File
# Begin Source File

SOURCE=.\plane.cpp
# End Source File
# Begin Source File

SOURCE=.\plats.cpp
# End Source File
# Begin Source File

SOURCE=.\player.cpp
# End Source File
# Begin Source File

SOURCE=..\pm_shared\pm_debug.c
# End Source File
# Begin Source File

SOURCE=..\pm_shared\pm_math.c
# End Source File
# Begin Source File

SOURCE=..\pm_shared\pm_shared.c
# End Source File
# Begin Source File

SOURCE=.\quake_items.cpp
# End Source File
# Begin Source File

SOURCE=.\quake_nail.cpp
# End Source File
# Begin Source File

SOURCE=.\quake_player.cpp
# End Source File
# Begin Source File

SOURCE=.\quake_rocket.cpp
# End Source File
# Begin Source File

SOURCE=.\schedule.cpp
# End Source File
# Begin Source File

SOURCE=.\singleplay_gamerules.cpp
# End Source File
# Begin Source File

SOURCE=.\skill.cpp
# End Source File
# Begin Source File

SOURCE=.\sound.cpp
# End Source File
# Begin Source File

SOURCE=.\spectator.cpp
# End Source File
# Begin Source File

SOURCE=.\subs.cpp
# End Source File
# Begin Source File

SOURCE=.\teamplay_gamerules.cpp
# End Source File
# Begin Source File

SOURCE=.\threewave_gamerules.cpp
# End Source File
# Begin Source File

SOURCE=.\triggers.cpp
# End Source File
# Begin Source File

SOURCE=.\util.cpp
# End Source File
# Begin Source File

SOURCE=..\..\game_shared\voice_gamemgr.cpp
# End Source File
# Begin Source File

SOURCE=.\weapons.cpp
# End Source File
# Begin Source File

SOURCE=.\world.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\activity.h
# End Source File
# Begin Source File

SOURCE=.\activitymap.h
# End Source File
# Begin Source File

SOURCE=.\animation.h
# End Source File
# Begin Source File

SOURCE=.\basemonster.h
# End Source File
# Begin Source File

SOURCE=.\cbase.h
# End Source File
# Begin Source File

SOURCE=.\cdll_dll.h
# End Source File
# Begin Source File

SOURCE=.\client.h
# End Source File
# Begin Source File

SOURCE=.\decals.h
# End Source File
# Begin Source File

SOURCE=.\defaultai.h
# End Source File
# Begin Source File

SOURCE=.\doors.h
# End Source File
# Begin Source File

SOURCE=.\effects.h
# End Source File
# Begin Source File

SOURCE=.\enginecallback.h
# End Source File
# Begin Source File

SOURCE=.\explode.h
# End Source File
# Begin Source File

SOURCE=.\extdll.h
# End Source File
# Begin Source File

SOURCE=.\func_break.h
# End Source File
# Begin Source File

SOURCE=.\gamerules.h
# End Source File
# Begin Source File

SOURCE=.\items.h
# End Source File
# Begin Source File

SOURCE=.\monsterevent.h
# End Source File
# Begin Source File

SOURCE=.\monsters.h
# End Source File
# Begin Source File

SOURCE=.\nodes.h
# End Source File
# Begin Source File

SOURCE=.\plane.h
# End Source File
# Begin Source File

SOURCE=.\player.h
# End Source File
# Begin Source File

SOURCE=..\pm_shared\pm_debug.h
# End Source File
# Begin Source File

SOURCE=..\pm_shared\pm_defs.h
# End Source File
# Begin Source File

SOURCE=..\pm_shared\pm_info.h
# End Source File
# Begin Source File

SOURCE=..\pm_shared\pm_materials.h
# End Source File
# Begin Source File

SOURCE=..\pm_shared\pm_movevars.h
# End Source File
# Begin Source File

SOURCE=..\pm_shared\pm_shared.h
# End Source File
# Begin Source File

SOURCE=.\quake_gun.h
# End Source File
# Begin Source File

SOURCE=.\saverestore.h
# End Source File
# Begin Source File

SOURCE=.\schedule.h
# End Source File
# Begin Source File

SOURCE=.\scripted.h
# End Source File
# Begin Source File

SOURCE=.\scriptevent.h
# End Source File
# Begin Source File

SOURCE=.\skill.h
# End Source File
# Begin Source File

SOURCE=.\soundent.h
# End Source File
# Begin Source File

SOURCE=.\spectator.h
# End Source File
# Begin Source File

SOURCE=.\teamplay_gamerules.h
# End Source File
# Begin Source File

SOURCE=.\threewave_gamerules.h
# End Source File
# Begin Source File

SOURCE=.\trains.h
# End Source File
# Begin Source File

SOURCE=.\util.h
# End Source File
# Begin Source File

SOURCE=.\vector.h
# End Source File
# Begin Source File

SOURCE=..\..\game_shared\voice_gamemgr.h
# End Source File
# Begin Source File

SOURCE=.\weapons.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
