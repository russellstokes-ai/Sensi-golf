extends Control
var world: Node2D
var expanded := false
var last_tap_msec := -1000
var map_tween: Tween
var reduced_motion := false

func _ready() -> void:
	clip_contents = true
	texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	mouse_default_cursor_shape = Control.CURSOR_POINTING_HAND
	gui_input.connect(_tap)
	_resize_map()

func _resize_map() -> void:
	var dimensions := Vector2(174,287) if expanded else Vector2(84,138)
	set_anchors_and_offsets_preset(Control.PRESET_TOP_RIGHT)
	offset_right = -8
	offset_top = 54
	if map_tween != null and map_tween.is_valid(): map_tween.kill()
	if reduced_motion or size.x < 1:
		offset_left = -8-dimensions.x
		offset_bottom = 54+dimensions.y
	else:
		map_tween = create_tween().set_parallel(true)
		map_tween.tween_property(self,"offset_left",-8-dimensions.x,0.25).set_trans(Tween.TRANS_CUBIC).set_ease(Tween.EASE_OUT)
		map_tween.tween_property(self,"offset_bottom",54+dimensions.y,0.25).set_trans(Tween.TRANS_CUBIC).set_ease(Tween.EASE_OUT)
	queue_redraw()

func _tap(event: InputEvent) -> void:
	if event is InputEventMouseButton and event.button_index == MOUSE_BUTTON_LEFT and event.pressed:
		var now := Time.get_ticks_msec()
		if now-last_tap_msec <= 320:
			expanded = not expanded
			_resize_map()
			last_tap_msec = -1000
		else: last_tap_msec = now
		accept_event()

func map_rect() -> Rect2:
	var factor := minf((size.x-8)/544.0,(size.y-8)/912.0)
	var dimensions := Vector2(544,912)*factor
	return Rect2((size-dimensions)/2,dimensions)

func map_position(at: Vector2) -> Vector2:
	var rect := map_rect()
	return rect.position+at*(rect.size.x/544.0)

func _draw() -> void:
	if world == null or world.textures.is_empty(): return
	draw_rect(Rect2(Vector2.ZERO,size),Color("101f18"))
	draw_rect(Rect2(1,1,size.x-2,size.y-2),Color("c89b4b"),false,2)
	var rect := map_rect()
	draw_texture_rect(world.textures.ground,rect,false)
	var factor := rect.size.x/544.0
	for object in world.placements:
		if object.type == "tree":
			var art: Dictionary = world.assets[object.id]
			var region: Array = art.rect
			var dimensions := Vector2(region[2],region[3])*(float(object.h)/float(region[3]))*factor
			draw_texture_rect_region(world.textures[art.image],Rect2(map_position(Vector2(object.x,object.y))-Vector2(dimensions.x/2,dimensions.y),dimensions),Rect2(region[0],region[1],region[2],region[3]))
		elif object.type == "flag":
			var at := map_position(Vector2(object.x,object.y))
			draw_line(at,at-Vector2(0,7),Color.WHITE,1)
			draw_colored_polygon(PackedVector2Array([at-Vector2(0,7),at+Vector2(5,-5),at-Vector2(0,3)]),Color("ff3030"))
	var at := map_position(world.ball_world)
	if world.native_state.get("can_shoot",false):
		var angle := float(world.native_state.get("aim_raw",0))*TAU/4096.0
		draw_line(at,at+Vector2(sin(angle),cos(angle))*12,Color("fff24a"),1)
	draw_circle(at,2.5,Color("102620"))
	draw_circle(at,1.5,Color.WHITE)
