extends Control
signal aim_changed(delta: int)
var game: Node
var anchor := Vector2.ZERO
var enabled := false

func _ready() -> void:
	mouse_filter = Control.MOUSE_FILTER_IGNORE

func refresh(snapshot: Dictionary) -> void:
	enabled = game.playing and not game.paused and snapshot.get("can_shoot",false)
	var angle := float(snapshot.get("aim_raw",0))*TAU/4096.0
	var world_at: Vector2 = game.world.ball_world+Vector2(sin(angle),cos(angle))*game.world.aim_length(snapshot)
	anchor = game.hud.get_global_transform().affine_inverse()*(game.world.get_global_transform_with_canvas()*world_at)
	queue_redraw()

func _unhandled_input(event: InputEvent) -> void:
	if not enabled or not is_visible_in_tree(): return
	if event is InputEventMouseButton and event.button_index == MOUSE_BUTTON_LEFT and event.pressed:
		var local: Vector2 = get_global_transform().affine_inverse()*event.position
		if absf(local.y-anchor.y) <= 36 and absf(local.x-anchor.x) <= 95 and absf(local.x-anchor.x) >= 10:
			aim_changed.emit(-32 if local.x < anchor.x else 32)
			get_viewport().set_input_as_handled()

