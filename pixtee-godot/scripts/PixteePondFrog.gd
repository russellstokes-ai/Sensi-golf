extends Node2D
## Uses the supplied transparent six-pose frog gait; no diagnostic-caption artwork.
var world: Node2D
var layers: Array[Sprite2D] = []
var frames: Array[Texture2D] = []
var origin := Vector2(434,374)
var elapsed := 0.0
func _ready() -> void:
	for i in range(6):
		frames.append(load("res://addons/pixtee_living_world/assets/premium/frog/hop/hop_%02d.png"%i))
	for part in range(1,4):
		var sprite := Sprite2D.new()
		sprite.texture = frames[0]
		sprite.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
		sprite.centered = false
		sprite.scale = Vector2.ONE*(15.0/90.0)
		sprite.position = -Vector2(39,75)*sprite.scale
		sprite.material = ShaderMaterial.new()
		sprite.material.shader = load("res://shaders/pixtee_creature_parts.gdshader")
		sprite.material.set_shader_parameter("frog",true)
		sprite.material.set_shader_parameter("part",part)
		add_child(sprite)
		layers.append(sprite)
	position = origin
func _process(_delta: float) -> void:
	if world == null: return
	visible = world.visual_quality > 0
	if not world.is_processing() or world.reduced_motion: return
	elapsed = world.presentation.time_seconds
	var phase := fposmod(elapsed+12.0,63.0)
	var outward := phase >= 24.0 and phase < 24.75
	var homeward := phase >= 40.0 and phase < 40.75
	var gait := phase-24.0 if outward else phase-40.0
	var fraction := clampf(gait/0.75,0.0,1.0)
	var offset := 4.0 if phase >= 24.75 and phase < 40.0 else 0.0
	if outward: offset = 4.0*fraction
	elif homeward: offset = 4.0*(1.0-fraction)
	position = origin+Vector2(offset,0)
	z_as_relative = false
	z_index = roundi(position.y)
	for sprite in layers:
		sprite.texture = frames[mini(5,int(fraction*6.0))] if outward or homeward else frames[0]
		sprite.flip_h = homeward or phase>=40.75
	# Throat tint is an isolated layer; the body and ground anchor never breathe-scale.
	var throat_light := 0.95+sin(elapsed*2.2)*0.05
	layers[2].modulate = Color(1,throat_light,throat_light,1)
