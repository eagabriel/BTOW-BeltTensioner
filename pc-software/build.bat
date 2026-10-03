@echo off
setlocal
cd /d "%~dp0"

echo === Instalando/atualizando dependencias de build ===
python -m pip install -q -r requirements.txt pyinstaller
if errorlevel 1 goto :err

echo.
echo === Limpando builds anteriores ===
if exist build rmdir /s /q build
if exist dist rmdir /s /q dist
if exist HoverBelt.spec del /q HoverBelt.spec

echo.
echo === PyInstaller ===
python -m PyInstaller ^
  --onefile ^
  --name HoverBelt ^
  --noconsole ^
  --icon "assets\BTOW.ico" ^
  --add-data "frontend;frontend" ^
  --hidden-import irsdk ^
  --hidden-import serial.tools.list_ports ^
  --hidden-import webview.platforms.edgechromium ^
  --collect-submodules uvicorn ^
  --collect-submodules fastapi ^
  --collect-submodules webview ^
  app.py
if errorlevel 1 goto :err

echo.
echo === OK ===
echo Executavel: %~dp0dist\HoverBelt.exe
echo (Requer WebView2 Runtime instalado — presente por padrao no Windows 11)
echo.
pause
exit /b 0

:err
echo.
echo *** FALHOU ***
pause
exit /b 1
