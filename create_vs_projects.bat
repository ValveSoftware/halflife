@if "%overbose%" == "" echo off

REM ----------------------------------
REM create_vs_projects.bat
REM create a usable sln and vcxproj for the goldsrc dlls
REM ----------------------------------

setlocal enabledelayedexpansion
goto Setup

:Setup
set _PYTHON_=vpython

set _VSVER_=2019
set _BUILD_=DEBUG
set _BUILDTYPE_=debugoptimized

REM TODO: better arg support here
if "%1" == "release" (
	set _BUILD_=RELEASE
	set _BUILDTYPE_=release
)

if "%_BUILD_%" == "DEBUG" (
	set _BUILDTYPE_=debugoptimized
)

set "_PGM_FILES_=%ProgramFiles%"
if not exist "!_PGM_FILES_!\Microsoft Visual Studio\%_VSVER_%\" (
	set "_PGM_FILES_=%ProgramFiles(x86)%"
)

set "VSINSTALLDIR=!_PGM_FILES_!\Microsoft Visual Studio\%_VSVER_%\BuildTools\"
set "_VC_VARS_=!VSINSTALLDIR!VC\Auxiliary\Build\vcvars32.bat"
if not exist "!_VC_VARS_!" (
	set "VSINSTALLDIR=!_PGM_FILES_!\Microsoft Visual Studio\%_VSVER_%\Professional\"
	set "_VC_VARS_=!VSINSTALLDIR!VC\Auxiliary\Build\vcvars32.bat"
)
if not exist "!_VC_VARS_!" (
	set "VSINSTALLDIR=!_PGM_FILES_!\Microsoft Visual Studio\%_VSVER_%\Community\"
	set "_VC_VARS_=!VSINSTALLDIR!VC\Auxiliary\Build\vcvars32.bat"
)
call "%_VC_VARS_%"

call %_PYTHON_% --version 2>NUL
if errorlevel 1 (
	echo %_PYTHON_% not installed, using system python3.
	set _PYTHON_=python3
	
	call !_PYTHON_! --version 2>NUL
	if errorlevel 1 (
		echo !_PYTHON_! ALSO not installed, using system python.
		set _PYTHON_=python
	)
)
goto GenerateSLN

:GenerateSLN
echo:
echo ------------------------------------------------------------------
echo cleaning previous sln artifacts.
RD /S /Q "build-%_BUILDTYPE_%-sln"
echo ------------------------------------------------------------------
call %_PYTHON_% devtools\meson\meson.py setup --buildtype %_BUILDTYPE_% --backend vs%_VSVER_% build-%_BUILDTYPE_%-sln

REM now we post-process the meson output
call %_PYTHON_% devtools\vs_add_build_steps.py %_BUILDTYPE_%
call %_PYTHON_% devtools\vs_add_launch_config.py cl_dll\client.vcxproj hl %_BUILDTYPE_%

goto End

:End
echo:
echo ------------------------------------------------------------------
echo Work Complete.