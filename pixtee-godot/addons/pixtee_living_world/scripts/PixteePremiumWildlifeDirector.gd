extends Node2D
class_name PixteePremiumWildlifeDirector

## Read-only ball proximity reacts cosmetically; no course-map or native-engine mutation.
@export var species: String = "frog"
@export var visual_alert_distance: float = 28.0
@export var escape_distance: float = 19.0
@export var reduced_motion: bool = false
@export var is_enabled: bool = true
var sprite: PixteePremiumLayeredSprite
var original_origin: Vector2
var _phase := "idle"
var _phase_time := 0.0
var _last_ball_tick := -1

func _ready() -> void:
    original_origin = position
    sprite = PixteePremiumLayeredSprite.new()
    add_child(sprite)
    sprite.animation_id = _animation_name()
    sprite.set_animation(sprite.animation_id)

func _process(delta: float) -> void:
    if not is_enabled or reduced_motion:
        return
    _phase_time += minf(delta, 0.1)
    if _phase == "alert" and _phase_time > 0.35:
        _phase = "escape"
        _phase_time = 0.0
        if species == "frog":
            sprite.set_animation("frog_hop")
    elif _phase == "escape" and _phase_time > 0.8:
        _phase = "idle"
        _phase_time = 0.0
        sprite.set_animation(_animation_name())

func _animation_name() -> String:
    match species:
        "rabbit": return "rabbit_layered_idle"
        "squirrel": return "squirrel_layered_idle"
        "dragonfly": return "dragonfly_heading_00"
        _: return "frog_breathe"

func accept_native_ball_ground_position(pos: Vector2, tick: int, is_airborne: bool) -> void:
    if reduced_motion or not is_enabled or is_airborne or tick == _last_ball_tick:
        return
    _last_ball_tick = tick
    var proximity := global_position.distance_to(pos)
    if proximity <= escape_distance and _phase == "idle":
        _phase = "alert"
        _phase_time = 0.0
        # External reaction burst (if integrated) subscribes to this state,
        # not to guessed collisions. Never invent a collision native event.
