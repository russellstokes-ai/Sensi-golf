extends Node2D
var world: Node2D
var layers: Array[Sprite2D] = []
var metadata: Dictionary
var frame := 0
var tree: Dictionary = {}
func _ready() -> void:
	metadata = JSON.parse_string(FileAccess.get_file_as_string("res://characters/squirrel-climb-v1.json"))
	for part in range(1,4):
		var sprite := Sprite2D.new()
		sprite.texture = load("res://characters/squirrel-climb-v1.png")
		sprite.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
		sprite.centered = false
		sprite.region_enabled = true
		sprite.scale = Vector2.ONE*float(metadata.pixel_scale)
		sprite.material = ShaderMaterial.new()
		sprite.material.shader = load("res://shaders/pixtee_creature_parts.gdshader")
		sprite.material.set_shader_parameter("part",part)
		sprite.material.set_shader_parameter("squirrel",true)
		add_child(sprite)
		layers.append(sprite)
	show_pose(0)
	var nearest := INF
	for item in world.placements:
		if item.type != "tree": continue
		var distance := Vector2(item.x,item.y).distance_squared_to(Vector2(81,718))
		if distance < nearest:
			nearest = distance
			tree = item
	visible = false
func _process(_delta: float) -> void:
	if tree.is_empty(): return
	# Shared presentation time freezes the entire gait during pause/reduced motion.
	var phase := fposmod(world.presentation.time_seconds+42.0,87.0)
	visible = world.visual_quality > 0 and phase < 7.0
	if not visible: return
	var progress := clampf(phase/3.8,0.0,1.0)
	position = Vector2(float(tree.x)-2.6,float(tree.y)-float(tree.h)*0.48*progress)
	z_as_relative = false
	z_index = roundi(float(tree.y))+(1 if progress < 0.62 else -1)
	modulate.a = clampf(minf(phase,7.0-phase)/0.45,0.0,1.0)
	show_pose(int(phase*6.0) if progress < 1.0 else 4)
func show_pose(index: int) -> void:
	frame = posmod(index,6)
	var rect: Array = metadata.rects[frame]
	for sprite in layers:
		sprite.region_rect = Rect2(rect[0],rect[1],rect[2],rect[3])
		sprite.position = -Vector2(metadata.anchor[0],metadata.anchor[1])*float(metadata.pixel_scale)
		sprite.material.set_shader_parameter("atlas_region",Vector4(float(rect[0])/sprite.texture.get_width(),0,float(rect[2])/sprite.texture.get_width(),1))
