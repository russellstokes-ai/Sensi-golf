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
    var req = round_host.requested_course()
    if int(req.get("resource_id",0)) != 1:
        push_error("Original round controller did not request hole one")
        quit(1)
        return
    print("PASS Godot 4.5 native CLASS LOAD: C++ ball physics, raw swing, ClassicGameSession, 18-hole requests")
    quit(0)
