
@echo off

cls

set pauseOpt=nopause

SET bDirVar=build-solution-x64

echo Removing Old Build Directory

rd %bDirVar% /Q /S

mkdir %bDirVar%

cd %bDirVar%

if exist "%CMAKE_PATH%/cmake.exe" (
"%CMAKE_PATH%/cmake" .. -G "Visual Studio 15 2017 Win64"
goto generated
)

cmake .. -G "Visual Studio 15 2017 Win64"

:generated

cd ..

if NOT %pauseOpt% == nopause (
    pause
)
