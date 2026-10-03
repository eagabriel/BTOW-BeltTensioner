"""Entry point pra empacotamento: sobe uvicorn em thread e abre janela WebView2.

Quando executado como .exe (PyInstaller), os assets de `frontend/` são extraídos
pra sys._MEIPASS pelo próprio PyInstaller — `backend.server` já resolve o path
via __file__, então nada muda.

Erros no boot vão pra btow-error.log ao lado do .exe.
"""
import os
import socket
import sys
import threading
import time
import traceback
from pathlib import Path


_server_failed = threading.Event()
_server_error: BaseException | None = None


def _exe_dir() -> Path:
    if getattr(sys, "frozen", False):
        return Path(sys.executable).resolve().parent
    return Path(__file__).resolve().parent


def _log_error(exc: BaseException) -> None:
    try:
        path = _exe_dir() / "btow-error.log"
        with path.open("w", encoding="utf-8") as f:
            f.write(f"BTOW crash em {time.strftime('%Y-%m-%d %H:%M:%S')}\n")
            f.write(f"Python: {sys.version}\n")
            f.write(f"Executable: {sys.executable}\n")
            f.write(f"frozen: {getattr(sys, 'frozen', False)}\n")
            f.write(f"_MEIPASS: {getattr(sys, '_MEIPASS', None)}\n\n")
            f.write("Traceback:\n")
            traceback.print_exception(type(exc), exc, exc.__traceback__, file=f)
    except Exception:
        pass


def _wait_port(host: str, port: int, timeout: float = 15.0) -> bool:
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        try:
            with socket.create_connection((host, port), timeout=0.3):
                return True
        except OSError:
            time.sleep(0.1)
    return False


def _run_server() -> None:
    global _server_error
    try:
        import uvicorn
        from backend.server import app
        # Builds PyInstaller --noconsole não possuem sys.stdout/sys.stderr.
        # A configuração padrão de logging do Uvicorn tenta abrir esses streams
        # e pode abortar antes de criar o socket. O aplicativo já grava falhas em
        # btow-error.log, então o servidor embutido não precisa desse logger.
        uvicorn.run(
            app,
            host="127.0.0.1",
            port=8000,
            log_config=None,
            access_log=False,
        )
    except BaseException as e:
        _server_error = e
        _server_failed.set()


def main() -> int:
    os.environ.setdefault("BELT_MOCK", "0")

    threading.Thread(target=_run_server, daemon=True).start()
    if not _wait_port("127.0.0.1", 8000):
        if _server_failed.is_set() and _server_error is not None:
            raise RuntimeError("uvicorn falhou durante a inicialização") from _server_error
        raise RuntimeError("uvicorn não subiu em 15s e não reportou uma exceção")

    import webview
    webview.create_window(
        "BTOW — Belt Tensioner",
        "http://127.0.0.1:8000",
        width=1400, height=900, resizable=True,
    )
    webview.start()
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except BaseException as e:
        _log_error(e)
        sys.exit(1)
