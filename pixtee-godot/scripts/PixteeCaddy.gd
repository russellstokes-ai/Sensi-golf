extends Node2D
var world: Node2D
var actor: PixteePremiumLayeredSprite
var green := PackedVector2Array()
var cup := Vector2(364,168)
var foot := Vector2(418,190)
var flag_alpha := 0.0
func _ready() -> void:
	var course: Dictionary = JSON.parse_string(FileAccess.get_file_as_string("res://courses/lakewood-hole-1/course-layout.json"))
	for point in course.surfaces.green: green.append(Vector2(point[0],point[1]))
	for candidate in [Vector2(360,234),Vector2(300,210),Vector2(298,125),Vector2(418,190)]:
		if not Geometry2D.is_point_in_polygon(candidate,green) and candidate.distance_to(cup)>28.0:
			foot = candidate
			break
	actor = PixteePremiumLayeredSprite.new()
	actor.preview_enabled = false
	add_child(actor)
	assert(actor.set_animation("caddy_stationary_breathe",4))
	var contact: Dictionary = JSON.parse_string(FileAccess.get_file_as_string("res://characters/caddy-contact-v1.json"))
	actor.scale = Vector2.ONE*float(contact.pixel_scale)
	actor.position = foot-Vector2(contact.feet[0],contact.feet[1])*float(contact.pixel_scale)
	_update_flag_layers()
func _process(delta: float) -> void:
	var frozen: bool = not world.is_processing() or world.reduced_motion
	actor.pause_presentation(frozen)
	var target := 1.0 if world.native_state.get("flag_removed_for_putting",false) else 0.0
	flag_alpha = target if frozen else move_toward(flag_alpha,target,minf(delta,0.1)*5.0)
	_update_flag_layers()
	queue_redraw()
func _update_flag_layers() -> void:
	for layer in actor._layers:
		if layer.name in ["pole","flag_cloth"]:
			layer.modulate.a = flag_alpha
			layer.visible = flag_alpha>0.0
func _draw() -> void:
	draw_set_transform(foot)
	draw_set_transform(foot,0.0,Vector2(3.3,1.1))
	draw_circle(Vector2.ZERO,1.0,Color(0.025,0.08,0.06,0.26),true,-1,true)
	draw_set_transform(Vector2.ZERO)
