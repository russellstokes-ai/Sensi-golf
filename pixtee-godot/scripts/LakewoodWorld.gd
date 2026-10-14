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
var golfer_facing_left := false
var player_sprite: Sprite2D
var player_upper: Sprite2D
var character_atlas: Dictionary = {}
var swing_time := 0.0
var previous_shot_tick := -1
var outfit_id := "lakewood-classic"
var appearance := {"skin":0,"hair":0,"face":0,"beard":0,"hairstyle":0}
var face_regions: Dictionary = {}
var pond_ducks: Node2D
var living_world: Node2D
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
		water.texture = load("res://addons/pixtee_living_world/assets/premium/river/tileable_flow/flow_00.png")
		water.texture_scale = Vector2(1.0,1.0)
		water.texture_repeat = CanvasItem.TEXTURE_REPEAT_ENABLED
		water.modulate.a = 0.14
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
	face_regions = JSON.parse_string(FileAccess.get_file_as_string("res://characters/face-regions-v1.json"))
	character_atlas = JSON.parse_string(FileAccess.get_file_as_string("res://characters/atlas-v1.json"))
	textures.female = load("res://characters/female-cute-v2.png")
	textures.male = load("res://characters/male-cute-v2.png")
	textures.male_idle = load("res://characters/male-directions-v1.png")
	player_sprite = Sprite2D.new()
	player_sprite.texture = textures.golfer
	player_sprite.centered = false
	player_sprite.region_enabled = true
	player_sprite.material = PixteeOutfit.material(outfit_id)
	add_child(player_sprite)
	player_upper = Sprite2D.new()
	player_upper.centered = false
	player_upper.region_enabled = true
	add_child(player_upper)
	var atmosphere := preload("res://scripts/PixteeCourseAtmosphere.gd").new()
	atmosphere.world = self
	atmosphere.z_index = 1
	add_child(atmosphere)
	pond_ducks = preload("res://scripts/PixteePondDucks.gd").new()
	pond_ducks.world = self
	pond_ducks.z_index = 2
	add_child(pond_ducks)
	var birds := preload("res://scripts/PixteeLayeredBirds.gd").new()
	birds.world = self
	birds.z_index = 3
	add_child(birds)
	var caddy := preload("res://scripts/PixteeCaddy.gd").new()
	caddy.name = "Caddy"
	caddy.world = self
	caddy.z_index = 1
	add_child(caddy)
	var living := preload("res://scripts/PixteeLivingWorld.gd").new()
	living_world = living
	living.world = self
	living.z_index = 1
	add_child(living)

func accept_snapshot(state: Dictionary, hold_golfer := false) -> void:
	native_state = state
	ball_world = Vector2(float(state.get("ball_x_raw",0)),float(state.get("ball_y_raw",0))) / 65536.0
	if state.get("green_mode",false):
		ball_world = ball_world * 0.5 + Vector2(state.green_origin_x,state.green_origin_y)
	# Keep the golfer at the lie while the ball travels. Only an actual native
	# ready-for-shot state moves the golfer to the next lie/recovery position.
	if not golfer_position_initialized or (int(state.get("hole_phase",-1)) == 0 and not hold_golfer):
		var heading := float(state.get("aim_raw",2016))*TAU/4096.0
		var direction := Vector2(sin(heading),cos(heading))
		golfer_world = ball_world-direction.rotated(PI/2.0)*7.0-direction*3.0
		golfer_position_initialized = true
		golfer_facing_left = sin(float(state.get("aim_raw",0))*TAU/4096.0) < 0.0
	update_player_sprite()
	queue_redraw()

func bird_frame_rect(species: String, frame: int, centre: Vector2) -> Rect2:
	# Preserve the original 64px animation canvas and a single pixel scale.
	# Cropping each flap to its own bounds and forcing it to 5px high made
	# the bird body change size whenever its wings changed position.
	var asset: Dictionary = assets[species+str(frame)]
	var canvas: Vector2 = textures[asset.image].get_size() * 0.18
	return Rect2(centre-canvas*0.5,canvas)

func _process(delta: float) -> void:
	if not reduced_motion:
		time += minf(delta,0.1)
		for child in get_children():
			if child is Polygon2D: child.material.set_shader_parameter("presentation_time",time)
		update_player_sprite()
		queue_redraw()

func _draw() -> void:
	if assets.is_empty(): return
	# Grounded silhouettes retain each approved sprite's proportions and alpha.
	# Draw every shadow before any object so foliage cannot shade foreground art.
	for object in placements:
		if object.type not in ["tree","photographer","person"]: continue
		var anchor := Vector2(object.x,object.y)
		if object.type == "golfer" and golfer_position_initialized: anchor = golfer_world
		if not view_rect.grow(70).has_point(anchor): continue
		if object.type in ["person","photographer"]:
			draw_ellipse(anchor+Vector2(0,0.5),Vector2(2.8,1.1),Color(0.025,0.08,0.06,0.24))
			continue
		var art: Dictionary = assets[object.id]
		var region: Array = art.rect
		var dimensions := Vector2(region[2],region[3]) * (float(object.h)/float(region[3]))
		draw_texture_rect_region(textures[art.image],Rect2(anchor-Vector2(dimensions.x*0.5,dimensions.y)+Vector2(4,5),dimensions),Rect2(region[0],region[1],region[2],region[3]),Color(0.06,0.10,0.13,0.20))
	# A grounded contact shadow follows the feet, never a detached upright copy.
	draw_ellipse(golfer_world+Vector2(0,1),Vector2(5,1.7),Color(0.025,0.08,0.06,0.30))
	for o in placements:
		var pos := Vector2(o.x,o.y)
		if o.type == "golfer": continue
		if not view_rect.grow(60).has_point(pos): continue
		if o.type == "board":
			draw_ellipse(pos+Vector2(1,1),Vector2(float(o.w)*0.45,2.2),Color(0.03,0.08,0.06,0.20))
			for i in boards.size():
				var b: Dictionary = boards[i]
				if b.x == o.x and b.y == o.y:
					draw_texture_rect(textures["board-%d" % i],Rect2(pos-Vector2(b.w/2.0,o.h+3),Vector2(b.w,b.h)),false)
			continue
		if o.type == "flag":
			if native_state.get("flag_removed_for_putting",false):
				draw_ellipse(pos,Vector2(1.2,0.7),Color(0.04,0.08,0.035))
				continue
			draw_line(pos,pos-Vector2(0,o.h),Color("ebe6cd"),0.7)
			var flutter := 0.0 if reduced_motion else sin(time*6.5)*0.6
			draw_colored_polygon(PackedVector2Array([pos-Vector2(0,o.h),pos+Vector2(8,-o.h+3+flutter),pos+Vector2(0,-o.h+7)]),Color("df3527"))
			continue
		var a: Dictionary = assets[o.id]
		var rect: Array = a.rect
		if a.has("frames") and not reduced_motion and not (o.type == "photographer" and reduced_flashes):
			var rate := 0.26 if o.type != "photographer" else 0.18
			rect = a.frames[int(time/rate+o.phase) % a.frames.size()]
		# Every frame keeps its own aspect ratio; there is no nonuniform sprite scale.
		var size := Vector2(float(rect[2]),float(rect[3])) * (float(o.h)/float(a.rect[3]))
		var angle := 0.0
		if o.type in ["tree","grass","flowers","reeds"] and not reduced_motion and visual_quality > 0:
			angle = sin(time*0.7+o.phase)*0.016
		var flipped: bool = golfer_facing_left if o.type == "golfer" else o.get("flipX",false)
		draw_set_transform(pos,angle,Vector2(-1 if flipped else 1,1))
		if o.type in ["person","photographer"]:
			var split := 0.58
			var breath := 0.0 if reduced_motion else sin(time*1.7+float(o.phase))*0.32
			draw_texture_rect_region(textures[a.image],Rect2(Vector2(-size.x/2,-size.y*(1-split)),Vector2(size.x,size.y*(1-split))),Rect2(rect[0],float(rect[1])+float(rect[3])*split,rect[2],float(rect[3])*(1-split)))
			draw_texture_rect_region(textures[a.image],Rect2(Vector2(-size.x/2,-size.y+breath),Vector2(size.x,size.y*split)),Rect2(rect[0],rect[1],rect[2],float(rect[3])*split))
		else:
			draw_texture_rect_region(textures[a.image],Rect2(Vector2(-size.x/2,-size.y),size),Rect2(rect[0],rect[1],rect[2],rect[3]))
		draw_set_transform(Vector2.ZERO)
	# Collidable birds are displayed from the native clock and altitude.
	if native_state.get("bird_active",false):
		var species: String = ["bird","gull","blackbird","kingfisher"][(int(native_state.get("bird_id",1))-1)%4]
		for member in range(int(native_state.get("bird_group_size",1))):
			var hit: bool = native_state.get("bird_hit",false) and member == int(native_state.get("bird_hit_member",0))
			if hit and not native_state.get("bird_hit_visible",true): continue
			if not hit and not native_state.get("bird_flock_active",true): continue
			var frame := 4 if reduced_motion else (int(native_state.get("authored_clock",0)/13)+member*2)%8
			var art: Dictionary = assets[species+str(frame)]
			var ground_at := Vector2(float(native_state.get("bird_flock_x_raw",native_state.bird_x_raw)),float(native_state.get("bird_flock_y_raw",native_state.bird_y_raw)))/65536.0
			var altitude := float(native_state.get("bird_flock_z_raw",native_state.bird_z_raw))/65536.0
			if member > 0:
				ground_at += Vector2(-member*12,8 if member == 1 else -8)
				altitude += 1 if member == 1 else -1
			if hit:
				ground_at = Vector2(float(native_state.bird_x_raw),float(native_state.bird_y_raw))/65536.0
				altitude = float(native_state.bird_z_raw)/65536.0
			var at := ground_at-Vector2(0,altitude)
			draw_circle(ground_at,2,Color(0.03,0.08,0.06,0.18),true,-1,true)
			draw_set_transform(at,0.35 if hit and not reduced_motion else 0.0)
			# Body and wing layers are rendered by PixteeLayeredBirds.
			draw_set_transform(Vector2.ZERO)
	if not native_state.is_empty():
		var z := maxf(0,float(native_state.get("ball_z_raw",0))/65536.0)
		var ball_texture: Texture2D = textures.ball
		var ball_size := ball_texture.get_size() * (3.0/ball_texture.get_height())
		draw_texture_rect(textures.ball,Rect2((ball_world-Vector2(1.5,z+1.5)).round(),ball_size),false)
		if native_state.get("can_shoot",false):
			var heading := float(native_state.get("aim_raw",0))*TAU/4096.0
			var direction := Vector2(sin(heading),cos(heading))
			var length := aim_length(native_state)
			for i in range(1,9):
				draw_circle((ball_world+direction*length*float(i)/8.0).round(),0.65,Color(1,1,0.8,0.55))

func set_outfit(id: String) -> void:
	if not PixteeOutfit.is_valid(id) or id == outfit_id: return
	outfit_id = id
	if player_sprite != null: player_sprite.material = PixteeOutfit.material(id,appearance)
	update_player_sprite()

func set_appearance(value: Dictionary) -> void:
	if not PixteeOutfit.valid_appearance(value): return
	appearance = value.duplicate(true)
	if player_sprite != null: player_sprite.material = PixteeOutfit.material(outfit_id,appearance)
	update_player_sprite()

func update_player_sprite() -> void:
	if player_sprite == null: return
	var female: bool = PixteeOutfit.female(outfit_id)
	var heading := posmod(int(native_state.get("aim_raw",2048))+2048,4096)
	var direction := posmod(roundi(float(heading)/512.0),8)
	var touch := int(native_state.get("touch_meter_phase",0))
	var frame := 0
	if touch > 0:
		frame = 3 if touch == 2 else mini(3,int(native_state.get("touch_meter_position",160))/300)
	elif int(native_state.get("hole_phase",0)) == 1:
		var tick := int(native_state.get("shot_tick",0))
		frame = 4+mini(3,tick/8) if tick<32 else 0
	var metadata: Dictionary
	var grid := Vector2(8,8)
	if female:
		player_sprite.texture = textures.female
		metadata = character_atlas.female_cute[frame*8+direction]
	else:
		player_sprite.texture = textures.male
		metadata = character_atlas.male_cute[frame*8+direction]
	var rect: Array = metadata.rect
	player_sprite.region_rect = Rect2(rect[0],rect[1],rect[2],rect[3])
	var base: Dictionary = character_atlas.female_cute[direction] if female else character_atlas.male_cute[direction]
	var factor := 21.0/float(base.height)
	player_sprite.scale = Vector2.ONE*factor
	player_sprite.flip_h = false
	var anchor: Array = metadata.anchor
	player_sprite.position = (golfer_world-Vector2(anchor[0],anchor[1])*factor).round()
	player_sprite.material.set_shader_parameter("atlas_grid",grid)
	player_sprite.material.set_shader_parameter("use_atlas_rect",true)
	player_sprite.material.set_shader_parameter("atlas_origin",Vector2(rect[0],rect[1])/player_sprite.texture.get_size())
	player_sprite.material.set_shader_parameter("atlas_extent",Vector2(rect[2],rect[3])/player_sprite.texture.get_size())
	var face: Array = face_regions["female" if female else "male"][frame*8+direction]
	player_sprite.material.set_shader_parameter("face_region",Vector4(face[0],face[1],face[2],face[3]))
	player_sprite.material.set_shader_parameter("layer_part",1)
	player_upper.texture = player_sprite.texture
	player_upper.region_rect = player_sprite.region_rect
	player_upper.scale = player_sprite.scale
	player_upper.position = player_sprite.position+Vector2(0,sin(time*2.0)*0.45 if frame == 0 and not reduced_motion else 0.0)
	if player_upper.material == null: player_upper.material = player_sprite.material.duplicate()
	for property in ["face_style","beard_style","hairstyle_style","face_region","skin_colour","hair_colour","skin_enabled","hair_enabled","shirt_colour","trouser_colour","cap_colour","shirt_enabled","trouser_enabled","cap_enabled","atlas_grid","use_atlas_rect","atlas_origin","atlas_extent"]:
		player_upper.material.set_shader_parameter(property,player_sprite.material.get_shader_parameter(property))
	player_upper.material.set_shader_parameter("layer_part",2)

func aim_length(snapshot: Dictionary) -> float:
	return lerpf(44.0,12.0,float(clampi(int(snapshot.get("club_index",0)),0,12))/12.0)

func draw_ellipse(at: Vector2, radius: Vector2, colour: Color) -> void:
	var points := PackedVector2Array()
	for i in range(24): points.append(at+Vector2(cos(i*TAU/24.0),sin(i*TAU/24.0))*radius)
	draw_colored_polygon(points,colour)
