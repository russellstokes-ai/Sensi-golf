extends SceneTree

func _initialize() -> void:
	call_deferred("_run")

func _run() -> void:
	var world = load("res://scripts/LakewoodWorld.gd").new()
	root.add_child(world)
	world.accept_snapshot({"ball_x_raw":256*65536,"ball_y_raw":850*65536,"green_mode":false,"hole_phase":0})
	var tee: Vector2 = world.golfer_world
	world.accept_snapshot({"ball_x_raw":300*65536,"ball_y_raw":500*65536,"green_mode":false,"hole_phase":1})
	assert(world.golfer_world == tee, "Golfer must remain at tee during flight")
	assert(world.ball_world == Vector2(300,500))
	world.accept_snapshot({"ball_x_raw":300*65536,"ball_y_raw":500*65536,"green_mode":false,"hole_phase":0})
	assert(world.golfer_world == Vector2(293,503), "Ready state places golfer at new lie")
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
	world.free()
	print("PLAYER_RENDERING_PASS: golfer anchored during flight; all 32 bird frames use fixed canvas scale")
	quit(0)
