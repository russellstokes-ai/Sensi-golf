extends Node2D
class_name PixteeSunlightPolish

## OPTIONAL authored shafts/pollen around approved foliage clearings.
## Nodes are anchored, so moving the camera does not move the sunshine.
## Never adds lens-shaped full-screen flares or modifies original textures.

var weather: PixteeWeatherDirector
var sunlit_anchors: PackedVector2Array = PackedVector2Array()
var enabled: bool = false
var reduced_motion: bool = false
var reduced_flashes: bool = false
var visual_quality: int = 1
var _time: float = 0.0

func set_approved_sunlit_anchors(anchors: PackedVector2Array) -> void:
	sunlit_anchors=anchors
	enabled=anchors.size()>0
	queue_redraw()

func _process(delta: float) -> void:
	if not enabled:
		return
	if not reduced_motion:
		_time+=clampf(delta,0,0.1)
	queue_redraw()

func _draw() -> void:
	if not enabled or weather==null or visual_quality==0:
		return
	var sunny: float = float(weather.conditions().get("sun",0.0))
	var cloudy: float = float(weather.conditions().get("cloud",0.0))
	var strength: float = sunny*(1.0-cloudy*0.72)
	if strength<0.3:
		return
	var count: int = mini(sunlit_anchors.size(),12 if visual_quality==2 else 6)
	for i in range(count):
		var at: Vector2 = sunlit_anchors[i]
		# Short translucent pixel shafts, never a neon bloom.
		var shimmer: float = 0.0 if reduced_motion else sin(_time*0.5+float(i)*1.73)*0.025
		var alpha: float = clampf(strength*(0.055+shimmer),0.0,0.09)
		var polygon := PackedVector2Array([at+Vector2(-9,-18),at+Vector2(-3,-18),at+Vector2(7,12),at+Vector2(-1,12)])
		draw_colored_polygon(polygon,Color(1.0,0.95,0.61,alpha))
		if reduced_flashes:
			continue
		for k in range(3):
			var drift: float = 0.0 if reduced_motion else sin(_time*0.7+float(i*3+k))*3.0
			var pollen: Vector2 = at+Vector2(float(k)*4-4+drift,float(k)*7-5)
			draw_rect(Rect2(pollen.round(),Vector2.ONE),Color(1.0,0.94,0.65,strength*0.13))
