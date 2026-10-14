extends Node2D
class_name PixteeWaterRainFX

## Rain-only micro-ripples in authored WATER polygons, not random effects
## over the fairway. Confirmed native water impacts are handled by the game's
## existing collision effects layer; avoid double contact rings.
var water_polygons: Array[PackedVector2Array] = []
var weather: PixteeWeatherDirector
var view_rect: Rect2 = Rect2(0,0,544,912)
var enabled: bool = false
var reduced_motion: bool = false
var visual_quality: int = 1
var _clock: float = 0.0

func set_water_regions(regions: Array[PackedVector2Array]) -> void:
	water_polygons = regions.duplicate()
	enabled = water_polygons.size()>0

func set_view_rect(rect: Rect2) -> void:
	if rect.size.x>0.0 and rect.size.y>0.0:
		view_rect=rect

func _process(delta: float) -> void:
	if not enabled or reduced_motion:
		return
	_clock += clampf(delta,0,0.1)
	queue_redraw()

func _draw() -> void:
	if not enabled or reduced_motion or weather==null or water_polygons.is_empty():
		return
	var rain: float = float(weather.conditions().get("rain",0))
	if rain<0.05:
		return
	var count: int = 8 if visual_quality==2 else (4 if visual_quality==1 else 2)
	for i in range(count):
		var phase: float = fposmod(_clock*0.75+float(i)*0.37,1.0)
		var px: float = view_rect.position.x+fposmod(float(i*73+27),view_rect.size.x)
		var py: float = view_rect.position.y+fposmod(float(i*139+49),view_rect.size.y)
		var at: Vector2 = Vector2(px,py)
		var on_water: bool = false
		for poly in water_polygons:
			if Geometry2D.is_point_in_polygon(at,poly):
				on_water=true
				break
		if on_water:
			draw_arc(at.round(),1.0+phase*3.0,0,TAU,12,Color(0.79,0.96,0.99,(1.0-phase)*rain*0.33),1.0,false)
