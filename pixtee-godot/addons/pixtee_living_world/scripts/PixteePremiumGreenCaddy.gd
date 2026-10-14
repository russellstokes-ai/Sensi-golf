extends PixteePremiumLayeredSprite
class_name PixteePremiumGreenCaddy

## Stationary flag holder. No walk state, no collision, no gameplay API writes.
@export var native_green_state: bool = false
@export var flag_wind_strength: float = 0.5

func _ready() -> void:
    animation_id = "caddy_stationary_breathe"
    super._ready()
    visible = native_green_state

func accept_native_green_state(is_on_green: bool) -> void:
    native_green_state = is_on_green
    visible = is_on_green

func accept_visual_wind(_wind: Vector2, intensity: float) -> void:
    flag_wind_strength = clampf(intensity, 0.0, 1.0)
    # Cloth sprites animate, the pole is immobile and readable.
    playback_speed = 0.7 + flag_wind_strength * 0.8
