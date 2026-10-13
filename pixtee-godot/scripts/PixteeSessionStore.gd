extends RefCounted
var generation := 0
var base := "user://pixtee/session"

func load_current(course_hash: String) -> Dictionary:
	var latest: Dictionary = {}
	for slot in range(2):
		var path := base+".%d.json" % slot
		if not FileAccess.file_exists(path): continue
		var file := FileAccess.open(path,FileAccess.READ)
		if file == null or file.get_length()>20*1024*1024: continue
		var parser := JSON.new()
		if parser.parse(file.get_as_text()) != OK: continue
		var envelope: Variant = parser.data
		if not envelope is Dictionary or not envelope.get("payload") is String: continue
		if envelope.payload.sha256_text() != envelope.get("sha256",""): continue
		if parser.parse(envelope.payload) != OK: continue
		var record: Variant = parser.data
		if not record is Dictionary or record.get("schema",0) != 1 or record.get("course_hash","") != course_hash or not record.get("native") is String: continue
		if not record.get("generation") is float and not record.get("generation") is int: continue
		if not record.get("attempt") is float and not record.get("attempt") is int: continue
		if not record.get("pending_press") is bool or record.generation < 1 or record.attempt < 1 or record.generation != floor(record.generation) or record.attempt != floor(record.attempt): continue
		if latest.is_empty() or record.generation > latest.generation: latest = record
	if not latest.is_empty(): generation = int(latest.generation)
	return latest

func save(native: String,course_hash: String,attempt: int,pending_press: bool) -> bool:
	if native.is_empty() or attempt<1: return false
	if DirAccess.make_dir_recursive_absolute(ProjectSettings.globalize_path(base.get_base_dir())) != OK: return false
	var next := generation+1
	var payload := JSON.stringify({"schema":1,"generation":next,"native":native,"course_hash":course_hash,"attempt":attempt,"pending_press":pending_press})
	var path := base+".%d.json" % (next%2)
	var file := FileAccess.open(path,FileAccess.WRITE)
	if file == null: return false
	file.store_string(JSON.stringify({"payload":payload,"sha256":payload.sha256_text()}))
	file.flush()
	var error := file.get_error()
	file.close()
	if error != OK: return false
	var restored := load_current(course_hash)
	return not restored.is_empty() and int(restored.generation)==next

func clear() -> void:
	for slot in range(2):
		var path := base+".%d.json" % slot
		if FileAccess.file_exists(path): DirAccess.remove_absolute(ProjectSettings.globalize_path(path))
