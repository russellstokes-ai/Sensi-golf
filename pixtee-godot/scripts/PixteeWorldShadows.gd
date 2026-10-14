extends Node2D
## Ground-only contact and cast geometry. Original sprites are never deformed.
var world: Node2D
var tree_profiles: Dictionary = {}
func _ready() -> void:
	z_index = -30
	for spec in world.placements:
		if spec.type != "tree": continue
		var art: Dictionary = world.assets[spec.id]
		var rect: Array = art.rect
		var image: Image = world.textures[art.image].get_image()
		var profile: Array[Vector3] = []
		var step := maxi(2,int(rect[3])/18)
		for row in range(0,int(rect[3]),step):
			var left := int(rect[2])
			var right := -1
			for column in range(0,int(rect[2]),2):
				if image.get_pixel(int(rect[0])+column,int(rect[1])+row).a>0.2:
					left = mini(left,column)
					right = maxi(right,column)
			if right >= left: profile.append(Vector3(float(left)/rect[2]-0.5,float(right)/rect[2]-0.5,1.0-float(row)/rect[3]))
		tree_profiles[spec.id] = profile
func _ellipse(at: Vector2, radius: Vector2, alpha: float) -> void:
	world.draw_ellipse_on(self,at,radius,Color(0.018,0.045,0.032,alpha))
func _cast_person(at: Vector2, height: float) -> void:
	var sun: float = world.presentation.sun_strength
	if sun<0.03: return
	var toward: Vector2 = world.presentation.sun_direction
	var width := Vector2(toward.y,-toward.x)*2.0
	var end := at+toward*height*0.65
	draw_colored_polygon(PackedVector2Array([at-width,at+width,end+width*0.45,end-width*0.45]),Color(0.025,0.06,0.038,0.18*sun))
	_ellipse(end,Vector2(1.5,1.2),0.12*sun)
func _draw() -> void:
	if world == null: return
	var sun: float = world.presentation.sun_strength
	var toward: Vector2 = world.presentation.sun_direction
	var side := Vector2(toward.y,-toward.x)
	for spec in world.placements:
		var at := Vector2(spec.x,spec.y)
		if not world.view_rect.grow(90).has_point(at): continue
		if spec.type == "tree":
			_ellipse(at,Vector2(4,1.8),0.16)
			if world.visual_quality == 0: continue
			var profile: Array = tree_profiles.get(spec.id,[])
			if profile.size()<2: continue
			var contour := PackedVector2Array()
			for entry in profile: contour.append(at+side*float(spec.w)*entry.x+toward*float(spec.h)*0.5*entry.z)
			for i in range(profile.size()-1,-1,-1):
				var entry: Vector3 = profile[i]
				contour.append(at+side*float(spec.w)*entry.y+toward*float(spec.h)*0.5*entry.z)
			draw_colored_polygon(contour,Color(0.02,0.055,0.037,0.15*sun))
		elif spec.type in ["person","photographer"]:
			_ellipse(at+Vector2(0,0.4),Vector2(2.6,1.0),0.23)
			_cast_person(at,float(spec.h))
		elif spec.type == "board":
			_ellipse(at,Vector2(float(spec.w)*0.42,1.4),0.18)
	_ellipse(world.golfer_world,Vector2(3.5,1.2),0.27)
	_cast_person(world.golfer_world,21.0)
	var caddy: Node = world.get_node_or_null("Caddy")
	if caddy != null:
		_ellipse(caddy.foot,Vector2(3.0,1.1),0.23)
		_cast_person(caddy.foot,21.0)
	if not world.native_state.is_empty():
		var height := maxf(0.0,float(world.native_state.get("ball_z_raw",0))/65536.0)
		var radius := clampf(1.2+height*0.025,1.2,2.3)
		_ellipse(world.ball_world,Vector2(radius,radius*0.65),clampf(0.32-height*0.007,0.1,0.32))
