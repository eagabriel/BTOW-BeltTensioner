import asyncio
import json
import os
import shutil
import time
from contextlib import asynccontextmanager
from dataclasses import asdict
from pathlib import Path

from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from fastapi.responses import FileResponse
from fastapi.staticfiles import StaticFiles

from .mapping import (AdvancedMapper, AdvancedMappingConfig, MappingConfig,
                      Torques, advanced_cfg_update, cfg_to_dict, cfg_update, compute)
from .config_store import config_directory, load_config, save_config
from .telemetry import IRacingSource, MockSource, OfflineSource

FRONTEND = Path(__file__).resolve().parent.parent / "frontend"
USE_MOCK = os.environ.get("BELT_MOCK", "0") == "1"
RATE_HZ = float(os.environ.get("BELT_HZ", "60"))


class State:
    def __init__(self):
        config_dir = config_directory()
        self.config_paths = {
            "belt": config_dir / "belt_tensioner.json",
            "advanced": config_dir / "belt_advanced.json",
        }
        legacy_advanced_path = config_dir / ("belt_" + "mai" + "ra.json")
        if not self.config_paths["advanced"].exists() and legacy_advanced_path.exists():
            try:
                shutil.copy2(legacy_advanced_path, self.config_paths["advanced"])
            except OSError:
                pass
        self.cfg, belt_error = load_config(self.config_paths["belt"], MappingConfig(), cfg_update)
        self.advanced_cfg, advanced_error = load_config(
            self.config_paths["advanced"], AdvancedMappingConfig(), advanced_cfg_update
        )
        self.config_errors = {"belt": belt_error, "advanced": advanced_error}
        if belt_error is None and not self.config_paths["belt"].exists():
            self.config_errors["belt"] = save_config(self.config_paths["belt"], self.cfg)
        if advanced_error is None and not self.config_paths["advanced"].exists():
            self.config_errors["advanced"] = save_config(self.config_paths["advanced"], self.advanced_cfg)
        self.advanced_mapper = AdvancedMapper()
        self.last_torques = Torques(0, 0)
        self.last_advanced_torques = Torques(0, 0)
        self.source = None
        self.mock_enabled = False
        self.source_error = None

    def enable_test_mode(self):
        self.source = MockSource()
        self.mock_enabled = True
        self.source_error = None

    def enable_iracing_mode(self):
        try:
            self.source = IRacingSource()
            self.mock_enabled = False
            self.source_error = None
        except Exception as e:
            self.source = OfflineSource()
            self.mock_enabled = False
            self.source_error = str(e)
            print(f"[telemetry] iRacing indisponível ({e}); saída mantida inerte.")

    def setup_source(self):
        if USE_MOCK:
            self.enable_test_mode()
            return
        self.enable_iracing_mode()

    def snapshot(self) -> dict:
        snap = {
            "config": cfg_to_dict(self.cfg),
            "torque": {"left": self.last_torques.left, "right": self.last_torques.right},
            "torque_advanced": {"left": self.last_advanced_torques.left, "right": self.last_advanced_torques.right},
            "advanced_config": asdict(self.advanced_cfg),
            "auto_scale": self.advanced_mapper.auto_scale_state(),
            "mock_enabled": self.mock_enabled,
            "source_error": self.source_error,
            "config_files": {
                key: {"path": str(path), "error": self.config_errors[key]}
                for key, path in self.config_paths.items()
            },
        }
        if isinstance(self.source, MockSource):
            snap["mock"] = self.source.state()
        return snap


state = State()


class Broadcaster:
    def __init__(self):
        self.clients: set[WebSocket] = set()

    async def add(self, ws: WebSocket):
        await ws.accept()
        self.clients.add(ws)

    def remove(self, ws: WebSocket):
        self.clients.discard(ws)

    async def send(self, payload: dict):
        text = json.dumps(payload)
        dead = []
        for ws in list(self.clients):
            try:
                await ws.send_text(text)
            except Exception:
                dead.append(ws)
        for ws in dead:
            self.remove(ws)


bcast = Broadcaster()


async def producer():
    if state.source is None:
        state.setup_source()
    sequence = 0
    while True:
        active_source = state.source
        async for sample in active_source.samples(RATE_HZ):
            # A mensagem test_mode troca state.source. Encerre o gerador antigo
            # e deixe o laço externo iniciar imediatamente a nova fonte.
            if active_source is not state.source:
                break
            compute_started_ns = time.perf_counter_ns()
            torques = compute(sample, state.cfg)
            advanced_torques = state.advanced_mapper.compute(sample, state.advanced_cfg)
            compute_us = (time.perf_counter_ns() - compute_started_ns) / 1_000
            state.last_torques = torques
            state.last_advanced_torques = advanced_torques
            if bcast.clients:
                sequence += 1
                payload = {
                    "type": "telemetry",
                    "seq": sequence,
                    "backend_send_ms": time.time_ns() / 1_000_000,
                    "backend_compute_us": compute_us,
                    **asdict(sample),
                    "torque": {"left": torques.left, "right": torques.right},
                    "torque_advanced": {"left": advanced_torques.left, "right": advanced_torques.right},
                    "auto_scale": state.advanced_mapper.auto_scale_state(),
                    "mock_enabled": state.mock_enabled,
                }
                if isinstance(active_source, MockSource):
                    payload["mock"] = active_source.state()
                await bcast.send(payload)


@asynccontextmanager
async def lifespan(app: FastAPI):
    task = asyncio.create_task(producer())
    try:
        yield
    finally:
        task.cancel()
        try:
            await task
        except (asyncio.CancelledError, Exception):
            pass


app = FastAPI(lifespan=lifespan)
app.mount("/static", StaticFiles(directory=FRONTEND), name="static")


@app.get("/")
def index():
    return FileResponse(FRONTEND / "index.html")


async def _handle_client_msg(msg: dict):
    t = msg.get("type")

    if t == "config":
        state.cfg = cfg_update(state.cfg, msg.get("patch", {}))
        if bool(msg.get("persist", False)):
            state.config_errors["belt"] = save_config(state.config_paths["belt"], state.cfg)
        return {"type": "state", **state.snapshot()}

    if t == "advanced_config":
        state.advanced_cfg = advanced_cfg_update(state.advanced_cfg, msg.get("patch", {}))
        state.advanced_mapper.reset()
        if bool(msg.get("persist", False)):
            state.config_errors["advanced"] = save_config(state.config_paths["advanced"], state.advanced_cfg)
        return {"type": "state", **state.snapshot()}

    if t == "test_mode":
        if bool(msg.get("enabled", False)):
            state.enable_test_mode()
        else:
            state.enable_iracing_mode()
        return {"type": "state", **state.snapshot()}

    if t == "mock_manual":
        if isinstance(state.source, MockSource):
            state.source.set_manual(msg.get("lat_g"), msg.get("lon_g"))
        return None

    if t == "mock_release":
        if isinstance(state.source, MockSource):
            state.source.release_manual()
        return None

    if t == "mock_sine":
        if isinstance(state.source, MockSource):
            state.source.set_sine(msg.get("hz", 0), msg.get("amp_g", 0))
        return {"type": "state", **state.snapshot()}

    if t == "mock_auto":
        if isinstance(state.source, MockSource):
            state.source.set_auto_demo(bool(msg.get("enabled", True)))
        return {"type": "state", **state.snapshot()}

    return None


@app.websocket("/ws")
async def ws_endpoint(ws: WebSocket):
    await bcast.add(ws)
    try:
        await ws.send_text(json.dumps({"type": "state", **state.snapshot()}))
        while True:
            raw = await ws.receive_text()
            try:
                msg = json.loads(raw)
            except json.JSONDecodeError:
                continue
            reply = await _handle_client_msg(msg)
            if reply:
                await ws.send_text(json.dumps(reply))
    except WebSocketDisconnect:
        pass
    except Exception:
        pass
    finally:
        bcast.remove(ws)
