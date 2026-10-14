extends Node2D
## A foot-anchored depth-sorted approved placement, with rigid visual layers.
var world: Node2D
var spec: Dictionary
var art: Dictionary
var texture: Texture2D
var board_texture: Texture2D
func configure(owner_world: Node2D, placement: Dictionary) -> void:
	world = owner_world
	spec = placement
	position = Vector2(spec.x,spec.y)
	z_index = roundi(position.y)
	texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	if spec.type == "board":
		for i in world.boards.size():
			if world.boards[i].x == spec.x and world.boards[i].y == spec.y: board_texture = world.textures["board-%d" % i]
	elif spec.type != "flag":
		art = world.assets[spec.id]
		texture = world.textures[art.image]
func _draw() -> void:
	if world == null: return
	if spec.type == "board":
		if board_texture != null:
			var dimensions := board_texture.get_size()*(float(spec.w)/board_texture.get_width())
			draw_texture_rect(board_texture,Rect2(Vector2(-dimensions.x/2,-float(spec.h)-3),dimensions),false)
		return
	if spec.type == "flag":
		if world.native_state.get("flag_removed_for_putting",false):
			world.draw_ellipse_on(self,Vector2.ZERO,Vector2(1.2,0.7),Color("132719"))
		else:
			draw_line(Vector2.ZERO,Vector2(0,-spec.h),Color("ebe6cd"),0.7)
			var flutter: float = world.presentation.breeze_at(position,2.0)
			draw_colored_polygon(PackedVector2Array([Vector2(0,-spec.h),Vector2(8,-spec.h+3+flutter),Vector2(0,-spec.h+7)]),Color("df3527"))
		return
	var rect: Array = art.rect
	if art.has("frames") and not world.reduced_motion and not (spec.type == "photographer" and world.reduced_flashes):
		var rate := 0.28 if spec.type != "photographer" else 0.2
		rect = art.frames[int(world.time/rate+spec.phase)%art.frames.size()]
	var region := Rect2(rect[0],rect[1],rect[2],rect[3])
	var dimensions := region.size*(float(spec.h)/float(art.rect[3]))
	var flip := -1.0 if spec.get("flipX",false) else 1.0
	var split := 0.0
	var angle := 0.0
	if spec.type in ["person","photographer"]:
		split = 0.58
	elif spec.type == "tree":
		split = 0.76
		angle = world.presentation.breeze_at(position,deg_to_rad(0.75))
	elif spec.type in ["grass","flowers","reeds"]:
		angle = world.presentation.breeze_at(position,deg_to_rad(1.1))
	if split > 0.0:
		draw_set_transform(Vector2.ZERO,0,Vector2(flip,1))
		var lower_height := dimensions.y*(1.0-split)
		draw_texture_rect_region(texture,Rect2(Vector2(-dimensions.x/2,-lower_height),Vector2(dimensions.x,lower_height)),Rect2(region.position+Vector2(0,region.size.y*split),region.size*Vector2(1,1-split)))
		draw_set_transform(Vector2(0,-lower_height),angle,Vector2(flip,1))
		# A shared strip of the original trunk keeps the canopy joint covered.
		# Human masks remain aligned exactly; genuine source frames supply motion.
		var overlap := minf(1.0,lower_height*0.35) if spec.type == "tree" else 0.0
		var source_overlap := overlap*region.size.y/dimensions.y
		draw_texture_rect_region(texture,Rect2(Vector2(-dimensions.x/2,-dimensions.y*split),Vector2(dimensions.x,dimensions.y*split+overlap)),Rect2(region.position,Vector2(region.size.x,region.size.y*split+source_overlap)))
	else:
		draw_set_transform(Vector2.ZERO,angle,Vector2(flip,1))
		draw_texture_rect_region(texture,Rect2(Vector2(-dimensions.x/2,-dimensions.y),dimensions),region)
	draw_set_transform(Vector2.ZERO)
