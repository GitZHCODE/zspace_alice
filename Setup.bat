@echo off

:: Create paths.lua if it doesn't exist
if not exist "paths.lua" (
	echo Writing paths.lua

	echo --Define Paths relative to this file>> paths.lua
	echo.>> paths.lua
	echo viewer_path = "Alice">> paths.lua
	echo viewer_deps_path = "Dependencies">> paths.lua
	echo exe_path = "EXE">> paths.lua
	echo zspace_core_path = "../zspace_core">> paths.lua
	echo zspace_toolsets_path = "../zspace_toolsets">> paths.lua
	echo zspace_deps_path = "../zspace_dependencies">> paths.lua
	echo sketches_path = "Sketches">> paths.lua
	echo.>> paths.lua
	echo rhino_dir = "C:/Program Files/Rhino 7 SDK">> paths.lua
	echo maya_dir = "C:/Program Files/Autodesk/Maya2023">> paths.lua
)

:: Get the omniverse path from paths.lua
setlocal enabledelayedexpansion

set paths_file=paths.lua
set zspace_deps_path=

:: Find zspace_deps path
REM Read the paths.lua file line by line
for /f "tokens=1,2 delims== " %%A in (%paths_file%) do (
    if "%%A"=="zspace_deps_path" (
        set zspace_deps_path=%%B
        REM Remove quotes from the path
        set zspace_deps_path=!zspace_deps_path:"=!
    )
)

rem Check if the path was extracted successfully
if "%zspace_deps_path%"=="" (
    echo Failed to extract the path for zspace_deps_path
    PAUSE
    exit
)

:: Create the Omnidlls folder if it doesn't exist or is empty
:: Test to see if plugin.info has been copied
set omni=false
if not exist "EXE\lib_omniverse" set omni=true
if not exist "EXE\lib_omniverse\python310.dll" set omni=true
if not exist "EXE\lib_omniverse\usd\ar\resources\plugInfo.json" set omni=true
if not exist "EXE\lib_omniverse\usd\omniverse\resources\plugInfo.json" set omni=true
if "%omni%"=="true" (
	if not exist "EXE\lib_omniverse" mkdir "EXE\lib_omniverse"       	

	echo Moving Omniverse DLLs...
	:: Copy omniverse DLLs
	for /r "%zspace_deps_path%\omniverse" %%f in (*.dll) do (
		xcopy "%%f" "EXE\lib_omniverse" /y >nul 2>&1
	)

	echo Copying Plugin Infos...
	:: Copy main plugin info usd folder
	xcopy "%zspace_deps_path%\omniverse\usd\release\lib\usd\" "EXE\lib_omniverse\usd\" /s /e
	:: Copy omniusdreader plugin folder
	xcopy "%zspace_deps_path%\omniverse\omni_usd_resolver\release\usd\omniverse\" "EXE\lib_omniverse\usd\omniverse\" /s /e
)

:: Run Premake
echo Running Premake
if exist "C:\Program Files\Rhino 7 SDK" (
	premake\premake5.exe vs2019 --interop=Rhino
) else (
	premake\premake5.exe vs2019 --interop=Default
)

PAUSE
