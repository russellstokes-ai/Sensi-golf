extends RefCounted
class_name PixteeOutfit
const SHIRTS := ["Classic red","Forest","Royal blue","Purple","Gold","White"]
const TROUSERS := ["Classic blue","Charcoal","Cream","Forest"]
const CAPS := ["Classic white","Red","Navy","Gold"]
const SHIRT_COLOURS := [Color("ed2716"),Color("35994c"),Color("3374e5"),Color("a957d2"),Color("e5bb34"),Color("e7e7dc")]
const TROUSER_COLOURS := [Color("2467c3"),Color("4f5662"),Color("d6c6a1"),Color("35794a")]
const CAP_COLOURS := [Color("f0f8ff"),Color("e94835"),Color("425b91"),Color("e6bd44")]

static func female(id: String) -> bool:
	return id.begins_with("female:")

static func indices(id: String) -> PackedInt32Array:
	if female(id): id = id.trim_prefix("female:")
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

static func material(id: String, appearance: Dictionary = {"skin":0,"hair":0,"face":0,"beard":0,"hairstyle":0}) -> ShaderMaterial:
	var kit := indices(id)
	if kit.size() != 3: kit = PackedInt32Array([0,0,0])
	var result := ShaderMaterial.new()
	result.shader = load("res://shaders/pixtee_golfer_palette.gdshader")
	result.set_shader_parameter("atlas_grid",Vector2(8,8))
	result.set_shader_parameter("shirt_colour",SHIRT_COLOURS[kit[0]])
	result.set_shader_parameter("trouser_colour",TROUSER_COLOURS[kit[1]])
	result.set_shader_parameter("cap_colour",CAP_COLOURS[kit[2]])
	result.set_shader_parameter("shirt_enabled",kit[0] != 0)
	result.set_shader_parameter("trouser_enabled",kit[1] != 0)
	result.set_shader_parameter("cap_enabled",kit[2] != 0)
	if valid_appearance(appearance):
		result.set_shader_parameter("skin_enabled",appearance.skin != 0)
		result.set_shader_parameter("hair_enabled",appearance.hair != 0)
		result.set_shader_parameter("skin_colour",SKIN_COLOURS[int(appearance.skin)])
		result.set_shader_parameter("hair_colour",(Color("252027") if appearance.hair == 0 and not female(id) else HAIR_COLOURS[int(appearance.hair)]))
		for key in ["face","beard","hairstyle"]: result.set_shader_parameter(key+"_style",int(appearance[key]))
	return result

const SKINS := ["Original","Fair","Warm","Tan","Brown","Deep"]
const HAIR := ["Original","Black","Chestnut","Blonde","Auburn","Silver"]
const SKIN_COLOURS := [Color("e6a66c"),Color("f4ccac"),Color("deb18b"),Color("c99061"),Color("95633e"),Color("613c2b")]
const HAIR_COLOURS := [Color("75400d"),Color("252027"),Color("62402b"),Color("d3a44f"),Color("954129"),Color("bcbcc1")]
const FACES := ["Natural","Freckles","Rosy cheeks"]
const BEARDS := ["Clean shaven","Stubble","Short beard"]
const HAIRSTYLES := ["Classic","Soft fringe"]
const APPEARANCE_DEFAULTS := {"skin":0,"hair":0,"face":0,"beard":0,"hairstyle":0}
static func valid_appearance(value: Variant) -> bool:
	if not value is Dictionary or value.size() != 5: return false
	var keys := ["skin","hair","face","beard","hairstyle"]
	var limits := [6,6,3,3,2]
	for i in keys.size():
		var index = value.get(keys[i])
		if not (index is int or index is float) or not is_finite(float(index)) or index != floor(index) or index < 0 or index >= limits[i]: return false
	return true
