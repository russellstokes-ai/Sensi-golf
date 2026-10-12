extends Node2D
class_name PixteeVerifiedGreenArrows

## Render-only green arrows from genuine recovered native C++ MAPS cells.
## Never guesses slopes from image gradients or changes shot/ball physics.
## 16 directions * 16 strengths; no sprite stretching or shader warping.

const SOURCE_HEADING_STEPS := 4096
const SOURCE_SLOPE_INCREMENT := 256
const MAX_AUTHORED_ARROW_CELLS := 2048

@export var arrows_enabled: bool = true:
	set(value):
		arrows_enabled = value
		queue_redraw()
@export var arrow_colour: Color = Color(0.96, 0.93, 0.76, 0.86)
@export var arrow_outline_colour: Color = Color(0.05, 0.12, 0.16, 0.75)

var _arrows: Array[Dictionary] = []
var _native_verified: bool = false

static func native_slope_vector(raw_direction: int) -> Vector2:
	# Recovered_ground.cpp uses trig_q14(dir) for X,
	# trig_q14(dir+1024) for Y: 0=down, 1024=right, 2048=up.
	var radians := TAU * float(raw_direction) / float(SOURCE_HEADING_STEPS)
	return Vector2(sin(radians), cos(radians))

func clear_field() -> void:
	_arrows.clear()
	_native_verified = false
	queue_redraw()

func load_native_maps_arrow_cells(cells: Array[Dictionary]) -> bool:
	## Strict, atomic input from course converter or native C++ bridge:
	## {world_x,world_y,direction_raw,magnitude,terrain_code,verified_native:true}
	## The caller's verified_native is a declared contract only;
	## actual source MAPS parity must be proved in native integration tests.
	if cells.size() > MAX_AUTHORED_ARROW_CELLS:
		return false
	var next: Array[Dictionary] = []
	for cell in cells:
		for field in ["world_x","world_y","direction_raw","magnitude","terrain_code","verified_native"]:
			if not cell.has(field):
				return false
		if cell["verified_native"] != true:
			return false
		var raw: int = int(cell["direction_raw"])
		var magnitude: int = int(cell["magnitude"])
		var terrain: int = int(cell["terrain_code"])
		if raw < 0 or raw >= SOURCE_HEADING_STEPS or raw % SOURCE_SLOPE_INCREMENT != 0:
			return false
		if magnitude < 0 or magnitude > 15:
			return false
		if terrain != 1 or magnitude == 0: # source GREEN H4 only
			continue
		var pos := Vector2(float(cell["world_x"]),float(cell["world_y"]))
		if not is_finite(pos.x) or not is_finite(pos.y):
			return false
		next.append({
			"position": pos.round(),
			"direction_raw": raw,
			"magnitude": magnitude
		})
	_arrows = next
	_native_verified = true
	queue_redraw()
	return true

func arrows_are_verified() -> bool:
	return _native_verified

func arrow_count() -> int:
	return _arrows.size()

func _draw() -> void:
	if not arrows_enabled or not _native_verified:
		return
	var scaled := get_global_transform().get_scale()
	# Never render an arrow under an inherited anisotropic transform.
	if absf(scaled.x - scaled.y) > 0.0001:
		return
	for arrow in _arrows:
		var center: Vector2 = arrow["position"]
		var vec: Vector2 = native_slope_vector(int(arrow["direction_raw"]))
		var strength: int = int(arrow["magnitude"])
		var length: float = 3.0 + float(strength) * 0.40
		var tail: Vector2 = (center - vec * length * 0.5).round()
		var tip: Vector2 = (center + vec * length * 0.5).round()
		var barb_left: Vector2 = (tip - vec.rotated(PI / 4.0) * 3.0).round()
		var barb_right: Vector2 = (tip - vec.rotated(-PI / 4.0) * 3.0).round()
		# Pixel legibility on green using 1px shadow (no atlas asset altered).
		var shadow := Vector2(1,1)
		draw_line(tail + shadow, tip + shadow, arrow_outline_colour, 1.0, false)
		draw_line(tip + shadow, barb_left + shadow, arrow_outline_colour, 1.0, false)
		draw_line(tip + shadow, barb_right + shadow, arrow_outline_colour, 1.0, false)
		draw_line(tail, tip, arrow_colour, 1.0, false)
		draw_line(tip, barb_left, arrow_colour, 1.0, false)
		draw_line(tip, barb_right, arrow_colour, 1.0, false)
