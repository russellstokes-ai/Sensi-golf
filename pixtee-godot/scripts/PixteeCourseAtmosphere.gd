extends Node2D

## Cosmetic weather clock; never consumes native randomness or changes collisions.
var world: Node2D
var elapsed := 0.0

func _process(delta: float) -> void:
	if world == null: return
	if not world.is_processing(): return
	if not world.reduced_motion: elapsed += minf(delta, 0.1)
	queue_redraw()

func _draw() -> void:
	if world == null or world.assets.is_empty(): return
	# Broad, feathered cloud shadows drift at less than one world pixel/second.
	if world.visual_quality > 0:
		for cloud in range(3):
			var centre := Vector2(fposmod(elapsed * 0.65 + cloud * 247.0, 1000.0)-220.0, 190.0+cloud*295.0)
			for lobe in range(4):
				for edge in range(5):
					draw_circle(centre+Vector2(lobe*42.0, sin(float(lobe)*2.0)*22.0), 68.0+edge*9.0, Color(0.10,0.16,0.23,0.005), true, -1.0, true)
	if world.reduced_motion: return
	for item in world.placements:
		var base := Vector2(item.x,item.y)
		if not world.view_rect.grow(65).has_point(base): continue
		if item.type == "tree" and world.visual_quality > 0:
			# A single occasional leaf per tree; staggered, bounded, no emitters accumulate.
			var cycle := fposmod(elapsed + float(item.phase)*17.0, 29.0)
			if cycle < 5.5:
				var at := base + Vector2(sin(cycle*2.4+float(item.phase))*3.0+cycle*1.7, -float(item.h)*0.7+cycle*float(item.h)*0.12)
				var alpha := minf(1.0,(5.5-cycle)*2.0)
				draw_line(at,at+Vector2(1.7,sin(cycle*4.0)),Color(0.62,0.65,0.22,alpha),1.0)
		if item.type == "photographer" and not world.reduced_flashes:
			var flash := fposmod(elapsed + float(item.phase)*19.0, 23.0)
			if flash < 0.09:
				var at := base + Vector2(2.0,-float(item.h)*0.72)
				draw_circle(at,2.5,Color(1.0,0.97,0.83,0.65),true,-1.0,true)
				draw_line(at-Vector2(4,0),at+Vector2(4,0),Color(1,1,0.95,0.8),0.7)
				draw_line(at-Vector2(0,4),at+Vector2(0,4),Color(1,1,0.95,0.8),0.7)
