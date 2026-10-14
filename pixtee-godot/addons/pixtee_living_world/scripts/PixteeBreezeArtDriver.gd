extends RefCounted
class_name PixteeBreezeArtDriver

## Non-destructive local sway resolver for approved grass, foliage, reeds and
## flags already drawn by the host. The host merely applies the returned small
## angle to its visual transform, never to terrain/collision/physics maps.
var weather: PixteeWeatherDirector

func sway_radians(world_position: Vector2, kind: String, visual_time: float, reduced_motion: bool = false) -> float:
	if weather==null or reduced_motion:
		return 0.0
	var strength: float = float(weather.conditions().get("wind",0.0))
	if strength<=0.02:
		return 0.0
	var degrees: float = 0.0
	match kind:
		"grass": degrees=2.0
		"flowers": degrees=2.7
		"reeds": degrees=3.2
		"tree": degrees=0.40
		"flag": degrees=3.1
		_: degrees=0.75
	var dir: Vector2 = weather.get_wind_direction()
	var sign_x: float = 1.0 if dir.x>=0.0 else -1.0
	var phase: float = world_position.x*0.067+world_position.y*0.038
	var lag: float = 0.7+0.3*sin(visual_time*0.28+phase*0.41)
	return deg_to_rad(degrees)*strength*lag*sin(visual_time*1.6+phase)*sign_x
