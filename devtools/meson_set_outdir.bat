@if "%overbose%" == "" echo off

set "_PROJECT_=%1"
set "_PROJECT_PATH_=%2"
set "_PROJECT_OUTPUT_=%3"

pushd
cd %_PROJECT_PATH_%
echo %_PROJECT_%=%_PROJECT_OUTPUT_% > OUTDIR

popd
