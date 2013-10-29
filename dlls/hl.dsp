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
# ADD CPP /nologo /G5 /MT /W3 /GR /Zi /O2 /I "..\dlls" /I "..\engine" /I "..\common" /I "..\pm_shared" /I "..\game_shared" /I "..\public" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "QUIVER" /D "VOXEL" /D "QUAKE2" /D "VALVE_DLL" /D "CLIENT_WEAPONS" /Fr /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /map /debug /machine:I386 /def:".\hl.def"
# SUBTRACT LINK32 /profile
# Begin Custom Build
InputDir=.\Releasehl
ProjDir=.
InputPath=.\Releasehl\hl.dll
InputName=hl
SOURCE="$(InputPath)"

BuildCmds= \
	call ..\filecopy.bat $(InputPath) $(ProjDir)\..\..\game\mod\dlls\$(InputName).dll \
	call ..\filecopy.bat $(InputDir)\$(InputName).pdb $(ProjDir)\..\..\game\mod\dlls\$(InputName).pdb \
	

"$(ProjDir)\..\..\game\mod\dlls\$(InputName).dll" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(ProjDir)\..\..\game\mod\dlls\$(InputName).pdb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
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
# ADD CPP /nologo /G5 /MTd /W3 /Gm /ZI /Od /I "..\\" /I "..\dlls" /I "..\engine" /I "..\common" /I "..\pm_shared" /I "..\game_shared" /I "..\public" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "QUIVER" /D "VOXEL" /D "QUAKE2" /D "VALVE_DLL" /D "CLIENT_WEAPONS" /FR /YX /FD /c
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
# Begin Custom Build
ProjDir=.
InputPath=.\debughl\hl.dll
InputName=hl
SOURCE="$(InputPath)"

"$(ProjDir)\..\..\game\mod\dlls\$(InputName).dll" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	call ..\filecopy.bat $(InputPath) $(ProjDir)\..\..\game\mod\dlls\$(InputName).dll

# End Custom Build

!ENDIF 

# Begin Target

# Name "hl - Win32 Release"
# Name "hl - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\aflock.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\agrunt.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\airtank.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\animating.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\animation.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\apache.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\barnacle.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\barney.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\bigmomma.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\bloater.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\bmodels.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\bullsquid.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\buttons.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cbase.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\client.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\combat.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\controller.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crossbow.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crowbar.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\defaultai.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\doors.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\effects.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\egon.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\explode.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\flyingmonster.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\func_break.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\func_tank.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gamerules.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gargantua.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gauss.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\genericmonster.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ggrenade.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\globals.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gman.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\h_ai.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\h_battery.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\h_cine.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\h_cycler.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\h_export.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\handgrenade.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\hassassin.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\headcrab.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\healthkit.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\hgrunt.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\wpn_shared\hl_wpn_glock.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\hornet.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\hornetgun.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\houndeye.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ichthyosaur.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\islave.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\items.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\leech.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\lights.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\maprules.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\monstermaker.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\monsters.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\monsterstate.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\mortar.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\mp5.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\multiplay_gamerules.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\nihilanth.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\nodes.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\observer.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\osprey.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pathcorner.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\plane.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\plats.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\player.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\pm_shared\pm_debug.c

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\pm_shared\pm_math.c

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\pm_shared\pm_shared.c

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\python.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\rat.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\roach.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\rpg.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\satchel.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\schedule.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\scientist.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\scripted.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shotgun.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\singleplay_gamerules.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\skill.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sound.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\soundent.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\spectator.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\squadmonster.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\squeakgrenade.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\subs.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\talkmonster.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\teamplay_gamerules.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\tempmonster.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\tentacle.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\triggers.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\tripmine.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\turret.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\util.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\game_shared\voice_gamemgr.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\weapons.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\world.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\xen.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\zombie.cpp

!IF  "$(CFG)" == "hl - Win32 Release"

!ELSEIF  "$(CFG)" == "hl - Win32 Debug"

# ADD CPP /GR

!ENDIF 

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

SOURCE=..\engine\eiface.h
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

SOURCE=.\flyingmonster.h
# End Source File
# Begin Source File

SOURCE=.\func_break.h
# End Source File
# Begin Source File

SOURCE=.\gamerules.h
# End Source File
# Begin Source File

SOURCE=.\hornet.h
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

SOURCE=.\squadmonster.h
# End Source File
# Begin Source File

SOURCE=.\talkmonster.h
# End Source File
# Begin Source File

SOURCE=.\teamplay_gamerules.h
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

SOURCE=.\weapons.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
