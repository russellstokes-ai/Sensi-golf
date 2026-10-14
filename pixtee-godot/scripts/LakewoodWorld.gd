extends Node2D

const ROOT := "res://courses/lakewood-hole-1/visual/"
var placements: Array = []
var assets: Dictionary = {}
var textures: Dictionary = {}
var boards: Array = []
var time := 0.0
var presentation := PixteePresentationState.new()
var object_nodes: Array[Node2D] = []
var player_rig: Node2D
var shadows: Node2D
var ball_presentation: Node2D
var water_nodes: Array[Polygon2D] = []
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
		water.z_index = -90
		var material := ShaderMaterial.new()
		material.shader = load("res://shaders/pixtee_subtle_water.gdshader")
		material.set_shader_parameter("presentation_time",time)
		material.set_shader_parameter("repeat_surface",true)
		material.set_shader_parameter("flow_direction",Vector2(0.3,1.0) if key == "river" else Vector2(1.0,0.2))
		material.set_shader_parameter("flow_texels_per_second",2.0 if key == "river" else 0.35)
		material.set_shader_parameter("use_flow_map",true)
		material.set_shader_parameter("bank_mask",load("res://courses/lakewood-hole-1/visual-fx/water-bank-distance.png"))
		var flow_line := PackedVector2Array()
		for point in layout.surfaces.flowLine: flow_line.append(Vector2(point[0],point[1]))
		var vertex_colors := PackedColorArray()
		for vertex in polygon:
			var toward := Vector2(1.0,0.2).normalized()
			if key == "river":
				var closest := INF
				for i in range(flow_line.size()-1):
					var nearest := Geometry2D.get_closest_point_to_segment(vertex,flow_line[i],flow_line[i+1])
					var distance := vertex.distance_squared_to(nearest)
					if distance<closest:
						closest = distance
						toward = (flow_line[i+1]-flow_line[i]).normalized()
			vertex_colors.append(Color(toward.x*0.5+0.5,toward.y*0.5+0.5,1.0,1.0))
		water.vertex_colors = vertex_colors
		var glints := PackedVector2Array()
		var random := RandomNumberGenerator.new()
		random.seed = 8931 if key == "river" else 4912
		for attempt in range(2000):
			var at := Vector2(random.randf_range(0,544),random.randf_range(0,912))
			if Geometry2D.is_point_in_polygon(at,polygon): glints.append(at)
			if glints.size() == 14: break
		material.set_shader_parameter("spark_positions",glints)
		material.set_shader_parameter("spark_count",4)
		water.material = material
		add_child(water)
		water_nodes.append(water)
	# Ground stays beneath water and objects; scale is uniform 4x export -> 1x world.
	var ground := Sprite2D.new()
	ground.texture = textures.ground
	ground.centered = false
	ground.scale = Vector2(0.25,0.25)
	ground.z_index = -100
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
	player_rig = Node2D.new()
	add_child(player_rig)
	player_rig.add_child(player_sprite)
	player_upper = Sprite2D.new()
	player_upper.centered = false
	player_upper.region_enabled = true
	player_rig.add_child(player_upper)
	var atmosphere := preload("res://scripts/PixteeCourseAtmosphere.gd").new()
	atmosphere.world = self
	atmosphere.z_index = 2100
	add_child(atmosphere)
	pond_ducks = preload("res://scripts/PixteePondDucks.gd").new()
	pond_ducks.world = self
	pond_ducks.z_index = 0
	add_child(pond_ducks)
	var birds := preload("res://scripts/PixteeLayeredBirds.gd").new()
	birds.world = self
	birds.z_index = 1800
	add_child(birds)
	var caddy := preload("res://scripts/PixteeCaddy.gd").new()
	caddy.name = "Caddy"
	caddy.world = self
	caddy.z_index = roundi(caddy.foot.y)
	add_child(caddy)
	var living := preload("res://scripts/PixteeLivingWorld.gd").new()
	living_world = living
	living.world = self
	living.z_index = 0
	add_child(living)
	for placement in placements:
		if placement.type == "golfer": continue
		var object := preload("res://scripts/PixteeCourseObject.gd").new()
		object.configure(self,placement)
		add_child(object)
		object_nodes.append(object)
	shadows = preload("res://scripts/PixteeWorldShadows.gd").new()
	shadows.world = self
	add_child(shadows)
	ball_presentation = preload("res://scripts/PixteeBallPresentation.gd").new()
	ball_presentation.world = self
	ball_presentation.z_index = 1800
	add_child(ball_presentation)

func accept_snapshot(state: Dictionary, hold_golfer := false) -> void:
	native_state = state
	presentation.wind_mph = float(state.get("wind_speed_mph_x10",0))/10.0 if state.get("wind_enabled",false) else 0.0
	var wind_angle := float(state.get("wind_toward_raw",1536))*TAU/4096.0
	presentation.wind_toward = Vector2(sin(wind_angle),cos(wind_angle))
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
	presentation.paused = not is_processing()
	presentation.reduced_motion = reduced_motion
	presentation.reduced_flashes = reduced_flashes
	presentation.quality = visual_quality
	presentation.view_rect = view_rect
	presentation.advance(delta)
	time = presentation.time_seconds
	if not is_processing(): return
	update_player_sprite()
	for object in object_nodes:
		object.visible = view_rect.grow(80).has_point(object.position)
		if object.visible: object.queue_redraw()
	shadows.queue_redraw()
	ball_presentation.queue_redraw()
	for water in water_nodes:
		water.material.set_shader_parameter("presentation_time",time)
		water.material.set_shader_parameter("sun_strength",presentation.sun_strength)
		water.material.set_shader_parameter("quality_enabled",1.0 if visual_quality>0 else 0.0)
		water.material.set_shader_parameter("spark_count",1 if visual_quality==0 else 4 if visual_quality==1 else 7)

func _draw() -> void:
	pass

func draw_ellipse_on(target: CanvasItem, at: Vector2, radius: Vector2, colour: Color) -> void:
	var points := PackedVector2Array()
	for i in range(24): points.append(at+Vector2(cos(i*TAU/24.0),sin(i*TAU/24.0))*radius)
	target.draw_colored_polygon(points,colour)

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
	if player_rig != null:
		player_rig.position = golfer_world
		player_rig.z_index = roundi(golfer_world.y)
	if player_sprite == null: return
	var female: bool = PixteeOutfit.female(outfit_id)
	# Atlas starts at north-facing (back view). A right-handed golfer stands
	# left of the travel line and faces the ball, one quarter-turn clockwise.
	var heading := posmod(int(native_state.get("aim_raw",2048))+3072,4096)
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
	player_sprite.position = -Vector2(anchor[0],anchor[1])*factor
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
	# These are complementary masks of a complete pose, not joint-overlap art.
	# Translating either mask exposes a transparent waist seam.
	player_upper.position = player_sprite.position
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
