extends CanvasLayer
## Presentation only. The C++ round continues to own every ball state.
var cover: ColorRect
var busy := false
var swapped := false
var elapsed := 0.0
var swap_action: Callable
func _ready() -> void:
	layer = 80
	cover = ColorRect.new()
	cover.color = Color(0.025,0.06,0.045,0.0)
	cover.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	add_child(cover)
	cover.hide()
func begin(action: Callable) -> void:
	if busy: return
	busy = true
	swapped = false
	elapsed = 0.0
	swap_action = action
	cover.color.a = 0.0
	cover.mouse_filter = Control.MOUSE_FILTER_STOP
	cover.show()
func advance(delta: float) -> void:
	if not busy: return
	elapsed += maxf(0.0,delta)
	if elapsed < 1.0:
		cover.color.a = 0.0
	elif elapsed < 1.32:
		cover.color.a = smoothstep(1.0,1.32,elapsed)
	else:
		cover.color.a = 1.0
		_swap_once()
		if elapsed >= 1.4: cover.color.a = 1.0-smoothstep(1.4,1.82,elapsed)
		if elapsed >= 1.82: cancel_discard()
func _swap_once() -> void:
	if swapped: return
	swapped = true
	if swap_action.is_valid(): swap_action.call()
func finish_immediately() -> void:
	if busy: _swap_once()
	cancel_discard()
func cancel_discard() -> void:
	busy = false
	swap_action = Callable()
	if cover != null:
		cover.color.a = 0.0
		cover.hide()
		cover.mouse_filter = Control.MOUSE_FILTER_IGNORE
