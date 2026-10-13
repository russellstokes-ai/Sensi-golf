extends SceneTree

func _initialize() -> void:
	call_deferred("_run")

func check(condition: bool, message: String) -> bool:
	if not condition:
		push_error("OWNER AUDIO: "+message)
		quit(1)
	return condition

func _run() -> void:
	var audio := PixteeCommercialAudio.new()
	root.add_child(audio)
	if not check(audio.load_owner_pack().is_empty(), "All 20 MP3s must decode"): return
	if not check(audio._catalog.size() == 20, "All supplied recordings must be indexed"): return
	if not check(not audio.is_fully_populated(), "Pack ingestion must not certify commercial completeness"): return
	audio.set_context("menu")
	var music: AudioStreamPlayer = audio._players["Music"]
	var ambience: AudioStreamPlayer = audio._players["Ambience"]
	if not check(music.playing and (music.stream as AudioStreamMP3).loop, "Menu theme must play and loop"): return
	audio.set_context("course")
	if not check(not music.playing and not audio._ambience_timer.is_stopped(), "Course starts intermittent ambience without gameplay music"): return
	if not check(absf(db_to_linear(ambience.volume_db)-0.75)<0.0001, "Nature recordings at 75 percent base volume"): return
	audio._ambience_timer.stop()
	audio._next_course_ambience()
	if not check(ambience.playing, "Selected nature clip plays"): return
	ambience.stop()
	audio._schedule_course_ambience()
	if not check(audio._ambience_timer.time_left>=8.0 and audio._ambience_timer.time_left<=20.0, "Quiet gap after completed clip"): return
	audio._ambience_timer.stop()
	audio._next_course_ambience()
	var event := {"confirmed_native":true,"event_id":"tree:1","event_type":"authored_tree_contact"}
	if not check(audio.consume_confirmed_native_event(event), "Real authored tree contact audible"): return
	if not check(not audio.consume_confirmed_native_event(event), "No repeated collision sound"): return
	event = {"confirmed_native":true,"event_id":"flag:1","event_type":"verified_flag_contact"}
	if not check(audio.consume_confirmed_native_event(event), "Native flag contact audible"): return
	event = {"confirmed_native":false,"event_id":"bird:1","event_type":"authored_bird_contact"}
	if not check(not audio.consume_confirmed_native_event(event), "Cosmetic birds cannot invent hits"): return
	audio.set_context("menu")
	if not check(ambience.stream_paused and music.playing, "Pause music and frozen ambience"): return
	audio.set_context("background")
	if not check(music.stream_paused, "Background app must be silent"): return
	audio.set_context("menu")
	if not check(not music.stream_paused, "Focus restoration resumes menu audio"): return
	audio.set_context("course")
	if not check(not ambience.stream_paused, "Resume course ambience"): return
	if not check(audio.consume_confirmed_native_event({"confirmed_native":true,"event_id":"cup:1","event_type":"cup_capture"}), "Native cup capture audible"): return
	audio.set_context("results")
	var cup_finished_across_score := false
	for voice in audio._sfx_pool:
		if voice.stream == audio._catalog["cup_capture"]:
			cup_finished_across_score = voice.playing and not voice.stream_paused
	if not check(cup_finished_across_score and not music.playing, "Score must let the cup sound finish without extra music"): return
	if not check(audio.set_channel_level("SFX",0.0) and AudioServer.is_bus_mute(AudioServer.get_bus_index("SFX")), "Independent mute"): return
	for i in range(16):
		if not check(audio.consume_confirmed_native_event({"confirmed_native":true,"event_id":"impact:%d"%i,"event_type":"fairway_contact"}), "Bounded pool must accept new impacts"): return
	audio.stop_all_for_application_pause()
	print("Owner audio PASS: 20 recordings decoded; context, loops, native routing, dedupe, mute and bounded voices")
	quit(0)
