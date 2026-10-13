extends RefCounted
class_name PixteeOutfit
const SHIRTS := ["Classic red","Forest","Royal blue","Purple","Gold","White"]
const TROUSERS := ["Classic blue","Charcoal","Cream","Forest"]
const CAPS := ["Classic white","Red","Navy","Gold"]
const SHIRT_COLOURS := [Color("ed2716"),Color("35994c"),Color("3374e5"),Color("a957d2"),Color("e5bb34"),Color("e7e7dc")]
const TROUSER_COLOURS := [Color("2467c3"),Color("4f5662"),Color("d6c6a1"),Color("35794a")]
const CAP_COLOURS := [Color("f0f8ff"),Color("e94835"),Color("425b91"),Color("e6bd44")]

static func indices(id: String) -> PackedInt32Array:
	if id == "lakewood-classic": return PackedInt32Array([0,0,0])
	var parts := id.split(":")
	if parts.size() != 4 or parts[0] != "kit": return PackedInt32Array()
	var result := PackedInt32Array()
	for i in range(1,4):
		if not parts[i].is_valid_int(): return PackedInt32Array()
		result.append(int(parts[i]))
	if result[0] < 0 or result[0] >= SHIRTS.size() or result[1] < 0 or result[1] >= TROUSERS.size() or result[2] < 0 or result[2] >= CAPS.size(): return PackedInt32Array()
	return result

static func is_valid(id: String) -> bool:
	return indices(id).size() == 3

static func material(id: String) -> ShaderMaterial:
	var kit := indices(id)
	if kit.size() != 3: kit = PackedInt32Array([0,0,0])
	var result := ShaderMaterial.new()
	result.shader = load("res://shaders/pixtee_golfer_palette.gdshader")
	result.set_shader_parameter("shirt_colour",SHIRT_COLOURS[kit[0]])
	result.set_shader_parameter("trouser_colour",TROUSER_COLOURS[kit[1]])
	result.set_shader_parameter("cap_colour",CAP_COLOURS[kit[2]])
	result.set_shader_parameter("shirt_enabled",kit[0] != 0)
	result.set_shader_parameter("trouser_enabled",kit[1] != 0)
	result.set_shader_parameter("cap_enabled",kit[2] != 0)
	return result
