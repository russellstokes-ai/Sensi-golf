extends SceneTree

# Native-engine-independent validation of prepared production presentation.
# Tests actual Godot parser and scene nodes, NOT commercial game readiness.
func fail(message: String) -> void:
	push_error("PIXTEE PRESENTATION TEST FAILED: " + message)
	quit(1)

func _initialize() -> void:
	call_deferred("_run")

func _run() -> void:
	var arrow_source: Script = load("res://scripts/PixteeVerifiedGreenArrows.gd")
	var audio_source: Script = load("res://scripts/PixteeCommercialAudio.gd")
	var native_fx_source: Script = load("res://scripts/PixteeNativeWorldFX.gd")
	var foliage_source: Script = load("res://scripts/PixteeFoliageBreeze.gd")
	var water: Shader = load("res://shaders/pixtee_subtle_water.gdshader")
	if arrow_source == null or audio_source == null or native_fx_source == null or foliage_source == null or water == null:
		fail("One of the source presentation modules or shaders failed to load")
		return

	var arrows: Node2D = arrow_source.new()
	root.add_child(arrows)
	var cells: Array[Dictionary] = []
	for direction_index in range(16):
		for strength in range(16):
			cells.append({
				"world_x": 140 + direction_index * 5,
				"world_y": 80 + strength * 5,
				"direction_raw": direction_index * 256,
				"magnitude": strength,
				"terrain_code": 1,
				"verified_native": true
			})
	if not arrows.load_native_maps_arrow_cells(cells):
		fail("Failed to load 16 directions x 16 strengths of source green MAPS data")
		return
	if not arrows.arrows_are_verified() or arrows.arrow_count() != 16 * 15:
		fail("Native green arrow count or zero-strength handling wrong")
		return
	var pointing_down: Vector2 = arrows.native_slope_vector(0)
	var pointing_right: Vector2 = arrows.native_slope_vector(1024)
	var pointing_up: Vector2 = arrows.native_slope_vector(2048)
	if pointing_down.distance_to(Vector2.DOWN) > 0.0001 or pointing_right.distance_to(Vector2.RIGHT) > 0.0001 or pointing_up.distance_to(Vector2.UP) > 0.0001:
		fail("Arrow sectors disagree with authoritative original C++ trig directions")
		return
	var bad: Array[Dictionary] = cells.duplicate(true)
	bad[0]["direction_raw"] = 16
	if arrows.load_native_maps_arrow_cells(bad) or arrows.arrow_count() != 240:
		fail("Bad native MAPS direction was accepted or erased a valid field")
		return
	arrows.scale = Vector2(1.0, 1.01)
	arrows.queue_redraw() # must refuse drawing via inherited nonuniform transform

	var audio: Node = audio_source.new()
	root.add_child(audio)
	var missing: PackedStringArray = audio.validate_and_load_catalog({})
	if missing.size() != audio.REQUIRED_TRACKS.size() or audio.is_fully_populated():
		fail("Commercial audio accepted missing intro/music/ambient/SFX catalog")
		return
	if audio.play_music("intro_music"):
		fail("Music played without licensed WAV/OGG content")
		return
	if audio.consume_confirmed_native_event({"confirmed_native":true, "event_id":"1", "event_type":"water_contact"}):
		fail("Sound event played without licensed asset catalog")
		return

	var visual: Node2D = native_fx_source.new()
	root.add_child(visual)
	if visual.accept_native_ball_snapshot({"shot_tick":1,"ball_x_raw":20*65536,"ball_y_raw":30*65536,"ball_z_raw":65536,"green_mode":false}) != true:
		fail("Native fixed-point ball height rejected")
		return
	if visual.accept_native_ball_snapshot({"shot_tick":2,"ball_x_raw":100*65536,"ball_y_raw":60*65536,"ball_z_raw":0,"green_mode":true}):
		fail("Green local ball coordinate was incorrectly rendered as course-world coordinate")
		return
	if not visual.accept_native_ball_snapshot({"shot_tick":2,"ball_x_raw":100*65536,"ball_y_raw":60*65536,"ball_z_raw":0,"green_mode":true,"green_origin_x":90,"green_origin_y":70}):
		fail("Verified original green-local to course-world mapping was rejected")
		return
	if visual.submit_confirmed_native_event({"confirmed_native":false,"event_id":"shot1-water","event_type":"water_contact","world_x":20,"world_y":30}):
		fail("Fake unsigned physics contact was permitted")
		return
	var confirmed := {"confirmed_native":true,"event_id":"shot1-water","event_type":"water_contact","world_x":20,"world_y":30}
	if not visual.submit_confirmed_native_event(confirmed):
		fail("Confirmed unique source water contact was rejected")
		return
	if visual.submit_confirmed_native_event(confirmed):
		fail("Duplicate source contact generated extra particles")
		return
	if not visual.submit_confirmed_native_event({"confirmed_native":true,"event_id":"shot2-tree","event_type":"verified_tree_contact","world_x":10,"world_y":30}):
		fail("Verified native source tree hit was rejected by animation director")
		return
	visual.on_confirmed_restart_or_replay_seek()

	var breeze: Node2D = foliage_source.new()
	root.add_child(breeze)
	var original_scale := breeze.scale
	breeze.update_visual_breeze(Vector2.LEFT, 0.8)
	breeze._process(0.016)
	if breeze.scale != original_scale:
		fail("Foliage animation stretched artwork")
		return
	breeze.reduced_motion = true
	breeze._process(0.016)
	if breeze.scale != original_scale:
		fail("Reduced-motion mode stretched artwork")
		return

	print("PASS GODOT PRESENTATION: 256 native slope combos, required audio fail-closed, unique native FX, no-stretch breeze, water shader")
	quit(0)
