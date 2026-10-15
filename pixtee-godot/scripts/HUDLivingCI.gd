extends SceneTree

func _initialize() -> void:
	call_deferred("_run")

func _run() -> void:
	var card = load("res://scripts/PixteeClubCard.gd").new()
	var meter = load("res://scripts/PixteeWhackMeter.gd").new()
	root.add_child(card)
	root.add_child(meter)
	card.size = Vector2(184,134)
	meter.size = Vector2(360,64)
	meter.accept_snapshot({"touch_meter_position":160,"touch_meter_phase":1,"touch_meter_active":true})
	assert(meter.marker_fraction()>0.85,"V2 meter begins at the right target")
	meter.accept_snapshot({"touch_meter_position":760,"touch_meter_phase":1,"touch_meter_active":true})
	assert(meter.marker_fraction()<0.4,"Increasing native power travels left")
	for surface in [3,5,6,7,1]:
		card.accept_snapshot({"surface_code":surface,"strokes":2,"club_index":0,"can_shoot":true})
		var lie: Dictionary = card.lie_visual()
		assert(lie.tee == false)
		if surface in [5,6,7]: assert(lie.occlusion>0,"Rough and sand obscure the lower ball")
	card.accept_snapshot({"surface_code":3,"strokes":0,"club_index":0,"can_shoot":true})
	assert(card.lie_visual().tee,"Initial shot displays a raised tee")
	card.accept_weather({"rain":0.8,"fog":0.1,"cloud":1.0,"sun":0.1,"wind":0.4},"rain",3.0,false)
	meter.accept_weather({"rain":0.8,"fog":0.1,"cloud":1.0,"sun":0.1,"wind":0.4},"rain",3.0)
	assert(card.sky.weather.rain==meter.weather.rain,"Header and rail consume the same weather")
	assert(card.sky.clock==meter.clock,"Effects share scene time")
	assert(meter.rail_rect().size.y==24.0)
	card.free()
	meter.free()
	print("HUD_LIVING_PASS: reversed meter, tee/rough/sand lies, shared weather/time, floating rail")
	quit(0)
