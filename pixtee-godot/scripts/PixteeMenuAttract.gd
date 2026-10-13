extends Node
## Two isolated C++ rounds. Script supplies authored demo inputs, never ball physics.
var game: Node
var golfers: Array = []
var accumulator := 0.0
var focus_ticks := 0
var focus := 0
var camera_initialized := false
const SHOTS := [[0,105,1946],[1,95,1942],[12,105,2121]]

func _process(delta: float) -> void:
	if game.playing or game.menu_screen == "results": return
	if golfers.is_empty():
		for i in range(2): golfers.append({"core":game.create_course_core(4),"shot":0,"delay":i*500,"generation":0,"last_ready_shot":-1,"anchor":Vector2(256,850)})
	accumulator += minf(delta,0.1)
	while accumulator >= game.TICK_SECONDS:
		accumulator -= game.TICK_SECONDS
		focus_ticks += 1
		for i in golfers.size(): _tick(i)
		if focus_ticks > 1800:
			focus = (focus+1)%golfers.size()
			focus_ticks = 0
	var native: PixteeRoundBridge = golfers[focus].core
	if native == null: return
	var snapshot := native.snapshot()
	game.world.accept_snapshot(snapshot)
	game.world.golfer_world = golfers[focus].anchor+Vector2(-7,3)
	game.world.update_player_sprite()
	game.fx.accept_native_ball_snapshot(snapshot)
	game.world.set_outfit("kit:1:1:0" if focus == 0 else "kit:2:2:2")
	var screen: Vector2 = game.get_viewport().get_visible_rect().size
	game.camera.zoom = Vector2.ONE*maxf(1.0,minf(screen.x,screen.y)/240.0)
	var half: Vector2 = screen/game.camera.zoom/2
	var target: Vector2 = game.world.ball_world
	target.x = 272 if half.x>=272 else clampf(target.x,half.x,544-half.x)
	target.y = 456 if half.y>=456 else clampf(target.y,half.y,912-half.y)
	game.camera.offset = Vector2.ZERO
	if not camera_initialized:
		game.camera.position = target
		camera_initialized = true
	else: game.camera.position = game.camera.position.lerp(target,1.0-exp(-delta*0.8))
	game.world.view_rect = Rect2(game.camera.position-half,half*2)
	game.world.set_process(true)
	game.fx.set_effects_running(true)

func _tick(index: int) -> void:
	var golfer: Dictionary = golfers[index]
	var native: PixteeRoundBridge = golfer.core
	if native == null: return
	if golfer.delay > 0:
		golfer.delay -= 1
		native.step_tick()
		return
	var snapshot := native.snapshot()
	if int(snapshot.get("hole_phase",0)) == 6 or golfer.shot > 5:
		golfer.core = game.create_course_core(4)
		golfer.shot = 0
		golfer.delay = 180
		golfer.generation += 1
		golfer.last_ready_shot = -1
		golfer.anchor = Vector2(256,850)
		return
	if snapshot.get("can_shoot",false):
		var at := Vector2(float(snapshot.ball_x_raw),float(snapshot.ball_y_raw))/65536.0
		if snapshot.get("green_mode",false): at = at*0.5+Vector2(snapshot.green_origin_x,snapshot.green_origin_y)
		golfer.anchor = at
		if golfer.last_ready_shot != golfer.shot:
			golfer.last_ready_shot = golfer.shot
			golfer.delay = 60
			return
		var shot: Array = SHOTS[mini(int(golfer.shot),2)]
		native.set_club_index(shot[0])
		native.set_aim_raw(shot[2])
		native.meter_click_raw(0)
		native.meter_click_raw(shot[1])
		native.meter_click_raw(63)
		golfer.shot += 1
		native.step_tick()
		golfer.delay = 0
	else:
		native.step_tick()
		if int(native.snapshot().get("hole_phase",0)) == 3: native.acknowledge_hazard()
	for event in native.drain_presentation_events():
		event.event_id = "attract:%d:%d:" % [index,int(golfer.generation)]+str(event.event_id)
		game.fx.submit_confirmed_native_event(event)
