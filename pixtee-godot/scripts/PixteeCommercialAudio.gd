extends Node
class_name PixteeCommercialAudio

## Source-owned, event-driven audio router for fully licensed original Pixtee sound.
## Not a substitute for actual audio assets: asset validation FAILS if missing.
## Godot renders audio only. Native C++ owns collision/outcome/score state.

const BUS_NAMES := ["Music", "SFX", "Ambience", "UI"]
const REQUIRED_TRACKS := [
	"intro_music", "menu_music", "gameplay_music", "hole_scored_music",
	"birdie_sting", "eagle_sting", "hole_in_one_sting",
	"club_whoosh", "club_strike", "fairway_hit", "rough_hit",
	"bunker_sand", "water_splash", "tree_impact", "flag_hit",
	"cup_rattle", "cup_capture", "oob_penalty", "menu_select",
	"menu_back", "ambient_birds", "ambient_wind",
	"ambient_water", "ambient_crowd", "photographer_flash"
]
const NATIVE_EVENT_TO_CUE := {
	"club_swing": "club_whoosh",
	"club_contact": "club_strike",
	"fairway_contact": "fairway_hit",
	"rough_contact": "rough_hit",
	"bunker_contact": "bunker_sand",
	"water_contact": "water_splash",
	"verified_tree_contact": "tree_impact",
	"verified_flag_contact": "flag_hit",
	"cup_lip": "cup_rattle",
	"cup_capture": "cup_capture",
	"world_oob": "oob_penalty",
	"mapped_oob": "oob_penalty"
}

var _catalog: Dictionary = {}
var _players: Dictionary = {}
var _sfx_pool: Array[AudioStreamPlayer] = []
var _event_dedup: Dictionary = {}
var _event_order: Array[String] = []
var _ready_for_commercial_audio := false

func _ready() -> void:
	# A separate bus for each of four volume controls, independent of gameplay.
	for name in BUS_NAMES:
		if AudioServer.get_bus_index(name) < 0:
			var index := AudioServer.get_bus_count()
			AudioServer.add_bus(index)
			AudioServer.set_bus_name(index, name)
	for name in ["Music", "Ambience", "UI"]:
		var player := AudioStreamPlayer.new()
		player.name = "Pixtee" + name
		player.bus = name
		add_child(player)
		_players[name] = player
	for i in range(12):
		var player := AudioStreamPlayer.new()
		player.name = "PixteeSFX_%02d" % i
		player.bus = "SFX"
		add_child(player)
		_sfx_pool.append(player)

func validate_and_load_catalog(source_paths: Dictionary) -> PackedStringArray:
	## Caller supplies RES paths to licensed ORIGINAL production masters.
	## Validated atomically: no bundled synthetic music or Sensi Golf audio.
	var loaded: Dictionary = {}
	var failures := PackedStringArray()
	for key in REQUIRED_TRACKS:
		if not source_paths.has(key):
			failures.append(key + ": missing required licensed sound")
			continue
		var path: String = str(source_paths[key])
		if not path.begins_with("res://") or not ResourceLoader.exists(path):
			failures.append(key + ": resource not present " + path)
			continue
		var stream := load(path)
		if not stream is AudioStream:
			failures.append(key + ": not a Godot AudioStream")
			continue
		loaded[key] = stream
	if not failures.is_empty():
		_catalog.clear()
		_ready_for_commercial_audio = false
		return failures
	_catalog = loaded
	_ready_for_commercial_audio = true
	return failures

func is_fully_populated() -> bool:
	return _ready_for_commercial_audio and _catalog.size() == REQUIRED_TRACKS.size()

func set_channel_level(channel: String, linear_volume: float) -> bool:
	if not BUS_NAMES.has(channel) or not is_finite(linear_volume):
		return false
	var index := AudioServer.get_bus_index(channel)
	if index < 0:
		return false
	var clamped := clampf(linear_volume, 0.0, 1.0)
	AudioServer.set_bus_mute(index, clamped == 0.0)
	if clamped > 0.0:
		AudioServer.set_bus_volume_db(index, linear_to_db(clamped))
	return true

func play_music(cue: String) -> bool:
	if not is_fully_populated() or not [
		"intro_music", "menu_music", "gameplay_music", "hole_scored_music",
		"birdie_sting", "eagle_sting", "hole_in_one_sting"
	].has(cue):
		return false
	var player: AudioStreamPlayer = _players.get("Music")
	if player == null:
		return false
	# Author-approved recording must provide genuine loop points or real
	# outro; commercial gap remains until crossfade/loop tests pass.
	player.stop()
	player.stream = _catalog[cue]
	player.play()
	return true

func play_ambience(cue: String) -> bool:
	if not is_fully_populated() or not [
		"ambient_birds", "ambient_wind", "ambient_water", "ambient_crowd"
	].has(cue):
		return false
	var player: AudioStreamPlayer = _players.get("Ambience")
	if player == null:
		return false
	player.stop()
	player.stream = _catalog[cue]
	player.play()
	return true

func play_ui(cue: String) -> bool:
	if not is_fully_populated() or not ["menu_select", "menu_back"].has(cue):
		return false
	var player: AudioStreamPlayer = _players.get("UI")
	if player == null:
		return false
	player.stream = _catalog[cue]
	player.play()
	return true

func consume_confirmed_native_event(event: Dictionary) -> bool:
	## Only actual C++ native event packets: no proximity-based SFX,
	## no repeated water/cup sound on every 70Hz tick.
	if not is_fully_populated():
		return false
	if event.get("confirmed_native", false) != true:
		return false
	var event_id: String = str(event.get("event_id", ""))
	var event_type: String = str(event.get("event_type", ""))
	if event_id.is_empty() or not NATIVE_EVENT_TO_CUE.has(event_type):
		return false
	if _event_dedup.has(event_id):
		return false
	var cue: String = NATIVE_EVENT_TO_CUE[event_type]
	var player: AudioStreamPlayer = null
	for available in _sfx_pool:
		if not available.playing:
			player = available
			break
	if player == null:
		return false # bounded voices, no uncontrolled audio explosion
	player.stream = _catalog[cue]
	player.play()
	_event_dedup[event_id] = true
	_event_order.append(event_id)
	if _event_order.size() > 4096:
		var oldest: String = _event_order.pop_front()
		_event_dedup.erase(oldest)
	return true

func reset_event_cursor_on_confirmed_new_round() -> void:
	_event_dedup.clear()
	_event_order.clear()

func stop_all_for_application_pause() -> void:
	for player in _players.values():
		player.stop()
	for player in _sfx_pool:
		player.stop()
