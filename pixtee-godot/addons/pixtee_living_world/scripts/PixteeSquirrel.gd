extends Node2D
class_name PixteeSquirrel

## High-quality six-frame squirrel sprite. NO TREE in any sprite or texture.
## Tree attachment points must come from an approved course-object manifest.
## Climbing, startled and hiding remain purely cosmetic.
const ART := "res://addons/pixtee_living_world/assets/premium/squirrel/poses/"
const ART_SCALE := 0.077
const FRAME_COUNT := 6

@export var climb_speed: float = 7.0
@export var scamper_speed: float = 32.0
@export var scare_distance: float = 26.0
@export var max_ball_height_for_scare: float = 3.0
@export var alert_seconds: float = 0.26
@export var hide_seconds: float = 2.5

var enabled: bool = false
var reduced_motion: bool = false
var reduced_flashes: bool = false
var visual_quality: int = 1
var ball_world: Vector2 = Vector2(-9999,-9999)
var ball_height: float = 9999.0
var tree_base: Vector2 = Vector2.ZERO
var tree_canopy: Vector2 = Vector2.ZERO
var state: String = "idle"
var _elapsed: float = 0.0
var _state_time: float = 0.0
var _progress: float = 0.30
var _ascending: bool = true
var _frame: int = -1
var _sprite: Sprite2D
var _reaction: PixteeReactionBurst
var _frames: Array[Texture2D] = []

func _ready() -> void:
	texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	_sprite = Sprite2D.new()
	_sprite.name="SquirrelSeparateFromTree"
	_sprite.centered=false
	_sprite.position=Vector2(-210,-396)*ART_SCALE
	_sprite.scale=Vector2.ONE*ART_SCALE
	_sprite.texture_filter=CanvasItem.TEXTURE_FILTER_NEAREST
	add_child(_sprite)
	for i in range(FRAME_COUNT):
		_frames.append(load(ART+"squirrel_%02d.png" % i))
	_reaction=PixteeReactionBurst.new()
	_reaction.name="CartoonSurpriseAndDust"
	_reaction.symbol_above_anchor=Vector2(1.0,-34.0)
	_reaction.z_index=4
	add_child(_reaction)
	_set_frame(0)
	visible=enabled

func configure_tree(base_anchor: Vector2, canopy_anchor: Vector2) -> bool:
	# Tree art and physics remain the course's own; this only defines safe motion.
	if base_anchor.distance_to(canopy_anchor) < 12.0:
		return false
	tree_base=base_anchor
	tree_canopy=canopy_anchor
	enabled=true
	reset_creature()
	return true

func update_ball_ground_position(p: Vector2, height: float=0.0) -> void:
	ball_world=p
	ball_height=maxf(0.0,height)

func _process(delta: float) -> void:
	if not enabled or tree_base.distance_to(tree_canopy) < 12.0:
		visible=false
		return
	var dt: float = clampf(delta,0.0,0.1)
	_elapsed+=dt
	_state_time+=dt
	_reaction.reduced_motion=reduced_motion
	_reaction.reduced_flashes=reduced_flashes
	_reaction.visual_quality=visual_quality
	var near_ground: bool = ball_height <= max_ball_height_for_scare and ball_world.distance_to(tree_base) < scare_distance
	if state == "climbing" and near_ground:
		state="alert"
		_state_time=0.0
		_reaction.play_shock()
	if state == "hidden":
		if _state_time>=hide_seconds and not near_ground:
			visible=true
			state="climbing"
			_state_time=0.0
			_ascending=false
		return
	if state=="alert":
		_set_frame(4)
		if _state_time>=alert_seconds:
			state="scamper"
			_state_time=0.0
			_reaction.add_dust(Vector2(0,-1),3)
	else:
		var total: float = maxf(1.0,tree_base.distance_to(tree_canopy))
		var speed: float = scamper_speed if state=="scamper" else climb_speed
		_progress=clampf(_progress+(1.0 if _ascending else -1.0)*speed*dt/total,0.0,1.0)
		if state=="scamper" and _progress>=0.99:
			state="hidden"
			_state_time=0.0
			visible=false
		elif state=="climbing":
			if _progress>=0.96:
				_ascending=false
			elif _progress<=0.12:
				_ascending=true
		var frame_index: int = (1 + int(_elapsed*5.0)%2) if state=="climbing" else 5
		_set_frame(0 if reduced_motion or visual_quality==0 else frame_index)
	position=tree_base.lerp(tree_canopy,_progress)
	queue_redraw()

func _draw() -> void:
	if visible:
		# A tiny bark-contact shade keeps the squirrel tied to the existing tree.
		draw_rect(Rect2(Vector2(-4,0),Vector2(8,2)),Color(0.06,0.04,0.02,0.20))

func _set_frame(index: int) -> void:
	index=clampi(index,0,FRAME_COUNT-1)
	if index!=_frame and _sprite!=null:
		_frame=index
		_sprite.texture=_frames[index]

func reset_creature() -> void:
	_elapsed=0.0
	_state_time=0.0
	_progress=0.3
	_ascending=true
	_frame=-1
	state="climbing"
	visible=enabled
	position=tree_base.lerp(tree_canopy,_progress)
	if _sprite!=null:
		_set_frame(0)
	if _reaction!=null:
		_reaction.reset_reaction()
