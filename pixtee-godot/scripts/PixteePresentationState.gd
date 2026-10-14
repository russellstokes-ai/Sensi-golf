extends RefCounted
class_name PixteePresentationState
## One cosmetic timebase. Never advances or owns native simulation.
var time_seconds := 0.0
var paused := false
var reduced_motion := false
var reduced_flashes := false
var quality := 1
var view_rect := Rect2(0,0,544,912)
var wind_toward := Vector2(0.707,-0.707)
var wind_mph := 0.0
var visual_breeze := 0.2
var sun_direction := Vector2(-0.6,0.8)
var sun_strength := 0.65
var cloud_cover := 0.35
var fog_density := 0.0
func advance(delta: float) -> void:
	if not paused and not reduced_motion: time_seconds += clampf(delta,0.0,0.1)
	if wind_mph > 0.0: visual_breeze = clampf(wind_mph/15.0,0.0,1.0)
func breeze_at(at: Vector2, amplitude: float) -> float:
	if reduced_motion or quality == 0: return 0.0
	# Pause freezes the shared clock, rather than snapping foliage to rest.
	var phase := at.dot(wind_toward)*0.009
	var gust := 0.75+0.25*sin(time_seconds*0.21)
	var wave := 0.25+0.75*sin(time_seconds*0.43+phase)
	return wave*gust*amplitude*visual_breeze*wind_toward.x
