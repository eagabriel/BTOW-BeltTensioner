import json
import os
from dataclasses import asdict
from pathlib import Path
from typing import Any, Callable, TypeVar


T = TypeVar("T")
CONFIG_VERSION = 1


def config_directory() -> Path:
    override = os.environ.get("HOVERBELT_CONFIG_DIR")
    if override:
        return Path(override).expanduser().resolve()
    base = Path(os.environ.get("LOCALAPPDATA", Path.home() / "AppData" / "Local"))
    return base / "HoverBelt"


def load_config(path: Path, default: T, updater: Callable[[T, dict], T]) -> tuple[T, str | None]:
    if not path.exists():
        return default, None
    try:
        payload = json.loads(path.read_text(encoding="utf-8"))
        values: Any = payload.get("config", payload) if isinstance(payload, dict) else None
        if not isinstance(values, dict):
            raise ValueError("o JSON não contém um objeto de configuração")
        return updater(default, values), None
    except Exception as exc:
        return default, f"Não foi possível carregar {path.name}: {exc}"


def save_config(path: Path, config: object) -> str | None:
    try:
        path.parent.mkdir(parents=True, exist_ok=True)
        temporary = path.with_suffix(path.suffix + ".tmp")
        payload = {"version": CONFIG_VERSION, "config": asdict(config)}
        temporary.write_text(json.dumps(payload, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
        os.replace(temporary, path)
        return None
    except Exception as exc:
        return f"Não foi possível salvar {path.name}: {exc}"
