from dataclasses import dataclass, asdict
import math

from .telemetry import Sample

G = 9.80665
TORQUE_MAX = 32767


@dataclass
class MappingConfig:
    pretension: int = 2000        # torque base em ambos os lados (cinto posicionado)
    k_lat: float = 12000.0        # torque adicional no lado externo por g lateral
    k_lon_brake: float = 8000.0   # torque adicional em ambos por g de frenagem
    k_vert: float = 6000.0        # torque adicional em ambos por g de |vert AC|
    invert_lat: bool = False      # inverte qual lado aperta (ajuste físico)
    deadzone_g: float = 0.05      # zona morta em g (lat)
    vert_deadzone_g: float = 0.1  # zona morta em g (vert AC) — evita ruído/gravidade


@dataclass
class Torques:
    left: int
    right: int


def _clamp(v: float) -> int:
    if v > TORQUE_MAX:
        return TORQUE_MAX
    if v < 0:
        return 0
    return int(v)


def compute(sample: Sample, cfg: MappingConfig) -> Torques:
    """G → torque por lado. Invariante: torque ≥ pretension em ambos os lados.

    O cinto é unipolar (só puxa). Aplicar torque no sentido oposto afrouxa e
    perde tração — por isso o efeito lateral é aditivo apenas no lado que
    precisa apertar; o outro fica na baseline (pré-tensão + adicional de freio).
    """
    if not sample.connected or not sample.on_track:
        return Torques(left=0, right=0)

    lat_g = sample.lat / G
    lon_g = sample.lon / G
    vert_ac_g = (sample.vert / G) - 1.0

    if abs(lat_g) < cfg.deadzone_g:
        lat_g = 0.0
    if abs(vert_ac_g) < cfg.vert_deadzone_g:
        vert_ac_g = 0.0

    baseline = float(cfg.pretension)
    if lon_g < 0:
        baseline += cfg.k_lon_brake * (-lon_g)
    baseline += cfg.k_vert * abs(vert_ac_g)

    lat_effect = cfg.k_lat * abs(lat_g)
    lat_side = -lat_g if cfg.invert_lat else lat_g

    left = baseline
    right = baseline
    if lat_side > 0:
        right += lat_effect
    elif lat_side < 0:
        left += lat_effect

    return Torques(left=_clamp(left), right=_clamp(right))


@dataclass
class AdvancedMappingConfig:
    """Mapeamento avançado com compensação de gravidade e torque unipolar."""

    pretension: int = 2000
    k_lat: float = 12000.0
    k_lon_brake: float = 8000.0
    k_vert: float = 6000.0
    lat_max_g: float = 1.5
    lon_max_g: float = 1.5
    vert_max_g: float = 1.0
    lat_deadzone_g: float = 0.05
    lon_deadzone_g: float = 0.03
    vert_deadzone_g: float = 0.10
    lat_curve: float = 0.0
    lon_curve: float = 0.0
    vert_curve: float = 0.0
    smoothing: float = 0.35
    left_attenuation_pct: float = 100.0
    right_attenuation_pct: float = 100.0
    invert_lat: bool = False
    subtract_gravity: bool = True
    auto_scale: bool = False


def _deadzone(value: float, deadzone: float, full_scale: float) -> float:
    magnitude = abs(value)
    if magnitude <= deadzone:
        return 0.0
    usable = max(full_scale - deadzone, 1e-6)
    return math.copysign(min((magnitude - deadzone) / usable, 1.0), value)


def _curve(value: float, curve: float) -> float:
    # -1 dá resposta mais rápida; +1 dá maior precisão perto de zero.
    power = 2.0 ** max(-1.0, min(1.0, curve))
    return math.copysign(abs(value) ** power, value)


def _soft_limit_positive(value: float) -> float:
    """Joelho suave em 1, preservando a região baixa e limitando a saída."""
    if value <= 0.8:
        return max(0.0, value)
    return min(1.0, 0.8 + 0.2 * math.tanh((value - 0.8) / 0.2))


class AdvancedMapper:
    AUTO_FLOOR_G = 1.0
    AUTO_ATTACK_G_PER_S = 2.0
    AUTO_DRAIN_G_PER_S = 1.0 / 300.0
    AUTO_MIN_SPEED_MPS = 8.9408

    def __init__(self) -> None:
        self._lat = self._lon = self._vert = 0.0
        self._auto_peaks = [self.AUTO_FLOOR_G] * 3
        self._auto_effective = [self.AUTO_FLOOR_G] * 3
        self._auto_enabled = False
        self._auto_learning = False
        self._was_on_track = False
        self._last_t: float | None = None

    def reset(self) -> None:
        self._lat = self._lon = self._vert = 0.0

    def _reset_auto(self) -> None:
        self._auto_peaks = [self.AUTO_FLOOR_G] * 3
        self._auto_effective = [self.AUTO_FLOOR_G] * 3

    def auto_scale_state(self) -> dict:
        return {
            "enabled": self._auto_enabled,
            "learning": self._auto_learning,
            "lat_max_g": self._auto_effective[0],
            "lon_max_g": self._auto_effective[1],
            "vert_max_g": self._auto_effective[2],
        }

    def _update_auto_scale(self, sample: Sample, cfg: AdvancedMappingConfig,
                           values_g: tuple[float, float, float]) -> tuple[float, float, float]:
        enabling = cfg.auto_scale and not self._auto_enabled
        entered_track = sample.on_track and not self._was_on_track
        self._was_on_track = sample.on_track
        self._auto_enabled = cfg.auto_scale
        if not cfg.auto_scale:
            self._auto_learning = False
            self._last_t = sample.t
            return cfg.lat_max_g, cfg.lon_max_g, cfg.vert_max_g
        if enabling or entered_track:
            self._reset_auto()

        dt = 1.0 / 60.0 if self._last_t is None else max(0.0, min(0.2, sample.t - self._last_t))
        self._last_t = sample.t
        self._auto_learning = (
            sample.connected and sample.on_track and sample.speed >= self.AUTO_MIN_SPEED_MPS
        )
        if self._auto_learning:
            for i, observed in enumerate(values_g):
                peak = max(self.AUTO_FLOOR_G, self._auto_peaks[i] - self.AUTO_DRAIN_G_PER_S * dt)
                if abs(observed) > peak:
                    peak += min(abs(observed) - peak, self.AUTO_ATTACK_G_PER_S * dt)
                self._auto_peaks[i] = min(5.0, peak)

        # Equivalente ao alpha 0,14 do MAIRA a 20 Hz, independente da taxa local.
        approach = 1.0 - (1.0 - 0.14) ** (dt * 20.0)
        for i, target in enumerate(self._auto_peaks):
            self._auto_effective[i] += (target - self._auto_effective[i]) * approach
            self._auto_effective[i] = max(0.1, min(5.0, self._auto_effective[i]))
        return tuple(self._auto_effective)

    def compute(self, sample: Sample, cfg: AdvancedMappingConfig) -> Torques:
        if not sample.connected or not sample.on_track:
            self.reset()
            self._update_auto_scale(sample, cfg, (0.0, 0.0, 0.0))
            return Torques(0, 0)

        long_accel, lat_accel, vert_accel = sample.lon, sample.lat, sample.vert
        if cfg.subtract_gravity:
            cp, sp = math.cos(sample.pitch), math.sin(sample.pitch)
            cr, sr = math.cos(sample.roll), math.sin(sample.roll)
            long_accel -= G * -sp
            lat_accel -= G * cp * sr
            vert_accel -= G * cp * cr

        lat_g, lon_g, vert_g = lat_accel / G, long_accel / G, vert_accel / G
        lat_max, lon_max, vert_max = self._update_auto_scale(sample, cfg, (lat_g, lon_g, vert_g))
        lat = _curve(_deadzone(lat_g, cfg.lat_deadzone_g, max(lat_max, 0.01)), cfg.lat_curve)
        lon = _curve(_deadzone(lon_g, cfg.lon_deadzone_g, max(lon_max, 0.01)), cfg.lon_curve)
        vert = _curve(_deadzone(vert_g, cfg.vert_deadzone_g, max(vert_max, 0.01)), cfg.vert_curve)

        alpha = 1.0 - max(0.0, min(0.95, cfg.smoothing))
        self._lat += alpha * (lat - self._lat)
        self._lon += alpha * (lon - self._lon)
        self._vert += alpha * (vert - self._vert)

        brake = max(0.0, -self._lon)
        vertical = abs(self._vert)
        baseline_extra = cfg.k_lon_brake * brake + cfg.k_vert * vertical
        baseline = cfg.pretension + baseline_extra
        lateral = cfg.k_lat * abs(self._lat)
        side = -self._lat if cfg.invert_lat else self._lat

        left_extra = baseline_extra + (lateral if side < 0 else 0.0)
        right_extra = baseline_extra + (lateral if side > 0 else 0.0)
        span = max(TORQUE_MAX - cfg.pretension, 1)
        left = cfg.pretension + span * _soft_limit_positive(left_extra / span)
        right = cfg.pretension + span * _soft_limit_positive(right_extra / span)
        # Compensação final da diferença mecânica entre os lados. É somente
        # atenuação: nunca aumenta o torque calculado pelo restante do mapa.
        left *= max(50.0, min(100.0, cfg.left_attenuation_pct)) / 100.0
        right *= max(50.0, min(100.0, cfg.right_attenuation_pct)) / 100.0
        return Torques(_clamp(left), _clamp(right))


def advanced_cfg_update(cfg: AdvancedMappingConfig, patch: dict) -> AdvancedMappingConfig:
    fields = {f: getattr(cfg, f) for f in cfg.__dataclass_fields__}
    for key, value in patch.items():
        if key not in fields:
            continue
        if isinstance(fields[key], bool):
            fields[key] = bool(value)
        elif key == "pretension":
            fields[key] = int(value)
        elif key in ("left_attenuation_pct", "right_attenuation_pct"):
            fields[key] = max(50.0, min(100.0, float(value)))
        else:
            fields[key] = float(value)
    return AdvancedMappingConfig(**fields)


def cfg_to_dict(cfg: MappingConfig) -> dict:
    return asdict(cfg)


def cfg_update(cfg: MappingConfig, patch: dict) -> MappingConfig:
    fields = {f: getattr(cfg, f) for f in cfg.__dataclass_fields__}
    for k, v in patch.items():
        if k not in fields:
            continue
        if k == "invert_lat":
            fields[k] = bool(v)
        elif k == "pretension":
            fields[k] = int(v)
        else:
            fields[k] = float(v)
    return MappingConfig(**fields)
