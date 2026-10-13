extends SceneTree

func _initialize() -> void:
	call_deferred("_run")

func _run() -> void:
	var scene = load("res://scenes/Lakewood.tscn").instantiate()
	root.add_child(scene)
	await process_frame
	if not scene.has_method("show_statistics"):
		push_error("Complete native scorecard/menu journey is missing")
		quit(1)
		return
	assert(scene.profile.load_profile("user://tests/menu-"+str(Time.get_ticks_usec())))
	scene.show_intro()
	assert(scene.menu_screen == "intro")
	scene.start_hole()
	assert(scene.intro_tween == null and scene.menu_screen == "game")
	scene.pause_game()
	var before: Dictionary = scene.core.snapshot()
	var water_time: float = scene.world.get_child(0).material.get_shader_parameter("presentation_time")
	await process_frame
	await process_frame
	assert(scene.world.get_child(0).material.get_shader_parameter("presentation_time") == water_time)
	scene.show_help()
	assert(scene.menu_screen == "help")
	scene.back_menu()
	assert(scene.menu_screen == "pause")
	scene.show_scorecard()
	assert(scene.menu_screen == "scorecard")
	scene.back_menu()
	scene.show_settings()
	assert(scene.profile.set_setting("graphics_quality",0))
	assert(scene.profile.set_setting("reduced_flashes",true))
	assert(scene.profile.set_setting("frame_limit",30))
	scene.apply_saved_settings()
	assert(scene.fx.visual_quality == 0 and scene.world.reduced_flashes)
	assert(Engine.max_fps == 30)
	assert(scene.core.snapshot() == before)
	assert(not scene.profile.set_setting("frame_limit",0))
	assert(not scene.profile.set_setting("graphics_quality",9))
	scene.back_menu()
	scene.resume_game()
	var sequence := [[0,105,1946],[1,95,1942],[12,105,2121]]
	for shot in sequence:
		scene.core.set_club_index(shot[0])
		scene.core.set_aim_raw(shot[2])
		scene.core.meter_click_raw(0)
		scene.core.meter_click_raw(shot[1])
		scene.core.meter_click_raw(63)
		assert(scene.core.step_ticks(4096))
	var scored: Dictionary = scene.core.snapshot()
	# Tee-off allocation can fail temporarily; completed scores remain savable.
	scene.attempt_id = 0
	scene.show_score(scored)
	assert(scene.menu_screen == "results" and not scene.hud.visible)
	assert(scene.profile.snapshot().statistics.played == 1)
	assert(scene.profile.snapshot().statistics.best == 3)
	assert(scene.last_result.relative_to_par==1) # Original positive means birdie.
	assert(scene.profile.snapshot().statistics.last.relative == -1)
	assert(scene.core.snapshot().holes_completed == 1)
	# Revisiting results or replaying a save cannot count one attempt twice.
	assert(scene.profile.record_committed_result(scene.attempt_id,scene.core.snapshot()))
	assert(scene.profile.snapshot().statistics.played == 1)
	scene.show_statistics()
	scene.back_menu()
	assert(scene.menu_screen == "results")
	scene.show_scorecard()
	scene.back_menu()
	assert(scene.menu_screen == "results")
	var restored = load("res://scripts/PixteePlayerProfile.gd").new()
	assert(restored.load_profile(scene.profile.storage_base()))
	assert(restored.snapshot().statistics.played == 1)
	assert(restored.snapshot().statistics.best == 3)
	assert(restored.snapshot().statistics.last.relative == -1)
	assert(restored.snapshot().settings.reduced_flashes)
	assert(not restored.record_committed_result(scene.attempt_id+1,{"total_strokes":2}))
	scene.show_title()
	scene.show_settings()
	await process_frame
	assert(scene.panel.size.x <= scene.menu_scroll.size.x)
	assert(scene.menu_scroll.get_v_scroll_bar().max_value >= scene.menu_scroll.size.y)
	print("MENU_JOURNEY_PASS: native score journal/dedupe, pause navigation, cosmetic presets, accessible scrolling, results lifecycle")
	quit(0)
