extends Node2D
## Cosmetic wildlife; reacts only to confirmed native impact events.
var world: Node2D
var ducks: Array[Dictionary] = []
var pond := PackedVector2Array()
var elapsed := 0.0
var seen: Dictionary = {}
var texture: Texture2D
var rects: Array
func _ready() -> void:
	texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	var course: Dictionary = JSON.parse_string(FileAccess.get_file_as_string("res://courses/lakewood-hole-1/course-layout.json"))
	for point in course.surfaces.water: pond.append(Vector2(point[0],point[1]))
	texture = load("res://characters/ducks-v1.png")
	rects = JSON.parse_string(FileAccess.get_file_as_string("res://characters/ducks-v1.json")).rects
	for i in range(2):
		var origin := Vector2(458,477) if i == 0 else Vector2(440,525)
		var body := Sprite2D.new()
		var head := Sprite2D.new()
		for sprite in [body,head]:
			sprite.centered = false
			sprite.texture = texture
			sprite.region_enabled = true
			sprite.scale = Vector2.ONE*(11.0/329.0)
			sprite.material = ShaderMaterial.new()
			sprite.material.shader = load("res://shaders/pixtee_duck_layers.gdshader")
			add_child(sprite)
		body.material.set_shader_parameter("part",1)
		head.material.set_shader_parameter("part",2)
		ducks.append({"origin":origin,"at":origin,"phase":float(i)*PI,"speed":0.09,"scared":0.0,"body":body,"head":head,"female":i == 1})
	_update_sprites()
func accept_native_event(event: Dictionary) -> bool:
	if event.get("confirmed_native",false) != true or event.get("event_type","") != "water_contact": return false
	var id: String = str(event.get("event_id",""))
	if id.is_empty() or seen.has(id) or not event.has("world_x") or not event.has("world_y"): return false
	var at := Vector2(float(event.world_x),float(event.world_y))
	if not is_finite(at.x) or not is_finite(at.y): return false
	seen[id] = true
	if seen.size() > 128: seen.erase(seen.keys()[0])
	for duck in ducks:
		if duck.at.distance_to(at) <= 55.0: duck.scared = 5.0
	return true
func _process(delta: float) -> void:
	if world == null or not world.is_processing() or world.reduced_motion: return
	var step := clampf(delta,0.0,0.1)
	elapsed += step
	for duck in ducks:
		duck.scared = maxf(0.0,float(duck.scared)-step)
		duck.speed = move_toward(float(duck.speed),0.48 if duck.scared > 0.0 else 0.09,step*0.16)
		duck.phase += float(duck.speed)*step
		var at: Vector2 = duck.origin+Vector2(cos(duck.phase)*12.0,sin(duck.phase)*10.0)
		if Geometry2D.is_point_in_polygon(at,pond): duck.at = at
	_update_sprites()
	queue_redraw()
func _update_sprites() -> void:
	for duck in ducks:
		var right: bool = -sin(float(duck.phase)) >= 0.0
		var index := (4 if duck.female else 0)+(1 if right else 3)
		var source: Array = rects[index]
		var region := Rect2(source[0],source[1],source[2],source[3])
		var at: Vector2 = duck.at-Vector2(region.size.x/2.0,region.size.y*0.55)*(11.0/329.0)
		for sprite in [duck.body,duck.head]:
			sprite.region_rect = region
			sprite.position = at
			sprite.visible = world.visual_quality > 0
			sprite.material.set_shader_parameter("facing_right",right)
			sprite.material.set_shader_parameter("atlas_region",Vector4(region.position.x/texture.get_width(),region.position.y/texture.get_height(),region.size.x/texture.get_width(),region.size.y/texture.get_height()))
		if not world.reduced_motion and world.is_processing(): duck.head.position.y += sin(elapsed*2.0+float(duck.phase))*0.12
func _draw() -> void:
	if world == null or world.visual_quality == 0: return
	for duck in ducks:
		var at: Vector2 = duck.at
		draw_arc(at,4.0,0.0,PI,16,Color(0.57,0.83,0.92,0.32),0.6,true)
		if not world.reduced_motion:
			var trail := Vector2(sin(duck.phase),-cos(duck.phase))*3.0
			draw_arc(at+trail,5.0,0.0,PI,16,Color(0.57,0.83,0.92,0.20),0.6,true)
