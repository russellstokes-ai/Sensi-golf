extends RefCounted
class_name PixteeUIStyle
static func frame(canvas: CanvasItem, rect: Rect2, highlight := false) -> void:
	canvas.draw_style_box(box(Color("071c16"),Color("e3be62") if highlight else Color("9e8248"),4),rect)
	canvas.draw_rect(rect.grow(-3),Color(0.08,0.16,0.12,0.9),false,1)
	canvas.draw_line(rect.position+Vector2(5,2),rect.position+Vector2(rect.size.x-5,2),Color("f5d991"),1)
static func box(fill: Color, rim: Color, radius := 4) -> StyleBoxFlat:
	var style := StyleBoxFlat.new()
	style.bg_color = fill
	style.border_color = rim
	style.set_border_width_all(1)
	style.set_corner_radius_all(radius)
	style.content_margin_left = 10
	style.content_margin_right = 10
	style.content_margin_top = 8
	style.content_margin_bottom = 8
	style.shadow_color = Color(0.01,0.04,0.03,0.24)
	style.shadow_size = 3
	style.shadow_offset = Vector2(0,2)
	return style
static func theme() -> Theme:
	var theme := Theme.new()
	theme.default_font_size = 16
	for type in ["OptionButton","LineEdit","Button"]:
		theme.set_color("font_color",type,Color("fff0c7"))
		theme.set_color("font_hover_color",type,Color("fff9db"))
		theme.set_color("font_disabled_color",type,Color("91a898"))
		theme.set_stylebox("normal",type,box(Color("102c23"),Color("917944")))
		theme.set_stylebox("hover",type,box(Color("183d2e"),Color("e2c374")))
		theme.set_stylebox("pressed",type,box(Color("0a211b"),Color("f7d47e")))
		theme.set_stylebox("focus",type,box(Color(0,0,0,0),Color("f5d98b")))
	theme.set_stylebox("panel","PopupMenu",box(Color("0b261e"),Color("c1a66a")))
	theme.set_stylebox("hover","PopupMenu",box(Color("2a4a35"),Color("b2a56c")))
	theme.set_color("font_color","PopupMenu",Color("fff0c7"))
	return theme
