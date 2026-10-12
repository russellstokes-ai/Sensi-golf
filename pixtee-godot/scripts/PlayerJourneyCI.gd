extends SceneTree

func _initialize() -> void:
	call_deferred("_run")

func _run() -> void:
	if not ResourceLoader.exists("res://scripts/PixteePlayerProfile.gd"):
		push_error("Persistent player/profile journey is missing")
		quit(1)
		return
	var Profile = load("res://scripts/PixteePlayerProfile.gd")
	var storage := "user://tests/player-profile-"+str(Time.get_ticks_usec())
	var profile = Profile.new()
	assert(profile.load_profile(storage))
	var draft: Dictionary = profile.begin_edit()
	draft.display_name = "Willow Player"
	assert(profile.apply_edit(draft))
	assert(profile.set_setting("reduced_motion",true))
	assert(profile.set_setting("Music",0.25))
	assert(not profile.set_setting("Music",INF))
	assert(not profile.set_setting("Music",1.1))
	draft = profile.begin_edit()
	draft.outfit_id = "shirt" # Supplied UI thumbnail is not a verified wearable.
	assert(not profile.apply_edit(draft))
	var restored = Profile.new()
	assert(restored.load_profile(storage))
	assert(restored.snapshot().display_name == "Willow Player")
	assert(restored.snapshot().settings.reduced_motion)
	assert(restored.snapshot().settings.Music == 0.25)
	# Tear the newest slot: the previous complete profile must survive.
	var newest: String = restored.last_slot_path()
	var file := FileAccess.open(newest,FileAccess.WRITE)
	assert(file != null)
	file.store_string("{torn-write")
	file.close()
	assert(restored.load_profile(storage))
	assert(restored.snapshot().display_name == "Willow Player")
	var scene = load("res://scenes/Lakewood.tscn").instantiate()
	root.add_child(scene)
	await process_frame
	scene.profile = restored
	scene.start_hole()
	scene.core.meter_click_raw(0)
	scene.core.meter_click_raw(105)
	scene.core.meter_click_raw(63)
	scene.core.step_ticks(10)
	scene.pause_game()
	var before: Dictionary = scene.core.snapshot()
	var clock: float = scene.world.time
	scene.show_player()
	scene.player_name.text = "Paused Player"
	scene.cancel_player_edit()
	assert(scene.profile.snapshot().display_name == "Willow Player")
	assert(scene.core.snapshot() == before)
	scene.show_player()
	scene.player_name.text = "Paused Player"
	assert(scene.apply_player_edit())
	assert(scene.core.snapshot() == before)
	assert(scene.world.time == clock and scene.paused)
	scene.resume_game()
	assert(not scene.paused and scene.fx.is_processing())
	# A real native penalty must recover through the live host and accept a shot.
	scene.start_hole()
	scene.core.set_aim_raw(0)
	scene.core.meter_click_raw(0)
	scene.core.meter_click_raw(105)
	scene.core.meter_click_raw(63)
	scene.core.step_ticks(4096)
	scene.advance_native_tick()
	assert(scene.core.snapshot().hole_phase == 0)
	assert(scene.core.snapshot().can_shoot)
	print("PLAYER_JOURNEY_PASS: durable profile, torn-write recovery, thumbnail rejection, pause edit invariance, live hazard recovery")
	quit(0)
