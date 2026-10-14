extends Node2D
class_name PixteePremiumLayeredSprite

## Cosmetic only: no gameplay/physics calls. Every layer uses the same authored canvas.
@export var animation_id: String = "rabbit_layered_idle"
@export var playback_speed: float = 1.0
@export var preview_enabled: bool = true
@export var pixel_snap: bool = true

const MANIFEST := "res://addons/pixtee_living_world/PREMIUM_ANIMATION_MANIFEST.json"
const ROOT := "res://addons/pixtee_living_world/assets/premium/"
var _layers: Array[Sprite2D] = []
var _sequences: Array = []
var _frames: int = 0
var _fps: float = 6.0
var _time: float = 0.0
var _running: bool = true

func _ready() -> void:
    texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
    if preview_enabled:
        set_animation(animation_id)

func set_animation(next_id: String, minimum_layers: int = 1) -> bool:
    if not FileAccess.file_exists(MANIFEST):
        push_error("Pixtee premium animation manifest unavailable")
        return false
    var decoded: Variant = JSON.parse_string(FileAccess.get_file_as_string(MANIFEST))
    if not (decoded is Dictionary) or not decoded.has(next_id):
        push_error("No authored Pixtee animation: " + next_id)
        return false
    var entry: Dictionary = decoded[next_id]
    if entry["layers"].size() < maxi(1,minimum_layers):
        return false
    var new_layers: Array = []
    var frame_len := -1
    for layer_name in entry["layers"].keys():
        var paths: Array = entry["layers"][layer_name]
        if frame_len < 0:
            frame_len = paths.size()
        if paths.is_empty() or paths.size() != frame_len:
            push_error("Unequal frame count: " + next_id)
            return false
        var textures: Array[Texture2D] = []
        for rel in paths:
            var path := ROOT + str(rel)
            if not ResourceLoader.exists(path):
                push_error("Missing authored frame: " + path)
                return false
            var tex: Texture2D = load(path)
            if tex == null or tex.get_size() != Vector2(float(entry["canvas"][0]), float(entry["canvas"][1])):
                push_error("Wrong frame dimensions: " + path)
                return false
            textures.append(tex)
        new_layers.append({"name": layer_name, "textures": textures})
    if new_layers.is_empty():
        return false
    for child in _layers:
        remove_child(child)
        child.queue_free()
    _layers.clear()
    _sequences.clear()
    animation_id = next_id
    _frames = frame_len
    _fps = float(entry["fps"])
    _time = 0.0
    for data in new_layers:
        var sprite := Sprite2D.new()
        sprite.name = str(data["name"])
        sprite.centered = false
        sprite.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
        add_child(sprite)
        _layers.append(sprite)
        _sequences.append(data["textures"])
    _show_frame(0)
    return true

func _process(delta: float) -> void:
    if not _running or _frames <= 0:
        return
    _time += minf(maxf(0.0, delta), 0.1) * maxf(0.0, playback_speed)
    _show_frame(int(floor(_time * _fps)) % _frames)
    # Logical position retains its fractional path; rendering owns snapping.

func _show_frame(frame: int) -> void:
    for i in range(_layers.size()):
        _layers[i].texture = _sequences[i][frame]

func pause_presentation(paused: bool) -> void:
    _running = not paused

func reset_presentation() -> void:
    _time = 0.0
    _show_frame(0)
