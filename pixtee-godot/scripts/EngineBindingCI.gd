extends SceneTree
# CI ENGINE INTEGRATION CHECK: Godot invokes native C++ restored Sensible Golf.
# No sprites, no fake math, no copyrighted course files.
func _initialize() -> void:
    # Headless --script may parse before the Godot project importer scans .gdextension.
    # Load the actual extension explicitly, then instantiate by registered class name.
    var native_extension = load("res://bin/pixtee.gdextension")
    if native_extension == null or not ClassDB.class_exists("PixteeGameplay"):
        push_error("Godot could not load recovered native gameplay library")
        quit(1)
        return
    var core = ClassDB.instantiate("PixteeGameplay")
    if not core.load_lakewood_hole1():
        push_error("Cannot instantiate native recovered C++ core: " + core.last_error())
        quit(1)
        return
    var snapshot = core.snapshot()
    if not bool(snapshot.get("ready", false)):
        push_error("Native simulation did not return a usable state")
        quit(1)
        return
    if not core.set_aim_raw(2048) or not core.set_club_index(0):
        push_error("Raw heading or original club assignment failed")
        quit(1)
        return
    if not core.meter_click_raw(0) or not core.meter_click_raw(105) or not core.meter_click_raw(63):
        push_error("Recovered 3-step input controller rejected known test-vector input")
        quit(1)
        return
    snapshot = core.snapshot()
    if int(snapshot.get("strokes",-1)) != 1:
        push_error("Native gameplay did not record exactly one original shot")
        quit(1)
        return
    if not ClassDB.class_exists("PixteeRoundBridge"):
        push_error("Godot native round class not registered")
        quit(1)
        return
    var round_host = ClassDB.instantiate("PixteeRoundBridge")
    var order = PackedInt32Array()
    var pars = PackedInt32Array()
    pars.resize(100)
    for i in range(18):
        order.push_back(i+1)
        pars[i+1] = 4
    if not round_host.begin_round(order, pars, 0x1234, 0xabcd):
        push_error("Recovered native 18-hole round initialization failed: " + round_host.last_error())
        quit(1)
        return
    # Verified original machine-code multiplayer choice through real Godot ABI.
    # Metric 17 ties favor last eligible player. Eliminated player skipped.
    var chosen = round_host.original_next_player_choice(
        4, 0,
        PackedInt32Array([17, 17, 17, 3]),
        PackedInt32Array([0, 0, 1, 0]),
        PackedInt32Array([0, 1, 2, 3])
    )
    if chosen != 1:
        push_error("Original x86 multiplayer chooser gave unexpected selection: " + str(chosen))
        quit(1)
        return
    var forced = round_host.original_next_player_choice(
        4, 1,
        PackedInt32Array([17, 17, 17, 3]),
        PackedInt32Array([0, 0, 1, 0]),
        PackedInt32Array([0, 1, 2, 3])
    )
    if forced != 3:
        push_error("Original x86 forced multiplayer chooser failed")
        quit(1)
        return
    # Source x86 0x40C665..0x40C6B2 original CPU difficulty fields.
    var cpu = round_host.original_cpu_search_parameters(6, false)
    if not cpu.get("verified",false) or int(cpu.get("candidate_stride",-1)) != 3 or int(cpu.get("variation_budget",-1)) != 0 or int(cpu.get("search_budget",-1)) != 106:
        push_error("Original CPU search parameter ABI parity failure")
        quit(1)
        return
    # Original Windows 0x40CCA4 CPU simulation shot force (not actual AI).
    var trial_force = round_host.original_cpu_trial_force(12,200,300,20)
    if not trial_force.get("verified",false) or int(trial_force.get("vertical_force",-1)) != 440 or int(trial_force.get("horizontal_force",-1)) != 540 or bool(trial_force.get("complete_cpu_shot_planner",true)):
        push_error("Original CPU trial velocity initialization ABI failed")
        quit(1)
        return
    var wrapped_trial = round_host.original_cpu_trial_force(-1,0,0,65535)
    if int(wrapped_trial.get("vertical_force",0)) != -65535 or int(wrapped_trial.get("horizontal_force",0)) != -65535:
        push_error("Original CPU shot trial 32-bit wrapping mismatch")
        quit(1)
        return
    # Genuine v1.014 24-row CPU search table, backed by actual x86 parity.
    var source_grid = round_host.original_cpu_scan_grid(4, 0)
    if not source_grid.get("verified",false) or int(source_grid.get("candidate_count",-1)) != 32 or int(source_grid.get("first_angular_extent",-1)) != 512 or int(source_grid.get("spacing",-1)) != 4 or int(source_grid.get("initial_remainder",-1)) != 128:
        push_error("Original CPU scan grid data or arithmetic mismatch in Godot")
        quit(1)
        return
    var source_grid_far = round_host.original_cpu_scan_grid(9, 3)
    if not source_grid_far.get("verified",false) or int(source_grid_far.get("candidate_count",-1)) != 35 or int(source_grid_far.get("second_angular_extent",-1)) != 116:
        push_error("Original CPU scan table highest proven source band failed")
        quit(1)
        return
    # Real source 0x40C71B..0x40C77F: all 32 candidate directions
    # for mode4/band0; 0,32,64,...,992, final angle1024.
    var ai_sweep = round_host.original_cpu_heading_sweep(4,0,512)
    var ai_angles = ai_sweep.get("headings",PackedInt32Array())
    if not ai_sweep.get("verified",false) or ai_angles.size()!=32 or ai_angles[0]!=0 or ai_angles[31]!=992 or int(ai_sweep.get("final_ball_heading",-1))!=1024 or int(ai_sweep.get("final_scan_counter",-1))!=0:
        push_error("Original CPU 32-angle heading sweep mismatch in native Godot")
        quit(1)
        return
    # The SHA-verified original Windows penalty table at 0x41FADF is
    # exposed by native code, not hand-copied into GDScript gameplay.
    var original_penalties = round_host.original_cpu_reference_penalties()
    if original_penalties.size() != 32 or original_penalties[0] != 200 or original_penalties[10] != 1000 or original_penalties[31] != 0:
        push_error("Original CPU built-in 32-WORD terrain penalty data missing")
        quit(1)
        return
    var original_course_score = round_host.original_cpu_quality_score(
        50,10,0,original_penalties)
    if not original_course_score.get("verified",false) or int(original_course_score.get("score_word",-1)) != 1050:
        push_error("Original CPU source penalty-table score parity failed")
        quit(1)
        return
    # Source x86 0x40C9FD: authentic CPU quality uses distance plus
    # source penalty WORD unless original green-mode == 1. Regression
    # passes a synthetic penalty table ONLY to verify the ABI/formula.
    var cpu_penalties = PackedInt32Array()
    cpu_penalties.resize(32)
    cpu_penalties[8] = 0x9000
    var cpu_score = round_host.original_cpu_quality_score(
        100,8,0,cpu_penalties)
    if not cpu_score.get("verified",false) or int(cpu_score.get("score_word",-1)) != 0x9064 or bool(cpu_score.get("complete_cpu_shot_planner",true)):
        push_error("Original CPU distance plus penalty scoring ABI failed")
        quit(1)
        return
    var green_score = round_host.original_cpu_quality_score(
        100,8,1,cpu_penalties)
    if int(green_score.get("score_word",-1)) != 100:
        push_error("Original CPU green-mode penalty bypass failed")
        quit(1)
        return
    if round_host.original_cpu_quality_score(100,32,0,cpu_penalties).get("verified",true):
        push_error("Original CPU score must reject unknown contact index")
        quit(1)
        return
    # Source x86 0x40CA8E: lowest signed score, earliest tie, only
    # with authentic ordered 14-byte records from the evaluator.
    # These synthetic regression records exercise the native contract;
    # they do NOT claim that the original trajectory scorer is recovered.
    var score_records = PackedByteArray()
    score_records.resize(32*14)
    for i in range(32):
        score_records[i*14+12] = 100
    score_records[5*14+12] = 248  # -8 as signed 16-bit
    score_records[5*14+13] = 255
    score_records[6*14+12] = 248
    score_records[6*14+13] = 255
    var scored = round_host.original_cpu_scored_record_choice(
        4,0,512,96,3,score_records,true)
    if not scored.get("verified",false) or not scored.get("found",false) or int(scored.get("original_index",-1)) != 5 or int(scored.get("heading",-1)) != 160 or int(scored.get("original_score_word",-1)) != 65528 or bool(scored.get("complete_cpu_shot_planner",true)):
        push_error("Original CPU source signed score record/heading join failed")
        quit(1)
        return
    var unverified = round_host.original_cpu_scored_record_choice(
        4,0,512,96,3,score_records,false)
    if unverified.get("verified",true):
        push_error("Original CPU source must reject invented candidate scores")
        quit(1)
        return
    var ai_pick = round_host.original_cpu_candidate_choice(
        PackedInt32Array([0, 1, -1]),
        PackedInt32Array([0, 0, 0]),
        PackedInt32Array([30, 11, 1])
    )
    if ai_pick != 1:
        push_error("Original x86 computer candidate selection failed")
        quit(1)
        return
    var cpu_noise = round_host.original_cpu_shot_variation(
        false, 50, 3, 99, 2, 4
    )
    if not cpu_noise.get("verified", false) or int(cpu_noise.get("captured_power", -1)) != 54 or int(cpu_noise.get("difficulty_variation", -1)) != 6 or int(cpu_noise.get("swing_adjuster", -999)) != -2 or int(cpu_noise.get("random_calls", -1)) != 3:
        push_error("Original computer shot stochastic branch failed native ABI test")
        quit(1)
        return
    var score_adjust = round_host.original_turn_score_adjust(
        3,
        PackedInt32Array([4,4,7]),
        PackedInt32Array([10,11,14]),
        PackedInt32Array([0,0,0]),
        PackedInt32Array([1,0,1])
    )
    if not score_adjust.get("verified",false) or int(score_adjust.get("corrected_index",-1)) != 1 or score_adjust.get("strokes",PackedInt32Array())[1] != 3 or score_adjust.get("totals",PackedInt32Array())[1] != 10:
        push_error("Source-verifed original turn-scoring correction failed Godot ABI")
        quit(1)
        return
    var tied = round_host.original_turn_score_adjust(
        3,
        PackedInt32Array([4,4,7]),
        PackedInt32Array([10,11,14]),
        PackedInt32Array([1,0,0]),
        PackedInt32Array([1,0,1])
    )
    if int(tied.get("corrected_index",-2)) != -1:
        push_error("Source verified tie/marked player score correction must be skipped")
        quit(1)
        return
    var comp = round_host.original_competition_turn_choice(
        4, 0, 0, 1, 0, 0,
        PackedInt32Array([0,1,2,3]),
        PackedInt32Array([0,0])
    )
    if not comp.get("verified",false) or int(comp.get("selected_slot",-1)) != 1 or int(comp.get("source_slot",-1)) != 0 or int(comp.get("ball_array",-1)) != 1 or int(comp.get("active_side",-1)) != 1:
        push_error("Original competition mode4/6 turn-owner source state mismatch")
        quit(1)
        return
    # Native multiplayer must be a real Godot class: two isolated player
    # rounds, explicit source turn selection, and separated original strokes.
    if not ClassDB.class_exists("PixteeMultiRoundBridge"):
        push_error("Native original multiplayer bridge was not registered")
        quit(1)
        return
    var multi = ClassDB.instantiate("PixteeMultiRoundBridge")
    if not multi.begin_multi_round(
        2, order, pars,
        PackedInt32Array([0x1234,0x5678]),
        PackedInt32Array([0xabcd,0x1234])):
        push_error("Native 2-player original-rule session failed: " + multi.last_error())
        quit(1)
        return
    var m_map = PackedByteArray()
    m_map.resize(0x60 + 4*128*2)
    m_map[0x54] = 0
    m_map[0x55] = 4
    m_map[0x56] = 0
    m_map[0x57] = 128
    var m_spt = PackedByteArray()
    m_spt.resize(50)
    m_spt[46] = 0
    m_spt[47] = 64
    var m_desc = PackedByteArray()
    m_desc.resize(8)
    m_desc[1] = 7 # original named code8 green descriptor
    var m_selector = PackedByteArray()
    m_selector.resize(8)
    if not multi.register_course_resource(
        1,m_map,m_spt,m_desc,m_selector,PackedByteArray()) or        not multi.load_next_hole_for_everyone():
        push_error("Original format course or parallel multiplayer hole load failed: " + multi.last_error())
        quit(1)
        return
    if not multi.propose_original_turn(
        0,PackedInt32Array([3,7]),PackedInt32Array([0,0]),
        PackedInt32Array([0,1])):
        push_error("Source original multiplayer turn selection failed: " + multi.last_error())
        quit(1)
        return
    var waiting = multi.snapshot()
    if not bool(waiting.get("turn_switch_pending",false)) or int(waiting.get("active_owner",-2)) != -1:
        push_error("Original turn must not become active until acknowledged")
        quit(1)
        return
    if multi.propose_original_turn(
        0,PackedInt32Array([3,7]),PackedInt32Array([0,0]),
        PackedInt32Array([0,1])):
        push_error("Original multiplayer accepted duplicate unacknowledged turn")
        quit(1)
        return
    if multi.acknowledge_original_turn() != 1:
        push_error("Original multiplayer active owner was not second player")
        quit(1)
        return
    if not multi.set_club_index(12) or        not multi.meter_click_raw(0) or        not multi.meter_click_raw(30) or        not multi.meter_click_raw(105):
        push_error("Original multiplayer raw stroke input failed: " + multi.last_error())
        quit(1)
        return
    var two_scores = multi.snapshot()
    var hole_strokes = two_scores.get("hole_strokes",PackedInt32Array())
    if hole_strokes.size()!=2 or hole_strokes[0]!=0 or hole_strokes[1]!=1:
        push_error("Original multiplayer round leaked ball or stroke state between players: " + str(hole_strokes))
        quit(1)
        return
    if not multi.propose_original_turn(
        0,PackedInt32Array([10,7]),PackedInt32Array([0,0]),
        PackedInt32Array([0,1])) or multi.acknowledge_original_turn()!=0:
        push_error("Original multiplayer failed handoff back to first player")
        quit(1)
        return
    var req = round_host.requested_course()
    if int(req.get("resource_id",0)) != 1:
        push_error("Original round controller did not request hole one")
        quit(1)
        return
    print("PASS Godot 4.5 native CLASS LOAD: C++ ball physics, raw swing, ClassicGameSession, 18-hole requests")
    quit(0)
