import threading

from serial import Serial, SerialException
from serial.tools import list_ports


class MotorLink:
    """Cliente serial pro controlador dos motores do cinto.

    Protocolo (conforme Src/util.c:1676-1691 do firmware):
      TY<v>\\n  → torque motor esquerdo (pwml), v ∈ [-32767, 32767]
      TX<v>\\n  → torque motor direito  (pwmr)
      T<v>\\n   → ambos
      E\\n      → para (libera override); EX/EY parciais
    """

    def __init__(self) -> None:
        self._ser: Serial | None = None
        self._lock = threading.Lock()
        self.last_reply: str = ""
        self.port: str = ""
        self.baud: int = 0
        self.last_error: str = ""

    @staticmethod
    def list_ports() -> list[dict]:
        return [
            {"device": p.device, "description": p.description or ""}
            for p in list_ports.comports()
        ]

    @property
    def is_open(self) -> bool:
        return bool(self._ser and self._ser.is_open)

    def open(self, port: str, baud: int = 115200) -> None:
        self.close()
        try:
            self._ser = Serial(port, baudrate=baud, timeout=0.02, write_timeout=0.05)
            self.port = port
            self.baud = baud
            self.last_error = ""
        except SerialException as e:
            self._ser = None
            self.last_error = str(e)
            raise

    def close(self) -> None:
        with self._lock:
            if self._ser and self._ser.is_open:
                try:
                    self._ser.write(b"E\n")
                    self._ser.flush()
                except Exception:
                    pass
                try:
                    self._ser.close()
                except Exception:
                    pass
            self._ser = None
            self.port = ""

    def stop(self) -> None:
        self._write("E\n")

    def send_pair(self, left: int, right: int) -> None:
        if left == right:
            self._write(f"T{int(left)}\n")
        else:
            self._write(f"TY{int(left)}\n")
            self._write(f"TX{int(right)}\n")

    def _write(self, s: str) -> None:
        with self._lock:
            if not (self._ser and self._ser.is_open):
                return
            try:
                self._ser.write(s.encode("ascii"))
                data = self._ser.read(128)
                if data:
                    self.last_reply = data.decode("ascii", errors="replace").strip()
            except SerialException as e:
                self.last_error = str(e)
                try:
                    self._ser.close()
                except Exception:
                    pass
                self._ser = None
                self.port = ""

    def status(self) -> dict:
        return {
            "connected": self.is_open,
            "port": self.port,
            "baud": self.baud,
            "reply": self.last_reply,
            "error": self.last_error,
        }
