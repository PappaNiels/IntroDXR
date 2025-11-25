@echo off
setlocal

set SCRIPT_DIR=%~dp0

set TARGET_DIR=%SCRIPT_DIR%build\x64\Release
set TARGET_DIR_DEBUG=%SCRIPT_DIR%build\x64\Debug
set ASSETS_DIR=%SCRIPT_DIR%assets

set LINK_PATH=%TARGET_DIR%\assets
set LINK_PATH_DEBUG=%TARGET_DIR_DEBUG%\assets

if not exist "%TARGET_DIR%" (
    mkdir "%TARGET_DIR%"
)

if not exist "%TARGET_DIR_DEBUG%" (
    mkdir "%TARGET_DIR_DEBUG%"
)

if not exist "%LINK_PATH%" (
    mklink /D "%LINK_PATH%" "%ASSETS_DIR%"
)

if not exist "%LINK_PATH_DEBUG%" (
    mklink /D "%LINK_PATH_DEBUG%" "%ASSETS_DIR%"
)

pause