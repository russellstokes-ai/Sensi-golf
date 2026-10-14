extends Node2D
class_name PixteeRabbit

## Approved-style six-frame rabbit, independent of tree/terrain. All movement
## is visual only, on the designer-authored safe course-edge path.
## Gameplay never reads or changes this animal's position/collider.

const ART := "res://addons/pixtee_living_world/assets/premium/rabbit/poses/"
const FRAME_COUNT := 6
const FRAME_CANVAS := Vector2(420.0,420.0)
const ART_SCALE := 0.077

@export var movement_speed: float = 6.0
@export var flee_speed: float = 34.0
@export var scare_distance: float = 23.0
@export var alert_seconds: float = 0.28
@export var hide_seconds: float = 2.2
@export var max_ball_height_for_scare: float = 3.0
@export var min_world_coordinate: Vector2 = Vector2.ZERO
@export var max_world_coordinate: Vector2 = Vector2(544,912)

var enabled: bool = false
var reduced_motion: bool = false
var reduced_flashes: bool = false
var visual_quality: int = 1
var path_points: PackedVector2Array = PackedVector2Array()
var retreat_point: Vector2 = Vector2.ZERO
var ball_world: Vector2 = Vector2(-9999,-9999)
var ball_height: float = 9999.0
var state: String = "idle"
var _elapsed: float = 0.0
var _state_time: float = 0.0
var _path_index: int = 1
var _frame: int = -1
var _frames: Array[Texture2D] = []
var _sprite: Sprite2D
var _reaction: PixteeReactionBurst
var _ground_shadow: Node2D

func _ready() -> void:
	texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	_sprite = Sprite2D.new()
	_sprite.name = "ApprovedRabbitArtwork"
	_sprite.centered = false
	_sprite.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	_sprite.position = Vector2(-210.0,-396.0) * ART_SCALE
	_sprite.scale = Vector2.ONE * ART_SCALE
	add_child(_sprite)
	for i in range(FRAME_COUNT):
		_frames.append(load(ART + "rabbit_%02d.png" % i))
	_reaction = PixteeReactionBurst.new()
	_reaction.name = "CartoonSurpriseAndDust"
	_reaction.symbol_above_anchor = Vector2(0.0,-35.0)
	_reaction.z_index = 4
	add_child(_reaction)
	_set_frame(0)
	visible = enabled

func configure_path(authored: PackedVector2Array, safe_escape: Vector2) -> bool:
	if authored.size() < 2 or not _within_course(safe_escape):
		return false
	for p in authored:
		if not _within_course(p):
			return false
	path_points = authored
	retreat_point = safe_escape
	enabled = true
	reset_creature()
	return true

func _within_course(p: Vector2) -> bool:
	return p.x >= min_world_coordinate.x and p.y >= min_world_coordinate.y and p.x <= max_world_coordinate.x and p.y <= max_world_coordinate.y

func update_ball_ground_position(p: Vector2, height: float = 0.0) -> void:
	ball_world = p
	ball_height = maxf(0.0,height)

func _process(delta: float) -> void:
	if not enabled or path_points.size() < 2:
		visible = false
		return
	var dt: float = clampf(delta,0.0,0.1)
	_elapsed += dt
	_state_time += dt
	_reaction.reduced_motion = reduced_motion
	_reaction.reduced_flashes = reduced_flashes
	_reaction.visual_quality = visual_quality
	var nearby: bool = ball_height <= max_ball_height_for_scare and ball_world.distance_to(position) <= scare_distance
	if state != "alert" and state != "flee" and state != "hidden" and nearby:
		state = "alert"
		_state_time = 0.0
		_reaction.play_shock()
	if state == "hidden":
		if _state_time >= hide_seconds and not nearby:
			reset_creature()
		return
	match state:
		"alert":
			_set_frame(4)
			if _state_time >= alert_seconds:
				state = "flee"
				_state_time = 0.0
				_reaction.add_dust(Vector2(0,-1),5,1.0 if retreat_point.x>=position.x else -1.0)
		"flee":
			position = position.move_toward(retreat_point,flee_speed*dt)
			_set_frame(5)
			if not reduced_motion:
				_sprite.position.y = -396.0*ART_SCALE - absf(sin(_elapsed*18.0))*0.75
			if position.distance_to(retreat_point) <= 0.8:
				state="hidden"
				_state_time=0.0
				visible=false
		"moving":
			var goal: Vector2 = path_points[_path_index]
			_sprite.flip_h = goal.x < position.x
			position = position.move_toward(goal, movement_speed*dt)
			var cycle: float = fposmod(_elapsed,1.75)
			if reduced_motion or visual_quality == 0:
				_set_frame(0)
			elif cycle < 0.52:
				_set_frame(0)
			elif cycle < 0.82:
				_set_frame(1)
			elif cycle < 1.11:
				_set_frame(2)
			else:
				_set_frame(3)
			_sprite.position.y = -396.0*ART_SCALE - (3.0*sin(PI*clampf((cycle-0.52)/0.60,0,1)) if cycle>0.52 and cycle<1.12 else 0.0)
			if position.distance_to(goal) <= 0.8:
				_path_index = (_path_index + 1) % path_points.size()
		_:
			_set_frame(0)
	queue_redraw()

func _draw() -> void:
	if visible:
		# Two stable contact-shadow layers add dimensionality without shader blur.
		draw_rect(Rect2(Vector2(-8,0),Vector2(17,2)),Color(0.03,0.08,0.05,0.28))
		draw_rect(Rect2(Vector2(-5,-1),Vector2(11,1)),Color(0.06,0.12,0.06,0.18))

func _set_frame(index: int) -> void:
	index = clampi(index,0,FRAME_COUNT-1)
	if index != _frame and _sprite != null:
		_frame = index
		_sprite.texture = _frames[_frame]

func reset_creature() -> void:
	_elapsed=0.0
	_state_time=0.0
	_frame=-1
	state="moving"
	if path_points.size()>1:
		position=path_points[0]
		_path_index=1
	visible=enabled
	if _sprite != null:
		_sprite.position=Vector2(-210,-396)*ART_SCALE
		_set_frame(0)
	if _reaction != null:
		_reaction.reset_reaction()
