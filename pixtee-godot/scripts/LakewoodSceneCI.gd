extends SceneTree

func _initialize() -> void:
	call_deferred("_run")

func _run() -> void:
	if not ResourceLoader.exists("res://scenes/Lakewood.tscn"):
		push_error("Approved Lakewood playable scene is missing")
		quit(1)
		return
	var scene = load("res://scenes/Lakewood.tscn").instantiate()
	root.add_child(scene)
	await process_frame
	scene.start_hole()
	await process_frame
	var s: Dictionary = scene.core.snapshot()
	if s.get("ball_x_raw") != 256 * 65536 or s.get("ball_y_raw") != 850 * 65536:
		push_error("Scene did not load approved native tee coordinates")
		quit(1)
		return
	if scene.world.placements.size() != 445:
		push_error("Approved visual placement count changed")
		quit(1)
		return
	# Existing native solver found this real tee-to-cup sequence. These are test
	# inputs only; interactive play uses the recovered native Welly producer.
	var sequence := [[0,105,1946],[4,95,1913],[12,75,2087],[12,20,248]]
	var seen: Dictionary = {}
	var impact_count := 0
	for shot in sequence:
		scene.core.set_club_index(shot[0])
		scene.core.set_aim_raw(shot[2])
		scene.core.meter_click_raw(0)
		scene.core.meter_click_raw(shot[1])
		scene.core.meter_click_raw(63)
		if not scene.core.step_ticks(4096):
			push_error(scene.core.last_error())
			quit(1)
			return
		for event in scene.core.drain_presentation_events():
			if seen.has(event.event_id) or not event.confirmed_native:
				push_error("Invalid or duplicated native event")
				quit(1)
				return
			seen[event.event_id] = true
			if event.event_type == "fairway_contact": impact_count += 1
		if not scene.core.drain_presentation_events().is_empty():
			quit(1)
			return
	s = scene.core.snapshot()
	if s.get("hole_phase") != 6 or s.get("strokes") != 4 or impact_count == 0 or s.has("snapshot_error"):
		push_error("Native tee-to-score sequence failed: " + str(s))
		quit(1)
		return
	# Product stops at one hole; the native 18-hole controller still owns score.
	scene.show_score(s)
	if scene.playing or scene.core.snapshot().get("holes_completed") != 1:
		push_error("Completed one-hole game must not offer live gameplay resume")
		quit(1)
		return
	print("LAKEWOOD_SCENE_PASS: 445 objects, native tee-to-score in 4 strokes, %d unique native events" % seen.size())
	scene.start_hole()
	scene.pause_game()
	scene.start_hole()
	if not scene.fx.is_processing():
		push_error("Effects must resume after pause -> restart")
		quit(1)
		return
	scene.core.set_aim_raw(0)
	scene.core.meter_click_raw(0)
	scene.core.meter_click_raw(105)
	scene.core.meter_click_raw(63)
	scene.core.step_ticks(4096)
	var oob_count := 0
	for event in scene.core.drain_presentation_events():
		if event.event_type == "world_oob": oob_count += 1
	if oob_count != 1:
		push_error("Native OOB must emit exactly once across 100-tick recovery, got %d" % oob_count)
		quit(1)
		return
	print("LAKEWOOD_OOB_EVENT_PASS: exactly one native penalty event")
	quit(0)
