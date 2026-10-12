extends SceneTree

func _initialize() -> void:
	call_deferred("_run")

func capture(scene: Node, filename: String) -> void:
	await process_frame
	await process_frame
	await RenderingServer.frame_post_draw
	var picture := root.get_texture().get_image()
	assert(picture.get_width() == root.size.x and picture.get_height() == root.size.y)
	assert(picture.save_png("res://review-captures/"+filename+".png") == OK)
	assert(scene.panel.size.x <= scene.menu_scroll.size.x)

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
	print("MENU_VISUAL_PASS: actual supplied artwork and menu renders captured in portrait/wide windows")
	quit(0)
