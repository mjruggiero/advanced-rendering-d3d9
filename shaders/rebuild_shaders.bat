@echo off
setlocal

REM ============================================================
REM Rebuild Direct3D 9 shader assembly files.
REM This script lives inside the shaders folder.
REM
REM Original project convention:
REM   .fx  -> vertex shader assembly .vsh
REM   .fxp -> pixel shader assembly .psh
REM
REM HDR.fx is loaded at runtime as an effect and is not compiled here.
REM ============================================================

set "SHADER_DIR=%~dp0"

REM Prefer the June 2010 DirectX SDK compiler.
REM DXSDK_DIR usually ends with a trailing backslash.
set "FXC=%DXSDK_DIR%Utilities\bin\x64\fxc.exe"

if not exist "%FXC%" (
    set "FXC=%DXSDK_DIR%Utilities\bin\x86\fxc.exe"
)

if not exist "%FXC%" (
    echo ERROR: fxc.exe was not found.
    echo.
    echo Checked:
    echo "%DXSDK_DIR%Utilities\bin\x64\fxc.exe"
    echo "%DXSDK_DIR%Utilities\bin\x86\fxc.exe"
    echo.
    echo Make sure the June 2010 DirectX SDK is installed and DXSDK_DIR is set.
    exit /b 1
)

echo FXC="%FXC%"

echo.
echo Rebuilding shaders in "%SHADER_DIR%"...
echo.

call :CompileVS ColorNOneTexture
if errorlevel 1 exit /b 1

call :CompileVS CookTorrance
if errorlevel 1 exit /b 1

call :CompilePS CookTorrance
if errorlevel 1 exit /b 1

call :CompileVS CreateShadowMap
if errorlevel 1 exit /b 1

call :CompilePS CreateShadowMap
if errorlevel 1 exit /b 1

call :CompileVS diffuse
if errorlevel 1 exit /b 1

call :CompilePS diffuse
if errorlevel 1 exit /b 1

call :CompileVS diffuseSpecular
if errorlevel 1 exit /b 1

call :CompilePS diffuseSpecular
if errorlevel 1 exit /b 1

call :CompilePS OnlyOneTexture
if errorlevel 1 exit /b 1

call :CompileVS Sphere
if errorlevel 1 exit /b 1

echo.
echo Shader rebuild complete.
exit /b 0


:CompileVS
set "NAME=%~1"
set "SRC=%SHADER_DIR%%NAME%.fx"
set "OUT=%SHADER_DIR%%NAME%.vsh"

if not exist "%SRC%" (
    echo WARNING: Missing "%SRC%" -- skipping vertex shader.
    exit /b 0
)

echo Compiling VS: %NAME%.fx -^> %NAME%.vsh
"%FXC%" /nologo /T vs_1_1 /E VS /Fc "%OUT%" "%SRC%"

if errorlevel 1 (
    echo ERROR: Failed compiling vertex shader "%SRC%"
    exit /b 1
)

exit /b 0


:CompilePS
set "NAME=%~1"
set "SRC=%SHADER_DIR%%NAME%.fxp"
set "OUT=%SHADER_DIR%%NAME%.psh"

if not exist "%SRC%" (
    echo WARNING: Missing "%SRC%" -- skipping pixel shader.
    exit /b 0
)

echo Compiling PS: %NAME%.fxp -^> %NAME%.psh
"%FXC%" /nologo /T ps_2_a /E PS /Fc "%OUT%" "%SRC%"

if errorlevel 1 (
    echo ERROR: Failed compiling pixel shader "%SRC%"
    exit /b 1
)

exit /b 0
