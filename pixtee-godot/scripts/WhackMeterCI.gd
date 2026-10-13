extends SceneTree

func _initialize() -> void:
	call_deferred("_run")

func _run() -> void:
	var scene = load("res://scenes/Lakewood.tscn").instantiate()
	root.add_child(scene)
	scene.start_hole()
	scene.set_process(false)
	var before: Dictionary = scene.core.snapshot()
	assert(before.welly_window_center == 63)
	scene.on_shoot()
	for tick in range(30): scene.core.tick_original_welly(false)
	scene.core.tick_original_welly(true)
	var captured: Dictionary = scene.core.snapshot()
	assert(captured.welly_power_captured and captured.welly_saved_power == 30)
	while scene.core.snapshot().welly_phase == 1: scene.core.tick_original_welly(false)
	while scene.core.snapshot().welly_counter > 64: scene.core.tick_original_welly(false)
	var ready: Dictionary = scene.core.snapshot()
	assert(ready.welly_press_accuracy == 63, "Marker must account for native decrement before acceptance")
	scene.meter.accept_snapshot(ready)
	assert(scene.meter.instruction == "3  SET ACCURACY")
	assert(scene.core.tick_original_welly(true))
	assert(scene.core.snapshot().strokes == 1)
	# Resizing retains a fixed linear axis and every native window cell.
	for width in [320.0,720.0,1200.0]:
		assert(scene.meter.accuracy_x(62,width) < scene.meter.accuracy_x(63,width))
		assert(scene.meter.accuracy_x(63,width) < scene.meter.accuracy_x(64,width))
	scene.free()
	await create_timer(0.5).timeout
	print("WHACK_METER_PASS: native capture, exact third-press reading, window metadata and responsive axis")
	quit(0)
