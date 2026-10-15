extends SceneTree
var views: Array[SubViewport] = []
var cards: Array[Control] = []
var meters: Array[Control] = []
var presets := ["sunny","breezy","rain","morning_mist"]

func _initialize() -> void:
	call_deferred("_run")

func _run() -> void:
	var dir := ProjectSettings.globalize_path("res://review-captures/hud-v2")
	assert(DirAccess.make_dir_recursive_absolute(dir)==OK)
	for i in 4:
		var view := SubViewport.new()
		view.size = Vector2i(360,230)
		view.render_target_update_mode = SubViewport.UPDATE_ALWAYS
		view.disable_3d = true
		root.add_child(view)
		views.append(view)
		var bg := TextureRect.new()
		bg.texture = load("res://courses/lakewood-hole-1/visual/ground.png")
		bg.expand_mode = TextureRect.EXPAND_IGNORE_SIZE
		bg.stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_COVERED
		bg.size = Vector2(360,230)
		bg.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
		view.add_child(bg)
		var card = load("res://scripts/PixteeClubCard.gd").new()
		card.size = Vector2(184,134)
		card.position = Vector2(8,8)
		view.add_child(card)
		card.set_process(false)
		card.accept_snapshot({"surface_code":[3,6,7,3][i],"strokes":0 if i==0 else 2,"club_index":0,"distance_to_hole":280,"club_nominal_max":240,"wind_enabled":true,"wind_speed_mph_x10":60,"wind_toward_raw":1536,"can_shoot":true})
		cards.append(card)
		var meter = load("res://scripts/PixteeWhackMeter.gd").new()
		meter.size = Vector2(360,64)
		meter.position = Vector2(0,163)
		view.add_child(meter)
		meter.set_process(false)
		meters.append(meter)
	# Contact sheet uses the actual game Controls, not a painted approximation.
	var first: PackedByteArray
	for frame in 72:
		var time := frame/12.0
		for i in 4:
			var condition: Dictionary = PixteeWeatherDirector.PRESETS[presets[i]].duplicate()
			cards[i].accept_weather(condition,presets[i],time,false)
			meters[i].accept_weather(condition,presets[i],time)
			var elapsed := maxf(0,time-.75)
			var position := clampi(160+int(elapsed*70*6) if elapsed<2.5 else 1210-int((elapsed-2.5)*70*7),0,1210)
			var phase := 0 if time<.75 else 1 if elapsed<2.5 else 2
			meters[i].accept_snapshot({"touch_meter_position":position,"touch_meter_phase":phase,"touch_meter_active":phase>0,"touch_meter_power":105,"touch_meter_red_half":[48,21,15,48][i],"touch_meter_yellow":[64,28,20,64][i],"can_shoot":phase==0})
		await process_frame
		await RenderingServer.frame_post_draw
		var sheet := Image.create(720,460,false,Image.FORMAT_RGBA8)
		for i in 4: sheet.blit_rect(views[i].get_texture().get_image(),Rect2i(0,0,360,230),Vector2i((i%2)*360,(i/2)*230))
		assert(sheet.save_png(dir+"/frame-%03d.png"%frame)==OK)
		if frame==0: first = sheet.get_data()
		if frame==24: assert(first!=sheet.get_data(),"Sky, lie foliage and rail must actually animate")
	var gallery := Image.create(720,920,false,Image.FORMAT_RGBA8)
	var names := ["sunny","sunny","sunny","sunny","breezy","rain","morning_mist","golden_hour"]
	for i in 8:
		var condition: Dictionary = PixteeWeatherDirector.PRESETS[names[i]].duplicate()
		cards[0].accept_weather(condition,names[i],2.1,false)
		cards[0].accept_snapshot({"surface_code":7 if i==5 else 5 if i==4 else 3,"strokes":0 if i<4 else 2,"club_index":0,"distance_to_hole":280,"club_nominal_max":240,"wind_enabled":true,"wind_speed_mph_x10":60,"wind_toward_raw":1536,"can_shoot":true})
		meters[0].accept_weather(condition,names[i],2.1)
		meters[0].accept_snapshot({"touch_meter_position":160 if i!=2 else 720,"touch_meter_phase":1 if i==2 else 2 if i==3 else 0,"touch_meter_active":i in [2,3],"touch_meter_power":60,"touch_meter_red_half":48,"touch_meter_yellow":64,"can_shoot":i!=0 and i not in [2,3]})
		await process_frame
		await RenderingServer.frame_post_draw
		gallery.blit_rect(views[0].get_texture().get_image(),Rect2i(0,0,360,230),Vector2i((i%2)*360,(i/2)*230))
	assert(gallery.save_png(dir+"/eight-states.png")==OK)
	for view in views: view.free()
	print("HUD_LIVING_VISUAL_PASS: 72 actual rendered frames + eight-state gallery, tee/rough/sand/fairway, four animated weather states")
	quit(0)
