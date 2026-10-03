@echo off
setlocal
cd /d "%~dp0"

echo === Build DEBUG (com console, sem --noconsole) ===
echo === Deps ===
python -m pip install -q -r requirements.txt pyinstaller
if errorlevel 1 goto :err

echo.
echo === Limpando ===
if exist build rmdir /s /q build
if exist dist rmdir /s /q dist
if exist BTOW-debug.spec del /q BTOW-debug.spec

echo.
echo === PyInstaller ===
python -m PyInstaller ^
  --onefile ^
  --name BTOW-debug ^
  --icon "assets\BTOW.ico" ^
  --add-data "frontend;frontend" ^
  --hidden-import irsdk ^
  --hidden-import serial.tools.list_ports ^
  --hidden-import webview.platforms.edgechromium ^
  --collect-submodules uvicorn ^
  --collect-submodules fastapi ^
  --collect-submodules webview ^
  --collect-all pywebview ^
  app.py
if errorlevel 1 goto :err

echo.
echo === OK ===
echo dist\BTOW-debug.exe (console visivel — rode dele mesmo pra ver erros)
echo.
pause
exit /b 0

:err
echo.
echo *** FALHOU ***
pause
exit /b 1
