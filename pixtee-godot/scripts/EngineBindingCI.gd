extends SceneTree
# CI ENGINE INTEGRATION CHECK: Godot invokes native C++ restored Sensible Golf.
# No sprites, no fake math, no copyrighted course files.
func _initialize() -> void:
    var core = PixteeGameplay.new()
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
    var round_host = PixteeRoundBridge.new()
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
