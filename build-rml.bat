@echo off
rem Build do fork com RmlUi (Windows, Visual Studio 18 / MSVC v145 + vcpkg).
rem Uso: build-rml.bat          -> configura e compila
rem      build-rml.bat build    -> so compila
set VSROOT=C:\Program Files\Microsoft Visual Studio\18\Community
call "%VSROOT%\VC\Auxiliary\Build\vcvars64.bat" >nul
if not defined VCPKG_ROOT set VCPKG_ROOT=C:\vcpkg
set PATH=%VSROOT%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;%VSROOT%\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja;%PATH%
cd /d "%~dp0"
if "%1"=="build" goto build
cmake --preset rml-release || exit /b 1
:build
cmake --build build/rml-release --parallel || exit /b 1
