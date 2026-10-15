extends Node
class_name PixteeWeatherDirector

## Presentation-only weather, designed for Godot 4.5.
## IMPORTANT: This never sets simulation wind, lie, friction, hazard or ball state.
## Only call set_native_wind_visuals with values confirmed by the C++ core.

signal preset_changed(name: String)

const PRESETS := {
	"sunny":        {"sun": 0.90, "cloud": 0.12, "rain": 0.00, "fog": 0.00, "wind": 0.24},
	"golden_hour": {"sun":0.72, "cloud":0.18, "rain":0.0, "fog":0.08, "wind":0.25},
	"bright_sun":   {"sun": 1.00, "cloud": 0.02, "rain": 0.00, "fog": 0.00, "wind": 0.16},
	"breezy":       {"sun": 0.75, "cloud": 0.23, "rain": 0.00, "fog": 0.00, "wind": 0.58},
	"partly_cloudy": {"sun": 0.65, "cloud": 0.52, "rain": 0.00, "fog": 0.00, "wind": 0.32},
	"overcast":     {"sun": 0.28, "cloud": 0.91, "rain": 0.00, "fog": 0.04, "wind": 0.42},
	"drizzle":      {"sun": 0.15, "cloud": 0.95, "rain": 0.32, "fog": 0.08, "wind": 0.30},
	"rain":         {"sun": 0.08, "cloud": 1.00, "rain": 0.82, "fog": 0.15, "wind": 0.54},
	"morning_mist": {"sun": 0.42, "cloud": 0.35, "rain": 0.00, "fog": 0.70, "wind": 0.10}
}

var preset_name: String = "breezy"
var reduced_motion: bool = false
var _values: Dictionary = PRESETS["breezy"].duplicate()
var _from: Dictionary = _values.duplicate()
var _target: Dictionary = _values.duplicate()
var _duration: float = 0.0
var _elapsed: float = 0.0
var _direction: Vector2 = Vector2.RIGHT
var _verified_wind_override: bool = false
var _verified_strength: float = 0.0

func set_preset(name: String, transition_seconds: float = 4.0) -> bool:
	if not PRESETS.has(name):
		return false
	preset_name = name
	_from = _values.duplicate()
	_target = PRESETS[name].duplicate()
	_elapsed = 0.0
	_duration = maxf(0.0, transition_seconds)
	if _duration == 0.0:
		_values = _target.duplicate()
	preset_changed.emit(name)
	return true

func set_visual_direction(direction: Vector2) -> void:
	# Cosmetic breeze direction only; do not pass this into gameplay.
	if direction.length_squared() > 0.0001:
		_direction = direction.normalized()

func set_native_wind_visuals(direction: Vector2, normalized_strength: float, verified: bool) -> bool:
	# The caller must explicitly assert the value came from an authoritative native snapshot.
	if not verified or not is_finite(normalized_strength):
		return false
	if direction.length_squared() > 0.0001:
		_direction = direction.normalized()
	_verified_strength = clampf(normalized_strength, 0.0, 1.0)
	_verified_wind_override = true
	return true

func clear_native_wind_override() -> void:
	_verified_wind_override = false

func get_wind_direction() -> Vector2:
	return _direction

func conditions() -> Dictionary:
	var snapshot := _values.duplicate()
	# 'wind' describes visual activity, not extra ball deflection.
	if _verified_wind_override:
		snapshot["wind"] = _verified_strength
	return snapshot

func _process(delta: float) -> void:
	if reduced_motion: return
	if _duration <= 0.0 or _elapsed >= _duration:
		return
	_elapsed = minf(_duration, _elapsed + clampf(delta, 0.0, 0.1))
	var t: float = _elapsed / _duration
	t = t * t * (3.0 - 2.0 * t)  # smoothstep; presentation time only.
	for field in _target.keys():
		_values[field] = lerpf(float(_from[field]), float(_target[field]), t)
