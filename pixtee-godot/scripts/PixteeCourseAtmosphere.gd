extends Node2D

## Cosmetic weather clock; never consumes native randomness or changes collisions.
var world: Node2D
var elapsed := 0.0

func _process(delta: float) -> void:
	if world == null: return
	if not world.is_processing(): return
	elapsed = world.time
	queue_redraw()

func _draw() -> void:
	if world == null or world.assets.is_empty(): return
	# Cloud shade is now owned by the single world-space atmosphere shader.
	if world.reduced_motion: return
	for item in world.placements:
		var base := Vector2(item.x,item.y)
		if not world.view_rect.grow(65).has_point(base): continue
		if item.type == "tree" and world.visual_quality > 0:
			# A single occasional leaf per tree; staggered, bounded, no emitters accumulate.
			var cycle := fposmod(elapsed + float(item.phase)*17.0, 29.0)
			if cycle < 5.5:
				var drift: Vector2 = world.presentation.wind_toward*cycle*(0.7+world.presentation.visual_breeze*2.0)
				var at := base + drift + Vector2(sin(cycle*2.4+float(item.phase))*3.0, -float(item.h)*0.7+cycle*float(item.h)*0.12)
				var alpha := minf(1.0,(5.5-cycle)*2.0)
				draw_set_transform(at,sin(cycle*2.0)*0.6)
				draw_colored_polygon(PackedVector2Array([Vector2(-2,0),Vector2(0,-1),Vector2(2,0),Vector2(0,1)]),Color(0.62,0.65,0.22,alpha))
				draw_line(Vector2(-1,0),Vector2(1,0),Color(0.35,0.45,0.12,alpha),0.5)
				draw_set_transform(Vector2.ZERO)
		if item.type == "photographer" and not world.reduced_flashes:
			var flash := fposmod(elapsed + float(item.phase)*19.0, 23.0)
			if flash < 0.16:
				var at := base + Vector2(2.0,-float(item.h)*0.72)
				draw_circle(at,2.5,Color(1.0,0.97,0.83,0.85),true,-1.0,true)
				draw_line(at-Vector2(4,0),at+Vector2(4,0),Color(1,1,0.95,0.8),0.7)
				draw_line(at-Vector2(0,4),at+Vector2(0,4),Color(1,1,0.95,0.8),0.7)
