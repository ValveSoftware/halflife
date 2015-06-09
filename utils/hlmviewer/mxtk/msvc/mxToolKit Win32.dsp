# Microsoft Developer Studio Project File - Name="mxToolKit Win32" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=mxToolKit Win32 - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "mxToolKit Win32.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "mxToolKit Win32.mak" CFG="mxToolKit Win32 - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "mxToolKit Win32 - Win32 Release" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "mxToolKit Win32 - Win32 Debug" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "mxToolKit Win32 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE RSC /l 0x807
# ADD RSC /l 0x807
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\mxtk.lib"

!ELSEIF  "$(CFG)" == "mxToolKit Win32 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GX /Z7 /Od /I "../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE RSC /l 0x807
# ADD RSC /l 0x807
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\mxtkd.lib"

!ENDIF 

# Begin Target

# Name "mxToolKit Win32 - Win32 Release"
# Name "mxToolKit Win32 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp"
# Begin Source File

SOURCE=..\src\win32\mx.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\mxAccelerator.cpp
# End Source File
# Begin Source File

SOURCE=..\src\common\mxBmp.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\mxButton.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\mxCheckBox.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\mxChoice.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\mxChooseColor.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\mxFileDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\mxGlWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\mxGroupBox.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\mxLabel.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\mxLineEdit.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\mxListBox.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\mxMenu.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\mxMenuBar.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\mxMessageBox.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\mxMultiLineEdit.cpp
# End Source File
# Begin Source File

SOURCE=..\src\common\mxpath.cpp
# End Source File
# Begin Source File

SOURCE=..\src\common\mxPcx.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\mxPopupMenu.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\mxProgressBar.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\mxRadioButton.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\mxSlider.cpp
# End Source File
# Begin Source File

SOURCE=..\src\common\mxstring.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\mxTab.cpp
# End Source File
# Begin Source File

SOURCE=..\src\common\mxTga.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\mxToggleButton.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\mxToolTip.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\mxTreeView.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\mxWidget.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\mxWindow.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=..\include\mx\gl.h
# End Source File
# Begin Source File

SOURCE=..\include\mx\mx.h
# End Source File
# Begin Source File

SOURCE=..\include\mx\mxAccelerator.h
# End Source File
# Begin Source File

SOURCE=..\include\mx\mxBmp.h
# End Source File
# Begin Source File

SOURCE=..\include\mx\mxButton.h
# End Source File
# Begin Source File

SOURCE=..\include\mx\mxCheckBox.h
# End Source File
# Begin Source File

SOURCE=..\include\mx\mxChoice.h
# End Source File
# Begin Source File

SOURCE=..\include\mx\mxChooseColor.h
# End Source File
# Begin Source File

SOURCE=..\include\mx\mxEvent.h
# End Source File
# Begin Source File

SOURCE=..\include\mx\mxFileDialog.h
# End Source File
# Begin Source File

SOURCE=..\include\mx\mxGlWindow.h
# End Source File
# Begin Source File

SOURCE=..\include\mx\mxGroupBox.h
# End Source File
# Begin Source File

SOURCE=..\include\mx\mxImage.h
# End Source File
# Begin Source File

SOURCE=..\include\mx\mxInit.h
# End Source File
# Begin Source File

SOURCE=..\include\mx\mxLabel.h
# End Source File
# Begin Source File

SOURCE=..\include\mx\mxLineEdit.h
# End Source File
# Begin Source File

SOURCE=..\include\mx\mxLinkedList.h
# End Source File
# Begin Source File

SOURCE=..\include\mx\mxListBox.h
# End Source File
# Begin Source File

SOURCE=..\include\mx\mxMenu.h
# End Source File
# Begin Source File

SOURCE=..\include\mx\mxMenuBar.h
# End Source File
# Begin Source File

SOURCE=..\include\mx\mxMessageBox.h
# End Source File
# Begin Source File

SOURCE=..\include\mx\mxMultiLineEdit.h
# End Source File
# Begin Source File

SOURCE=..\include\mx\mxpath.h
# End Source File
# Begin Source File

SOURCE=..\include\mx\mxPcx.h
# End Source File
# Begin Source File

SOURCE=..\include\mx\mxPopupMenu.h
# End Source File
# Begin Source File

SOURCE=..\include\mx\mxProgressBar.h
# End Source File
# Begin Source File

SOURCE=..\include\mx\mxRadioButton.h
# End Source File
# Begin Source File

SOURCE=..\include\mx\mxSlider.h
# End Source File
# Begin Source File

SOURCE=..\include\mx\mxstring.h
# End Source File
# Begin Source File

SOURCE=..\include\mx\mxTab.h
# End Source File
# Begin Source File

SOURCE=..\include\mx\mxTga.h
# End Source File
# Begin Source File

SOURCE=..\include\mx\mxToggleButton.h
# End Source File
# Begin Source File

SOURCE=..\include\mx\mxToolTip.h
# End Source File
# Begin Source File

SOURCE=..\include\mx\mxTreeView.h
# End Source File
# Begin Source File

SOURCE=..\include\mx\mxWidget.h
# End Source File
# Begin Source File

SOURCE=..\include\mx\mxWindow.h
# End Source File
# End Group
# End Target
# End Project
