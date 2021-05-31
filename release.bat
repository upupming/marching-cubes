@REM https://stackoverflow.com/a/56499225/8242705
:: set the path to your visual studio vcvars script, it is different for every version of Visual Studio.
set VSTOOLS="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
set WIN_DEPLOY_QT="C:/Qt/5.15.2/msvc2019_64/bin/windeployqt.exe"
@REM https://stackoverflow.com/a/203116/8242705
For /f "tokens=1-3 delims=/ " %%a in ('date /t') do (set mydate=%%c-%%a-%%b)
For /f "tokens=1-2 delims=/:" %%a in ("%TIME%") do (set mytime=%%a-%%b)
set APP_NAME=vscode-qt-qml-vcpkg-template
set RELEASE_NAME=%APP_NAME%-release-%mydate%_%mytime%

:: make sure we found them
if not exist %VSTOOLS% (
    echo VS 2017 Build Tools are missing!
    exit
)

:: call that script, which essentially sets up the VS Developer Command Prompt
WHERE cl.exe
IF %ERRORLEVEL% NEQ 0 call %VSTOOLS%

:: Set some variables for the source directory and the build directory
set SrcDir=%CD%
set BuildDir=%CD%\build-release

:: Make the build directory if it doesn't exist
if not exist "%BuildDir%" mkdir "%BuildDir%"

:: Make sure you configure with CMake from the build directory
cd "%BuildDir%"

:: Call CMake to configure the build (generates the build scripts)
cmake -S %SrcDir% -B %BuildDir% -GNinja -DCMAKE_BUILD_TYPE=Release

:: Call CMake again to build the project
cmake --build %BuildDir%

%WIN_DEPLOY_QT% --release --qmldir %SrcDir% %BuildDir%/%APP_NAME%.exe

zip -r %SrcDir%/%RELEASE_NAME%.zip . -x .\.cmake\* .\CMakeFiles\* .\cmake_install.cmake .\CMakeCache.txt .\.ninja_deps .\.ninja_log .\build.ninja .\*\*autogen*\* .\vcpkg_installed\* .\*.log .\*.manifest .\compile_commands.json

exit
