extends Node2D
## Full authored flap canvases split into complementary body/wing layers.
## Native clock, trajectory, altitude and contact identity remain authoritative.
var world: Node2D
var birds: Array[Dictionary] = []
func _ready() -> void:
	texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	for member in range(3):
		var body := Sprite2D.new()
		var wings := Sprite2D.new()
		for sprite in [body,wings]:
			sprite.scale = Vector2.ONE*0.18
			sprite.material = ShaderMaterial.new()
			sprite.material.shader = load("res://shaders/pixtee_bird_layers.gdshader")
			sprite.visible = false
			add_child(sprite)
		body.material.set_shader_parameter("part",1)
		wings.material.set_shader_parameter("part",2)
		birds.append({"body":body,"wings":wings})
func _process(_delta: float) -> void:
	if world == null: return
	var state: Dictionary = world.native_state
	var active: bool = state.get("bird_active",false)
	var count: int = int(state.get("bird_group_size",1)) if active else 0
	var species: String = ["bird","gull","blackbird","kingfisher"][posmod(int(state.get("bird_id",1))-1,4)]
	for member in range(3):
		var hit: bool = state.get("bird_hit",false) and member == int(state.get("bird_hit_member",0))
		var visible_member: bool = member<count and (state.get("bird_hit_visible",true) if hit else state.get("bird_flock_active",true))
		for sprite in [birds[member].body,birds[member].wings]: sprite.visible = visible_member
		if not visible_member: continue
		var frame := 4 if world.reduced_motion else (int(state.get("authored_clock",0)/13)+member*2)%8
		var art: Dictionary = world.assets[species+str(frame)]
		var at := Vector2(float(state.get("bird_flock_x_raw",state.bird_x_raw)),float(state.get("bird_flock_y_raw",state.bird_y_raw)))/65536.0
		var altitude := float(state.get("bird_flock_z_raw",state.bird_z_raw))/65536.0
		if member>0:
			at += Vector2(-member*12,8 if member == 1 else -8)
			altitude += 1 if member == 1 else -1
		if hit:
			at = Vector2(float(state.bird_x_raw),float(state.bird_y_raw))/65536.0
			altitude = float(state.bird_z_raw)/65536.0
		for sprite in [birds[member].body,birds[member].wings]:
			sprite.texture = world.textures[art.image]
			sprite.position = at-Vector2(0,altitude)
			sprite.rotation = 0.35 if hit and not world.reduced_motion else 0.0
			sprite.material.set_shader_parameter("body_centre",Vector2(0.54,0.48) if species == "gull" else Vector2(0.64,0.55))
			sprite.material.set_shader_parameter("body_extent",Vector2(0.23,0.085) if species == "gull" else Vector2(0.23,0.13))
