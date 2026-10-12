extends Node2D
class_name PixteeFoliageBreeze

## Attach to a pivot-anchored, approved foliage/flower/reed or flag node.
## Rotates a complete approved sprite around its natural anchor:
## NO scaling, no UV deformation, no alternate physics/wind on the ball.
## Time phase uses presentation-only coordinates; NOT recovered C++ RNG.

@export var breeze_direction: Vector2 = Vector2.RIGHT
@export_range(0.0, 3.0, 0.1) var breeze_strength: float = 0.7
@export_range(0.0, 2.0, 0.1) var sway_amplitude_degrees: float = 1.2
@export_range(0.1, 2.0, 0.1) var sway_speed: float = 0.65
@export var reduced_motion: bool = false

var _rest_rotation: float = 0.0
var _elapsed: float = 0.0
var _phase: float = 0.0

func _ready() -> void:
	_rest_rotation = rotation
	_phase = fposmod(global_position.x * 0.073 + global_position.y * 0.039, TAU)

func _process(delta: float) -> void:
	if reduced_motion or is_zero_approx(breeze_strength):
		rotation = _rest_rotation
		return
	_elapsed += minf(delta, 0.1)
	var cosine := sin(_elapsed * sway_speed * TAU + _phase)
	var gust := 0.75 + 0.25 * sin(_elapsed * 0.32 + _phase * 0.5)
	var lean := signf(breeze_direction.x)
	if absf(breeze_direction.x) < 0.01:
		lean = 1.0
	var angle := deg_to_rad(sway_amplitude_degrees) * breeze_strength
	rotation = _rest_rotation + cosine * gust * angle * lean

func update_visual_breeze(direction: Vector2, strength: float) -> void:
	if direction.length_squared() > 0.001:
		breeze_direction = direction.normalized()
	breeze_strength = clampf(strength, 0.0, 3.0)
