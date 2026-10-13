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
var menu_screen := "title"
var return_screen := "title"
var menu_scroll: ScrollContainer
var attempt_id := 0
var last_result: Dictionary = {}
var result_saved := false
var intro_tween: Tween

func _ready() -> void:
	profile.load_profile()
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
	menu.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	layer.add_child(menu)
	hud = Control.new()
	hud.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	layer.add_child(hud)
	hud.mouse_filter = Control.MOUSE_FILTER_IGNORE
	hud.hide()
	make_hud()
	get_viewport().size_changed.connect(update_ui_layout)
	update_ui_layout()
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

func clear_menu() -> void:
	for child in menu.get_children():
		menu.remove_child(child)
		child.queue_free()
	menu.show()
	var shade := ColorRect.new()
	shade.color = Color(0.025,0.085,0.065,0.92)
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
	clear_menu()
	panel.add_child(image_control("logo",Vector2(288,180)))
	panel.add_child(text_line("LAKEWOOD · WILLOW CREEK\nHole 1 · Par 4"))
	add_music_toggle()
	panel.add_child(button("Play",start_hole))
	panel.add_child(button("Player",show_player))
	panel.add_child(button("Golfers",show_golfers))
	panel.add_child(button("Statistics",show_statistics))
	panel.add_child(button("Settings",show_settings))
	panel.add_child(button("How to play",show_help))
	camera.position = Vector2(256,700)

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
	panel.add_child(button("Skip",show_title))
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
	for child in world.get_children():
		if child is Polygon2D: child.material.set_shader_parameter("quality_enabled",0.0 if reduced_motion or int(settings.graphics_quality) == 0 else 1.0)
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
	panel.add_child(text_line("Classic Lakewood kit"))
	player_name = LineEdit.new()
	player_name.text = str(player_draft.display_name)
	player_name.max_length = 32
	player_name.custom_minimum_size = Vector2(288,44)
	panel.add_child(player_name)
	player_error = text_line("")
	panel.add_child(player_error)
	panel.add_child(button("Apply",func(): apply_player_edit()))
	panel.add_child(button("Cancel",cancel_player_edit))

func apply_player_edit() -> bool:
	player_draft.display_name = player_name.text
	if not profile.apply_edit(player_draft):
		player_error.text = "Enter a name; check save storage."
		return false
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
	club_card.offset_right = 172
	club_card.offset_top = 54
	club_card.offset_bottom = 180
	club_card.club_selected.connect(select_club)
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

func start_hole() -> void:
	stop_intro()
	menu_screen = "game"
	core = PixteeRoundBridge.new()
	var order := PackedInt32Array()
	order.resize(18)
	order.fill(1)
	var pars := PackedInt32Array()
	pars.resize(100)
	pars[1] = 4
	if not core.begin_round(order,pars,0x1234,0xabcd):
		push_error(core.last_error())
		return
	if not core.register_course_resource(1,FileAccess.get_file_as_bytes(NATIVE_COURSE+"MAPM.bin"),FileAccess.get_file_as_bytes(NATIVE_COURSE+"SPT.bin"),FileAccess.get_file_as_bytes(NATIVE_COURSE+"MAPI-descriptors.bin"),FileAccess.get_file_as_bytes(NATIVE_COURSE+"MAPI-selectors.bin"),FileAccess.get_file_as_bytes(NATIVE_COURSE+"MAPS.bin")):
		push_error(core.last_error())
		return
	if not core.load_next_hole():
		push_error(core.last_error())
		return
	core.set_aim_raw(2048)
	core.set_verified_flag_object(true) # Exact approved manifest flag at native SPT cup.
	var tree_data: Dictionary = JSON.parse_string(FileAccess.get_file_as_string(COURSE+"authored-tree-volumes.json"))
	var volumes := PackedInt32Array()
	for tree in tree_data.trees:
		volumes.append(int(tree.id))
		for field in ["x","y","trunk_radius","trunk_height","canopy_radius","canopy_bottom","canopy_top"]:
			volumes.append(int(round(float(tree[field])*65536.0)))
	if not core.set_authored_tree_objects(volumes):
		push_error(core.last_error())
		return
	var arrow_cells: Array[Dictionary] = []
	for cell in core.verified_native_green_arrow_cells(16): arrow_cells.append(cell)
	if not arrows.load_native_maps_arrow_cells(arrow_cells) or arrows.arrow_count()==0:
		push_error("Native Lakewood slope field unavailable")
		return
	attempt_id = profile.begin_attempt()
	last_result.clear()
	result_saved = false
	player_draft.clear()
	playing = true
	paused = false
	accumulator = 0
	pending_press = false
	world.set_process(true)
	fx.set_process(true)
	fx.on_confirmed_restart_or_replay_seek()
	audio.reset_event_cursor_on_confirmed_new_round()
	audio.set_context("course")
	menu.hide()
	hud.show()
	update_view(core.snapshot())

func on_shoot() -> void:
	if not playing or paused or core == null: return
	var s: Dictionary = core.snapshot()
	if s.get("original_welly_active",false):
		if s.get("welly_power_captured",false) and int(s.get("welly_phase",0)) == 1: return
		pending_press = true
	elif s.get("can_shoot",false):
		core.start_original_welly()

func select_club(index: int) -> void:
	if playing and not paused and core != null and core.snapshot().get("can_shoot",false):
		core.set_club_index(index)
		update_view(core.snapshot())

func change_aim(amount: int) -> void:
	if playing and not paused and core != null and core.snapshot().get("can_shoot",false):
		core.nudge_aim_raw(amount)
		update_view(core.snapshot())

func _process(delta: float) -> void:
	if not playing or paused or core == null: return
	accumulator += minf(delta,0.25)
	var ticks := 0
	while accumulator >= TICK_SECONDS and ticks < 16:
		advance_native_tick()
		accumulator -= TICK_SECONDS
		ticks += 1
	var s: Dictionary = core.snapshot()
	if int(s.get("hole_phase",-1)) == 6:
		show_score(s)
	update_view(s)

func advance_native_tick() -> void:
	if paused or core == null: return
	var native_before: Dictionary = core.snapshot()
	if native_before.get("original_welly_active",false):
		core.tick_original_welly(pending_press)
		pending_press = false
	core.step_tick()
	for event in core.drain_presentation_events():
		if event.get("event_type","") == "club_contact":
			# Launch lie comes directly from the preceding native snapshot.
			event["launch_surface_code"] = native_before.get("surface_code",-1)
			event["launch_club_index"] = native_before.get("club_index",-1)
		fx.submit_confirmed_native_event(event)
		audio.consume_confirmed_native_event(event)
	# The recovered core alone completes the original 100-tick recovery.
	if int(core.snapshot().get("hole_phase",-1)) == 3:
		core.acknowledge_hazard()

func update_view(s: Dictionary) -> void:
	world.accept_snapshot(s)
	fx.accept_native_ball_snapshot(s)
	var screen := get_viewport().get_visible_rect().size
	# Keep a close world crop on high-density phones and unfolded windows.
	# Both camera axes share the same scale; native coordinates never change.
	camera.zoom = Vector2.ONE*maxf(1.0,minf(screen.x,screen.y)/240.0)
	# Reserve actual HUD space. The world is cropped, never squeezed to fit.
	var half_view := Vector2(screen.x,maxf(120,screen.y-128*ui_scale)) / camera.zoom / 2.0
	camera.offset.y = 56.0*ui_scale/camera.zoom.y
	var at: Vector2 = world.ball_world
	camera.position.x = 272 if half_view.x >= 272 else clampf(at.x,half_view.x,544-half_view.x)
	camera.position.y = 456 if half_view.y >= 456 else clampf(at.y-60,half_view.y,912-half_view.y)
	var rendered_half := screen/camera.zoom/2.0
	world.view_rect = Rect2(camera.position+camera.offset-rendered_half,rendered_half*2)
	status.text = "H1 · Par 4 · Stroke %d\n%d to cup" % [s.get("strokes",0),s.get("distance_to_hole",0)]
	club_card.accept_snapshot(s)
	minimap.queue_redraw()
	camera.force_update_scroll()
	aim_touch.refresh(s)
	meter.reduced_motion = reduced_motion
	meter.accept_snapshot(s)

func pause_game() -> void:
	paused = true
	accumulator = 0
	world.set_process(false)
	fx.set_process(false)
	hud.hide()
	show_pause()

func show_pause() -> void:
	audio.set_context("menu")
	menu_screen = "pause"
	clear_menu()
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
	fx.set_process(true)
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
	fx.set_process(false)
	hud.hide()
	last_result = core.snapshot().duplicate(true)
	result_saved = save_result()
	show_results()

func save_result() -> bool:
	# A temporary storage failure at tee-off must remain recoverable at results.
	if attempt_id == 0: attempt_id = profile.begin_attempt()
	return profile.record_committed_result(attempt_id,last_result)

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
	if what == NOTIFICATION_APPLICATION_FOCUS_OUT and playing and not paused:
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
