extends PixteePremiumLayeredSprite
class_name PixteePremiumDragonfly

## Eight top-down directional renderings with four frame wing motion.
## Headings are visual art transforms: never influence native ball movement.
@export var flight_direction: Vector2 = Vector2.RIGHT
@export var buzzing: bool = true

func _ready() -> void:
    animation_id = "dragonfly_heading_00"
    super._ready()

func accept_flight_direction(direction: Vector2) -> void:
    if direction.length_squared() < 0.0001:
        return
    flight_direction = direction.normalized()
    var sector := posmod(int(round(flight_direction.angle() / (PI / 4.0))), 8)
    var next := "dragonfly_heading_%02d" % sector
    if animation_id != next:
        set_animation(next)
