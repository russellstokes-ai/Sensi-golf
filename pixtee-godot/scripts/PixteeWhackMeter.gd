extends Control

## Presentation only. All readings, capture state and lie boundaries come from C++.
signal pressed
var state: Dictionary = {}
var instruction := "1  START SWING"

func _ready() -> void:
	custom_minimum_size = Vector2(0,82)
	mouse_default_cursor_shape = Control.CURSOR_POINTING_HAND
	gui_input.connect(_input_meter)

func _input_meter(event: InputEvent) -> void:
	if event is InputEventMouseButton and event.button_index == MOUSE_BUTTON_LEFT and event.pressed:
		pressed.emit()
		accept_event()

func accept_snapshot(snapshot: Dictionary) -> void:
	state = snapshot
	var active: bool = state.get("original_welly_active",false)
	var captured: bool = state.get("welly_power_captured",false)
	var phase: int = int(state.get("welly_phase",0))
	instruction = "1  START SWING"
	if active:
		instruction = "2  SET POWER" if not captured else ("RETURNING…" if phase == 1 else "3  SET ACCURACY")
	elif state.get("original_welly_rejected",false):
		instruction = "MISSED WINDOW · START AGAIN"
	elif not state.get("can_shoot",false):
		instruction = "BALL IN PLAY"
	tooltip_text = "Start, select power, then stop the return in red. Yellow left hooks; yellow right slices. Outside yellow misses."
	queue_redraw()

func accuracy_x(reading: float, width: float) -> float:
	# Fixed axis: changing lie changes the zones, never the length of the bar.
	return 10.0 + (reading+0.5)/71.0*(width-20.0)

func _draw() -> void:
	var width := size.x
	var font := ThemeDB.fallback_font
	draw_style_box(_panel(),Rect2(Vector2.ZERO,size))
	draw_string(font,Vector2(10,19),instruction,HORIZONTAL_ALIGNMENT_LEFT,width-20,14,Color("fff2b5"))
	var rail := Rect2(10,30,width-20,24)
	draw_rect(rail.grow(2),Color("070e16"))
	draw_rect(rail,Color("435965"))
	var cell := (width-20)/71.0
	if state.has("welly_window_low"):
		var low := int(state.welly_window_low)
		var high := int(state.welly_window_high)
		var centre := int(state.welly_window_center)
		draw_rect(Rect2(accuracy_x(low,width)-cell/2,31,(high-low+1)*cell,22),Color("f4c84b"))
		draw_rect(Rect2(accuracy_x(centre,width)-cell/2,31,cell,22),Color("e64032"))
		# A narrow white notch identifies the exact native straight reading.
		draw_line(Vector2(accuracy_x(centre,width),55),Vector2(accuracy_x(centre,width),60),Color.WHITE,2)
	var phase := int(state.get("welly_phase",0))
	var active: bool = state.get("original_welly_active",false)
	if active:
		var x: float
		if phase == 1:
			x = 10.0+clampf(float(state.get("welly_counter",0))/105.0,0,1)*(width-20)
		else:
			x = accuracy_x(clampf(float(state.get("welly_press_accuracy",0)),0,70),width)
		draw_circle(Vector2(x,42),7,Color("111923"))
		draw_circle(Vector2(x,42),5,Color("fffde8"))
		draw_circle(Vector2(x-1,40),1,Color.WHITE)
	if state.get("welly_power_captured",false):
		var power := int(state.get("welly_saved_power",0))
		draw_string(font,Vector2(10,73),"POWER %d%%" % roundi(power*100.0/105.0),HORIZONTAL_ALIGNMENT_LEFT,110,12,Color("fff2b5"))
	else:
		draw_string(font,Vector2(10,73),"TAP BAR OR SWING",HORIZONTAL_ALIGNMENT_LEFT,150,11,Color("cbd3d4"))
	draw_string(font,Vector2(maxf(155,width-190),73),"HOOK  ·  STRAIGHT  ·  SLICE",HORIZONTAL_ALIGNMENT_RIGHT,180,10,Color("f4c84b"))

func _panel() -> StyleBoxFlat:
	var panel := StyleBoxFlat.new()
	panel.bg_color = Color("172821")
	panel.border_color = Color("997b39")
	panel.set_border_width_all(2)
	return panel
