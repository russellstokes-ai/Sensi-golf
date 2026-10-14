extends SceneTree
func _initialize() -> void:
	call_deferred("_run")
func _run() -> void:
	if not ResourceLoader.exists("res://scripts/PixteePresentationState.gd"):
		push_error("Reference polish shared presentation state missing")
		quit(1)
		return
	var shared = load("res://scripts/PixteePresentationState.gd").new()
	shared.advance(0.05)
	assert(is_equal_approx(shared.time_seconds,0.05))
	shared.paused = true
	shared.advance(0.1)
	assert(is_equal_approx(shared.time_seconds,0.05))
	shared.paused = false
	shared.reduced_motion = true
	shared.advance(0.1)
	assert(is_equal_approx(shared.time_seconds,0.05))
	var world = load("res://scripts/LakewoodWorld.gd").new()
	root.add_child(world)
	world.accept_snapshot({"ball_x_raw":256*65536,"ball_y_raw":850*65536,"green_mode":false,"hole_phase":0,"aim_raw":2048})
	assert(world.ball_presentation != null and world.shadows != null)
	var climber: Node = world.living_world.climbing_squirrel
	assert(climber != null and climber.layers.size() == 3)
	assert(climber.metadata.frames == 6)
	assert(climber.metadata.pixel_scale > 0.02)
	climber.show_pose(5)
	assert(climber.frame == 5)
	for layer in climber.layers:
		assert(layer.region_rect.size == Vector2(362,724))
	assert(world.placements.size() == 445)
	assert(world.object_nodes.size() == 444)
	assert(world.player_rig.z_index == roundi(world.golfer_world.y))
	for object in world.object_nodes:
		assert(object.z_index == roundi(object.position.y))
		assert(object.world == world)
	var before: Dictionary = world.native_state.duplicate(true)
	world._process(0.05)
	assert(world.native_state == before)
	var clock: float = world.presentation.time_seconds
	world.set_process(false)
	world._process(0.1)
	assert(world.presentation.time_seconds == clock)
	world.set_process(true)
	world.reduced_motion = true
	world._process(0.1)
	assert(world.presentation.time_seconds == clock)
	var actor: Node = world.living_world.actors[2]
	actor.position = Vector2(1.25,2.75)
	actor._process(0.02)
	assert(actor.position == Vector2(1.25,2.75),"Animation must not quantize logical motion")
	world.free()
	print("VISUAL_POLISH_PASS: shared freeze, foot-depth sorting, immutable native state and fractional animation")
	quit(0)
