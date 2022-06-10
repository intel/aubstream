REM
REM Copyright (C) 2022 Intel Corporation
REM
REM SPDX-License-Identifier: MIT
REM

@echo off

setlocal enabledelayedexpansion

:: Defaults
set USE_NINJA=0
set BUILD_TYPE=Debug
set BITS=64
set VS=2022
set DO_BUILD=0

:: Parse arguments
goto :arg_parse
:arg_loop
shift
:arg_parse
if "%~1" == "" goto :configure
if "%~1" == "-ninja" (
    set USE_NINJA=1
    goto :arg_loop
)
if "%~1" == "-build" (
    set BUILD_TYPE=%~2
    shift
    goto :arg_loop
)
if "%~1" == "-bits" (
    set BITS=%~2
    shift
    goto :arg_loop
)
if "%~1" == "-vs" (
    set VS=%~2
    shift
    goto :arg_loop
)
if "%~1" == "-do-build" (
    set DO_BUILD=1
    goto :arg_loop
)

if "%~1" == "-h" (
    goto :Usage
)
if "%~1" == "-help" (
    goto :Usage
)
echo Ignoring unknown argument: %1
goto :arg_loop

:configure

set PLATFORM=Windows%BITS%
if "%USE_NINJA%" == "1" (
    set COMPILER=Ninja
    set PLATFORM_FLAGS=-DCMAKE_BUILD_TYPE=%BUILD_TYPE%
    set PLATFORM=%PLATFORM%\%BUILD_TYPE%
    goto :configure_2
)

if "%VS%" == "2019" (
    set COMPILER=Visual Studio 16 2019
    set PLATFORM_FLAGS=-Thost=x64
    if "%BITS%" == "64" (
        set PLATFORM_FLAGS=!PLATFORM_FLAGS! -Ax64
    ) else (
        set PLATFORM_FLAGS=!PLATFORM_FLAGS! -Awin32
    )
) else if "%VS%" == "2022" (
    set COMPILER=Visual Studio 17 2022
    set PLATFORM_FLAGS=-Thost=x64
    if "%BITS%" == "64" (
        set PLATFORM_FLAGS=!PLATFORM_FLAGS! -Ax64
    ) else (
        set PLATFORM_FLAGS=!PLATFORM_FLAGS! -Awin32
    )
) else (
    set COMPILER=Visual Studio 15 2017
    set PLATFORM_FLAGS=-Thost=x64
    if "%BITS%" == "64" (
        set COMPILER=!COMPILER! Win64
    )
)

:configure_2
set PROJECT_DIRECTORY=%~dp0
if "%BRANCH%" == "" (
    set BUILD_DIRECTORY=%PROJECT_DIRECTORY%build_master\%PLATFORM%
    set BRANCH_FLAGS=
) else (
    set BUILD_DIRECTORY=%PROJECT_DIRECTORY%build\%PLATFORM%
    set BRANCH_FLAGS=-DBRANCH_TYPE=%BRANCH%
)

set CMAKE_EXE="cmake.exe"

echo -- Looking for cmake.exe
WHERE cmake.exe
IF %ERRORLEVEL% NEQ 0 set SUCCESS=1 & echo ERROR: Unable to find cmake at %CMAKE_EXE% & goto :FAIL
echo -- Looking for cmake.exe -- found

if not exist %BUILD_DIRECTORY% mkdir %BUILD_DIRECTORY%

pushd .
cd %BUILD_DIRECTORY%
:: echo %CMAKE_EXE% -Wno-dev -G "%COMPILER%" %PLATFORM_FLAGS% %BRANCH_FLAGS% -DBUILD_BITS=%BITS% %PROJECT_DIRECTORY% -Daubstream_build_tests=1
%CMAKE_EXE% -G "%COMPILER%" %PLATFORM_FLAGS% %BRANCH_FLAGS% -DBUILD_BITS=%BITS% %PROJECT_DIRECTORY% -Daubstream_build_tests=1
if !ERRORLEVEL! NEQ 0 set SUCCESS=!ERRORLEVEL! & popd & echo ERROR: cmake failed with !ERRORLEVEL!.  Exiting... & goto :FAIL
popd

:: Perform Build
set BUILD_FLAGS=
if "%USE_NINJA%" == "1" (
    set BUILD_FLAGS=-- all aub_stream aub_stream_all_hw
) else (
    set BUILD_FLAGS=--config %BUILD_TYPE%
)
if "%DO_BUILD%" == "1" (
    %CMAKE_EXE% --build "%BUILD_DIRECTORY%" %BUILD_FLAGS%
)

:EXIT
EXIT /B %ERRORLEVEL%

:FAIL
echo.
echo Error
EXIT /B 1

:Usage
set script=build.bat
echo Usage: %script% ^[OPTIONS^]
echo Valid Options:
echo    -vs ^<2017^|2019^>         Select Visual Studio 2017 or Visual Studio 2019
echo                               ^(ignored when using -ninja^) ^[default: 2019^]
echo    -ninja                  Use ninja as the build tool.  Must be run from developer command prompt.
echo                               ^[default: off^]
echo    -bits ^<32^|64^>           Select 32-bit or 64-bit build. ^[default: 64^]
echo                               ^(ignored when using -ninja^)
echo    -build ^<Debug^|Release^|ReleaseInternal^>
echo                            Select build type. ^[default: Debug^]
echo    -do-build               Perform build after cmake configuration. ^[default: off^]
echo    -h, -help               Display this help message and exit
exit /B 0
