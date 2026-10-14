extends Node2D
class_name PixteeNativeWorldFX

## Read-only original-native event-driven VFX. Does not call C++ step_tick,
## update native physics, choose hazard/ball outcome, or share game PRNG.
## Cosmetic particle/ripple shapes are additive; never stretch Pixtee sprites.
## Proven native event packet integration required before commercial sign-off.

const MAX_LIVE_FLECKS := 128
const MAX_RIPPLES := 16
const MAX_REMEMBERED_NATIVE_EVENTS := 4096
const EVENT_PRESETS := {
	"authored_board_contact": {"count":4,"color":Color(0.64,0.47,0.24),"life":0.35},
	"cup_lip": {"count":3,"color":Color(0.95,0.90,0.63),"life":0.3},
	"authored_bird_contact": {"count":8,"color":Color(0.75,0.57,0.32),"life":0.85},
    "authored_tree_contact": {"count": 6, "color": Color(0.47,0.66,0.35), "life": 0.55},
	"fairway_contact": {"count": 5, "color": Color(0.76,0.80,0.42), "life": 0.40},
	"rough_contact": {"count": 7, "color": Color(0.51,0.68,0.32), "life": 0.55},
	"bunker_contact": {"count": 9, "color": Color(0.86,0.72,0.48), "life": 0.65},
	"water_contact": {"count": 10, "color": Color(0.64,0.87,0.93), "life": 0.52},
	"verified_tree_contact": {"count": 6, "color": Color(0.47,0.66,0.35), "life": 0.55},
	"cup_capture": {"count": 7, "color": Color(0.92,0.84,0.52), "life": 0.57},
	"verified_flag_contact": {"count": 3, "color": Color(0.97,0.92,0.76), "life": 0.30}
}

@export var reduced_motion: bool = false
@export_enum("LOW", "STANDARD", "HIGH") var visual_quality: int = 1

var _particles: Array[Dictionary] = []
var _ripples: Array[Dictionary] = []
var _seen_events: Dictionary = {}
var _event_order: Array[String] = []
var _ball_world: Vector2 = Vector2.ZERO
var _ball_z: float = 0.0
var _ball_snapshot_verified: bool = false
var _last_submitted_native_tick: int = -1

func accept_native_ball_snapshot(snapshot: Dictionary) -> bool:
	## Native bridge provides original fixed-point state directly.
	## Never derive altitude from a fake parabolic animation in Godot.
	if not snapshot.has("ball_x_raw") or not snapshot.has("ball_y_raw"):
		return false
	if not snapshot.has("ball_z_raw") or not snapshot.has("shot_tick"):
		return false
	# Native green mode stores local coordinates at 2x world resolution.
	# The existing bridge has not yet exported its green-region origin.
	# Never draw a shadow in the wrong place by treating local coords as world.
	if not snapshot.has("green_mode"):
		return false
	var x: float = float(snapshot["ball_x_raw"]) / 65536.0
	var y: float = float(snapshot["ball_y_raw"]) / 65536.0
	if snapshot["green_mode"] == true:
		if not snapshot.has("green_origin_x") or not snapshot.has("green_origin_y"):
			return false
		x = x * 0.5 + float(snapshot["green_origin_x"])
		y = y * 0.5 + float(snapshot["green_origin_y"])
	var z: float = maxf(0.0, float(snapshot["ball_z_raw"]) / 65536.0)
	if not is_finite(x) or not is_finite(y) or not is_finite(z):
		return false
	var native_tick: int = int(snapshot["shot_tick"])
	if native_tick < 0:
		return false
	_ball_world = Vector2(x, y)
	_ball_z = z
	_ball_snapshot_verified = true
	_last_submitted_native_tick = native_tick
	queue_redraw()
	return true

func submit_confirmed_native_event(event: Dictionary) -> bool:
	## The host must send an authoritative C++ event, with a stable ID.
	## No inference from ball proximity, sprite rectangles or 70Hz polling.
	if event.get("confirmed_native", false) != true:
		return false
	var event_id: String = str(event.get("event_id", ""))
	var event_type: String = str(event.get("event_type", ""))
	if event_id.is_empty() or not EVENT_PRESETS.has(event_type):
		return false
	if _seen_events.has(event_id):
		return false
	if not event.has("world_x") or not event.has("world_y"):
		return false
	var pos := Vector2(float(event["world_x"]), float(event["world_y"]))
	if not is_finite(pos.x) or not is_finite(pos.y):
		return false

	if event_type == "authored_bird_contact": pos.y -= maxf(0.0,float(event.get("world_z",0.0)))
	var preset: Dictionary = EVENT_PRESETS[event_type]
	var count: int = int(preset["count"])
	if visual_quality == 0 or reduced_motion:
		count = mini(count, 2)
	elif visual_quality == 2:
		count += 3
	var remaining: int = maxi(0, MAX_LIVE_FLECKS - _particles.size())
	count = mini(count, remaining)
	# Visual-only deterministic seed, independent from gameplay PRNG.
	var random := RandomNumberGenerator.new()
	random.seed = hash(event_id)
	for i in range(count):
		var theta := random.randf_range(0.0, TAU)
		var strength := random.randf_range(5.0, 16.0)
		var duration: float = float(preset["life"])
		_particles.append({
			"position": pos,
			"velocity": Vector2(cos(theta), sin(theta)) * strength,
			"age": 0.0,
			"duration": duration,
			"color": preset["color"]
		})
	if event_type == "water_contact" and not reduced_motion:
		if _ripples.size() < MAX_RIPPLES:
			_ripples.append({"position":pos, "age":0.0, "duration":0.8})

	_spawn_godot_burst(pos,preset)
	_seen_events[event_id] = true
	_event_order.append(event_id)
	if _event_order.size() > MAX_REMEMBERED_NATIVE_EVENTS:
		var old: String = _event_order.pop_front()
		_seen_events.erase(old)
	queue_redraw()
	return true

func _process(delta: float) -> void:
	if reduced_motion or visual_quality == 0:
		for child in get_children(): child.queue_free()
	# Presentation only; no calls into original 936/65536 second physics.
	var dt := minf(maxf(delta, 0.0), 0.1)
	for index in range(_particles.size()-1, -1, -1):
		var fleck: Dictionary = _particles[index]
		fleck["age"] = float(fleck["age"]) + dt
		if fleck["age"] >= fleck["duration"]:
			_particles.remove_at(index)
		else:
			fleck["position"] = fleck["position"] + fleck["velocity"] * dt
			_particles[index] = fleck
	for index in range(_ripples.size()-1, -1, -1):
		var ripple: Dictionary = _ripples[index]
		ripple["age"] = float(ripple["age"]) + dt
		if ripple["age"] >= ripple["duration"]:
			_ripples.remove_at(index)
		else:
			_ripples[index] = ripple
	if not _particles.is_empty() or not _ripples.is_empty():
		queue_redraw()

func _draw() -> void:
	# Render a separate soft ground shadow from actual native ball altitude.
	# Approved ball sprite remains the native camera/visual layer's responsibility.
	if _ball_snapshot_verified:
		var shadow_opacity := clampf(0.34 - _ball_z * 0.009, 0.09, 0.34)
		draw_circle((_ball_world + Vector2(1.0, 1.0)).round(), 1.7,
			Color(0.02, 0.06, 0.07, shadow_opacity), true, -1.0, false)
	for fleck in _particles:
		var duration: float = float(fleck["duration"])
		var fraction := clampf(1.0 - float(fleck["age"]) / duration, 0.0, 1.0)
		var colour: Color = fleck["color"]
		colour.a *= fraction
		var at: Vector2 = fleck["position"]
		draw_rect(Rect2(at.round(), Vector2.ONE), colour, true)
	for ripple in _ripples:
		var fraction: float = float(ripple["age"]) / float(ripple["duration"])
		var alpha := (1.0 - fraction) * 0.55
		var radius := 1.0 + 9.0 * fraction
		var world_pos: Vector2 = ripple["position"]
		draw_arc(world_pos.round(), radius, 0.0, TAU, 16,
			Color(0.69,0.90,0.95,alpha),1.0,false)

func on_confirmed_restart_or_replay_seek() -> void:
	for child in get_children(): child.queue_free()
	_particles.clear()
	_ripples.clear()
	_seen_events.clear()
	_event_order.clear()
	_ball_snapshot_verified = false
	_last_submitted_native_tick = -1
	queue_redraw()

func set_effects_running(running: bool) -> void:
	set_process(running)
	for child in get_children():
		if child is CPUParticles2D: child.speed_scale = 1.0 if running else 0.0

func _spawn_godot_burst(at: Vector2,preset: Dictionary) -> void:
	if reduced_motion or visual_quality == 0 or get_child_count() >= 12: return
	var burst := CPUParticles2D.new()
	burst.position = at
	burst.one_shot = true
	burst.explosiveness = 1.0
	burst.amount = int(preset.count)
	burst.lifetime = float(preset.life)
	burst.direction = Vector2(0,-1)
	burst.spread = 65.0
	burst.gravity = Vector2(0,30)
	burst.initial_velocity_min = 8.0
	burst.initial_velocity_max = 22.0
	burst.scale_amount_min = 0.25
	burst.scale_amount_max = 0.75
	burst.color = preset.color
	var texture := GradientTexture2D.new()
	texture.width = 4
	texture.height = 4
	texture.fill = GradientTexture2D.FILL_RADIAL
	texture.fill_from = Vector2(0.5,0.5)
	texture.fill_to = Vector2(1.0,0.5)
	var gradient := Gradient.new()
	gradient.colors = PackedColorArray([Color.WHITE,Color(1,1,1,0)])
	texture.gradient = gradient
	burst.texture = texture
	add_child(burst)
	burst.finished.connect(burst.queue_free)
	burst.emitting = true
