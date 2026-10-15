extends SceneTree
func _initialize() -> void:
	call_deferred("_run")
func _run() -> void:
	var scene = load("res://scenes/Lakewood.tscn").instantiate()
	root.add_child(scene)
	scene.start_hole(false,true)
	scene.world.living_world.weather.set_preset("sunny",0)
	for i in 5: await process_frame
	assert(scene.club_card.state.club_nominal_max==240)
	assert(scene.club_card.lie_visual().tee)
	assert(scene.club_card.sky.clock==scene.meter.clock)
	await RenderingServer.frame_post_draw
	assert(root.get_texture().get_image().save_png("res://review-captures/hud-v2/live-course.png")==OK)
	scene.pause_game()
	var frozen: float = scene.club_card.sky.clock
	for i in 5: await process_frame
	assert(scene.club_card.sky.clock==frozen and scene.meter.clock==frozen)
	scene.resume_game()
	root.size = Vector2i(720,720)
	for i in 5: await process_frame
	await RenderingServer.frame_post_draw
	assert(scene.club_card.get_global_rect().end.x<scene.minimap.get_global_rect().position.x)
	assert(root.get_texture().get_image().save_png("res://review-captures/hud-v2/live-wide.png")==OK)
	scene.free()
	print("HUD_LIVING_GAME_VISUAL_PASS: actual native scene, truthful Max/tee, shared-clock pause, portrait/wide layout")
	quit(0)
