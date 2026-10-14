extends Node2D
class_name PixteeWeatherVisuals

## One pooled/batched, world-space 2D presentation layer.
## Never changes Pixtee course polygons, native wind or gameplay state.
## The host must supply the currently visible rect in original course coordinates.

const MAX_RAIN := 95
const MAX_WIND_STROKES := 18
const MAX_MIST := 9

var weather: PixteeWeatherDirector
var view_rect: Rect2 = Rect2(0.0, 0.0, 544.0, 912.0)
var quality: int = 1                 # 0 low, 1 normal, 2 high
var reduced_motion: bool = false
var reduced_flashes: bool = false
var visual_time: float = 0.0
var paused: bool = false
var weather_enabled: bool = true
var enable_cloud_shadows: bool = false  # Lakewood already owns its cloud-shadow layer.
var _visual_seed: int = 72241

func set_view_rect(rect: Rect2) -> void:
	if rect.size.x > 0.0 and rect.size.y > 0.0:
		view_rect = rect

func _process(delta: float) -> void:
	if paused or not weather_enabled:
		return
	if not reduced_motion:
		visual_time += minf(delta, 0.1)
	queue_redraw()

func _hash01(index: int, salt: int = 0) -> float:
	# Deterministic presentation pattern only; never share the core PRNG.
	var x: float = sin(float(index * 71 + salt * 131 + _visual_seed) * 12.9898) * 43758.5453
	return x - floorf(x)

func _draw() -> void:
	if weather == null or not weather_enabled:
		return
	var c: Dictionary = weather.conditions()
	var cloud := float(c.get("cloud", 0.0))
	var rain := float(c.get("rain", 0.0))
	var fog := float(c.get("fog", 0.0))
	var sun := float(c.get("sun", 0.0))
	var wind := float(c.get("wind", 0.0))
	if quality > 0:
		if enable_cloud_shadows:
			_draw_cloud_shadows(cloud)
		_draw_sun_dapple(sun, cloud)
		_draw_mist(fog)
	if not reduced_motion:
		_draw_wind_trails(wind, rain)
		_draw_rain(rain)
	elif rain > 0.0:
		# Weather remains perceivable with accessibility setting enabled.
		_draw_static_rain(rain)

func _draw_cloud_shadows(cloud: float) -> void:
	if cloud < 0.09:
		return
	var drift := 0.0 if reduced_motion else visual_time * 0.52
	var r := view_rect.grow(120.0)
	for i in range(4):
		var at := Vector2(r.position.x + fposmod(float(i) * 168.0 + drift, r.size.x), r.position.y + r.size.y * _hash01(i, 4))
		# Pixel-soft overlapping shapes, not full-screen dimming or blurred tiles.
		for j in range(3):
			draw_circle((at + Vector2(float(j - 1) * 25.0, sin(float(j) * 2.7) * 13.0)).round(), 31.0 + float(j) * 9.0, Color(0.08, 0.14, 0.18, cloud * 0.023))

func _draw_sun_dapple(sun: float, cloud: float) -> void:
	if sun < 0.3:
		return
	var strength := sun * (1.0 - cloud * 0.6)
	var t := 0.0 if reduced_motion else visual_time
	var origin := view_rect.position
	for i in range(10 if quality == 2 else 5):
		var x := origin.x + _hash01(i, 14) * view_rect.size.x
		var y := origin.y + _hash01(i, 15) * view_rect.size.y
		var phase := sin(t * 0.55 + float(i))
		var c := Color(1.0, 0.94, 0.61, strength * (0.025 + 0.018 * phase))
		draw_rect(Rect2(Vector2(x, y).round(), Vector2(9.0, 3.0)), c)
	# Sun glints are world decoration, never a bright screen-centre flare.
	if not reduced_flashes and quality == 2 and strength > 0.72:
		var glint := origin + view_rect.size * Vector2(0.72, 0.16)
		var alpha := (0.06 if reduced_motion else (sin(t * 0.45) + 1.0) * 0.035) * strength
		draw_line(glint - Vector2(3.0, 0.0), glint + Vector2(3.0, 0.0), Color(1, 0.94, 0.70, alpha), 1.0)
		draw_line(glint - Vector2(0.0, 3.0), glint + Vector2(0.0, 3.0), Color(1, 0.94, 0.70, alpha), 1.0)

func _draw_mist(fog: float) -> void:
	if fog < 0.05:
		return
	var drift := 0.0 if reduced_motion else visual_time * 0.30
	for i in range(MAX_MIST if quality == 2 else 5):
		var x := view_rect.position.x + fposmod(_hash01(i, 22) * view_rect.size.x + drift, maxf(1.0, view_rect.size.x))
		var y := view_rect.position.y + _hash01(i, 25) * view_rect.size.y
		var at := Vector2(x, y).round()
		# Narrow wisps; avoid covering flag, ball or slope direction markers.
		for j in range(3):
			draw_line(at + Vector2(float(j) * 2.0, 0.0), at + Vector2(17.0 + float(j) * 2.0, 0.0), Color(0.81, 0.88, 0.86, 0.08 * fog), 1.0)

func _draw_wind_trails(strength: float, rain: float) -> void:
	if strength < 0.10 or quality == 0:
		return
	var wind_direction := weather.get_wind_direction().normalized()
	var perpendicular := Vector2(-wind_direction.y, wind_direction.x)
	var count := mini(MAX_WIND_STROKES, int(4.0 + strength * (9.0 if quality == 2 else 5.0)))
	var w := maxf(view_rect.size.x, view_rect.size.y) * 1.5
	for i in range(count):
		var random_x := _hash01(i, 31)
		var random_y := _hash01(i, 32)
		var travel := fposmod((visual_time * (15.0 + strength * 21.0) + random_x * w), w) - w * 0.25
		var start := view_rect.position + Vector2(random_x * view_rect.size.x, random_y * view_rect.size.y) + wind_direction * travel
		# Short, gently curved 1-pixel strokes; visual wind only.
		var length := 6.0 + strength * 12.0
		var wave := sin(visual_time * 1.5 + float(i)) * 1.5
		var line := PackedVector2Array([start, start + wind_direction * length * 0.5 + perpendicular * wave, start + wind_direction * length])
		var col := Color(0.96, 0.98, 0.89, clampf(0.13 + strength * 0.15 - rain * 0.04, 0.0, 0.32))
		draw_polyline(line, col, 1.0, false)

func _draw_rain(rain: float) -> void:
	if rain < 0.02:
		return
	var strength := float(weather.conditions().get("wind", 0.0))
	var dir := weather.get_wind_direction()
	var count := mini(MAX_RAIN, int(rain * (105.0 if quality == 2 else (53.0 if quality == 1 else 18.0))))
	var size := view_rect.size
	for i in range(count):
		var speed := 50.0 + _hash01(i, 52) * 35.0
		var x := fposmod(_hash01(i, 50) * size.x + dir.x * strength * visual_time * speed * 0.45, maxf(1.0, size.x))
		var y := fposmod(_hash01(i, 51) * size.y + visual_time * speed, maxf(1.0, size.y))
		var at := (view_rect.position + Vector2(x, y)).round()
		var tail := Vector2(dir.x * strength * 3.0, 3.0 + rain * 2.0)
		draw_line(at, (at + tail).round(), Color(0.72, 0.89, 0.97, 0.25 + rain * 0.27), 1.0, false)
		if i % 13 == 0:
			var impact_y := view_rect.position.y + size.y * _hash01(i, 53)
			var x_at := view_rect.position.x + size.x * _hash01(i, 54)
			draw_circle(Vector2(x_at, impact_y).round(), 1.0, Color(0.79, 0.94, 0.98, rain * 0.17))

func _draw_static_rain(rain: float) -> void:
	for i in range(12):
		var at := view_rect.position + Vector2(_hash01(i, 65) * view_rect.size.x, _hash01(i, 66) * view_rect.size.y)
		draw_line(at.round(), (at + Vector2(1.0, 2.0)).round(), Color(0.76, 0.90, 0.96, rain * 0.22), 1.0, false)
