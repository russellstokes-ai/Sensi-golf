extends SceneTree
func _initialize() -> void:
	call_deferred("_run")

func _run() -> void:
	var scene = load("res://scenes/Lakewood.tscn").instantiate()
	root.add_child(scene)
	scene.start_hole()
	scene.set_process(false)
	var tap := InputEventMouseButton.new()
	tap.button_index = MOUSE_BUTTON_LEFT
	tap.pressed = true
	scene.club_card._tap(tap)
	assert(scene.core.snapshot().club_index == 1,"Card tap cycles club without popup")
	for i in range(11): scene.club_card._tap(tap)
	assert(scene.core.snapshot().club_index == 12)
	scene.core.set_aim_raw(4090)
	scene.update_view(scene.core.snapshot())
	tap.position = scene.aim_touch.get_global_transform()*(scene.aim_touch.anchor+Vector2(43,0))
	scene.aim_touch._unhandled_input(tap)
	assert(scene.core.snapshot().aim_raw == 26, "Right aim wraps through native direction range")
	tap.position = scene.aim_touch.get_global_transform()*(scene.aim_touch.anchor-Vector2(43,0))
	scene.aim_touch._unhandled_input(tap)
	assert(scene.core.snapshot().aim_raw == 4090)
	assert(scene.world.golfer_facing_left)
	scene.minimap._tap(tap)
	scene.minimap._tap(tap)
	assert(scene.minimap.expanded)
	var rect: Rect2 = scene.minimap.map_rect()
	assert(is_equal_approx(rect.size.x/544.0,rect.size.y/912.0), "Minimap preserves course proportions")
	scene.on_shoot()
	scene.update_view(scene.core.snapshot())
	assert(not scene.aim_touch.enabled, "No aim changes during meter")
	assert(not scene.club_card.state.can_shoot)
	var before_lock: int = scene.core.snapshot().club_index
	scene.club_card._tap(tap)
	assert(scene.core.snapshot().club_index == before_lock)
	scene.pause_game()
	scene.free()
	await create_timer(0.5).timeout
	print("MOBILE_HUD_PASS: integrated club picker, native aiming wrap, double-tap map, aspect ratio and shot/pause locks")
	quit(0)
