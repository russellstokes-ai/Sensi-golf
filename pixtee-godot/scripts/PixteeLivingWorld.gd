extends Node2D
var world: Node2D
var actors: Array[Node] = []
var weather: PixteeWeatherDirector
var visuals: PixteeWeatherVisuals
var ecology_time := 0.0
var sightings: Array[Dictionary] = []
var seen: Dictionary = {}
var atmosphere: ColorRect
var appearance_seed := 3419
var climbing_squirrel: Node2D
var dragonfly_origin := Vector2.ZERO
var dragonfly_heading := 2
func _ready() -> void:
	weather = PixteeWeatherDirector.new()
	add_child(weather)
	weather.set_preset("morning_mist",0)
	visuals = PixteeWeatherVisuals.new()
	visuals.weather = weather
	visuals.enable_cloud_shadows = false
	visuals.enable_sun_dapple = false
	visuals.enable_mist = false
	add_child(visuals)
	atmosphere = ColorRect.new()
	atmosphere.mouse_filter = Control.MOUSE_FILTER_IGNORE
	atmosphere.size = Vector2(544,912)
	atmosphere.z_index = 2050
	atmosphere.material = ShaderMaterial.new()
	atmosphere.material.shader = load("res://shaders/pixtee_weather_atmosphere.gdshader")
	add_child(atmosphere)
	for entry in [["rabbit_layered_idle",Vector2(210,785),18.0],["squirrel_layered_idle",Vector2(81,718),12.0],["frog_breathe",Vector2(434,374),10.0],["dragonfly_heading_02",Vector2(404,322),8.0]]:
		var actor := PixteePremiumLayeredSprite.new()
		actor.preview_enabled = false
		add_child(actor)
		if not actor.set_animation(entry[0],2): push_error("Living World actor failed: "+str(entry[0]))
		var manifest: Dictionary = JSON.parse_string(FileAccess.get_file_as_string(PixteePremiumLayeredSprite.MANIFEST))
		var canvas: Array = manifest[entry[0]].canvas
		var factor: float = float(entry[2])/float(canvas[1])
		actor.scale = Vector2.ONE*factor
		actor.position = entry[1]-Vector2(float(canvas[0])/2,float(canvas[1]))*factor
		actors.append(actor)
		if str(entry[0]).begins_with("squirrel"):
			var nearest: Dictionary = {}
			var distance := INF
			for item in world.placements:
				if item.type != "tree": continue
				var d := Vector2(item.x,item.y).distance_squared_to(Vector2(81,718))
				if d < distance:
					distance = d
					nearest = item
			if not nearest.is_empty():
				actor.position = Vector2(nearest.x,nearest.y)-Vector2(float(canvas[0])/2*factor,float(canvas[1])*factor+float(nearest.h)*0.55)
			for layer in actor._layers:
				if layer.name == "shadow": layer.visible = false
		if str(entry[0]).begins_with("rabbit") or str(entry[0]).begins_with("squirrel"):
			actor.visible = false
			actor.modulate.a = 0.0
			sightings.append({"actor":actor,"origin":actor.position,"tree":str(entry[0]).begins_with("squirrel"),"wait":0.0 if str(entry[0]).begins_with("rabbit") else 25.0,"age":0.0,"number":0,"duration":12.0,"flee":false})
	climbing_squirrel = preload("res://scripts/PixteeSquirrelClimb.gd").new()
	climbing_squirrel.world = world
	add_child(climbing_squirrel)
	dragonfly_origin = actors[3].position
func _process(_delta: float) -> void:
	var frozen: bool = not world.is_processing() or world.reduced_motion
	if frozen and world.visual_quality == 0:
		for sighting in sightings: sighting.actor.visible = false
	if not frozen:
		ecology_time = world.time
		_update_sightings(minf(_delta,0.1))
		_update_dragonfly()
	if world.native_state.get("physics_authority","") == "Recovered Sensible Golf ClassicGameSession":
		weather.set_native_wind_visuals(world.presentation.wind_toward,clampf(world.presentation.wind_mph/15.0,0.0,1.0),true)
	var conditions := weather.conditions()
	world.presentation.sun_strength = float(conditions.sun)
	world.presentation.cloud_cover = float(conditions.cloud)
	world.presentation.fog_density = float(conditions.fog)
	if world.presentation.wind_mph == 0.0: world.presentation.visual_breeze = float(conditions.wind)
	atmosphere.visible = world.visual_quality > 0
	atmosphere.material.set_shader_parameter("visual_time",ecology_time)
	atmosphere.material.set_shader_parameter("focus_points",Vector4(world.ball_world.x,world.ball_world.y,364,168))
	atmosphere.material.set_shader_parameter("sun_strength",float(conditions.sun))
	atmosphere.material.set_shader_parameter("fog_density",float(conditions.fog))
	atmosphere.material.set_shader_parameter("cloud_cover",float(conditions.cloud))
	atmosphere.material.set_shader_parameter("wind_direction",world.presentation.wind_toward)
	weather.reduced_motion = frozen
	visuals.paused = frozen
	visuals.reduced_motion = world.reduced_motion
	visuals.reduced_flashes = world.reduced_flashes
	visuals.quality = world.visual_quality
	visuals.set_view_rect(world.view_rect)
	for actor in actors:
		actor.z_as_relative = false
		actor.z_index = roundi(actor.position.y+actor._layers[0].texture.get_height()*actor.scale.y)
		actor.pause_presentation(frozen)
		if not actor in [sightings[0].actor,sightings[1].actor]: actor.visible = world.visual_quality > 0

func _hash01(number: int, salt: int) -> float:
	# Presentation variation only; never consumes or changes native gameplay RNG.
	return fposmod(sin(float(number*71+salt*131+appearance_seed))*43758.5453,1.0)
func _update_dragonfly() -> void:
	var actor: PixteePremiumLayeredSprite = actors[3]
	# Small pond-edge patrol with slow hover holds and occasional faster darts.
	var t := ecology_time
	var phase := t*0.13+sin(t*0.21)*0.28
	var offset := Vector2(cos(phase)*13.0,sin(phase)*6.0)
	var tangent := Vector2(-sin(phase)*13.0,cos(phase)*6.0)
	var heading := fposmod(atan2(tangent.x,-tangent.y),TAU)
	if absf(wrapf(heading-float(dragonfly_heading)*TAU/8.0,-PI,PI))>TAU/16.0+0.05:
		dragonfly_heading = posmod(roundi(heading*8.0/TAU),8)
		actor.set_animation("dragonfly_heading_%02d"%dragonfly_heading,3)
	actor.position = dragonfly_origin+offset+Vector2(0,sin(t*2.3)*0.45)
func _update_sightings(delta: float) -> void:
	for i in sightings.size():
		var sighting: Dictionary = sightings[i]
		var actor: PixteePremiumLayeredSprite = sighting.actor
		if sighting.wait > 0.0:
			sighting.wait = maxf(0.0,float(sighting.wait)-delta)
			actor.visible = false
			continue
		sighting.age += delta*(3.0 if sighting.flee else 1.0)
		if sighting.age >= sighting.duration:
			sighting.number += 1
			sighting.wait = 35.0+_hash01(int(sighting.number),i)*75.0
			sighting.duration = 9.0+_hash01(int(sighting.number),i+10)*9.0
			sighting.age = 0.0
			sighting.flee = false
			actor.visible = false
			continue
		actor.visible = world.visual_quality > 0
		var age: float = sighting.age
		var duration: float = sighting.duration
		actor.modulate.a = clampf(minf(age,duration-age)/0.7,0.0,1.0)
		var ingress := 1.0-clampf(age/1.4,0.0,1.0)
		var egress := clampf((age-duration+1.4)/1.4,0.0,1.0)
		actor.position = sighting.origin+Vector2((egress-ingress)*5.0,0.0)
		for layer in actor._layers:
			if layer.name == "shadow": continue
			layer.position.y = 0.0 if sighting.tree else -absf(sin(age*5.5))*12.0*(ingress+egress)
func accept_native_event(event: Dictionary) -> bool:
	if event.get("confirmed_native",false) != true or not event.get("event_type","") in ["water_contact","fairway_contact","rough_contact","ball_bounce","authored_tree_contact"]: return false
	var id := str(event.get("event_id",""))
	if id.is_empty() or seen.has(id) or not event.has("world_x") or not event.has("world_y"): return false
	var at := Vector2(float(event.world_x),float(event.world_y))
	if not is_finite(at.x) or not is_finite(at.y): return false
	seen[id] = true
	if seen.size()>128: seen.erase(seen.keys()[0])
	for sighting in sightings:
		if sighting.wait>0.0: continue
		var actor: PixteePremiumLayeredSprite = sighting.actor
		var extent: Vector2 = actor._layers[0].texture.get_size()*actor.scale
		var foot: Vector2 = actor.position+Vector2(extent.x/2.0,extent.y)
		if foot.distance_to(at)<35.0: sighting.flee = true
	return true
func reset_confirmed_event_cursor() -> void:
	seen.clear()
