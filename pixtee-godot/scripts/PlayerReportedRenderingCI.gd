extends SceneTree

func _initialize() -> void:
	call_deferred("_run")

func _run() -> void:
	var world = load("res://scripts/LakewoodWorld.gd").new()
	root.add_child(world)
	world.accept_snapshot({"ball_x_raw":256*65536,"ball_y_raw":850*65536,"green_mode":false,"hole_phase":0,"aim_raw":2048})
	var tee: Vector2 = world.golfer_world
	world.accept_snapshot({"ball_x_raw":300*65536,"ball_y_raw":500*65536,"green_mode":false,"hole_phase":1})
	assert(world.golfer_world == tee, "Golfer must remain at tee during flight")
	assert(world.ball_world == Vector2(300,500))
	world.accept_snapshot({"ball_x_raw":300*65536,"ball_y_raw":500*65536,"green_mode":false,"hole_phase":0,"aim_raw":2048})
	assert(world.golfer_world == Vector2(293,503), "Ready state places golfer at new lie")
	for heading in [0,512,1024,1536,2048,2560,3072,3584]:
		world.accept_snapshot({"ball_x_raw":300*65536,"ball_y_raw":500*65536,"hole_phase":0,"aim_raw":heading})
		var direction := Vector2(sin(float(heading)*TAU/4096.0),cos(float(heading)*TAU/4096.0))
		assert(world.golfer_world.is_equal_approx(Vector2(300,500)-direction.rotated(PI/2.0)*7-direction*3),"Right-handed stance orbits the ball")
		assert(not world.player_sprite.flip_h,"Changing aim must never mirror the whole golfer")
	for species in ["bird","gull","blackbird","kingfisher"]:
		var first: Rect2 = world.bird_frame_rect(species,0,Vector2.ZERO)
		for frame in range(8):
			assert(world.bird_frame_rect(species,frame,Vector2.ZERO)==first, "Bird animation canvas and scale must stay fixed")
	var atmosphere: Node2D
	for child in world.get_children():
		if child.get_script() == load("res://scripts/PixteeCourseAtmosphere.gd"): atmosphere = child
	assert(atmosphere != null, "Course atmosphere must be attached")
	world.set_process(false)
	var frozen: float = atmosphere.elapsed
	atmosphere._process(0.1)
	assert(atmosphere.elapsed == frozen, "Pause freezes atmosphere")
	world.set_process(true)
	world.reduced_motion = true
	atmosphere._process(0.1)
	assert(atmosphere.elapsed == frozen, "Reduced motion freezes atmosphere")
	var living: Node
	for child in world.get_children():
		if child.get_script() == load("res://scripts/PixteeLivingWorld.gd"): living = child
	assert(living != null and living.actors.size() == 4)
	for actor in living.actors:
		assert(actor._layers.size() >= 2,"Every live animal needs independently authored layers")
		assert(is_equal_approx(actor.scale.x,actor.scale.y))
		var canvas: Vector2 = actor._layers[0].texture.get_size()
		for layer in actor._layers: assert(layer.texture.get_size() == canvas)
		var animation: String = actor.animation_id
		assert(not actor.set_animation("rabbit_poses",2),"A whole-pose fallback must not replace a layered actor")
		assert(actor.animation_id == animation)
		actor.pause_presentation(true)
		var clock: float = actor._time
		actor._process(0.1)
		assert(actor._time == clock)
	var ducks: Node
	for child in world.get_children():
		if child.get_script() == load("res://scripts/PixteePondDucks.gd"): ducks = child
	assert(ducks != null and ducks.ducks.size() == 2)
	for duck in ducks.ducks:
		assert(Geometry2D.is_point_in_polygon(duck.at,ducks.pond))
		assert(duck.body.material.get_shader_parameter("part") == 1)
		assert(duck.head.material.get_shader_parameter("part") == 2)
		assert(is_equal_approx(duck.body.scale.x,duck.body.scale.y))
	var impact := {"confirmed_native":true,"event_id":"pond:1","event_type":"water_contact","world_x":ducks.ducks[0].at.x,"world_y":ducks.ducks[0].at.y}
	assert(ducks.accept_native_event(impact))
	assert(not ducks.accept_native_event(impact))
	assert(ducks.ducks[0].scared > 0.0)
	assert(not ducks.accept_native_event({"event_id":"fake","event_type":"water_contact","world_x":450,"world_y":480}))
	world.reduced_motion = false
	for tick in range(2000):
		ducks._process(0.02)
		for duck in ducks.ducks: assert(Geometry2D.is_point_in_polygon(duck.at,ducks.pond))
	world.set_process(false)
	var duck_at: Vector2 = ducks.ducks[0].at
	ducks._process(0.1)
	assert(ducks.ducks[0].at == duck_at)
	world.free()
	print("PLAYER_RENDERING_PASS: golfer anchored during flight; all 32 bird frames use fixed canvas scale")
	quit(0)
