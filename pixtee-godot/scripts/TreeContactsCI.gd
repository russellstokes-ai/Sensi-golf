extends SceneTree

func _initialize() -> void:
	call_deferred("_run")

func _run() -> void:
	var scene = load("res://scenes/Lakewood.tscn").instantiate()
	root.add_child(scene)
	await process_frame
	var profile = load("res://scripts/PixteePlayerProfile.gd").new()
	assert(profile.load_profile("user://tests/tree-contact-"+str(Time.get_ticks_usec())))
	scene.profile = profile
	var manifest: Dictionary = JSON.parse_string(FileAccess.get_file_as_string(scene.COURSE+"authored-tree-volumes.json"))
	var ids: Dictionary = {}
	for tree in manifest.trees:
		ids[int(tree.id)] = true
	assert(ids.size() == 17)
	# Genuine native input only. Search a bounded set of directions for a real
	# contact with the approved course's registered volumes; no visual hit guess.
	for club_index in [12,7,0]:
		for aim in range(0,4096,128):
			scene.start_hole()
			scene.core.set_club_index(club_index)
			scene.core.set_aim_raw(aim)
			scene.core.meter_click_raw(0)
			scene.core.meter_click_raw(105)
			scene.core.meter_click_raw(63)
			assert(scene.core.step_ticks(4096),scene.core.last_error())
			var seen: Dictionary = {}
			for event in scene.core.drain_presentation_events():
				assert(not seen.has(event.event_id))
				seen[event.event_id] = true
				if event.event_type != "authored_tree_contact": continue
				assert(event.confirmed_native and ids.has(int(event.tree_id)))
				assert(event.contact_part in ["trunk","canopy"])
				print("LAKEWOOD_AUTHORED_TREE_CONTACT_PASS club=",club_index," aim=",aim," id=",event.tree_id," part=",event.contact_part)
				scene.free()
				await create_timer(0.5).timeout # Retire audio mixer voices before quitting.
				quit(0)
				return
	push_error("No genuine authored tree contact from approved Lakewood native input sweep")
	quit(1)
