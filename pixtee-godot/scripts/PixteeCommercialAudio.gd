extends Node
class_name PixteeCommercialAudio

## Source-owned, event-driven audio router for fully licensed original Pixtee sound.
## Not a substitute for actual audio assets: asset validation FAILS if missing.
## Godot renders audio only. Native C++ owns collision/outcome/score state.

const BUS_NAMES := ["Music", "SFX", "Ambience", "UI"]
const REQUIRED_TRACKS := [
	"menu_music",
	"birdie_sting", "eagle_sting", "hole_in_one_sting",
	"club_whoosh", "club_strike", "rough_club_strike", "fairway_hit", "rough_hit",
	"bunker_sand", "water_splash", "tree_impact", "flag_hit",
	"bird_impact", "cup_rattle", "cup_capture", "oob_penalty", "menu_select",
	"menu_back", "course_ambient_1", "course_ambient_2", "course_ambient_3",
	"course_ambient_4", "course_ambient_5", "course_ambient_6", "photographer_flash"
]
const NATIVE_EVENT_TO_CUE := {
	"club_swing": "club_whoosh",
	"club_contact": "club_strike",
	"ball_bounce": "small_bounce",
	"fairway_contact": "fairway_hit",
	"rough_contact": "rough_hit",
	"bunker_contact": "bunker_sand",
	"water_contact": "water_splash",
	"verified_tree_contact": "tree_impact",
	"authored_tree_contact": "tree_impact",
	"authored_bird_contact": "bird_impact",
	"authored_rock_contact": "rock_impact",
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
var _ambience_timer: Timer
var _ambience_rng := RandomNumberGenerator.new()
var _context := ""

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
	_players["Ambience"].volume_db = linear_to_db(0.75)
	_players["Ambience"].finished.connect(_schedule_course_ambience)
	_ambience_rng.randomize() # Cosmetic scheduling only; never native gameplay PRNG.
	_ambience_timer = Timer.new()
	_ambience_timer.one_shot = true
	add_child(_ambience_timer)
	_ambience_timer.timeout.connect(_next_course_ambience)
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
	return _ready_for_commercial_audio and missing_required_cues().is_empty()

func missing_required_cues() -> PackedStringArray:
	var missing := PackedStringArray()
	for cue in REQUIRED_TRACKS:
		if not _catalog.has(cue): missing.append(cue)
	return missing

func load_owner_pack() -> PackedStringArray:
	# Importing owner recordings does not certify complete audio or rights.
	var base := "res://audio/owner-pack-v1/"
	var manifest: Variant = JSON.parse_string(FileAccess.get_file_as_string(base+"manifest.json"))
	var failures := PackedStringArray()
	if not manifest is Dictionary or not manifest.get("cues") is Dictionary:
		failures.append("Owner audio manifest invalid")
		return failures
	var loaded: Dictionary = {}
	for cue in manifest.cues:
		var stream: Variant = load(base+str(manifest.cues[cue]))
		if not stream is AudioStream or stream.get_length() <= 0:
			failures.append(str(cue)+": recording could not be decoded")
		else:
			# Both recordings are labelled ball-into-hole by the owner.
			# Keep the immutable intake manifest; do not call variant 2 a lip-out.
			loaded["cup_capture_variant" if cue == "cup_rattle" else cue] = stream
	for cue in {"putter_strike":"Putter hitting sound.mp3", "large_bounce":"Golf ball large bounce.mp3", "small_bounce":"Golf ball small bounce.mp3"}:
		var filenames := {"putter_strike":"Putter hitting sound.mp3", "large_bounce":"Golf ball large bounce.mp3", "small_bounce":"Golf ball small bounce.mp3"}
		var recording: Variant = load("res://audio/owner-supplement-v1/"+str(filenames[cue]))
		if not recording is AudioStream or recording.get_length() <= 0:
			failures.append(str(cue)+": recording could not be decoded")
		else:
			loaded[cue] = recording
	if failures.is_empty():
		_catalog = loaded
		_ready_for_commercial_audio = false
	return failures

func set_context(context: String) -> void:
	if context == _context: return
	var previous := _context
	_context = context
	if context == "menu":
		_ambience_timer.paused = true
		_players["Ambience"].stream_paused = true
		for voice in _sfx_pool: voice.stream_paused = true
		play_music("menu_music")
	elif context == "course":
		_ambience_timer.paused = false
		_players["Music"].stop()
		_players["Ambience"].stream_paused = false
		for voice in _sfx_pool: voice.stream_paused = false
		if not _players["Ambience"].playing and _ambience_timer.is_stopped():
			_ambience_timer.start(_ambience_rng.randf_range(2.0,6.0))
	elif context == "results":
		_ambience_timer.paused = true
		_players["Music"].stop()
		_players["Ambience"].stream_paused = true
		# The native cup recording must finish across the scoring transition.
		for voice in _sfx_pool: voice.stream_paused = false
	elif context == "background":
		_ambience_timer.paused = true
		for player in _players.values(): player.stream_paused = true
		for voice in _sfx_pool: voice.stream_paused = true
	if previous == "background" and context == "menu":
		_players["Music"].stream_paused = false

func _next_course_ambience() -> void:
	if _context != "course": return
	play_ambience("course_ambient_%d" % _ambience_rng.randi_range(1,6))

func _schedule_course_ambience() -> void:
	# Playback duration determines when this starts; never overlap nature clips.
	if _context == "course":
		_ambience_timer.start(_ambience_rng.randf_range(8.0,20.0))

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
	if not _catalog.has(cue) or not [
		"intro_music", "menu_music", "gameplay_music", "hole_scored_music",
		"birdie_sting", "eagle_sting", "hole_in_one_sting"
	].has(cue):
		return false
	var player: AudioStreamPlayer = _players.get("Music")
	if player == null:
		return false
	# Author-approved recording must provide genuine loop points or real
	# outro; commercial gap remains until crossfade/loop tests pass.
	if player.playing and player.stream == _catalog[cue]:
		player.stream_paused = false
		return true
	player.stop()
	player.stream = _catalog[cue]
	if cue == "menu_music" and player.stream is AudioStreamMP3:
		var looped := player.stream as AudioStreamMP3
		looped.loop = true
	player.stream_paused = false
	player.play()
	return true

func play_ambience(cue: String) -> bool:
	if not _catalog.has(cue) or not [
		"ambient_birds", "ambient_wind", "ambient_water", "ambient_crowd"
	].has(cue) and not cue.begins_with("course_ambient_"):
		return false
	var player: AudioStreamPlayer = _players.get("Ambience")
	if player == null:
		return false
	player.stop()
	player.stream = _catalog[cue]
	player.play()
	return true

func play_ui(cue: String) -> bool:
	if not _catalog.has(cue) or not ["menu_select", "menu_back"].has(cue):
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
	if event.get("confirmed_native", false) != true:
		return false
	var event_id: String = str(event.get("event_id", ""))
	var event_type: String = str(event.get("event_type", ""))
	if event_id.is_empty() or not NATIVE_EVENT_TO_CUE.has(event_type):
		return false
	if _event_dedup.has(event_id):
		return false
	var cue: String = NATIVE_EVENT_TO_CUE[event_type]
	if event_type == "ball_bounce":
		cue = "large_bounce" if int(event.get("rebound_vz_raw",0)) >= 65536 else "small_bounce"
	if event_type == "club_contact" and int(event.get("launch_surface_code",-1)) in [5,6]:
		cue = "rough_club_strike"
	if event_type == "club_contact" and int(event.get("launch_club_index",-1)) == 12:
		cue = "putter_strike"
	if cue == "cup_capture" and _catalog.has("cup_capture_variant") and _ambience_rng.randi_range(0,1) == 1:
		cue = "cup_capture_variant"
	if not _catalog.has(cue): return false
	var player: AudioStreamPlayer = null
	for available in _sfx_pool:
		if not available.playing:
			player = available
			break
	if player == null:
		# Bounded voice stealing accepts new contacts under heavy load.
		player = _sfx_pool.pop_front()
		_sfx_pool.append(player)
		player.stop()
	player.stream = _catalog[cue]
	player.stream_paused = false
	player.play()
	_event_dedup[event_id] = true
	_event_order.append(event_id)
	if _event_order.size() > 4096:
		var oldest: String = _event_order.pop_front()
		_event_dedup.erase(oldest)
	return true

func reset_event_cursor_on_confirmed_new_round() -> void:
	for player in _sfx_pool: player.stop()
	_players["Ambience"].stop()
	_ambience_timer.stop()
	_context = ""
	_event_dedup.clear()
	_event_order.clear()

func stop_all_for_application_pause() -> void:
	_ambience_timer.stop()
	for player in _players.values():
		player.stop()
	for player in _sfx_pool:
		player.stop()

func _exit_tree() -> void:
	stop_all_for_application_pause()
	for player in _players.values(): player.stream = null
	for player in _sfx_pool: player.stream = null
	_catalog.clear()
