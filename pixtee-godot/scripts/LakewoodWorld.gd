extends Node2D

const ROOT := "res://courses/lakewood-hole-1/visual/"
var placements: Array = []
var assets: Dictionary = {}
var textures: Dictionary = {}
var boards: Array = []
var time := 0.0
var reduced_motion := false
var reduced_flashes := false
var visual_quality := 1
var native_state: Dictionary = {}
var ball_world := Vector2(256,850)
var golfer_world := Vector2(249,853)
var golfer_position_initialized := false
var view_rect := Rect2(0,0,544,912)

func _ready() -> void:
	texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	var data: Dictionary = JSON.parse_string(FileAccess.get_file_as_string(ROOT+"render-assets.json"))
	placements = data.objects
	assets = data.assets
	boards = data.boards
	for item in assets.values():
		var key: String = item.image
		if not textures.has(key):
			textures[key] = load(ROOT+key+".png")
	textures.ground = load(ROOT+"ground.png")
	textures.ball = load(ROOT+"ball.png")
	for i in boards.size():
		textures["board-%d" % i] = load(ROOT+"board-%d.png" % i)
	var layout: Dictionary = JSON.parse_string(FileAccess.get_file_as_string("res://courses/lakewood-hole-1/course-layout.json"))
	for key in ["water", "river"]:
		var water := Polygon2D.new()
		var polygon := PackedVector2Array()
		for p in layout.surfaces[key]:
			polygon.append(Vector2(p[0],p[1]))
		water.polygon = polygon
		water.texture = textures.water0
		water.texture_scale = Vector2(0.12,0.12)
		water.texture_repeat = CanvasItem.TEXTURE_REPEAT_ENABLED
		water.modulate.a = 0.22
		water.z_index = -1
		var material := ShaderMaterial.new()
		material.shader = load("res://shaders/pixtee_subtle_water.gdshader")
		material.set_shader_parameter("presentation_time",time)
		material.set_shader_parameter("repeat_surface",true)
		material.set_shader_parameter("flow_direction",Vector2(0.3,1.0) if key == "river" else Vector2(1.0,0.2))
		material.set_shader_parameter("flow_texels_per_second",2.0 if key == "river" else 0.35)
		water.material = material
		add_child(water)
	# Ground stays beneath water and objects; scale is uniform 4x export -> 1x world.
	var ground := Sprite2D.new()
	ground.texture = textures.ground
	ground.centered = false
	ground.scale = Vector2(0.25,0.25)
	ground.z_index = -2
	add_child(ground)
	var atmosphere := preload("res://scripts/PixteeCourseAtmosphere.gd").new()
	atmosphere.world = self
	atmosphere.z_index = 1
	add_child(atmosphere)

func accept_snapshot(state: Dictionary) -> void:
	native_state = state
	ball_world = Vector2(float(state.get("ball_x_raw",0)),float(state.get("ball_y_raw",0))) / 65536.0
	if state.get("green_mode",false):
		ball_world = ball_world * 0.5 + Vector2(state.green_origin_x,state.green_origin_y)
	# Keep the golfer at the lie while the ball travels. Only an actual native
	# ready-for-shot state moves the golfer to the next lie/recovery position.
	if not golfer_position_initialized or int(state.get("hole_phase",-1)) == 0:
		golfer_world = ball_world + Vector2(-7,3)
		golfer_position_initialized = true
	queue_redraw()

func bird_frame_rect(species: String, frame: int, centre: Vector2) -> Rect2:
	# Preserve the original 64px animation canvas and a single pixel scale.
	# Cropping each flap to its own bounds and forcing it to 5px high made
	# the bird body change size whenever its wings changed position.
	var asset: Dictionary = assets[species+str(frame)]
	var canvas: Vector2 = textures[asset.image].get_size() * 0.125
	return Rect2(centre-canvas*0.5,canvas)

func _process(delta: float) -> void:
	if not reduced_motion:
		time += minf(delta,0.1)
		for child in get_children():
			if child is Polygon2D: child.material.set_shader_parameter("presentation_time",time)
		queue_redraw()

func _draw() -> void:
	if assets.is_empty(): return
	# Grounded silhouettes retain each approved sprite's proportions and alpha.
	# Draw every shadow before any object so foliage cannot shade foreground art.
	for object in placements:
		if object.type not in ["tree","golfer","photographer"]: continue
		var anchor := Vector2(object.x,object.y)
		if object.type == "golfer" and golfer_position_initialized: anchor = golfer_world
		if not view_rect.grow(70).has_point(anchor): continue
		var art: Dictionary = assets[object.id]
		var region: Array = art.rect
		var dimensions := Vector2(region[2],region[3]) * (float(object.h)/float(region[3]))
		draw_texture_rect_region(textures[art.image],Rect2(anchor-Vector2(dimensions.x*0.5,dimensions.y)+Vector2(4,5),dimensions),Rect2(region[0],region[1],region[2],region[3]),Color(0.06,0.10,0.13,0.20))
	for o in placements:
		var pos := Vector2(o.x,o.y)
		if o.type == "golfer":
			pos = golfer_world if golfer_position_initialized else pos
		if not view_rect.grow(60).has_point(pos): continue
		if o.type == "board":
			for i in boards.size():
				var b: Dictionary = boards[i]
				if b.x == o.x and b.y == o.y:
					draw_texture_rect(textures["board-%d" % i],Rect2(pos-Vector2(b.w/2.0,o.h+3),Vector2(b.w,b.h)),false)
			continue
		if o.type == "flag":
			draw_line(pos,pos-Vector2(0,o.h),Color("ebe6cd"),0.7)
			var flutter := 0.0 if reduced_motion else sin(time*6.5)*0.6
			draw_colored_polygon(PackedVector2Array([pos-Vector2(0,o.h),pos+Vector2(8,-o.h+3+flutter),pos+Vector2(0,-o.h+7)]),Color("df3527"))
			continue
		var a: Dictionary = assets[o.id]
		var rect: Array = a.rect
		if a.has("frames") and not reduced_motion and not (o.type == "photographer" and reduced_flashes):
			var rate := 0.85 if o.type != "photographer" else 0.43
			rect = a.frames[int(time/rate+o.phase) % a.frames.size()]
		# Every frame keeps its own aspect ratio; there is no nonuniform sprite scale.
		var size := Vector2(float(rect[2]),float(rect[3])) * (float(o.h)/float(a.rect[3]))
		var angle := 0.0
		if o.type in ["tree","grass","flowers","reeds"] and not reduced_motion and visual_quality > 0:
			angle = sin(time*0.7+o.phase)*0.009
		draw_set_transform(pos,angle,Vector2(-1 if o.get("flipX",false) else 1,1))
		draw_texture_rect_region(textures[a.image],Rect2(Vector2(-size.x/2,-size.y),size),Rect2(rect[0],rect[1],rect[2],rect[3]))
		draw_set_transform(Vector2.ZERO)
	# Side-on birds use the supplied flight frames. Cosmetic time and path only;
	# no bird or ambient animation reads or advances the gameplay PRNG.
	if not reduced_motion and visual_quality > 0:
		var flight := fposmod(time,79.0)
		if flight > 11.0 and flight < 35.0:
			var progress := (flight-11.0)/24.0
			var species: String = ["bird","gull","blackbird","kingfisher"][int(time/79.0) % 4]
			var a: Dictionary = assets[species+str(int(time/0.19) % 8)]
			var at := Vector2(-25+progress*600,650+sin(progress*3.5)*15)
			draw_texture_rect(textures[a.image],bird_frame_rect(species,int(time/0.19) % 8,at),false)
	if not native_state.is_empty():
		var z := maxf(0,float(native_state.get("ball_z_raw",0))/65536.0)
		var ball_texture: Texture2D = textures.ball
		var ball_size := ball_texture.get_size() * (3.0/ball_texture.get_height())
		draw_texture_rect(textures.ball,Rect2((ball_world-Vector2(1.5,z+1.5)).round(),ball_size),false)
		if native_state.get("can_shoot",false):
			var heading := float(native_state.get("aim_raw",0))*TAU/4096.0
			var direction := Vector2(sin(heading),cos(heading))
			for i in range(3,12,2):
				draw_circle((ball_world+direction*i*3).round(),0.7,Color(1,1,0.8,0.7))
