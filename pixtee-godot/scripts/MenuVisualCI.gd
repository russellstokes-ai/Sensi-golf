extends SceneTree

func _initialize() -> void:
	call_deferred("_run")

func capture(scene: Node, filename: String, check_menu := true) -> void:
	await process_frame
	await process_frame
	await RenderingServer.frame_post_draw
	var picture := root.get_texture().get_image()
	assert(picture.get_width() == root.size.x and picture.get_height() == root.size.y)
	assert(picture.save_png("res://review-captures/"+filename+".png") == OK)
	if check_menu: assert(scene.panel.size.x <= scene.menu_scroll.size.x)
	else: assert(is_equal_approx(scene.camera.zoom.x,scene.camera.zoom.y))

func _run() -> void:
	assert(DirAccess.make_dir_recursive_absolute(ProjectSettings.globalize_path("res://review-captures")) == OK)
	var scene = load("res://scenes/Lakewood.tscn").instantiate()
	root.add_child(scene)
	await process_frame
	assert(scene.profile.load_profile("user://tests/visual-"+str(Time.get_ticks_usec())))
	scene.show_title()
	await capture(scene,"title-portrait")
	scene.show_player()
	await capture(scene,"player-portrait")
	scene.cancel_player_edit()
	scene.show_settings()
	await capture(scene,"settings-portrait")
	scene.back_menu()
	scene.show_help()
	await capture(scene,"help-portrait")
	scene.start_hole()
	scene.pause_game()
	scene.show_scorecard()
	await capture(scene,"scorecard-portrait")
	scene.back_menu()
	root.size = Vector2i(1200,1000)
	await process_frame
	scene.update_ui_layout()
	await capture(scene,"pause-wide")
	scene.start_hole()
	scene.paused = true
	scene.world.set_process(false)
	scene.fx.set_process(false)
	for shot in [[0,105,1946],[1,95,1942]]:
		scene.core.set_club_index(shot[0])
		scene.core.set_aim_raw(shot[2])
		scene.core.meter_click_raw(0)
		scene.core.meter_click_raw(shot[1])
		scene.core.meter_click_raw(63)
		assert(scene.core.step_ticks(4096))
	assert(scene.core.snapshot().green_mode)
	assert(scene.arrows.arrows_are_verified() and scene.arrows.arrow_count()>0)
	root.size = Vector2i(360,760)
	await process_frame
	scene.update_ui_layout()
	await capture(scene,"green-portrait",false)
	root.size = Vector2i(1200,1000)
	await process_frame
	scene.update_ui_layout()
	await capture(scene,"green-wide",false)
	print("GREEN_VISUAL_PASS: actual native green and slope arrows rendered in portrait/wide crops")
	print("MENU_VISUAL_PASS: actual supplied artwork and menu renders captured in portrait/wide windows")
	scene.free()
	await create_timer(0.5).timeout # Retire audio mixer voices before quitting.
	quit(0)
