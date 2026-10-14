extends SceneTree
func _initialize() -> void:
	call_deferred("_run")
func viewport() -> SubViewport:
	var view := SubViewport.new()
	view.size = Vector2i(128,128)
	view.transparent_bg = true
	view.render_target_update_mode = SubViewport.UPDATE_ALWAYS
	root.add_child(view)
	return view
func copy_layer(source: Sprite2D, target: SubViewport, part: int) -> Sprite2D:
	var sprite := Sprite2D.new()
	sprite.texture = source.texture
	sprite.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	sprite.centered = false
	sprite.region_enabled = true
	sprite.region_rect = source.region_rect
	sprite.scale = source.scale*4.0
	sprite.position = Vector2(64,112)+source.position*4.0
	sprite.material = source.material.duplicate()
	sprite.material.set_shader_parameter("layer_part",part)
	target.add_child(sprite)
	return sprite
func _run() -> void:
	var world = load("res://scripts/LakewoodWorld.gd").new()
	root.add_child(world)
	world.set_process(false)
	var sheet := Image.create(1024,256,false,Image.FORMAT_RGBA8)
	sheet.fill(Color.TRANSPARENT)
	for gender in range(2):
		world.set_outfit("female:kit:0:0:0" if gender == 1 else "kit:0:0:0")
		for direction in range(8):
			world.accept_snapshot({"aim_raw":direction*512,"hole_phase":0,"ball_x_raw":256*65536,"ball_y_raw":850*65536,"green_mode":false})
			world.time = 0.8+direction*0.3
			world.update_player_sprite()
			var whole := viewport()
			var jointed := viewport()
			copy_layer(world.player_sprite,whole,0)
			copy_layer(world.player_sprite,jointed,1)
			copy_layer(world.player_upper,jointed,2)
			await process_frame
			await RenderingServer.frame_post_draw
			var original := whole.get_texture().get_image()
			var composed := jointed.get_texture().get_image()
			assert(original.get_data() == composed.get_data(),"Two body layers must exactly reconstruct the complete sprite at every idle phase")
			sheet.blit_rect(composed,Rect2i(0,0,128,128),Vector2i(direction*128,gender*128))
			whole.queue_free()
			jointed.queue_free()
	assert(DirAccess.make_dir_recursive_absolute(ProjectSettings.globalize_path("res://review-captures")) == OK)
	assert(sheet.save_png("res://review-captures/player-stances-and-joints.png") == OK)
	print("PLAYER_JOINT_VISUAL_PASS: both golfers, eight shot headings, exact layered silhouette reconstruction")
	print("PLAYER_JOINT_REVIEW_PNG:"+Marshalls.raw_to_base64(sheet.save_png_to_buffer()))
	world.free()
	quit(0)
