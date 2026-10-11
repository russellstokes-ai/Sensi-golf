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
    var req = round_host.requested_course()
    if int(req.get("resource_id",0)) != 1:
        push_error("Original round controller did not request hole one")
        quit(1)
        return
    print("PASS Godot 4.5 native CLASS LOAD: C++ ball physics, raw swing, ClassicGameSession, 18-hole requests")
    quit(0)
