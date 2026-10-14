extends Node2D
class_name PixteeBallTrail

## Approved artwork is never drawn, stretched or re-coloured here.
## Trail follows the recovered engine's confirmed 16.16 ball coordinates and z.
## Expected integration: call accept_native_snapshot after the existing world snapshot.

const FIXED_TO_WORLD := 1.0 / 65536.0
const MAX_POINTS := 10
const TRAIL_LIFE := 0.17
const MAX_STEP_DISTANCE := 52.0

@export var enabled: bool = true
@export var reduced_motion: bool = false
@export_enum("LOW", "STANDARD", "HIGH") var visual_quality: int = 1
@export var minimum_world_step: float = 0.55

var _points: Array[Dictionary] = []
var _previous_tick: int = -1
var _previous_position: Vector2 = Vector2.ZERO
var _last_projected: Vector2 = Vector2.ZERO

func accept_native_snapshot(snapshot: Dictionary) -> bool:
	for key in ["ball_x_raw", "ball_y_raw", "ball_z_raw", "shot_tick", "green_mode", "hole_phase"]:
		if not snapshot.has(key):
			reset_trail()
			return false
	var position := Vector2(float(snapshot["ball_x_raw"]), float(snapshot["ball_y_raw"])) * FIXED_TO_WORLD
	if bool(snapshot["green_mode"]):
		if not snapshot.has("green_origin_x") or not snapshot.has("green_origin_y"):
			reset_trail()
			return false
		position = position * 0.5 + Vector2(float(snapshot["green_origin_x"]), float(snapshot["green_origin_y"]))
	var z: float = maxf(0.0, float(snapshot["ball_z_raw"]) * FIXED_TO_WORLD)
	if not (is_finite(position.x) and is_finite(position.y) and is_finite(z)):
		reset_trail()
		return false
	_last_projected = position - Vector2(0.0, z)
	var tick := int(snapshot["shot_tick"])
	# Only show a trail for the actual in-flight shot, never for putting or an idle ball.
	if not enabled or reduced_motion or visual_quality == 0 or int(snapshot["hole_phase"]) != 1 or z <= 0.25:
		reset_trail()
		return true
	if _previous_tick < 0 or tick < _previous_tick:
		_points.clear()
		_previous_tick = tick
		_previous_position = position
		return true
	if tick == _previous_tick:
		return true
	var step := position.distance_to(_previous_position)
	_previous_position = position
	_previous_tick = tick
	if step > MAX_STEP_DISTANCE:
		_points.clear() # teleport/seek/recovery; no cross-map streaks.
		return true
	if step < minimum_world_step:
		return true
	_points.append({"at": _last_projected, "age": 0.0})
	while _points.size() > MAX_POINTS:
		_points.pop_front()
	queue_redraw()
	return true

func _process(delta: float) -> void:
	var dt := clampf(delta, 0.0, 0.1)
	for i in range(_points.size() - 1, -1, -1):
		_points[i]["age"] = float(_points[i]["age"]) + dt
		if float(_points[i]["age"]) > TRAIL_LIFE:
			_points.remove_at(i)
	if not _points.is_empty():
		queue_redraw()

func _draw() -> void:
	if reduced_motion or not enabled or visual_quality == 0:
		return
	for i in range(_points.size()):
		var p: Dictionary = _points[i]
		var fade := clampf(1.0 - float(p["age"]) / TRAIL_LIFE, 0.0, 1.0)
		var alpha := fade * 0.45 * float(i + 1) / float(_points.size())
		var at: Vector2 = p["at"]
		# Tiny pixel-aligned warm-white ghosts, rather than a large glowing beam.
		draw_rect(Rect2(at.round(), Vector2.ONE), Color(0.98, 0.96, 0.82, alpha), true)

func get_sample_count() -> int:
	return _points.size()

func get_last_projected_position() -> Vector2:
	return _last_projected

func reset_trail() -> void:
	_points.clear()
	_previous_tick = -1
	_previous_position = Vector2.ZERO
	queue_redraw()
