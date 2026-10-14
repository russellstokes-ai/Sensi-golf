extends Node
class_name PixteeWeatherAudioAdapter

## Optional weather-only ambience mixer; disabled without independently
## licensed/provided weather loops. Does NOT replace PixteeCommercialAudio.
## Reuses game's Ambience bus when supplied. Never ship placeholder tracks.

var weather: PixteeWeatherDirector
var enabled: bool = false
var ambience_gain: float = 0.55
var _rain: AudioStreamPlayer
var _wind: AudioStreamPlayer

func _ready() -> void:
	_rain=AudioStreamPlayer.new()
	_rain.name="RainLoopOptional"
	_wind=AudioStreamPlayer.new()
	_wind.name="WindLoopOptional"
	for player in [_rain,_wind]:
		player.bus = "Ambience" if AudioServer.get_bus_index("Ambience")>=0 else "Master"
		add_child(player)
		player.volume_db=-80.0

func supply_approved_loop(stream: AudioStream, kind: String) -> bool:
	if stream==null:
		return false
	var player: AudioStreamPlayer = _rain if kind=="rain" else (_wind if kind=="wind" else null)
	if player==null:
		return false
	player.stop()
	player.stream=stream
	return true

func _process(delta: float) -> void:
	if weather==null:
		return
	var c: Dictionary=weather.conditions()
	var r: float = pow(float(c.get("rain",0.0)),1.15)*ambience_gain if enabled else 0.0
	var w: float = float(c.get("wind",0.0))*0.50*ambience_gain if enabled else 0.0
	_update_player(_rain,r,delta)
	_update_player(_wind,w,delta)

func _update_player(player: AudioStreamPlayer, target_gain: float, dt: float) -> void:
	if player==null or player.stream==null:
		return
	if target_gain>0.002 and not player.playing:
		player.play()
	var target_db: float = linear_to_db(maxf(target_gain,0.0001))
	player.volume_db=move_toward(player.volume_db,target_db,60.0*clampf(dt,0.0,0.1))
	if target_gain<=0.002 and player.volume_db<=-70.0 and player.playing:
		player.stop()
