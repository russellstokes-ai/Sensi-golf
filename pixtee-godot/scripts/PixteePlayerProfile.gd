extends RefCounted

# Cosmetic preferences and a record of committed native scores only.
# This journal cannot restore or choose a native shot outcome.
const DEFAULTS := {"display_name":"Player", "outfit_id":"lakewood-classic", "appearance":{"skin":0,"hair":0}, "settings":{"reduced_motion":false,"reduced_flashes":false,"graphics_quality":1,"frame_limit":60,"Music":0.8,"SFX":1.0,"Ambience":1.0,"UI":0.8},"statistics":{"round_sequence":0,"last_recorded":0,"played":0,"strokes":0,"best":0,"last":{}}}
var _state: Dictionary = DEFAULTS.duplicate(true)
var _base := "user://pixtee/player"
var _generation := 0
var _slot := -1

func snapshot() -> Dictionary:
	return _state.duplicate(true)

func begin_edit() -> Dictionary:
	return {"display_name":_state.display_name,"outfit_id":_state.outfit_id,"appearance":_state.appearance.duplicate(true)}

func _valid(data: Dictionary) -> bool:
	if not data.get("display_name") is String or not data.get("outfit_id") is String or not data.get("settings") is Dictionary: return false
	var name: String = data.display_name
	if name.strip_edges() != name or name.is_empty() or name.length() > 32: return false
	for character in name:
		if character.unicode_at(0) < 32: return false
	if not PixteeOutfit.is_valid(data.outfit_id): return false
	if not PixteeOutfit.valid_appearance(data.get("appearance")): return false
	var settings: Dictionary = data.settings
	if settings.size() != DEFAULTS.settings.size(): return false
	for key in DEFAULTS.settings:
		if not settings.has(key): return false
		if key in ["reduced_motion","reduced_flashes"]:
			if not settings[key] is bool: return false
		elif key == "graphics_quality":
			if not _integer(settings[key],0,2): return false
		elif key == "frame_limit":
			if not _integer(settings[key],30,120) or not int(settings[key]) in [30,60,90,120]: return false
		else:
			if not (settings[key] is float or settings[key] is int): return false
			var level := float(settings[key])
			if not is_finite(level) or level < 0.0 or level > 1.0: return false
	var stats = data.get("statistics")
	if not stats is Dictionary or stats.size() != DEFAULTS.statistics.size(): return false
	for key in ["round_sequence","last_recorded","played","strokes","best"]:
		if not _integer(stats.get(key),0,1000000000): return false
	if stats.last_recorded > stats.round_sequence or stats.played > stats.round_sequence: return false
	if stats.played == 0: return stats.strokes == 0 and stats.best == 0 and stats.last is Dictionary and stats.last.is_empty()
	return stats.strokes >= stats.played and stats.last_recorded > 0 and stats.best >= 1 and stats.last is Dictionary and _integer(stats.last.get("strokes"),1,65535) and stats.last.get("par") == 4 and stats.last.get("relative") == stats.last.strokes-4 and stats.best <= stats.last.strokes and stats.last.strokes <= stats.strokes

func _integer(value: Variant, minimum: int, maximum: int) -> bool:
	return (value is int or value is float) and is_finite(float(value)) and value == floor(value) and value >= minimum and value <= maximum

func _read_slot(index: int) -> Dictionary:
	var path := _base+"."+str(index)+".json"
	if not FileAccess.file_exists(path): return {}
	var file := FileAccess.open(path,FileAccess.READ)
	if file == null or file.get_length() > 65536: return {}
	var parser := JSON.new()
	if parser.parse(file.get_as_text()) != OK: return {}
	var envelope = parser.data
	if not envelope is Dictionary or not envelope.get("payload") is String: return {}
	var payload: String = envelope.payload
	if payload.sha256_text() != envelope.get("sha256",""): return {}
	if parser.parse(payload) != OK: return {}
	var record = parser.data
	if not record is Dictionary or not _integer(record.get("schema"),1,3) or not record.get("state") is Dictionary: return {}
	if record.schema == 1:
		# Upgrade the prior cosmetic-only journal without discarding saved names.
		if not record.state.get("settings") is Dictionary: return {}
		var settings: Dictionary = DEFAULTS.settings.duplicate(true)
		settings.merge(record.state.settings,true)
		record.state.settings = settings
		record.state.statistics = DEFAULTS.statistics.duplicate(true)
	if int(record.schema) < 3: record.state.appearance = DEFAULTS.appearance.duplicate(true)
	if not _valid(record.state): return {}
	# JSON numbers decode as floats; expose stable integer palette selections.
	for key in ["skin","hair"]: record.state.appearance[key] = int(record.state.appearance[key])
	var generation = record.get("generation",0)
	if not (generation is int or generation is float) or generation < 1 or generation != floor(generation): return {}
	return record

func load_profile(base := "user://pixtee/player") -> bool:
	if not base.begins_with("user://") or ".." in base: return false
	_base = base
	_generation = 0
	_slot = -1
	_state = DEFAULTS.duplicate(true)
	for index in range(2):
		var record := _read_slot(index)
		if not record.is_empty() and int(record.generation) > _generation:
			_generation = int(record.generation)
			_slot = index
			_state = record.state.duplicate(true)
	return true

func last_slot_path() -> String:
	return _base+"."+str(_slot)+".json"

func _save(candidate: Dictionary) -> bool:
	if not _valid(candidate): return false
	if DirAccess.make_dir_recursive_absolute(ProjectSettings.globalize_path(_base.get_base_dir())) != OK: return false
	var next := 0 if _slot != 0 else 1
	var generation := _generation+1
	var payload := JSON.stringify({"schema":3,"generation":generation,"state":candidate})
	var file := FileAccess.open(_base+"."+str(next)+".json",FileAccess.WRITE)
	if file == null: return false
	file.store_string(JSON.stringify({"payload":payload,"sha256":payload.sha256_text()}))
	file.flush()
	var error := file.get_error()
	file.close()
	if error != OK: return false
	var verified := _read_slot(next)
	if verified.is_empty() or int(verified.generation) != generation: return false
	_state = candidate.duplicate(true)
	_generation = generation
	_slot = next
	return true

func apply_edit(draft: Dictionary) -> bool:
	if not draft.get("display_name") is String: return false
	var candidate := snapshot()
	candidate.display_name = draft.display_name.strip_edges()
	candidate.outfit_id = draft.get("outfit_id","")
	candidate.appearance = draft.get("appearance",{}).duplicate(true) if draft.get("appearance") is Dictionary else {}
	return _save(candidate)

func set_setting(key: String, value: Variant) -> bool:
	if not DEFAULTS.settings.has(key): return false
	var candidate := snapshot()
	candidate.settings[key] = value
	return _save(candidate)

func storage_base() -> String:
	return _base

func begin_attempt() -> int:
	var candidate := snapshot()
	candidate.statistics.round_sequence += 1
	if not _save(candidate): return 0
	return int(candidate.statistics.round_sequence)

func record_committed_result(attempt: int, native: Dictionary) -> bool:
	# The host passes only a snapshot immediately after native score commit.
	if native.get("physics_authority") != "Recovered Sensible Golf ClassicGameSession" or native.get("has_hole",true) != false or native.get("holes_completed") != 1: return false
	var strokes = native.get("total_strokes")
	if not _integer(strokes,1,65535) or native.get("cumulative_par") != 4: return false
	# Source scoreboard stores par minus strokes. Journal/display use the
	# conventional strokes minus par; preserve native counters unchanged.
	if native.get("relative_to_par") != 4-strokes or attempt <= 0 or attempt != int(_state.statistics.round_sequence): return false
	if attempt == int(_state.statistics.last_recorded): return _state.statistics.last.strokes == strokes
	var candidate := snapshot()
	var stats: Dictionary = candidate.statistics
	stats.last_recorded = attempt
	stats.played += 1
	stats.strokes += int(strokes)
	stats.best = int(strokes) if stats.best == 0 else mini(int(stats.best),int(strokes))
	stats.last = {"strokes":int(strokes),"par":int(native.cumulative_par),"relative":-int(native.relative_to_par)}
	return _save(candidate)
