extends Node

const TICK_SECONDS := 936.0/65536.0
const COURSE := "res://courses/lakewood-hole-1/"
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
var club: Label
var meter: ProgressBar
var shoot: Button
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

func _ready() -> void:
	profile.load_profile()
	audio = PixteeCommercialAudio.new()
	add_child(audio)
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
	hud.hide()
	make_hud()
	get_viewport().size_changed.connect(update_ui_layout)
	update_ui_layout()
	show_title()

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
		art.texture = load(UI+state+".png"))
	result.pressed.connect(action)
	return result

func clear_menu() -> void:
	for child in menu.get_children(): child.queue_free()
	menu.show()
	var shade := ColorRect.new()
	shade.color = Color(0.025,0.085,0.065,0.92)
	menu.add_child(shade)
	shade.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	var center := CenterContainer.new()
	menu.add_child(center)
	center.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	panel = VBoxContainer.new()
	panel.add_theme_constant_override("separation",12)
	center.add_child(panel)

func text_line(text: String) -> Label:
	var label := Label.new()
	label.text = text
	label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	label.add_theme_color_override("font_color",Color("fff2c2"))
	return label

func show_title() -> void:
	hud.hide()
	clear_menu()
	panel.add_child(image_control("logo",Vector2(288,180)))
	panel.add_child(text_line("LAKEWOOD · WILLOW CREEK\nHole 1 · Par 4"))
	panel.add_child(button("Play",start_hole))
	panel.add_child(button("Player",show_player))
	panel.add_child(button("Golfers",show_golfers))
	panel.add_child(button("Settings",show_settings))
	camera.position = Vector2(256,700)

func show_golfers() -> void:
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
	panel.add_child(button("Back",show_title))

func show_settings() -> void:
	clear_menu()
	panel.add_child(text_line("SETTINGS"))
	var motion := CheckButton.new()
	motion.text = "Reduced motion"
	motion.button_pressed = bool(profile.snapshot().settings.reduced_motion)
	motion.toggled.connect(func(value: bool):
		if profile.set_setting("reduced_motion",value): apply_saved_settings())
	panel.add_child(motion)
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
	panel.add_child(button("Back",show_pause if playing else show_title))

func apply_saved_settings() -> void:
	var settings: Dictionary = profile.snapshot().settings
	reduced_motion = bool(settings.reduced_motion)
	world.reduced_motion = reduced_motion
	fx.reduced_motion = reduced_motion
	for child in world.get_children():
		if child is Polygon2D: child.material.set_shader_parameter("quality_enabled",0.0 if reduced_motion else 1.0)
	for channel in ["Music","SFX","Ambience","UI"]:
		audio.set_channel_level(channel,float(settings[channel]))

func show_player() -> void:
	if playing and not paused: pause_game()
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
	show_pause() if playing else show_title()
	return true

func cancel_player_edit() -> void:
	player_draft.clear()
	show_pause() if playing else show_title()

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
	bottom.offset_top = -176
	club = text_line("Driver")
	bottom.add_child(club)
	var controls := HBoxContainer.new()
	controls.alignment = BoxContainer.ALIGNMENT_CENTER
	bottom.add_child(controls)
	controls.add_child(button("− Club",func(): change_club(-1),Vector2(82,40)))
	controls.add_child(button("◀",func(): change_aim(-32),Vector2(70,40)))
	controls.add_child(button("▶",func(): change_aim(32),Vector2(70,40)))
	controls.add_child(button("+ Club",func(): change_club(1),Vector2(82,40)))
	meter = ProgressBar.new()
	meter.max_value = 105
	meter.show_percentage = false
	meter.custom_minimum_size.y = 20
	bottom.add_child(meter)
	shoot = button("Swing",on_shoot)
	bottom.add_child(shoot)

func start_hole() -> void:
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
	if not core.register_course_resource(1,FileAccess.get_file_as_bytes(COURSE+"MAPM.bin"),FileAccess.get_file_as_bytes(COURSE+"SPT.bin"),FileAccess.get_file_as_bytes(COURSE+"MAPI-descriptors.bin"),FileAccess.get_file_as_bytes(COURSE+"MAPI-selectors.bin"),PackedByteArray()):
		push_error(core.last_error())
		return
	if not core.load_next_hole():
		push_error(core.last_error())
		return
	core.set_aim_raw(2048)
	core.set_verified_flag_object(true) # Exact approved manifest flag at native SPT cup.
	playing = true
	paused = false
	accumulator = 0
	pending_press = false
	world.set_process(true)
	fx.set_process(true)
	fx.on_confirmed_restart_or_replay_seek()
	audio.reset_event_cursor_on_confirmed_new_round()
	menu.hide()
	hud.show()
	update_view(core.snapshot())

func on_shoot() -> void:
	if not playing or paused or core == null: return
	var s: Dictionary = core.snapshot()
	if s.get("original_welly_active",false):
		pending_press = true
	elif s.get("can_shoot",false):
		core.start_original_welly()

func change_club(direction: int) -> void:
	if playing and not paused and core != null: core.cycle_club(direction)

func change_aim(amount: int) -> void:
	if playing and not paused and core != null: core.nudge_aim_raw(amount)

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
	if core.snapshot().get("original_welly_active",false):
		core.tick_original_welly(pending_press)
		pending_press = false
	core.step_tick()
	for event in core.drain_presentation_events():
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
	var half_view := Vector2(screen.x,maxf(120,screen.y-240*ui_scale)) / camera.zoom / 2.0
	camera.offset.y = 56.0*ui_scale/camera.zoom.y
	var at: Vector2 = world.ball_world
	camera.position.x = 272 if half_view.x >= 272 else clampf(at.x,half_view.x,544-half_view.x)
	camera.position.y = 456 if half_view.y >= 456 else clampf(at.y-60,half_view.y,912-half_view.y)
	var rendered_half := screen/camera.zoom/2.0
	world.view_rect = Rect2(camera.position+camera.offset-rendered_half,rendered_half*2)
	status.text = "H1 · Par 4 · Stroke %d\n%d to cup" % [s.get("strokes",0),s.get("distance_to_hole",0)]
	club.text = CLUBS[int(s.get("club_index",0))]
	meter.value = clampf(float(s.get("welly_counter",0)),0,105)
	shoot.disabled = not s.get("can_shoot",false) and not s.get("original_welly_active",false)
	shoot.text = "Swing" if not s.get("original_welly_active",false) else ("Power" if s.get("welly_phase",1) == 1 else "Accuracy")

func pause_game() -> void:
	paused = true
	accumulator = 0
	world.set_process(false)
	fx.set_process(false)
	hud.hide()
	show_pause()

func show_pause() -> void:
	clear_menu()
	panel.add_child(text_line("PAUSED"))
	panel.add_child(button("Resume",resume_game))
	panel.add_child(button("Player",show_player))
	panel.add_child(button("Settings",show_settings))
	panel.add_child(button("Restart hole",start_hole))
	panel.add_child(button("Main menu",func(): playing=false; hud.hide(); show_title()))

func resume_game() -> void:
	if not playing or core == null: return
	paused = false
	world.set_process(true)
	fx.set_process(true)
	menu.hide()
	hud.show()

func show_score(s: Dictionary) -> void:
	paused = true
	playing = false
	if not core.commit_scored_hole():
		push_error(core.last_error())
		return
	clear_menu()
	panel.add_child(image_control("trophy",Vector2(150,150)))
	panel.add_child(text_line("WILLOW CREEK\n%d strokes · Par 4" % s.strokes))
	panel.add_child(button("Play again",start_hole))
	panel.add_child(button("Main menu",func(): playing=false; hud.hide(); show_title()))

func _notification(what: int) -> void:
	if what == NOTIFICATION_APPLICATION_FOCUS_OUT and playing and not paused:
		pause_game()

func _unhandled_key_input(event: InputEvent) -> void:
	if event is InputEventKey and event.pressed and not event.echo and playing:
		if event.keycode == KEY_ESCAPE:
			if not player_draft.is_empty(): cancel_player_edit()
			else: pause_game()
		elif event.keycode == KEY_SPACE and not paused:
			on_shoot()
