extends Node
class_name PixteeCameraFeel

## Additive camera punch; never owns Camera2D.position, zoom or HUD offset.
## Integration MUST add get_punch_offset(camera.zoom) to the base offset each view update.
## Event authentication and deduplication protect against fake/replayed impacts.

const EVENT_STRENGTH := {
	"club_contact": 1.8,
	"fairway_contact": 0.8,
	"rough_contact": 1.1,
	"bunker_contact": 1.3,
	"water_contact": 1.6,
	"authored_tree_contact": 2.4,
	"verified_tree_contact": 2.4,
	"verified_flag_contact": 1.5,
	"cup_lip": 0.9,
	"cup_capture": 1.1
}
const MAX_SEEN := 256

var enabled: bool = true
var reduced_motion: bool = false
var intensity: float = 1.0
var _time_left: float = 0.0
var _duration: float = 0.0
var _strength_px: float = 0.0
var _elapsed: float = 0.0
var _seen: Dictionary = {}
var _order: Array[String] = []

func accept_native_event(event: Dictionary) -> bool:
	if event.get("confirmed_native", false) != true:
		return false
	var event_id: String = str(event.get("event_id", ""))
	var kind: String = str(event.get("event_type", ""))
	if event_id.is_empty() or not EVENT_STRENGTH.has(kind) or _seen.has(event_id):
		return false
	_seen[event_id] = true
	_order.append(event_id)
	if _order.size() > MAX_SEEN:
		_seen.erase(_order.pop_front())
	if enabled and not reduced_motion:
		_strength_px = minf(3.0, float(EVENT_STRENGTH[kind]) * maxf(0.0, intensity))
		_duration = 0.11 if kind == "club_contact" else 0.15
		_time_left = _duration
		_elapsed = 0.0
	return true

func _process(delta: float) -> void:
	if _time_left > 0.0:
		var step := clampf(delta, 0.0, 0.1)
		_time_left = maxf(0.0, _time_left - step)
		_elapsed += step

func get_punch_offset(camera_zoom: Vector2) -> Vector2:
	if reduced_motion or not enabled or _time_left <= 0.0:
		return Vector2.ZERO
	var factor := _time_left / maxf(_duration, 0.001)
	# Screen-pixel magnitude is invariant across Fold / portrait zoom.
	var jitter := Vector2(sin(_elapsed * 91.0 + 0.7), sin(_elapsed * 117.0 + 1.9))
	return Vector2(jitter.x / maxf(0.1, camera_zoom.x), jitter.y / maxf(0.1, camera_zoom.y)) * _strength_px * factor * factor

func reset_punch() -> void:
	_time_left = 0.0
	_elapsed = 0.0
	_seen.clear()
	_order.clear()
