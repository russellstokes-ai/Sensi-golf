extends Control
# Engineering harness only. This is NOT the approved game renderer or
# Whack-o-meter. Tests raw recovered C++ input and physics without any
# parallel gameplay simulation in GDScript.
var core: PixteeGameplay
var accumulator := 0.0
const TICK_SECONDS := 936.0 / 65536.0
@onready var trace: RichTextLabel = $Trace
@onready var launch: Button = $Launch

func _ready() -> void:
    core = PixteeGameplay.new()
    launch.pressed.connect(_on_launch)
    if not core.load_lakewood_hole1():
        trace.text = "C++ engine setup failed: " + core.last_error()
        launch.disabled = true
        return
    core.set_aim_raw(2048) # Aim towards authored green, not south off-map
    core.set_club_index(0)
    _show_trace()

func _process(delta: float) -> void:
    if core == null: return
    accumulator += min(delta, 0.25)
    var executed := 0
    while accumulator >= TICK_SECONDS and executed < 16:
        core.step_tick()
        accumulator -= TICK_SECONDS
        executed += 1
    _show_trace()

func _on_launch() -> void:
    # ONLY test vectors. Never relabel these as a live meter scheduler.
    # A true three-click UI must sample the original-derived meter at
    # the authoritative 936/65536s simulation tick.
    var state := core.snapshot()
    if int(state.get("hole_phase", -1)) != 0: return
    if not core.meter_click_raw(0): return
    if not core.meter_click_raw(105): return
    core.meter_click_raw(63)

func _show_trace() -> void:
    var s := core.snapshot()
    trace.text = "[b]PIXTEE / RECOVERED CORE — INTEGRATION HARNESS[/b]\n"
    trace.text += "[color=yellow]NOT THE FINAL PLAYABLE HOLE OR ARTWORK[/color]\n\n"
    trace.text += "Native authority: " + str(s.get("gameplay_authority", "?")) + "\n"
    trace.text += "Ticks: " + str(s.get("tick", "?")) + "\n"
    trace.text += "X, Y, Z: %.3f / %.3f / %.3f\n" % [
        float(s.get("ball_x", 0)), float(s.get("ball_y", 0)),
        float(s.get("ball_z", 0))]
    trace.text += "Strokes: " + str(s.get("strokes", "?")) + "\n"
    trace.text += "Lie code: " + str(s.get("surface_code", "?")) + "\n"
    trace.text += "Hole phase: " + str(s.get("hole_phase", "?")) + "\n"
    trace.text += "Error: " + str(s.get("error", "")) + "\n\n"
    trace.text += "Lakewood terrain is a FIRST-PASS adapter.\n"
    trace.text += "Meter, visuals, course fine geometry and special branches remain gated."
