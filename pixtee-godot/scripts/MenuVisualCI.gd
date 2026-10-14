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
	# A real rendered pixel must match its selected palette, not multiply the
	# source colour a second time. Exercise the same atlas used by the player UI.
	var palette_view := SubViewport.new()
	palette_view.size = Vector2i(362,181)
	palette_view.transparent_bg = true
	palette_view.render_target_update_mode = SubViewport.UPDATE_ALWAYS
	root.add_child(palette_view)
	var sprite := Sprite2D.new()
	sprite.texture = load("res://courses/lakewood-hole-1/visual/golfer.png")
	sprite.centered = false
	sprite.region_enabled = true
	sprite.region_rect = Rect2(0,0,362,181)
	sprite.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	sprite.material = PixteeOutfit.material("kit:3:2:1",{"skin":4,"hair":5,"face":2,"beard":0,"hairstyle":1})
	sprite.material.set_shader_parameter("atlas_grid",Vector2(6,4))
	palette_view.add_child(sprite)
	await process_frame
	await RenderingServer.frame_post_draw
	var source: Image = sprite.texture.get_image()
	var rendered := palette_view.get_texture().get_image()
	var checked := false
	for y in range(181):
		for x in range(362):
			var original := source.get_pixel(x,y)
			if original.a > 0.99 and original.r > 0.8 and original.g < 0.1 and original.b < 0.1:
				var changed := rendered.get_pixel(x,y)
				assert(changed.b > changed.r and changed.b > 0.3,"Purple shirt must render purple, not source-red multiplied")
				checked = true
				break
		if checked: break
	assert(checked)
	palette_view.queue_free()
	assert(DirAccess.make_dir_recursive_absolute(ProjectSettings.globalize_path("res://review-captures")) == OK)
	var scene = load("res://scenes/Lakewood.tscn").instantiate()
	root.add_child(scene)
	await process_frame
	assert(scene.profile.load_profile("user://tests/visual-"+str(Time.get_ticks_usec())))
	scene.profile.set_setting("graphics_quality",2)
	scene.profile.set_setting("reduced_motion",false)
	scene.apply_saved_settings()
	scene.show_title()
	await capture(scene,"title-portrait")
	print("TITLE_REVIEW_PNG:"+Marshalls.raw_to_base64(root.get_texture().get_image().save_png_to_buffer()))
	scene.show_player()
	var selectors: Array[OptionButton] = []
	for child in scene.panel.get_children():
		if child is OptionButton: selectors.append(child)
		if child is HBoxContainer:
			for choice in child.get_children():
				if choice is OptionButton: selectors.append(choice)
	assert(selectors.size()==9)
	var selections := [1,3,2,1,4,5,2,0,1]
	for i in range(9):
		selectors[i].select(selections[i])
		selectors[i].item_selected.emit(selections[i])
	assert(scene.player_draft.outfit_id == "female:kit:3:2:1","Real selectors must produce one stable combined outfit")
	assert(scene.player_draft.appearance == {"skin":4,"hair":5,"face":2,"beard":0,"hairstyle":1})
	await capture(scene,"player-portrait")
	print("PLAYER_REVIEW_PNG:"+Marshalls.raw_to_base64(root.get_texture().get_image().save_png_to_buffer()))
	selectors[0].select(0)
	selectors[0].item_selected.emit(0)
	selectors[7].select(2)
	selectors[7].item_selected.emit(2)
	await capture(scene,"beard-portrait")
	print("BEARD_REVIEW_PNG:"+Marshalls.raw_to_base64(root.get_texture().get_image().save_png_to_buffer()))
	scene.cancel_player_edit()
	scene.show_settings()
	await capture(scene,"settings-portrait")
	scene.back_menu()
	scene.show_help()
	await capture(scene,"help-portrait")
	scene.start_hole(false,false)
	await capture(scene,"whack-meter-portrait",false)
	var meter_image := root.get_texture().get_image().get_region(Rect2i(0,root.size.y-240,root.size.x,240))
	print("WHACK_METER_PNG:"+Marshalls.raw_to_base64(meter_image.save_png_to_buffer()))
	print("MOBILE_HUD_PNG:"+Marshalls.raw_to_base64(root.get_texture().get_image().save_png_to_buffer()))
	scene.set_process(false)
	scene.camera.position = Vector2(408,400)
	scene.camera.offset = Vector2.ZERO
	scene.world.view_rect = Rect2(scene.camera.position-Vector2(120,253),Vector2(240,506))
	scene.camera.force_update_scroll()
	await capture(scene,"river-review",false)
	print("RIVER_REVIEW_PNG:"+Marshalls.raw_to_base64(root.get_texture().get_image().save_png_to_buffer()))
	scene.set_process(true)
	scene.pause_game()
	scene.show_scorecard()
	await capture(scene,"scorecard-portrait")
	scene.back_menu()
	root.size = Vector2i(1200,1000)
	await process_frame
	scene.update_ui_layout()
	await capture(scene,"pause-wide")
	scene.start_hole(false,false)
	scene.paused = true
	scene.world.set_process(false)
	scene.fx.set_effects_running(false)
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
	print("GREEN_REVIEW_PNG:"+Marshalls.raw_to_base64(root.get_texture().get_image().save_png_to_buffer()))
	scene.core.set_club_index(12)
	scene.sync_attended_flag_for_native(scene.core)
	scene.world.accept_snapshot(scene.core.snapshot())
	scene.world.get_node("Caddy")._process(0.1)
	await capture(scene,"putting-caddy",false)
	print("CADDY_REVIEW_PNG:"+Marshalls.raw_to_base64(root.get_texture().get_image().save_png_to_buffer()))
	root.size = Vector2i(1200,1000)
	await process_frame
	scene.update_ui_layout()
	await capture(scene,"green-wide",false)
	print("GREEN_VISUAL_PASS: actual native green and slope arrows rendered in portrait/wide crops")
	print("MENU_VISUAL_PASS: actual supplied artwork and menu renders captured in portrait/wide windows")
	scene.free()
	await create_timer(0.5).timeout # Retire audio mixer voices before quitting.
	quit(0)
