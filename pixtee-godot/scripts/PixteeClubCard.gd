extends Control
signal club_selected(index: int)
const CLUBS := ["Driver","3 Wood","5 Wood","2 Iron","3 Iron","4 Iron","5 Iron","6 Iron","7 Iron","8 Iron","9 Iron","Wedge","Putter"]
var state: Dictionary = {}
var picker: PopupPanel
var list: ItemList
var ball: Texture2D
var club_icon: Texture2D
var lie_art: Texture2D

func _ready() -> void:
	texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	lie_art = load("res://courses/lakewood-hole-1/visual/ground.png")
	ball = load("res://courses/lakewood-hole-1/visual/ball.png")
	club_icon = load("res://ui/supplied/club.png")
	gui_input.connect(_tap)

func accept_snapshot(snapshot: Dictionary) -> void:
	state = snapshot
	mouse_default_cursor_shape = Control.CURSOR_POINTING_HAND if state.get("can_shoot",false) else Control.CURSOR_ARROW
	queue_redraw()

func _tap(event: InputEvent) -> void:
	if event is InputEventMouseButton and event.button_index == MOUSE_BUTTON_LEFT and event.pressed:
		if state.get("can_shoot",false):
			club_selected.emit((int(state.get("club_index",0))+1)%CLUBS.size())
		accept_event()

func wind_direction() -> Vector2:
	var angle := float(state.get("wind_toward_raw",1536))*TAU/4096.0
	return Vector2(sin(angle),cos(angle))

func _draw() -> void:
	draw_rect(Rect2(Vector2.ZERO,size),Color("101f18"))
	draw_rect(Rect2(1,1,size.x-2,size.y-2),Color("c89b4b"),false,2)
	var surface := int(state.get("surface_code",3))
	var ground := Color("457826")
	if surface == 7: ground = Color("d5b578")
	elif surface in [5,6]: ground = Color("285725")
	elif surface in [1,8,9,10]: ground = Color("71a938")
	var lie := Rect2(4,4,size.x-8,42)
	var x := float(state.get("ball_x_raw",256*65536))/65536.0
	var y := float(state.get("ball_y_raw",850*65536))/65536.0
	if state.get("green_mode",false):
		x = x*0.5+float(state.get("green_origin_x",0))
		y = y*0.5+float(state.get("green_origin_y",0))
	var region := Rect2(Vector2(x*4-lie.size.x/2,y*4-21),lie.size)
	draw_texture_rect_region(lie_art,lie,region)
	draw_rect(Rect2(4,34,size.x-8,14),ground)
	var diameter := 26.0
	var dimensions := ball.get_size()*(diameter/ball.get_height())
	draw_texture_rect(ball,Rect2(Vector2(size.x/2-dimensions.x/2,10),dimensions),false)
	var font := ThemeDB.fallback_font
	draw_string(font,Vector2(8,77),"%d" % int(state.get("distance_to_hole",0)),HORIZONTAL_ALIGNMENT_LEFT,size.x*0.58-10,28,Color("fff4cc"))
	draw_string(font,Vector2(9,91),"To hole",HORIZONTAL_ALIGNMENT_LEFT,75,12,Color("d5ddc5"))
	draw_line(Vector2(size.x*0.60,49),Vector2(size.x*0.60,92),Color("506b55"),0.7)
	var centre := Vector2(size.x*0.80,65)
	var wind := wind_direction()
	if state.get("wind_enabled",false) and int(state.get("wind_speed_mph_x10",0))>0:
		var side := Vector2(-wind.y,wind.x)
		var needle := PackedVector2Array([centre+wind*10.0,centre-wind*5.0+side*4.2,centre-wind*2.8,centre-wind*5.0-side*4.2])
		draw_colored_polygon(needle,Color("82d4f1"))
		draw_line(centre+wind*9,centre-wind*3+side*3.2,Color("d9f5fc"),1,true)
	else:
		draw_circle(centre,2.0,Color("a5c3b0"),true,-1,true)
	draw_string(font,Vector2(size.x*0.65,90),"%d mph" % roundi(float(state.get("wind_speed_mph_x10",0))/10.0),HORIZONTAL_ALIGNMENT_LEFT,size.x*0.35-3,12,Color("fff4cc"))
	var icon_size := club_icon.get_size()*(26.0/club_icon.get_height())
	draw_texture_rect(club_icon,Rect2(Vector2(9,93),icon_size),false)
	draw_string(font,Vector2(43,112),CLUBS[int(state.get("club_index",0))],HORIZONTAL_ALIGNMENT_LEFT,size.x-47,17,Color("ffedb1"))
	if not state.get("can_shoot",false): draw_rect(Rect2(Vector2.ZERO,size),Color(0,0,0,0.16))
