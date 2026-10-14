extends Node2D
var world: Node2D
func _draw() -> void:
	if world.native_state.is_empty(): return
	var state: Dictionary = world.native_state
	var height := maxf(0.0,float(state.get("ball_z_raw",0))/65536.0)
	var texture: Texture2D = world.textures.ball
	var dimensions := texture.get_size()*(3.0/texture.get_height())
	draw_texture_rect(texture,Rect2((world.ball_world-Vector2(dimensions.x/2,height+dimensions.y/2)).round(),dimensions),false)
	if not state.get("can_shoot",false): return
	var direction := Vector2(sin(float(state.get("aim_raw",0))*TAU/4096.0),cos(float(state.get("aim_raw",0))*TAU/4096.0))
	var length: float = world.aim_length(state)
	for i in range(1,9):
		draw_circle((world.ball_world+direction*length*float(i)/8.0).round(),0.65,Color(1,1,0.8,0.6),true,-1,true)
	var end: Vector2 = world.ball_world+direction*length
	var pulse := 0.5 if world.reduced_motion else 0.5+0.5*sin(world.time*3.7)
	draw_arc(end,2.3+0.25*pulse,0,TAU,20,Color(1,0.25,0.18,0.5+0.2*pulse),0.6,true)
	draw_circle(end,0.7,Color("ff5040"),true,-1,true)
