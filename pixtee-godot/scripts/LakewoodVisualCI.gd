extends SceneTree

func _initialize() -> void:
	call_deferred("_run")

func _run() -> void:
	var scene = load("res://scenes/Lakewood.tscn").instantiate()
	root.add_child(scene)
	for i in 5: await process_frame
	await RenderingServer.frame_post_draw
	root.get_texture().get_image().save_png("res://../../lakewood-menu.png")
	scene.start_hole()
	for i in 5: await process_frame
	await RenderingServer.frame_post_draw
	root.get_texture().get_image().save_png("res://../../lakewood-tee.png")
	print("LAKEWOOD_VISUAL_CAPTURED")
	quit()
