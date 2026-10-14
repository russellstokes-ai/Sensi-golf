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
	scene.start_hole(false,false)
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
	# A high-density or wide window must retain uniform art and a close crop.
	root.size = Vector2i(1080,1920)
	await process_frame
	scene.update_ui_layout()
	if absf(scene.camera.zoom.x-scene.camera.zoom.y)>0.0001 or scene.camera.zoom.x<4.0:
		push_error("High-density phone camera must crop uniformly")
		quit(1)
		return
	if absf(scene.menu.scale.x-scene.menu.scale.y)>0.0001:
		quit(1)
		return
	root.size = Vector2i(1200,1000)
	await process_frame
	scene.update_ui_layout()
	if scene.world.view_rect.size.y>=912:
		push_error("Wide window must not fit the whole course")
		quit(1)
		return
	root.size = Vector2i(360,760)
	await process_frame
	scene.update_ui_layout()
	# Existing native solver found this real tee-to-cup sequence. These are test
	# inputs only; interactive play uses the recovered native Welly producer.
	assert(scene.arrows.arrows_are_verified() and scene.arrows.arrow_count()>0)
	assert(s.get("green_origin_x")==304 and s.get("green_origin_y")==120)
	s = scene.core.snapshot()
	for cell in scene.core.verified_native_green_arrow_cells(16):
		assert(cell.verified_native and cell.terrain_code==1)
		assert(cell.direction_raw==0 and cell.magnitude==1)
		assert(cell.world_x>=312 and cell.world_x<=416 and cell.world_y>=128 and cell.world_y<=204)
	assert(scene.core.snapshot()==s) # Reading arrows must never advance gameplay.
	var sequence := [[0,105,1946],[1,95,1942],[12,105,2121]]
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
	if s.get("hole_phase") != 6 or s.get("strokes") != 3 or impact_count == 0 or s.has("snapshot_error"):
		push_error("Native tee-to-score sequence failed: " + str(s))
		quit(1)
		return
	# Product stops at one hole; the native 18-hole controller still owns score.
	scene.show_score(s)
	if scene.playing or scene.core.snapshot().get("holes_completed") != 1:
		push_error("Completed one-hole game must not offer live gameplay resume")
		quit(1)
		return
	print("LAKEWOOD_SCENE_PASS: 445 objects, native tee-to-score in 3 strokes, %d unique native events" % seen.size())
	print("LAKEWOOD_GREEN_SCENE_PASS: actual MAPS, native-derived arrows, unchanged gameplay on field reads")
	scene.start_hole(false,false)
	scene.pause_game()
	scene.start_hole(false,false)
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
	scene.free()
	await create_timer(0.5).timeout # Retire audio mixer voices before quitting.
	quit(0)
