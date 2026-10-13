extends SceneTree
func _initialize() -> void:
	call_deferred("_run")

func _run() -> void:
	var scene = load("res://scenes/Lakewood.tscn").instantiate()
	root.add_child(scene)
	scene.set_process(false)
	assert(scene.profile.load_profile("user://tests/commercial-"+str(Time.get_ticks_usec())))
	scene.session_store.base = "user://tests/session-"+str(Time.get_ticks_usec())
	var draft: Dictionary = scene.profile.begin_edit()
	draft.outfit_id = "kit:3:2:1"
	assert(scene.profile.apply_edit(draft))
	assert(scene.profile.snapshot().outfit_id == "kit:3:2:1")
	scene.start_hole()
	assert(scene.world.player_sprite.material.get_shader_parameter("shirt_enabled"))
	scene.on_shoot()
	for tick in range(30): scene.core.tick_original_welly(false)
	scene.save_session_checkpoint()
	var before: Dictionary = scene.core.snapshot()
	var save: Dictionary = scene.session_store.load_current(scene.course_hash)
	assert(not save.is_empty())
	var restored: PixteeRoundBridge = scene.create_course_core(0)
	assert(restored.restore_native_session(save.native))
	assert(restored.snapshot() == before, "Mid-meter native state including RNG/context resumes exactly")
	for tick in range(15):
		scene.core.tick_original_welly(false)
		restored.tick_original_welly(false)
	assert(restored.snapshot() == scene.core.snapshot())
	var stable: Dictionary = restored.snapshot()
	assert(not restored.restore_native_session(save.native+"bad"))
	var failed: Dictionary = restored.snapshot()
	assert(not str(failed.error).is_empty())
	failed.erase("error")
	stable.erase("error")
	assert(failed == stable, "Corrupt replay cannot partially replace live state")
	# Torn newest write falls back to the older verified save slot.
	assert(scene.session_store.save(save.native,scene.course_hash,scene.attempt_id,false))
	var damaged := FileAccess.open(scene.session_store.base+".%d.json" % (scene.session_store.generation%2),FileAccess.WRITE)
	damaged.store_string("broken")
	damaged.close()
	assert(not scene.session_store.load_current(scene.course_hash).is_empty())
	# Menu play runs isolated recovered cores and cannot advance the player.
	var player_before: Dictionary = scene.core.snapshot()
	for child in scene.get_children():
		if child.get_script() == load("res://scripts/PixteeMenuAttract.gd"):
			scene.playing = false
			child._process(0.02)
			var saw_strokes := [false,false]
			for tick in range(1300):
				child._tick(0)
				child._tick(1)
				for actor in range(2):
					if child.golfers[actor].core.snapshot().strokes > 0: saw_strokes[actor] = true
			assert(saw_strokes[0] and saw_strokes[1])
			scene.playing = true
	assert(scene.core.snapshot() == player_before)
	# Find a genuine native bird impact; never inject a visual-proximity cue.
	var hit := false
	for clock in [1420,1460,1500,1540]:
		if hit: break
		for power in range(50,101,2):
			var native: PixteeRoundBridge = scene.create_course_core(0)
			native.step_ticks(clock)
			native.set_aim_raw(2048)
			native.meter_click_raw(0)
			native.meter_click_raw(power)
			native.meter_click_raw(63)
			for tick in range(240):
				native.step_tick()
				for event in native.drain_presentation_events():
					if event.event_type == "authored_bird_contact":
						assert(scene.audio.consume_confirmed_native_event(event))
						assert(scene.fx.submit_confirmed_native_event(event))
						var copy: PixteeRoundBridge = scene.create_course_core(0)
						assert(copy.restore_native_session(native.save_native_session()))
						assert(copy.snapshot() == native.snapshot())
						hit = true
						break
				if hit or int(native.snapshot().hole_phase) != 1: break
			if hit: break
	assert(hit, "Real authored native bird hit must be reachable and recoverable")
	scene.session_store.clear()
	assert(scene.session_store.load_current(scene.course_hash).is_empty())
	scene.free()
	await create_timer(0.5).timeout
	print("COMMERCIAL_JOURNEY_PASS: saved outfits, original human mode, native bird hit/audio/FX, exact replay and torn-save recovery")
	quit(0)
