extends Node2D
var world: Node2D
var actors: Array[Node] = []
var weather: PixteeWeatherDirector
var visuals: PixteeWeatherVisuals
func _ready() -> void:
	weather = PixteeWeatherDirector.new()
	add_child(weather)
	weather.set_preset("partly_cloudy",0)
	visuals = PixteeWeatherVisuals.new()
	visuals.weather = weather
	visuals.enable_cloud_shadows = false
	add_child(visuals)
	for entry in [["rabbit_layered_idle",Vector2(210,785),18.0],["squirrel_layered_idle",Vector2(81,718),12.0],["frog_breathe",Vector2(434,374),10.0],["dragonfly_heading_02",Vector2(404,322),8.0]]:
		var actor := PixteePremiumLayeredSprite.new()
		actor.preview_enabled = false
		add_child(actor)
		if not actor.set_animation(entry[0],2): push_error("Living World actor failed: "+str(entry[0]))
		var manifest: Dictionary = JSON.parse_string(FileAccess.get_file_as_string(PixteePremiumLayeredSprite.MANIFEST))
		var canvas: Array = manifest[entry[0]].canvas
		var factor: float = float(entry[2])/float(canvas[1])
		actor.scale = Vector2.ONE*factor
		actor.position = entry[1]-Vector2(float(canvas[0])/2,float(canvas[1]))*factor
		actors.append(actor)
func _process(_delta: float) -> void:
	var frozen: bool = not world.is_processing() or world.reduced_motion
	weather.reduced_motion = frozen
	visuals.paused = frozen
	visuals.reduced_motion = world.reduced_motion
	visuals.reduced_flashes = world.reduced_flashes
	visuals.quality = world.visual_quality
	visuals.set_view_rect(world.view_rect)
	for actor in actors:
		actor.pause_presentation(frozen)
		actor.visible = world.visual_quality > 0
