@echo off
cd /d "%~dp0"

start "" cmd /c "timeout /t 2 /nobreak >nul && start http://localhost:8000"

echo BTOW - Belt Tensioner rodando em http://localhost:8000
echo (fecha esta janela ou Ctrl+C para encerrar)
echo.
python -m uvicorn backend.server:app --host 127.0.0.1 --port 8000
