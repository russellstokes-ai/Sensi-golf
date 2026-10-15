extends Node

const TICK_SECONDS := 936.0/65536.0
const COURSE := "res://courses/lakewood-hole-1/"
const NATIVE_COURSE := COURSE+"native-green-v1/"
const UI := "res://ui/supplied/"
const CLUBS := ["Driver","3 Wood","5 Wood","2 Iron","3 Iron","4 Iron","5 Iron","6 Iron","7 Iron","8 Iron","9 Iron","Wedge","Putter"]
var core: PixteeRoundBridge
var world: Node2D
var camera: Camera2D
var fx: PixteeNativeWorldFX
var arrows: PixteeVerifiedGreenArrows
var menu: Control
var hud: Control
var status: Label
var club_card: Control
var minimap: Control
var aim_touch: Control
var meter: Control
var playing := false
var paused := false
var accumulator := 0.0
var pending_press := false
var reduced_motion := false
var panel: VBoxContainer
var ui_scale := 1.0
var profile = preload("res://scripts/PixteePlayerProfile.gd").new()
var audio: PixteeCommercialAudio
var player_name: LineEdit
var player_draft: Dictionary = {}
var player_error: Label
var player_preview: TextureRect
var menu_screen := "title"
var return_screen := "title"
var menu_scroll: ScrollContainer
var attempt_id := 0
var last_result: Dictionary = {}
var result_saved := false
var intro_tween: Tween
var session_store := preload("res://scripts/PixteeSessionStore.gd").new()
var course_hash := ""
var recovery_error := ""
var shot_transition: CanvasLayer
var shot_was_active := false

func _ready() -> void:
	profile.load_profile()
	var hash_context := HashingContext.new()
	hash_context.start(HashingContext.HASH_SHA256)
	hash_context.update("pixtee-native-authoring-v5:touch-meter:boards:soft-ground:bird-flocks:attended-flag:authored-wind-v1:touch-meter-v2".to_utf8_buffer())
	for resource in ["MAPM.bin","SPT.bin","MAPI-descriptors.bin","MAPI-selectors.bin","MAPS.bin"]:
		hash_context.update(FileAccess.get_file_as_bytes(NATIVE_COURSE+resource))
	course_hash = hash_context.finish().hex_encode()
	session_store.load_current(course_hash)
	audio = PixteeCommercialAudio.new()
	add_child(audio)
	for failure in audio.load_owner_pack(): push_error(failure)
	world = Node2D.new()
	world.set_script(load("res://scripts/LakewoodWorld.gd"))
	add_child(world)
	camera = Camera2D.new()
	camera.zoom = Vector2(1.5,1.5)
	add_child(camera)
	fx = PixteeNativeWorldFX.new()
	add_child(fx)
	arrows = PixteeVerifiedGreenArrows.new()
	add_child(arrows)
	apply_saved_settings()
	var layer := CanvasLayer.new()
	add_child(layer)
	menu = Control.new()
	menu.theme = PixteeUIStyle.theme()
	menu.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	layer.add_child(menu)
	hud = Control.new()
	hud.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	layer.add_child(hud)
	hud.mouse_filter = Control.MOUSE_FILTER_IGNORE
	hud.hide()
	shot_transition = preload("res://scripts/PixteeShotTransition.gd").new()
	add_child(shot_transition)
	make_hud()
	get_viewport().size_changed.connect(update_ui_layout)
	update_ui_layout()
	var attract := preload("res://scripts/PixteeMenuAttract.gd").new()
	attract.game = self
	add_child(attract)
	var autosave := Timer.new()
	autosave.wait_time = 1.0
	autosave.autostart = true
	add_child(autosave)
	autosave.timeout.connect(save_session_checkpoint)
	show_intro()

func update_ui_layout() -> void:
	var screen := get_viewport().get_visible_rect().size
	ui_scale = maxf(0.5,minf(screen.x/360.0,screen.y/760.0))
	for control in [menu,hud]:
		control.set_anchors_and_offsets_preset(Control.PRESET_TOP_LEFT)
		control.size = screen/ui_scale
		control.scale = Vector2.ONE*ui_scale
	if playing and core != null:
		update_view(core.snapshot())

func image_control(file: String, dimensions: Vector2) -> TextureRect:
	var image := TextureRect.new()
	image.texture = load(UI+file+".png")
	image.expand_mode = TextureRect.EXPAND_IGNORE_SIZE
	image.stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_CENTERED
	image.custom_minimum_size = dimensions
	image.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	image.mouse_filter = Control.MOUSE_FILTER_IGNORE
	return image

func button(text: String, action: Callable, dimensions := Vector2(288,51)) -> Button:
	var result := Button.new()
	result.custom_minimum_size = dimensions
	result.text = text
	result.add_theme_color_override("font_color",Color("fff2c2"))
	result.add_theme_font_size_override("font_size",18)
	for state in ["normal","hover","pressed","focus"]:
		result.add_theme_stylebox_override(state,StyleBoxEmpty.new())
	var art := image_control("button_normal",dimensions)
	result.add_child(art)
	art.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	art.show_behind_parent = true
	result.draw.connect(func():
		var state := "button_normal"
		if result.disabled: state = "button_disabled"
		elif result.button_pressed: state = "button_pressed"
		elif result.is_hovered() or result.has_focus(): state = "button_selected"
		var texture: Texture2D = load(UI+state+".png")
		if art.texture != texture: art.texture = texture)
	result.pressed.connect(action)
	return result

func clear_menu(background_alpha := 0.92) -> void:
	for child in menu.get_children():
		menu.remove_child(child)
		child.queue_free()
	menu.show()
	var shade := ColorRect.new()
	shade.color = Color(0.025,0.085,0.065,background_alpha)
	menu.add_child(shade)
	shade.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	var margin := MarginContainer.new()
	menu.add_child(margin)
	margin.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	for edge in ["left","right","top","bottom"]: margin.add_theme_constant_override("margin_"+edge,16)
	menu_scroll = ScrollContainer.new()
	menu_scroll.horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	margin.add_child(menu_scroll)
	var center := CenterContainer.new()
	center.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	center.size_flags_vertical = Control.SIZE_EXPAND_FILL
	menu_scroll.add_child(center)
	panel = VBoxContainer.new()
	panel.custom_minimum_size.x = 300
	panel.add_theme_constant_override("separation",12)
	center.add_child(panel)

func text_line(text: String) -> Label:
	var label := Label.new()
	label.text = text
	label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	label.add_theme_color_override("font_color",Color("fff2c2"))
	return label

func show_title() -> void:
	audio.set_context("menu")
	stop_intro()
	menu_screen = "title"
	hud.hide()
	clear_menu(0.10)
	world.set_process(true)
	panel.add_child(image_control("logo",Vector2(288,180)))
	panel.add_child(text_line("LAKEWOOD · WILLOW CREEK\nHole 1 · Par 4"))
	add_music_toggle()
	var checkpoint: Dictionary = session_store.load_current(course_hash)
	if not checkpoint.is_empty() and int(checkpoint.attempt)>int(profile.snapshot().statistics.last_recorded): panel.add_child(button("Continue",resume_saved_game))
	panel.add_child(button("Play",start_hole))
	panel.add_child(button("Player",show_player))
	panel.add_child(button("Statistics",show_statistics))
	panel.add_child(button("Settings",show_settings))
	panel.add_child(button("How to play",show_help))

func stop_intro() -> void:
	if intro_tween != null and intro_tween.is_valid(): intro_tween.kill()
	intro_tween = null

func show_intro() -> void:
	if reduced_motion:
		show_title()
		return
	menu_screen = "intro"
	clear_menu()
	camera.position = Vector2(256,700)
	var logo := image_control("logo",Vector2(288,180))
	logo.modulate.a = 0.0
	panel.add_child(logo)
	intro_tween = create_tween()
	intro_tween.tween_property(logo,"modulate:a",1.0,0.7).set_trans(Tween.TRANS_SINE).set_ease(Tween.EASE_OUT)
	intro_tween.tween_interval(1.2)
	intro_tween.tween_callback(show_title)

func show_golfers() -> void:
	open_auxiliary("golfers")
	clear_menu()
	var list := ItemList.new()
	list.custom_minimum_size = Vector2(300,330)
	var data: Dictionary = JSON.parse_string(FileAccess.get_file_as_string("res://ui/ai-golfer-characters.json"))
	var description := text_line("")
	description.custom_minimum_size = Vector2(300,80)
	description.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	for character in data.characters: list.add_item(character.name)
	list.item_selected.connect(func(index: int): description.text = data.characters[index].personality)
	panel.add_child(text_line("PIXTEE GOLFERS"))
	panel.add_child(list)
	panel.add_child(description)
	panel.add_child(button("Back",back_menu))

func show_settings() -> void:
	open_auxiliary("settings")
	clear_menu()
	panel.add_child(text_line("SETTINGS"))
	var motion := CheckButton.new()
	motion.text = "Reduced motion"
	motion.button_pressed = bool(profile.snapshot().settings.reduced_motion)
	motion.toggled.connect(func(value: bool):
		if profile.set_setting("reduced_motion",value): apply_saved_settings())
	panel.add_child(motion)
	var flashes := CheckButton.new()
	flashes.text = "Reduce flashes"
	flashes.button_pressed = bool(profile.snapshot().settings.reduced_flashes)
	flashes.toggled.connect(func(value: bool):
		if profile.set_setting("reduced_flashes",value): apply_saved_settings())
	panel.add_child(flashes)
	panel.add_child(text_line("Graphics"))
	var quality := OptionButton.new()
	for title in ["Low","Standard","High"]: quality.add_item(title)
	quality.select(int(profile.snapshot().settings.graphics_quality))
	quality.custom_minimum_size.y = 44
	quality.item_selected.connect(func(index: int):
		if profile.set_setting("graphics_quality",index): apply_saved_settings())
	panel.add_child(quality)
	panel.add_child(text_line("Frame limit"))
	var frames := OptionButton.new()
	var limits := [30,60,90,120]
	for limit in limits: frames.add_item(str(limit)+" FPS",limit)
	frames.select(limits.find(int(profile.snapshot().settings.frame_limit)))
	frames.custom_minimum_size.y = 44
	frames.item_selected.connect(func(index: int):
		if profile.set_setting("frame_limit",frames.get_item_id(index)): apply_saved_settings())
	panel.add_child(frames)
	panel.add_child(text_line("Weather"))
	var weather_choice := OptionButton.new()
	var presets: Array = PixteeWeatherDirector.PRESETS.keys()
	for preset in presets: weather_choice.add_item(str(preset).replace("_"," ").capitalize())
	weather_choice.select(presets.find(profile.snapshot().settings.weather_preset))
	weather_choice.custom_minimum_size.y = 44
	weather_choice.item_selected.connect(func(index: int):
		if profile.set_setting("weather_preset",presets[index]): apply_saved_settings())
	panel.add_child(weather_choice)
	panel.add_child(text_line("Wind · next round"))
	var wind_speed := OptionButton.new()
	var speeds := [0,40,60,80,120,150]
	for speed in speeds: wind_speed.add_item("Calm" if speed == 0 else str(speed/10)+" mph")
	wind_speed.select(speeds.find(int(profile.snapshot().settings.wind_speed_mph_x10)))
	wind_speed.custom_minimum_size.y = 44
	wind_speed.item_selected.connect(func(index: int): profile.set_setting("wind_speed_mph_x10",speeds[index]))
	panel.add_child(wind_speed)
	var wind_direction := OptionButton.new()
	var headings := [2048,1536,1024,512,0,3584,3072,2560]
	for title in ["Toward north","Toward northeast","Toward east","Toward southeast","Toward south","Toward southwest","Toward west","Toward northwest"]: wind_direction.add_item(title)
	wind_direction.select(headings.find(int(profile.snapshot().settings.wind_toward_raw)))
	wind_direction.custom_minimum_size.y = 44
	wind_direction.item_selected.connect(func(index: int): profile.set_setting("wind_toward_raw",headings[index]))
	panel.add_child(wind_direction)
	for channel in ["Music","SFX","Ambience","UI"]:
		panel.add_child(text_line(channel))
		var slider := HSlider.new()
		slider.min_value = 0.0
		slider.max_value = 1.0
		slider.step = 0.05
		slider.value = float(profile.snapshot().settings[channel])
		slider.custom_minimum_size = Vector2(288,32)
		slider.drag_ended.connect(func(changed: bool):
			if changed and profile.set_setting(channel,slider.value): apply_saved_settings())
		# Keyboard/accessibility changes also persist when focus leaves the control.
		slider.focus_exited.connect(func():
			if profile.set_setting(channel,slider.value): apply_saved_settings())
		panel.add_child(slider)
	panel.add_child(button("Back",back_menu))

func apply_saved_settings() -> void:
	var settings: Dictionary = profile.snapshot().settings
	reduced_motion = bool(settings.reduced_motion)
	world.reduced_motion = reduced_motion
	fx.reduced_motion = reduced_motion
	world.reduced_flashes = bool(settings.reduced_flashes)
	world.visual_quality = int(settings.graphics_quality)
	fx.visual_quality = int(settings.graphics_quality)
	Engine.max_fps = int(settings.frame_limit)
	if world.living_world.weather.preset_name != settings.weather_preset:
		world.living_world.weather.set_preset(settings.weather_preset,0.0 if reduced_motion else 3.0)
	for child in world.get_children():
		if child is Polygon2D: child.material.set_shader_parameter("quality_enabled",0.0 if reduced_motion or int(settings.graphics_quality) == 0 else 1.0)
	world.set_outfit(profile.snapshot().outfit_id)
	world.set_appearance(profile.snapshot().appearance)
	world.queue_redraw()
	for channel in ["Music","SFX","Ambience","UI"]:
		audio.set_channel_level(channel,float(settings[channel]))

func show_player() -> void:
	if playing and not paused: pause_game()
	open_auxiliary("player")
	player_draft = profile.begin_edit()
	clear_menu()
	panel.add_child(text_line("PLAYER"))
	# This is the actual approved golfer, not a clothing thumbnail.
	var preview := TextureRect.new()
	var atlas := AtlasTexture.new()
	atlas.atlas = load(COURSE+"visual/golfer.png")
	var frame: Array = world.assets.golfer.frames[0]
	atlas.region = Rect2(float(frame[0]),float(frame[1]),float(frame[2]),float(frame[3]))
	preview.texture = atlas
	preview.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	preview.expand_mode = TextureRect.EXPAND_IGNORE_SIZE
	preview.stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_CENTERED
	preview.custom_minimum_size = Vector2(180,180)
	panel.add_child(preview)
	player_preview = preview
	preview.material = PixteeOutfit.material(player_draft.outfit_id,player_draft.appearance)
	var gender := OptionButton.new()
	gender.add_item("Male golfer")
	gender.add_item("Female golfer")
	gender.select(1 if PixteeOutfit.female(player_draft.outfit_id) else 0)
	gender.custom_minimum_size = Vector2(288,44)
	gender.item_selected.connect(func(index: int):
		player_draft.outfit_id = ("female:" if index == 1 else "")+str(player_draft.outfit_id).trim_prefix("female:")
		refresh_player_preview())
	panel.add_child(gender)
	add_outfit_controls()
	add_appearance_controls()
	refresh_player_preview()
	player_name = LineEdit.new()
	player_name.text = str(player_draft.display_name)
	player_name.max_length = 32
	player_name.custom_minimum_size = Vector2(288,44)
	panel.add_child(player_name)
	player_error = text_line("")
	panel.add_child(player_error)
	# Keep commit/cancel reachable while the appearance list scrolls.
	menu_scroll.get_parent().add_theme_constant_override("margin_bottom",134)
	var footer := CenterContainer.new()
	menu.add_child(footer)
	footer.set_anchors_and_offsets_preset(Control.PRESET_BOTTOM_WIDE)
	footer.offset_top = -126
	footer.offset_bottom = -12
	var actions := VBoxContainer.new()
	actions.name = "PlayerActions"
	actions.add_theme_constant_override("separation",8)
	footer.add_child(actions)
	actions.add_child(button("Apply",func(): apply_player_edit()))
	actions.add_child(button("Cancel",cancel_player_edit))

func apply_player_edit() -> bool:
	player_draft.display_name = player_name.text
	if not profile.apply_edit(player_draft):
		player_error.text = "Enter a name; check save storage."
		return false
	world.set_outfit(profile.snapshot().outfit_id)
	world.set_appearance(profile.snapshot().appearance)
	player_draft.clear()
	back_menu()
	return true

func cancel_player_edit() -> void:
	player_draft.clear()
	back_menu()

func make_hud() -> void:
	var top := HBoxContainer.new()
	var backing := ColorRect.new()
	backing.color = Color(0.025,0.085,0.065,0.88)
	hud.add_child(backing)
	backing.set_anchors_and_offsets_preset(Control.PRESET_TOP_WIDE)
	backing.offset_bottom = 64
	backing.mouse_filter = Control.MOUSE_FILTER_IGNORE
	hud.add_child(top)
	top.set_anchors_and_offsets_preset(Control.PRESET_TOP_WIDE)
	status = text_line("Willow Creek · Par 4")
	status.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	top.add_child(status)
	top.add_child(button("Ⅱ",pause_game,Vector2(52,51)))
	var bottom := VBoxContainer.new()
	hud.add_child(bottom)
	bottom.set_anchors_and_offsets_preset(Control.PRESET_BOTTOM_WIDE)
	bottom.offset_top = -64
	club_card = preload("res://scripts/PixteeClubCard.gd").new()
	hud.add_child(club_card)
	club_card.set_anchors_and_offsets_preset(Control.PRESET_TOP_LEFT)
	club_card.offset_left = 8
	club_card.offset_right = 192
	club_card.offset_top = 54
	club_card.offset_bottom = 188
	club_card.club_selected.connect(select_club)
	club_card.weather_source = world.living_world.weather
	club_card.time_source = world.presentation
	minimap = preload("res://scripts/PixteeMiniMap.gd").new()
	minimap.world = world
	hud.add_child(minimap)
	aim_touch = preload("res://scripts/PixteeAimTouch.gd").new()
	aim_touch.game = self
	hud.add_child(aim_touch)
	aim_touch.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	aim_touch.aim_changed.connect(change_aim)
	meter = preload("res://scripts/PixteeWhackMeter.gd").new()
	meter.pressed.connect(on_shoot)
	bottom.add_child(meter)
	meter.weather_source = world.living_world.weather
	meter.time_source = world.presentation

func start_hole(resuming := false, use_wind := true) -> void:
	shot_transition.cancel_discard()
	shot_was_active = false
	stop_intro()
	menu_screen = "game"
	core = create_course_core(0,use_wind)
	if core == null: return
	world.set_outfit(profile.snapshot().outfit_id)
	world.set_appearance(profile.snapshot().appearance)
	var arrow_cells: Array[Dictionary] = []
	for cell in core.verified_native_green_arrow_cells(16): arrow_cells.append(cell)
	if not arrows.load_native_maps_arrow_cells(arrow_cells) or arrows.arrow_count()==0:
		push_error("Native Lakewood slope field unavailable")
		return
	attempt_id = int(profile.snapshot().statistics.round_sequence) if resuming else profile.begin_attempt()
	last_result.clear()
	result_saved = false
	player_draft.clear()
	playing = true
	paused = false
	accumulator = 0
	pending_press = false
	world.set_process(true)
	fx.set_effects_running(true)
	fx.on_confirmed_restart_or_replay_seek()
	audio.reset_event_cursor_on_confirmed_new_round()
	world.pond_ducks.reset_confirmed_event_cursor()
	world.living_world.reset_confirmed_event_cursor()
	audio.set_context("course")
	menu.hide()
	hud.show()
	update_view(core.snapshot())
	if not resuming: save_session_checkpoint()

func on_shoot() -> void:
	if not playing or paused or core == null or shot_transition.busy: return
	var s: Dictionary = core.snapshot()
	if s.get("touch_meter_active",false) or s.get("original_welly_active",false):
		pending_press = true
	elif s.get("can_shoot",false):
		sync_attended_flag_for_native(core)
		core.start_touch_meter()

func select_club(index: int) -> void:
	if playing and not paused and not shot_transition.busy and core != null and core.snapshot().get("can_shoot",false):
		core.set_club_index(index)
		sync_attended_flag_for_native(core)
		update_view(core.snapshot())

func change_aim(amount: int) -> void:
	if playing and not paused and not shot_transition.busy and core != null and core.snapshot().get("can_shoot",false):
		core.nudge_aim_raw(amount)
		update_view(core.snapshot())

func _process(delta: float) -> void:
	if not playing or paused or core == null:
		if not paused or not playing: shot_transition.advance(delta)
		return
	accumulator += minf(delta,0.25)
	var ticks := 0
	while accumulator >= TICK_SECONDS and ticks < 16:
		advance_native_tick()
		accumulator -= TICK_SECONDS
		ticks += 1
	var s: Dictionary = core.snapshot()
	var phase := int(s.get("hole_phase",-1))
	if shot_was_active and phase in [0,6] and not shot_transition.busy:
		shot_was_active = false
		shot_transition.begin(func():
			var current: Dictionary = core.snapshot()
			update_view(current)
			if int(current.get("hole_phase",-1)) == 6: show_score(current))
	elif phase == 6 and not shot_transition.busy:
		show_score(s)
	shot_transition.advance(delta)
	update_view(s,shot_transition.busy and not shot_transition.swapped)

func advance_native_tick() -> void:
	if paused or core == null: return
	sync_attended_flag_for_native(core)
	var native_before: Dictionary = core.snapshot()
	if native_before.get("touch_meter_active",false):
		core.tick_touch_meter(pending_press)
		pending_press = false
	elif native_before.get("original_welly_active",false):
		core.tick_original_welly(pending_press)
		pending_press = false
	core.step_tick()
	if int(core.snapshot().get("hole_phase",-1)) in [1,2,3]: shot_was_active = true
	for event in core.drain_presentation_events():
		if event.get("event_type","") == "club_contact":
			# Launch lie comes directly from the preceding native snapshot.
			event["launch_surface_code"] = native_before.get("surface_code",-1)
			event["launch_club_index"] = native_before.get("club_index",-1)
		fx.submit_confirmed_native_event(event)
		audio.consume_confirmed_native_event(event)
		world.pond_ducks.accept_native_event(event)
		world.living_world.accept_native_event(event)
	# The recovered core alone completes the original 100-tick recovery.
	if int(core.snapshot().get("hole_phase",-1)) == 3:
		core.acknowledge_hazard()

func update_view(s: Dictionary, hold_golfer := false) -> void:
	world.accept_snapshot(s,hold_golfer)
	arrows.visible = bool(s.get("can_shoot",false)) and int(s.get("club_index",0)) == 12
	arrows.ball_focus = world.ball_world
	arrows.queue_redraw()
	fx.accept_native_ball_snapshot(s)
	var screen := get_viewport().get_visible_rect().size
	# Keep a close world crop on high-density phones and unfolded windows.
	# Both camera axes share the same scale; native coordinates never change.
	camera.zoom = Vector2.ONE*maxf(1.0,minf(screen.x,screen.y)/240.0)
	# Reserve actual HUD space. The world is cropped, never squeezed to fit.
	var half_view := Vector2(screen.x,maxf(120,screen.y-128*ui_scale)) / camera.zoom / 2.0
	if not hold_golfer: camera.offset.y = 56.0*ui_scale/camera.zoom.y
	var at: Vector2 = world.golfer_world if int(s.get("hole_phase",0)) in [2,3] else world.ball_world
	if not hold_golfer:
		camera.position.x = 272 if half_view.x >= 272 else clampf(at.x,half_view.x,544-half_view.x)
		camera.position.y = 456 if half_view.y >= 456 else clampf(at.y-60,half_view.y,912-half_view.y)
	var rendered_half := screen/camera.zoom/2.0
	world.view_rect = Rect2(camera.position+camera.offset-rendered_half,rendered_half*2)
	status.text = "H1 · Par 4 · Stroke %d\n%d to cup" % [s.get("strokes",0),s.get("distance_to_hole",0)]
	minimap.reduced_motion = reduced_motion
	minimap.queue_redraw()
	camera.force_update_scroll()
	var presentation: Dictionary = s.duplicate()
	if shot_transition != null and shot_transition.busy: presentation.can_shoot = false
	club_card.accept_snapshot(presentation)
	aim_touch.refresh(presentation)
	meter.reduced_motion = reduced_motion
	meter.accept_snapshot(presentation)

func pause_game() -> void:
	shot_transition.finish_immediately()
	if not playing: return
	save_session_checkpoint()
	paused = true
	accumulator = 0
	world.set_process(false)
	fx.set_effects_running(false)
	hud.hide()
	show_pause()

func show_pause() -> void:
	audio.set_context("menu")
	menu_screen = "pause"
	clear_menu(0.70)
	panel.add_child(text_line("PAUSED"))
	add_music_toggle()
	panel.add_child(button("Resume",resume_game))
	panel.add_child(button("Player",show_player))
	panel.add_child(button("Scorecard",show_scorecard))
	panel.add_child(button("How to play",show_help))
	panel.add_child(button("Settings",show_settings))
	panel.add_child(button("Restart hole",func(): confirm_leave(true)))
	panel.add_child(button("Main menu",func(): confirm_leave(false)))

func resume_game() -> void:
	if not playing or core == null: return
	audio.set_context("course")
	menu_screen = "game"
	paused = false
	world.set_process(true)
	fx.set_effects_running(true)
	menu.hide()
	hud.show()

func show_score(_s: Dictionary) -> void:
	audio.set_context("results")
	paused = true
	playing = false
	if not core.commit_scored_hole():
		push_error(core.last_error())
		return
	world.set_process(false)
	fx.set_effects_running(false)
	hud.hide()
	last_result = core.snapshot().duplicate(true)
	result_saved = save_result()
	show_results()

func save_result() -> bool:
	# A temporary storage failure at tee-off must remain recoverable at results.
	if attempt_id == 0: attempt_id = profile.begin_attempt()
	var saved: bool = profile.record_committed_result(attempt_id,last_result)
	if saved: session_store.clear()
	return saved

func show_results() -> void:
	menu_screen = "results"
	clear_menu()
	panel.add_child(image_control("trophy",Vector2(150,150)))
	panel.add_child(text_line("WILLOW CREEK\n%d strokes · Par %d" % [last_result.get("total_strokes",0),last_result.get("cumulative_par",4)]))
	panel.add_child(text_line(relative_score(-int(last_result.get("relative_to_par",0)))))
	if not result_saved:
		panel.add_child(text_line("Score complete. Statistics could not be saved."))
		panel.add_child(button("Retry save",func():
			result_saved = save_result()
			show_results()))
	panel.add_child(button("Scorecard",show_scorecard))
	panel.add_child(button("Statistics",show_statistics))
	panel.add_child(button("Play again",start_hole))
	panel.add_child(button("Main menu",func(): playing=false; hud.hide(); show_title()))

func open_auxiliary(screen: String) -> void:
	if playing and not paused: pause_game()
	if menu_screen in ["title","pause","results"]: return_screen = menu_screen
	menu_screen = screen

func back_menu() -> void:
	match return_screen:
		"pause": show_pause()
		"results": show_results()
		_: show_title()

func paragraph(content: String) -> Label:
	var label := text_line(content)
	label.custom_minimum_size.x = 300
	label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	return label

func relative_score(relative: int) -> String:
	return "Even par" if relative == 0 else ("+%d" % relative if relative > 0 else str(relative))

func show_help() -> void:
	open_auxiliary("help")
	clear_menu()
	panel.add_child(text_line("HOW TO PLAY"))
	panel.add_child(paragraph("Choose a club with − Club and + Club. Use the arrows to aim."))
	panel.add_child(paragraph("Tap the bottom bar to start, tap to select power, then tap to set accuracy."))
	panel.add_child(paragraph("Watch the ball land before taking the next shot. Water and out-of-bounds penalties recover automatically."))
	panel.add_child(paragraph("Pause freezes the shot. Player and settings edits keep your ball and score unchanged. Escape opens pause; settings include reduced motion and flashes."))
	panel.add_child(button("Back",back_menu))

func show_scorecard() -> void:
	open_auxiliary("scorecard")
	clear_menu()
	panel.add_child(text_line("SCORECARD"))
	panel.add_child(text_line("Willow Creek · Hole 1 · Par 4"))
	var s: Dictionary = core.snapshot() if core != null else {}
	if s.get("has_hole",false):
		panel.add_child(text_line("Strokes: %d · In play" % s.get("strokes",0)))
	elif not last_result.is_empty():
		panel.add_child(text_line("Strokes: %d · %s" % [last_result.total_strokes,relative_score(-int(last_result.relative_to_par))]))
	else:
		panel.add_child(text_line("No round started"))
	panel.add_child(button("Back",back_menu))

func show_statistics() -> void:
	open_auxiliary("statistics")
	clear_menu()
	var stats: Dictionary = profile.snapshot().statistics
	panel.add_child(text_line("PLAYER STATISTICS"))
	panel.add_child(paragraph(str(profile.snapshot().display_name)))
	panel.add_child(text_line("Completed rounds: %d" % stats.played))
	if stats.played > 0:
		panel.add_child(text_line("Best: %d strokes\nAverage: %.1f strokes" % [stats.best,float(stats.strokes)/float(stats.played)]))
		if stats.best_calm>0: panel.add_child(text_line("Best calm round: %d" % stats.best_calm))
		if stats.best_wind>0: panel.add_child(text_line("Best wind round: %d" % stats.best_wind))
		panel.add_child(text_line("Last round: %d · %s" % [stats.last.strokes,relative_score(int(stats.last.relative))]))
	else: panel.add_child(text_line("Complete a round to record your first score."))
	panel.add_child(button("Back",back_menu))

func confirm_leave(restart: bool) -> void:
	open_auxiliary("confirmation")
	clear_menu()
	panel.add_child(text_line("Restart this hole?" if restart else "Leave this round?"))
	panel.add_child(paragraph("This unfinished round will not count in your statistics."))
	panel.add_child(button("Restart" if restart else "Leave",func():
		if restart: start_hole()
		else:
			playing = false
			pending_press = false
			fx.on_confirmed_restart_or_replay_seek()
			show_title()))
	panel.add_child(button("Cancel",back_menu))

func _notification(what: int) -> void:
	if what == NOTIFICATION_WM_CLOSE_REQUEST: save_session_checkpoint()
	if what in [NOTIFICATION_APPLICATION_FOCUS_OUT,NOTIFICATION_APPLICATION_PAUSED] and playing and not paused:
		pause_game()
	if audio != null:
		if what == NOTIFICATION_APPLICATION_PAUSED or what == NOTIFICATION_APPLICATION_FOCUS_OUT:
			audio.set_context("background")
		elif what == NOTIFICATION_APPLICATION_RESUMED or what == NOTIFICATION_APPLICATION_FOCUS_IN:
			audio.set_context("results" if menu_screen == "results" else ("course" if playing and not paused else "menu"))

func _unhandled_key_input(event: InputEvent) -> void:
	if event is InputEventKey and event.pressed and not event.echo and menu_screen == "intro" and event.keycode in [KEY_ESCAPE,KEY_ENTER,KEY_SPACE]:
		show_title()
		return
	if event is InputEventKey and event.pressed and not event.echo and playing:
		if event.keycode == KEY_ESCAPE:
			if not player_draft.is_empty(): cancel_player_edit()
			else: pause_game()

func add_music_toggle() -> void:
	var toggle := CheckButton.new()
	toggle.text = "Music"
	toggle.button_pressed = float(profile.snapshot().settings.Music) > 0.0
	toggle.toggled.connect(func(enabled: bool):
		if profile.set_setting("Music",0.8 if enabled else 0.0): apply_saved_settings())
	panel.add_child(toggle)

func add_outfit_controls() -> void:
	var kit: Array = Array(PixteeOutfit.indices(player_draft.outfit_id))
	for part in range(3):
		var row := HBoxContainer.new()
		panel.add_child(row)
		row.add_child(text_line(["Shirt","Trousers","Cap"][part]))
		var choice := OptionButton.new()
		choice.custom_minimum_size = Vector2(210,44)
		for title in [PixteeOutfit.SHIRTS,PixteeOutfit.TROUSERS,PixteeOutfit.CAPS][part]: choice.add_item(title)
		choice.select(kit[part])
		row.add_child(choice)
		choice.item_selected.connect(func(index: int):
			kit[part] = index
			player_draft.outfit_id = ("female:" if PixteeOutfit.female(player_draft.outfit_id) else "")+"kit:%d:%d:%d" % [kit[0],kit[1],kit[2]]
			refresh_player_preview())

func create_course_core(source_mode: int, use_wind := false) -> PixteeRoundBridge:
	var native = PixteeRoundBridge.new()
	var order := PackedInt32Array()
	order.resize(18)
	order.fill(1)
	var pars := PackedInt32Array()
	pars.resize(100)
	pars[1] = 4
	if not native.begin_round(order,pars,0x1234,0xabcd):
		push_error(native.last_error())
		return null
	if not native.register_course_resource(1,FileAccess.get_file_as_bytes(NATIVE_COURSE+"MAPM.bin"),FileAccess.get_file_as_bytes(NATIVE_COURSE+"SPT.bin"),FileAccess.get_file_as_bytes(NATIVE_COURSE+"MAPI-descriptors.bin"),FileAccess.get_file_as_bytes(NATIVE_COURSE+"MAPI-selectors.bin"),FileAccess.get_file_as_bytes(NATIVE_COURSE+"MAPS.bin")):
		push_error(native.last_error())
		return null
	if not native.load_next_hole():
		push_error(native.last_error())
		return null
	native.set_aim_raw(2016 if source_mode == 0 else 2048)
	native.set_verified_flag_object(true) # Exact approved manifest flag at native SPT cup.
	var tree_data: Dictionary = JSON.parse_string(FileAccess.get_file_as_string(COURSE+"authored-tree-volumes.json"))
	var volumes := PackedInt32Array()
	for tree in tree_data.trees:
		volumes.append(int(tree.id))
		for field in ["x","y","trunk_radius","trunk_height","canopy_radius","canopy_bottom","canopy_top"]:
			volumes.append(int(round(float(tree[field])*65536.0)))
	for i in world.boards.size():
		var board: Dictionary = world.boards[i]
		volumes.append(100+i)
		var radius := maxf(1.0,float(board.w)*0.45)
		var height := maxf(1.0,float(board.h))
		for value in [board.x,board.y,radius,height,radius,0.0,height]:
			volumes.append(roundi(float(value)*65536.0))
	if not native.set_authored_tree_objects(volumes):
		push_error(native.last_error())
		return null
	if not native.set_original_player_mode_5a(source_mode): return null
	if not native.set_authored_birds_enabled(true): return null
	if not native.set_soft_ground_enabled(true): return null
	if use_wind:
		var settings: Dictionary = profile.snapshot().settings
		if not native.set_authored_wind(int(settings.wind_toward_raw),int(settings.wind_speed_mph_x10),int(settings.wind_speed_mph_x10)>0): return null
	return native

func save_session_checkpoint() -> void:
	if not playing or core == null or attempt_id<1: return
	var native := core.save_native_session()
	if not session_store.save(native,course_hash,attempt_id,pending_press): recovery_error = "Game could not be saved. Check free storage."

func resume_saved_game() -> void:
	var record := session_store.load_current(course_hash)
	if record.is_empty() or int(record.attempt)<=int(profile.snapshot().statistics.last_recorded): return
	start_hole(true)
	if core == null or not core.restore_native_session(record.native):
		playing = false
		hud.hide()
		show_title()
		panel.add_child(text_line("Saved game could not be verified."))
		return
	attempt_id = int(record.attempt)
	pending_press = bool(record.pending_press)
	fx.on_confirmed_restart_or_replay_seek()
	world.pond_ducks.reset_confirmed_event_cursor()
	world.living_world.reset_confirmed_event_cursor()
	update_view(core.snapshot())
	pause_game()

func refresh_player_preview() -> void:
	var female: bool = PixteeOutfit.female(player_draft.outfit_id)
	var texture: Texture2D = load("res://characters/female-cute-v2.png") if female else load("res://characters/male-cute-v2.png")
	var atlas := AtlasTexture.new()
	atlas.atlas = texture
	var grid := Vector2(8,8)
	var metadata: Dictionary = world.character_atlas.female_cute[4] if female else world.character_atlas.male_cute[4]
	var region: Array = metadata.rect
	atlas.region = Rect2(region[0],region[1],region[2],region[3])
	player_preview.texture = atlas
	player_preview.material = PixteeOutfit.material(player_draft.outfit_id,player_draft.appearance)
	var face: Array = world.face_regions["female" if female else "male"][4]
	player_preview.material.set_shader_parameter("face_region",Vector4(face[0],face[1],face[2],face[3]))
	player_preview.material.set_shader_parameter("use_atlas_rect",true)
	player_preview.material.set_shader_parameter("atlas_origin",atlas.region.position/texture.get_size())
	player_preview.material.set_shader_parameter("atlas_extent",atlas.region.size/texture.get_size())

func add_appearance_controls() -> void:
	for part in range(5):
		var key: String = ["skin","hair","face","beard","hairstyle"][part]
		var row := HBoxContainer.new()
		panel.add_child(row)
		row.add_child(text_line(["Skin","Hair colour","Face","Beard","Hairstyle"][part]))
		var choice := OptionButton.new()
		choice.custom_minimum_size = Vector2(210,44)
		for title in [PixteeOutfit.SKINS,PixteeOutfit.HAIR,PixteeOutfit.FACES,PixteeOutfit.BEARDS,PixteeOutfit.HAIRSTYLES][part]: choice.add_item(title)
		choice.select(int(player_draft.appearance[key]))
		row.add_child(choice)
		choice.item_selected.connect(func(index: int):
			player_draft.appearance[key] = index
			refresh_player_preview())

func sync_attended_flag_for_native(native: PixteeRoundBridge) -> void:
	var state := native.snapshot()
	if not state.get("can_shoot",false): return
	var removed: bool = state.get("green_mode",false) and int(state.get("club_index",0)) == 12
	if state.get("flag_removed_for_putting",false) != removed:
		if not native.set_flag_removed_for_putting(removed): push_error(native.last_error())
