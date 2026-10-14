extends Node2D
class_name PixteeImpactPolish

## Native-confirmed, presentation-only contact flecks. Do NOT install both
## this renderer and existing PixteeNativeWorldFX collision bursts for the
## same event without disabling one. Native outcomes always take precedence.
const MAX_PARTICLES := 96
const MAX_RINGS := 16
const EVENT_SET := {
	"club_contact":{"colour":Color(0.91,0.88,0.61),"count":4,"life":0.23},
	"fairway_contact":{"colour":Color(0.59,0.78,0.37),"count":6,"life":0.38},
	"rough_contact":{"colour":Color(0.38,0.59,0.28),"count":7,"life":0.44},
	"bunker_contact":{"colour":Color(0.90,0.73,0.44),"count":10,"life":0.58},
	"water_contact":{"colour":Color(0.69,0.89,0.97),"count":10,"life":0.48},
	"authored_tree_contact":{"colour":Color(0.47,0.70,0.30),"count":8,"life":0.53},
	"verified_tree_contact":{"colour":Color(0.47,0.70,0.30),"count":8,"life":0.53},
	"verified_flag_contact":{"colour":Color(0.96,0.87,0.65),"count":4,"life":0.32},
	"cup_lip":{"colour":Color(1.0,0.90,0.65),"count":3,"life":0.26},
	"cup_capture":{"colour":Color(1.0,0.89,0.52),"count":7,"life":0.56}
}
@export var reduced_motion: bool = false
@export var reduced_flashes: bool = false
@export var visual_quality: int = 1
@export var enabled: bool = false  # fail-safe: existing game owns impact effects

var _flecks: Array[Dictionary] = []
var _rings: Array[Dictionary] = []
var _seen: Dictionary = {}
var _seen_order: Array[String] = []

func accept_native_event(event: Dictionary) -> bool:
	if not enabled or event.get("confirmed_native",false) != true:
		return false
	var event_id: String = str(event.get("event_id",""))
	var kind: String = str(event.get("event_type",""))
	if event_id.is_empty() or _seen.has(event_id) or not EVENT_SET.has(kind):
		return false
	if not event.has("world_x") or not event.has("world_y"):
		return false
	var base := Vector2(float(event["world_x"]),float(event["world_y"]))
	if not is_finite(base.x) or not is_finite(base.y):
		return false
	_seen[event_id] = true
	_seen_order.append(event_id)
	if _seen_order.size()>256:
		_seen.erase(_seen_order.pop_front())
	var preset: Dictionary = EVENT_SET[kind]
	var count: int = int(preset["count"]) if visual_quality > 0 else 2
	if visual_quality == 2:
		count += 2
	count = mini(count, MAX_PARTICLES - _flecks.size())
	var seeded := RandomNumberGenerator.new()
	seeded.seed = hash(event_id + kind)
	for i in range(count):
		var angle: float = seeded.randf_range(-PI,0.0)
		var speed: float = seeded.randf_range(6.0,17.0)
		_flecks.append({"pos":base,"vel":Vector2(cos(angle),sin(angle))*speed,"age":0.0,"life":float(preset["life"]),"colour":preset["colour"],"size":1.0 if i % 4 != 0 else 2.0})
	if kind=="water_contact" and _rings.size()<MAX_RINGS:
		_rings.append({"pos":base,"age":0.0,"life":0.72,"type":"water"})
	elif kind=="cup_capture" and _rings.size()<MAX_RINGS and not reduced_flashes:
		_rings.append({"pos":base,"age":0.0,"life":0.33,"type":"cup"})
	queue_redraw()
	return true

func _process(delta: float) -> void:
	var dt: float = clampf(delta,0.0,0.1)
	for i in range(_flecks.size()-1,-1,-1):
		var p: Dictionary = _flecks[i]
		p["age"]=float(p["age"])+dt
		if float(p["age"])>=float(p["life"]):
			_flecks.remove_at(i)
		else:
			p["pos"]=p["pos"]+p["vel"]*dt
			p["vel"]=p["vel"]+Vector2(0,21)*dt
			_flecks[i]=p
	for i in range(_rings.size()-1,-1,-1):
		var p: Dictionary = _rings[i]
		p["age"]=float(p["age"])+dt
		if float(p["age"])>=float(p["life"]):
			_rings.remove_at(i)
		else:
			_rings[i]=p
	if not _flecks.is_empty() or not _rings.is_empty():
		queue_redraw()

func _draw() -> void:
	for p in _flecks:
		var t: float = 1.0-float(p["age"])/float(p["life"])
		var c: Color = p["colour"]
		c.a = t*0.85
		var at: Vector2 = p["pos"]
		draw_rect(Rect2(at.round(),Vector2.ONE*float(p["size"])),c)
	for r in _rings:
		var t: float = float(r["age"])/float(r["life"])
		var centre: Vector2 = r["pos"]
		var is_water: bool = str(r["type"])=="water"
		var c := Color(0.76,0.91,0.97,0.45*(1.0-t)) if is_water else Color(1.0,0.91,0.55,0.48*(1.0-t))
		draw_arc(centre.round(),2.0+t*(8.0 if is_water else 4.0),0,TAU,20,c,1.0,false)

func reset_effects() -> void:
	_flecks.clear()
	_rings.clear()
	_seen.clear()
	_seen_order.clear()
	queue_redraw()
