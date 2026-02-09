extends HSlider
@onready var timer : Timer = $"../../../../../../Timer"

func _ready() -> void:
	value = timer.wait_time

func _on_drag_ended(_value_changed: bool) -> void:
	if not _value_changed: return
	timer.wait_time = value
	timer.start()
