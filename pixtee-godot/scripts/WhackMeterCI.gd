extends SceneTree

func _initialize() -> void:
	call_deferred("_run")

func _run() -> void:
	var scene = load("res://scenes/Lakewood.tscn").instantiate()
	root.add_child(scene)
	scene.start_hole(false,false)
	scene.set_process(false)
	var before: Dictionary = scene.core.snapshot()
	assert(before.welly_window_center == 63)
	var tap := InputEventMouseButton.new()
	tap.button_index = MOUSE_BUTTON_LEFT
	tap.pressed = true
	scene.meter._input_meter(tap)
	for tick in range(100): scene.core.tick_touch_meter(false)
	var power_at: int = scene.core.snapshot().touch_meter_position
	scene.on_shoot()
	scene.advance_native_tick()
	var captured: Dictionary = scene.core.snapshot()
	assert(captured.touch_meter_phase == 2 and captured.touch_meter_power == 45)
	assert(captured.touch_meter_position == power_at,"Second tap reverses immediately without jumping")
	scene.meter.accept_snapshot(captured)
	assert(scene.meter.can_tap(),"Return accepts third tap immediately")
	var saved: String = scene.core.save_native_session()
	var copy: PixteeRoundBridge = scene.create_course_core(0)
	assert(copy.restore_native_session(saved))
	assert(copy.snapshot() == scene.core.snapshot(),"Touch meter resumes exactly")
	while scene.core.snapshot().touch_meter_position > 160:
		scene.core.tick_touch_meter(false)
	assert(scene.core.tick_touch_meter(true))
	assert(scene.core.snapshot().strokes == 1)
	# Resizing retains a fixed linear axis and every native window cell.
	for width in [320.0,720.0,1200.0]:
		assert(scene.meter.accuracy_x(62,width) < scene.meter.accuracy_x(63,width))
		assert(scene.meter.accuracy_x(63,width) < scene.meter.accuracy_x(64,width))
	scene.free()
	await create_timer(0.5).timeout
	print("WHACK_METER_PASS: native capture, exact third-press reading, window metadata and responsive axis")
	quit(0)
