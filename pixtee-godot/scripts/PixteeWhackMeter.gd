extends Control

## Native readings only; tweens/shader animate decoration, never timing or input.
signal pressed
var state: Dictionary = {}
var reduced_motion := false
var clock := 0.0
var transition := 0.0
var tween: Tween
var glow: ShaderMaterial
var trail: Array[float] = []
var previous_reading := -1

func _ready() -> void:
	custom_minimum_size = Vector2(0,64)
	mouse_filter = Control.MOUSE_FILTER_STOP
	gui_input.connect(_input_meter)
	var overlay := ColorRect.new()
	overlay.mouse_filter = Control.MOUSE_FILTER_IGNORE
	overlay.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	glow = ShaderMaterial.new()
	glow.shader = load("res://shaders/pixtee_meter_glow.gdshader")
	overlay.material = glow
	add_child(overlay)

func can_tap() -> bool:
	if state.get("can_shoot",false): return true
	return state.get("original_welly_active",false) and not (state.get("welly_power_captured",false) and int(state.get("welly_phase",0)) == 1)

func _input_meter(event: InputEvent) -> void:
	if event is InputEventMouseButton and event.button_index == MOUSE_BUTTON_LEFT and event.pressed:
		if can_tap(): pressed.emit()
		accept_event()

func accept_snapshot(snapshot: Dictionary) -> void:
	var changed: bool = state.get("original_welly_active",false) != snapshot.get("original_welly_active",false) or state.get("welly_power_captured",false) != snapshot.get("welly_power_captured",false) or state.get("welly_phase",0) != snapshot.get("welly_phase",0)
	state = snapshot
	mouse_default_cursor_shape = Control.CURSOR_POINTING_HAND if can_tap() else Control.CURSOR_ARROW
	if changed:
		trail.clear()
		if tween != null and tween.is_valid(): tween.kill()
		transition = 0.0 if reduced_motion else 1.0
		if not reduced_motion:
			tween = create_tween()
			tween.tween_property(self,"transition",0.0,0.32).set_trans(Tween.TRANS_QUAD).set_ease(Tween.EASE_OUT)
	var reading := int(state.get("welly_counter",0)) if int(state.get("welly_phase",0)) == 1 else int(state.get("welly_press_accuracy",0))
	if reading != previous_reading and state.get("original_welly_active",false) and not reduced_motion:
		trail.append(marker_fraction())
		if trail.size() > 5: trail.pop_front()
	previous_reading = reading
	if reduced_motion or not state.get("original_welly_active",false): trail.clear()
	queue_redraw()

func _process(delta: float) -> void:
	# Hidden/paused HUD freezes both shader and tween decoration.
	if not is_visible_in_tree():
		if tween != null and tween.is_valid(): tween.pause()
		return
	if tween != null and tween.is_valid(): tween.play()
	if not reduced_motion: clock += minf(delta,0.1)
	if glow != null:
		glow.set_shader_parameter("presentation_time",clock)
		glow.set_shader_parameter("ready",1.0 if state.get("can_shoot",false) else 0.0)
		glow.set_shader_parameter("transition",transition)
		glow.set_shader_parameter("motion_enabled",0.0 if reduced_motion else 1.0)
	queue_redraw()

func accuracy_x(reading: float, width: float) -> float:
	return 12.0+(reading+0.5)/71.0*(width-24.0)

func marker_fraction() -> float:
	if int(state.get("welly_phase",0)) == 1:
		return clampf(float(state.get("welly_counter",0))/105.0,0,1)
	return (clampf(float(state.get("welly_press_accuracy",0)),0,70)+0.5)/71.0

func _draw() -> void:
	var width := size.x
	draw_rect(Rect2(Vector2.ZERO,size),Color("10261f"))
	draw_rect(Rect2(1,1,width-2,size.y-2),Color("a88a45"),false,2)
	var rail := Rect2(12,17,width-24,26)
	draw_rect(rail.grow(3),Color("060e16"))
	draw_rect(rail,Color("344752"))
	draw_line(Vector2(13,18),Vector2(width-13,18),Color("68818a"),1)
	var cell := (width-24)/71.0
	if state.has("welly_window_low"):
		var low := int(state.welly_window_low)
		var high := int(state.welly_window_high)
		var centre := int(state.welly_window_center)
		draw_rect(Rect2(accuracy_x(low,width)-cell/2,18,(high-low+1)*cell,24),Color("ffeb23"))
		draw_rect(Rect2(accuracy_x(centre,width)-cell/2,18,cell,24),Color("ff2525"))
		draw_line(Vector2(accuracy_x(centre,width),44),Vector2(accuracy_x(centre,width),49),Color.WHITE,2)
	if state.get("original_welly_active",false):
		for i in trail.size():
			draw_circle(Vector2(12+trail[i]*(width-24),30),3+i*0.4,Color(1.0,0.94,0.63,0.04+i*0.035),true,-1,true)
		var at := Vector2(12+marker_fraction()*(width-24),30)
		draw_circle(at+Vector2(1,3),9,Color(0,0,0,0.45),true,-1,true)
		draw_circle(at,9,Color("13222c"),true,-1,true)
		draw_circle(at,7,Color("c4d8df"),true,-1,true)
		draw_circle(at-Vector2(1,1),5.5,Color("fffef1"),true,-1,true)
		draw_circle(at-Vector2(2,3),2,Color.WHITE,true,-1,true)
		for dot in [Vector2(2,0),Vector2(0,3),Vector2(-3,1)]:
			draw_circle(at+dot,0.8,Color("95b3be"),true,-1,true)
	if state.get("welly_power_captured",false):
		var saved := clampf(float(state.get("welly_saved_power",0))/105.0,0,1)
		var x := 12+saved*(width-24)
		draw_colored_polygon(PackedVector2Array([Vector2(x-3,9),Vector2(x+3,9),Vector2(x,14)]),Color("fff1a8"))
