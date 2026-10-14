extends Node2D
class_name PixteeReactionBurst

## Shared cartoon pixel-art visual reaction. Add as a CHILD of the subject.
## It never creates a gameplay collider and never alters the native engine.
## A crisp illustrated exclamation mark is accompanied by genuinely finite
## 1-pixel flecks/dust; this is not a permanently painted sprite feature.

const SHOCK_LIFE: float = 0.62
const DUST_LIFE: float = 0.45
const DUST_LIMIT: int = 18

@export var reduced_motion: bool = false
@export var reduced_flashes: bool = false
@export var visual_quality: int = 1
@export var symbol_above_anchor: Vector2 = Vector2(0.0, -33.0)

var _shock_age: float = -1.0
var _dust: Array[Dictionary] = []
var _phase: float = 0.0

func play_shock() -> void:
	_shock_age = 0.0
	_phase = 0.0
	queue_redraw()

func add_dust(at: Vector2 = Vector2.ZERO, count: int = 4, facing: float = 1.0) -> void:
	if visual_quality == 0 or reduced_motion:
		return
	count = mini(count, DUST_LIMIT - _dust.size())
	for i in range(count):
		var angle := float(i) * (PI / maxf(1.0,float(count - 1)))
		var vx := facing * (3.5 + float(i) * 1.2)
		_dust.append({
			"pos": at + Vector2(float(i % 2) * 1.2 - 1.0, 0.0),
			"velocity": Vector2(-vx, -2.2 - sin(angle) * 3.0),
			"age": 0.0,
			"life": DUST_LIFE,
			"size": 2.0 if i % 3 == 0 else 1.0
		})
	queue_redraw()

func _process(delta: float) -> void:
	var dt: float = clampf(delta,0.0,0.1)
	if _shock_age >= 0.0:
		_shock_age += dt
		if _shock_age >= SHOCK_LIFE:
			_shock_age = -1.0
	for i in range(_dust.size()-1,-1,-1):
		var d: Dictionary = _dust[i]
		d["age"] = float(d["age"]) + dt
		if float(d["age"]) >= float(d["life"]):
			_dust.remove_at(i)
		else:
			d["pos"] = d["pos"] + d["velocity"] * dt
			d["velocity"] = d["velocity"] + Vector2(0.0,17.0) * dt
			_dust[i] = d
	if _shock_age >= 0.0 or not _dust.is_empty():
		queue_redraw()

func _draw() -> void:
	for d in _dust:
		var fade: float = 1.0 - float(d["age"]) / maxf(float(d["life"]),0.01)
		var p: Vector2 = d["pos"]
		var size: float = float(d["size"])
		draw_rect(Rect2(p.round(),Vector2(size,size)),Color(0.96,0.87,0.64,fade*0.74))
	if _shock_age < 0.0 or visual_quality == 0:
		return
	var u: float = clampf(_shock_age / SHOCK_LIFE,0.0,1.0)
	var fade: float = clampf((1.0-u)/0.30,0.0,1.0)
	var bounce: float = sin(minf(1.0,u/0.23) * PI * 0.5)
	var shift_y: float = -4.0 * bounce
	var centre: Vector2 = (symbol_above_anchor + Vector2(0.0, shift_y)).round()
	# Shadow/outline around golden symbol, 1-pixel detail. Stable across phone DPI.
	var ink := Color(0.30,0.13,0.11,fade)
	var gold := Color(1.0,0.83,0.28,fade)
	var bright := Color(1.0,0.96,0.73,fade)
	draw_rect(Rect2(centre+Vector2(-2,-9),Vector2(5,10)),ink)
	draw_rect(Rect2(centre+Vector2(-1,-8),Vector2(3,8)),gold)
	draw_rect(Rect2(centre+Vector2(0,-8),Vector2(1,4)),bright)
	draw_rect(Rect2(centre+Vector2(-2,2),Vector2(5,4)),ink)
	draw_rect(Rect2(centre+Vector2(-1,3),Vector2(3,2)),gold)
	if reduced_motion or reduced_flashes:
		return
	# Brief outward comic rays, not a flashing full-screen white layer.
	if u < 0.46:
		var burst_alpha: float = (1.0 - u/0.46)*fade*0.85
		var rays := [Vector2(-10,-7),Vector2(10,-7),Vector2(-12,1),Vector2(12,1)]
		for j in range(rays.size()):
			var at: Vector2 = centre+rays[j]
			var ray_dir: Vector2 = (rays[j]-Vector2(0,-3)).normalized()
			draw_line(at,at+ray_dir*3.0,Color(1.0,0.96,0.76,burst_alpha),1.0,false)

func reset_reaction() -> void:
	_shock_age = -1.0
	_dust.clear()
	queue_redraw()
