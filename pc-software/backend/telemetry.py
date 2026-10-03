import asyncio
import math
import random
import time
from dataclasses import dataclass
from typing import AsyncIterator

G = 9.80665


async def precise_sleep(seconds: float) -> None:
    """Usa o timer de alta resolução do Python fora do event loop no Windows."""
    await asyncio.to_thread(time.sleep, seconds)


@dataclass
class Sample:
    t: float          # segundos desde o início da fonte
    lat: float        # LatAccel  m/s²  (direita positivo)
    lon: float        # LongAccel m/s²  (frente positivo)
    vert: float       # VertAccel m/s²  (cima positivo; inclui reação à gravidade)
    connected: bool
    pitch: float = 0.0  # radianos, nariz para cima positivo
    roll: float = 0.0   # radianos, lado esquerdo para cima positivo
    on_track: bool = False
    speed: float = 0.0  # m/s
    captured_ms: float = 0.0
    source_tick: int | None = None
    source_tick_rate: int | None = None


class MockSource:
    """Fonte sintética aleatória e suavizada, com override manual no frontend.

    Estado:
      manual_lat_g/manual_lon_g  → se != None, sobrescrevem qualquer geração.
      sine_hz, sine_amp_g        → sine no vertical (em g) somado à gravidade.
      auto_demo                  → quando True e sem override manual/sine,
                                   gera alvos aleatórios com rampas suaves.
    """

    def __init__(self, seed: int | None = None) -> None:
        self.manual_lat_g: float | None = None
        self.manual_lon_g: float | None = None
        self.sine_hz: float = 0.0
        self.sine_amp_g: float = 0.0
        self.auto_demo: bool = True
        self._rng = random.Random(seed)

    def set_manual(self, lat_g: float | None, lon_g: float | None) -> None:
        self.manual_lat_g = None if lat_g is None else float(lat_g)
        self.manual_lon_g = None if lon_g is None else float(lon_g)

    def release_manual(self) -> None:
        self.manual_lat_g = None
        self.manual_lon_g = None

    def set_sine(self, hz: float, amp_g: float) -> None:
        self.sine_hz = max(0.0, float(hz))
        self.sine_amp_g = max(0.0, float(amp_g))

    def set_auto_demo(self, enabled: bool) -> None:
        self.auto_demo = bool(enabled)

    def state(self) -> dict:
        return {
            "manual_lat_g": self.manual_lat_g,
            "manual_lon_g": self.manual_lon_g,
            "sine_hz": self.sine_hz,
            "sine_amp_g": self.sine_amp_g,
            "auto_demo": self.auto_demo,
        }

    async def samples(self, hz: float = 60.0) -> AsyncIterator[Sample]:
        dt = 1.0 / hz
        t0 = time.monotonic()
        next_sample = t0
        next_target = 0.0
        lat_g = lon_g = vert_ac_g = 0.0
        target_lat_g = target_lon_g = target_vert_ac_g = 0.0
        while True:
            t = time.monotonic() - t0

            if self.auto_demo and t >= next_target:
                # Faixas intencionalmente moderadas. A interpolação abaixo
                # limita jerk e evita degraus perigosos no tensionador.
                target_lat_g = self._rng.uniform(-0.80, 0.80)
                target_lon_g = self._rng.uniform(-1.00, 0.30)
                target_vert_ac_g = self._rng.uniform(-0.30, 0.35)
                next_target = t + self._rng.uniform(0.70, 1.80)

            # Filtro de 1ª ordem, tau ~= 250 ms, independente da taxa escolhida.
            alpha = 1.0 - math.exp(-dt / 0.25)
            if self.auto_demo:
                lat_g += (target_lat_g - lat_g) * alpha
                lon_g += (target_lon_g - lon_g) * alpha
                vert_ac_g += (target_vert_ac_g - vert_ac_g) * alpha
            else:
                lat_g += (0.0 - lat_g) * alpha
                lon_g += (0.0 - lon_g) * alpha
                vert_ac_g += (0.0 - vert_ac_g) * alpha

            if self.manual_lat_g is not None:
                lat = self.manual_lat_g * G
            else:
                lat = lat_g * G

            if self.manual_lon_g is not None:
                lon = self.manual_lon_g * G
            else:
                lon = lon_g * G

            vert = G * (1.0 + vert_ac_g)
            if self.sine_amp_g > 0 and self.sine_hz > 0:
                vert = G * (1.0 + self.sine_amp_g * math.sin(2 * math.pi * self.sine_hz * t))

            yield Sample(
                t=t, lat=lat, lon=lon, vert=vert, connected=True,
                pitch=0.0, roll=0.0, on_track=True, speed=30.0,
                captured_ms=time.time_ns() / 1_000_000,
            )
            next_sample += dt
            delay = next_sample - time.monotonic()
            if delay > 0:
                await precise_sleep(delay)
            else:
                next_sample = time.monotonic()


class IRacingSource:
    """Lê LatAccel/LongAccel/VertAccel da memória compartilhada do iRacing."""

    def __init__(self) -> None:
        try:
            import irsdk
        except ImportError as e:
            raise RuntimeError(
                "pyirsdk não instalado. Rode `pip install pyirsdk` ou use BELT_MOCK=1."
            ) from e
        self._ir = irsdk.IRSDK()

    async def samples(self, hz: float = 60.0) -> AsyncIterator[Sample]:
        dt = 1.0 / hz
        t0 = time.monotonic()
        next_sample = t0
        connected = False
        while True:
            if not connected:
                try:
                    # O SDK pode aguardar pelo simulador; não bloqueie o event loop
                    # nem impeça a interface/websocket de iniciar enquanto ele está offline.
                    connected = bool(await asyncio.to_thread(self._ir.startup))
                except Exception:
                    connected = False
            if connected and getattr(self._ir, "is_connected", False):
                try:
                    lat = float(self._ir["LatAccel"] or 0.0)
                    lon = float(self._ir["LongAccel"] or 0.0)
                    vert = float(self._ir["VertAccel"] or 0.0)
                    pitch = float(self._ir["Pitch"] or 0.0)
                    roll = float(self._ir["Roll"] or 0.0)
                    on_track = bool(self._ir["IsOnTrack"])
                    speed = float(self._ir["Speed"] or 0.0)
                    source_buffer = getattr(self._ir, "_var_buffer_latest", None)
                    header = getattr(self._ir, "_header", None)
                    yield Sample(
                        t=time.monotonic() - t0,
                        lat=lat, lon=lon, vert=vert,
                        connected=True,
                        pitch=pitch, roll=roll, on_track=on_track, speed=speed,
                        captured_ms=time.time_ns() / 1_000_000,
                        source_tick=(int(source_buffer.tick_count) if source_buffer is not None else None),
                        source_tick_rate=(int(header.tick_rate) if header is not None else None),
                    )
                except Exception:
                    connected = False
                    yield Sample(
                        t=time.monotonic() - t0, lat=0, lon=0, vert=0, connected=False,
                        captured_ms=time.time_ns() / 1_000_000,
                    )
            else:
                connected = False
                yield Sample(
                    t=time.monotonic() - t0, lat=0, lon=0, vert=0, connected=False,
                    captured_ms=time.time_ns() / 1_000_000,
                )
            next_sample += dt
            delay = next_sample - time.monotonic()
            if delay > 0:
                await precise_sleep(delay)
            else:
                next_sample = time.monotonic()


class OfflineSource:
    """Fonte inerte usada quando o iRacing não pode ser inicializado.

    Nunca produz estímulos sintéticos: o modo de teste só é criado por uma
    solicitação explícita da interface.
    """

    async def samples(self, hz: float = 60.0) -> AsyncIterator[Sample]:
        dt = 1.0 / hz
        t0 = time.monotonic()
        while True:
            yield Sample(
                t=time.monotonic() - t0, lat=0, lon=0, vert=0,
                connected=False, on_track=False,
                captured_ms=time.time_ns() / 1_000_000,
            )
            await precise_sleep(dt)
