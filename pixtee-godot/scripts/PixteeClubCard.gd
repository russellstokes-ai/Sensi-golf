extends Control
signal club_selected(index: int)
const CLUBS := ["Driver","3 Wood","5 Wood","2 Iron","3 Iron","4 Iron","5 Iron","6 Iron","7 Iron","8 Iron","9 Iron","Wedge","Putter"]
var state: Dictionary = {}
var sky: Control
var club_icon: Texture2D
var weather_source: Node
var time_source: RefCounted
var reduced_motion := false
var foreground: Control

func _ready() -> void:
	texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	club_icon = load("res://ui/supplied/club.png")
	sky = preload("res://scripts/PixteeHUDWeather.gd").new()
	add_child(sky)
	sky.position = Vector2(4,4)
	sky.size = Vector2(size.x-8,57)
	foreground = Control.new()
	foreground.mouse_filter = Control.MOUSE_FILTER_IGNORE
	foreground.draw.connect(_draw_wind.bind(foreground))
	add_child(foreground)
	gui_input.connect(_tap)

func lie_visual() -> Dictionary:
	var surface := int(state.get("surface_code",3))
	return {"surface":surface,"tee":int(state.get("strokes",1))==0 and not state.get("green_mode",false) and state.get("terrain_available",true),"occlusion":5.0 if surface==7 else 7.0 if surface==6 else 4.0 if surface==5 else 0.0}

func accept_snapshot(snapshot: Dictionary) -> void:
	state = snapshot.duplicate()
	mouse_default_cursor_shape = Control.CURSOR_POINTING_HAND if state.get("can_shoot",false) else Control.CURSOR_ARROW
	if sky != null: sky.lie = lie_visual(); sky.queue_redraw()
	queue_redraw()
	if foreground != null: foreground.queue_redraw()

func accept_weather(values: Dictionary, preset: String, time: float, reduce: bool) -> void:
	reduced_motion = reduce
	if sky == null: return
	sky.weather = values.duplicate()
	sky.preset = preset
	sky.clock = time
	sky.queue_redraw()

func _process(_delta: float) -> void:
	if not is_visible_in_tree(): return
	if sky != null: sky.size = Vector2(size.x-8,57)
	if weather_source != null and time_source != null:
		accept_weather(weather_source.conditions(),weather_source.preset_name,time_source.time_seconds,time_source.reduced_motion)

func _tap(event: InputEvent) -> void:
	if event is InputEventMouseButton and event.button_index == MOUSE_BUTTON_LEFT and event.pressed:
		if state.get("can_shoot",false): club_selected.emit((int(state.get("club_index",0))+1)%CLUBS.size())
		accept_event()

func wind_direction() -> Vector2:
	var angle := float(state.get("wind_toward_raw",1536))*TAU/4096.0
	return Vector2(sin(angle),cos(angle))

func _draw() -> void:
	# Pixel-stepped gold bezel and a deep evergreen face; no generic grey panel.
	draw_style_box(PixteeUIStyle.box(Color("071c16"),Color("151f18"),1),Rect2(-1,1,size.x+2,size.y+2))
	draw_rect(Rect2(Vector2.ZERO,size),Color("cf9f3c"))
	draw_rect(Rect2(1,1,size.x-2,size.y-2),Color("f5cf69"),false,1)
	draw_rect(Rect2(3,3,size.x-6,size.y-6),Color("071c16"))
	draw_line(Vector2(4,2),Vector2(size.x-4,2),Color("fff0a1"),1)
	var font := ThemeDB.fallback_font
	draw_string(font,Vector2(10,94),str(int(state.get("distance_to_hole",0))),HORIZONTAL_ALIGNMENT_LEFT,size.x*.54,33,Color("fff9df"))
	draw_string(font,Vector2(size.x*.59,91),"To hole",HORIZONTAL_ALIGNMENT_LEFT,size.x*.40-3,12,Color("f0f2df"))
	draw_line(Vector2(9,101),Vector2(size.x-9,101),Color("36594b"),1)
	var dims := club_icon.get_size()*(23.0/club_icon.get_height()) if club_icon else Vector2.ZERO
	if club_icon: draw_texture_rect(club_icon,Rect2(Vector2(10,105),dims),false)
	var club: String = CLUBS[clampi(int(state.get("club_index",0)),0,12)]
	draw_string(font,Vector2(37,123),club,HORIZONTAL_ALIGNMENT_LEFT,size.x-91,15,Color("ffefac"))
	if state.has("club_nominal_max"):
		draw_line(Vector2(size.x-56,107),Vector2(size.x-56,127),Color("36594b"),1)
		draw_string(font,Vector2(size.x-50,114),"Max",HORIZONTAL_ALIGNMENT_LEFT,45,8,Color("99b4a0"))
		draw_string(font,Vector2(size.x-50,127),str(state.club_nominal_max),HORIZONTAL_ALIGNMENT_LEFT,45,14,Color("fff8d9"))

func _draw_wind(canvas: CanvasItem) -> void:
	# Drawn by a foreground child, above the animated sky without clipping labels.
	var font := ThemeDB.fallback_font
	var centre := Vector2(size.x-19,27)
	canvas.draw_style_box(PixteeUIStyle.box(Color(.025,.12,.15,.22),Color(0,0,0,0),2),Rect2(size.x-73,10,67,45))
	var wind := wind_direction()
	var speed := int(state.get("wind_speed_mph_x10",0)) if state.get("wind_enabled",false) else 0
	if speed>0:
		var side := Vector2(-wind.y,wind.x)
		canvas.draw_colored_polygon(PackedVector2Array([centre+wind*8,centre-wind*5+side*4,centre-wind*2,centre-wind*5-side*4]),Color("f2ffff"))
	else: canvas.draw_circle(centre,2,Color("dcf4ed"))
	canvas.draw_string(font,Vector2(size.x-68,49),"%d mph" % roundi(float(speed)/10.0),HORIZONTAL_ALIGNMENT_RIGHT,58,11,Color("ffffff"))
