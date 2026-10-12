extends RefCounted

# Cosmetic data only. Native round state never enters this journal.
const DEFAULTS := {"display_name":"Player", "outfit_id":"lakewood-classic", "settings":{"reduced_motion":false,"Music":0.8,"SFX":1.0,"Ambience":0.7,"UI":0.8}}
var _state: Dictionary = DEFAULTS.duplicate(true)
var _base := "user://pixtee/player"
var _generation := 0
var _slot := -1

func snapshot() -> Dictionary:
	return _state.duplicate(true)

func begin_edit() -> Dictionary:
	return {"display_name":_state.display_name,"outfit_id":_state.outfit_id}

func _valid(data: Dictionary) -> bool:
	if not data.get("display_name") is String or not data.get("outfit_id") is String or not data.get("settings") is Dictionary: return false
	var name: String = data.display_name
	if name.strip_edges() != name or name.is_empty() or name.length() > 32: return false
	for character in name:
		if character.unicode_at(0) < 32: return false
	if data.outfit_id != "lakewood-classic": return false
	var settings: Dictionary = data.settings
	if settings.size() != DEFAULTS.settings.size(): return false
	for key in DEFAULTS.settings:
		if not settings.has(key): return false
		if key == "reduced_motion":
			if not settings[key] is bool: return false
		else:
			if not (settings[key] is float or settings[key] is int): return false
			var level := float(settings[key])
			if not is_finite(level) or level < 0.0 or level > 1.0: return false
	return true

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
	if not record is Dictionary or record.get("schema") != 1 or not record.get("state") is Dictionary: return {}
	if not _valid(record.state): return {}
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
	var payload := JSON.stringify({"schema":1,"generation":generation,"state":candidate})
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
	return _save(candidate)

func set_setting(key: String, value: Variant) -> bool:
	if not DEFAULTS.settings.has(key): return false
	var candidate := snapshot()
	candidate.settings[key] = value
	return _save(candidate)
