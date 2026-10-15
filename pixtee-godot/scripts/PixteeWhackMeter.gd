extends Control
## The sole shot control. Native tick/readings own timing; this clock only decorates.
signal pressed
var state: Dictionary = {}
var touch_mode := true
var reduced_motion := false
var clock := 0.0
var transition := 0.0
var glow: ShaderMaterial
var weather := {"rain":0.0,"fog":0.0,"cloud":0.2,"sun":0.8,"wind":0.3}
var weather_preset := "sunny"
var weather_source: Node
var time_source: RefCounted
var trail: Array[float] = []
var previous_reading := -1
var last_phase := 0

func _ready() -> void:
	custom_minimum_size = Vector2(0,64)
	mouse_filter = Control.MOUSE_FILTER_STOP
	gui_input.connect(_input_meter)
	var overlay := ColorRect.new()
	overlay.mouse_filter = Control.MOUSE_FILTER_IGNORE
	overlay.show_behind_parent = true
	overlay.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	glow = ShaderMaterial.new()
	glow.shader = load("res://shaders/pixtee_meter_glow.gdshader")
	overlay.material = glow
	add_child(overlay)

func can_tap() -> bool:
	if state.get("can_shoot",false): return true
	if touch_mode: return state.get("touch_meter_active",false)
	return state.get("original_welly_active",false) and not (state.get("welly_power_captured",false) and int(state.get("welly_phase",0))==1)

func _input_meter(event: InputEvent) -> void:
	if event is InputEventMouseButton and event.button_index==MOUSE_BUTTON_LEFT and event.pressed:
		if can_tap(): pressed.emit()
		accept_event()

func accept_snapshot(snapshot: Dictionary) -> void:
	touch_mode = snapshot.has("touch_meter_position") and not snapshot.get("original_welly_active",false)
	state = snapshot.duplicate()
	if touch_mode:
		state.original_welly_active = snapshot.get("touch_meter_active",false)
		state.welly_phase = snapshot.get("touch_meter_phase",0)
		state.welly_power_captured = state.welly_phase==2
		state.welly_saved_power = snapshot.get("touch_meter_power",0)
	var phase := int(state.get("welly_phase",0))
	if phase!=last_phase:
		transition = 0.0 if reduced_motion else 1.0
		trail.clear()
	last_phase = phase
	var reading := int(state.get("touch_meter_position",160)) if touch_mode else int(state.get("welly_counter",0))
	if reading!=previous_reading and state.get("original_welly_active",false) and not reduced_motion:
		trail.append(marker_fraction())
		if trail.size()>5: trail.pop_front()
	previous_reading = reading
	if not state.get("original_welly_active",false): trail.clear()
	mouse_default_cursor_shape = Control.CURSOR_POINTING_HAND if can_tap() else Control.CURSOR_ARROW
	_update_material()
	queue_redraw()

func accept_weather(values: Dictionary, preset: String, time: float) -> void:
	weather = values.duplicate()
	weather_preset = preset
	clock = time
	_update_material()
	queue_redraw()

func _process(delta: float) -> void:
	if not is_visible_in_tree(): return
	if time_source != null and weather_source != null:
		reduced_motion = time_source.reduced_motion
		accept_weather(weather_source.conditions(),weather_source.preset_name,time_source.time_seconds)
	elif not reduced_motion: clock += minf(delta,.1)
	if reduced_motion: transition = 0; trail.clear()
	else: transition = maxf(0,transition-delta*3.2)
	_update_material()
	queue_redraw()

func rail_rect() -> Rect2:
	return Rect2(12,22,maxf(1,size.x-24),24)

func accuracy_x(reading: float, width: float) -> float:
	return 12.0+(1.0-(reading+0.5)/71.0)*(width-24.0)

func marker_fraction() -> float:
	if touch_mode: return 1.0-clampf(float(state.get("touch_meter_position",160))/1210.0,0,1)
	if int(state.get("welly_phase",0))==1: return 1.0-clampf(float(state.get("welly_counter",0))/105.0,0,1)
	return 1.0-(clampf(float(state.get("welly_press_accuracy",0)),0,70)+.5)/71.0

func target_bounds() -> Vector3:
	if touch_mode:
		# Idle bounds also come from the current native lie, not a hardcoded tee size.
		var span := clampi(int(state.get("welly_window_high",70))-int(state.get("welly_window_low",55))+1,1,16)
		var active: bool = state.get("touch_meter_active",false)
		var red := float(state.get("touch_meter_red_half",48)) if active else float(maxi(8,span*3))
		var yellow := float(state.get("touch_meter_yellow",64)) if active else float(maxi(12,span*4))
		return Vector3(1.0-160.0/1210.0,red/1210.0,yellow/1210.0)
	var centre := float(state.get("welly_window_center",63))
	return Vector3(1.0-(centre+.5)/71.0,.5/71.0,float(state.get("welly_window_high",70)-state.get("welly_window_low",55))/142.0)

func _update_material() -> void:
	if glow==null: return
	glow.set_shader_parameter("control_size",size)
	glow.set_shader_parameter("presentation_time",clock)
	glow.set_shader_parameter("ready",1.0 if state.get("can_shoot",false) else 0.0)
	glow.set_shader_parameter("phase",float(state.get("welly_phase",0)))
	glow.set_shader_parameter("transition",transition)
	glow.set_shader_parameter("motion_enabled",0.0 if reduced_motion else 1.0)
	glow.set_shader_parameter("target",target_bounds())
	glow.set_shader_parameter("marker",marker_fraction())
	glow.set_shader_parameter("rain",float(weather.get("rain",0)))
	glow.set_shader_parameter("fog",float(weather.get("fog",0)))
	glow.set_shader_parameter("golden",1.0 if weather_preset in ["golden_hour","sunset"] else 0.0)

func _draw() -> void:
	var rail := rail_rect()
	# Quiet scale inside the rail: power increases toward the left.
	for percent in [0,25,50,75,100]:
		var x: float = rail.position.x+(1.0-(160.0+percent*10.5)/1210.0)*rail.size.x
		draw_line(Vector2(x,42),Vector2(x,46),Color(.80,.93,1,.40),1)
	if state.get("original_welly_active",false):
		for i in trail.size():
			var at := Vector2(rail.position.x+trail[i]*rail.size.x,34)
			draw_circle(at,3+i*.45,Color(.38,.89,1,.025+i*.025),true,-1,true)
		var pointer := Vector2(rail.position.x+marker_fraction()*rail.size.x,34)
		# Slim luminous needle; outline keeps its exact centre readable over red/yellow.
		draw_line(pointer-Vector2(0,10),pointer+Vector2(0,10),Color("063449"),4,true)
		draw_line(pointer-Vector2(0,9),pointer+Vector2(0,9),Color("f3ffff"),2,true)
		draw_circle(pointer-Vector2(0,12),2.3,Color("ffffff"),true,-1,true)
		draw_colored_polygon(PackedVector2Array([pointer+Vector2(-3,14),pointer+Vector2(3,14),pointer+Vector2(0,10)]),Color("c8f7ff"))
	if state.get("welly_power_captured",false):
		var saved := 1.0-(160.0+float(state.get("welly_saved_power",0))*10.0)/1210.0 if touch_mode else 1.0-float(state.get("welly_saved_power",0))/105.0
		var x := rail.position.x+saved*rail.size.x
		draw_colored_polygon(PackedVector2Array([Vector2(x-3,15),Vector2(x+3,15),Vector2(x,20)]),Color("ffe69d"))
	if reduced_motion: return
	var rain := float(weather.get("rain",0))
	var wind := float(weather.get("wind",0))
	if rain>.01:
		for i in 18:
			var x := 15+fposmod(i*23.3,rail.size.x-6)
			var cycle := fposmod(clock*1.8+i*.618,1.0)
			var y := -5+cycle*27
			draw_line(Vector2(x+2, y),Vector2(x,y+4),Color(.35,.77,1,rain*.6),1,true)
			if cycle>.82:
				draw_arc(Vector2(x,22),1+(cycle-.82)*18,PI,TAU,8,Color(.3,.85,1,rain*(1-cycle)*3),1,true)
	elif wind>.2:
		for i in 3:
			var at := Vector2(12+fposmod(i*113+clock*(13+i*2),rail.size.x),14+sin(clock*1.1+i*2)*4)
			draw_set_transform(at,sin(clock+i)*.5)
			draw_colored_polygon(PackedVector2Array([Vector2(-3,0),Vector2(0,-1.6),Vector2(3,0),Vector2(0,1.6)]),Color("e7b24e") if weather_preset in ["golden_hour","sunset"] else Color("99c557"))
			draw_set_transform(Vector2.ZERO)
	if state.get("can_shoot",false) or state.get("original_welly_active",false):
		for i in 7:
			var phase := fposmod(clock*.65+i*.618,1.0)
			var x := rail.position.x+rail.size.x*(.77+fmod(i*.073,.22))
			var at := Vector2(x,22-phase*12)
			draw_circle(at,0.6+phase*.4,Color(1,.83,.24,sin(phase*PI)*.75),true,-1,true)
