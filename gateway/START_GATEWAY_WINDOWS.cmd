@echo off
setlocal
cd /d "%~dp0"

where python >nul 2>nul
if errorlevel 1 (
  echo Python nie zostal znaleziony w PATH.
  echo Uzyj X4DataGateway.exe z artifactu GitHub Actions albo zainstaluj Python 3.12+.
  pause
  exit /b 1
)

python windows_app.py
