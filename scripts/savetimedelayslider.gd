extends HSlider
@export var modifies_timer : String = ""
@onready var timer : Timer = $"../../../../../../Timer"
@onready var reload_timer : Timer = $"../../../../../../Reload Timer"

func _on_drag_ended(_value_changed: bool) -> void:
	if not _value_changed: return
	if modifies_timer == "reload_timer_delay":
		reload_timer.wait_time = value
		reload_timer.start()
	elif modifies_timer == "save_timer_delay":
		timer.wait_time = value
		timer.start()
	$"../../../../../..".SettingManager.timer_map[modifies_timer] = value

func _on_control_startup(_should_load_last_project: Variant) -> void:
	if modifies_timer == "reload_timer_delay": value = reload_timer.wait_time
	elif modifies_timer == "save_timer_delay": value = timer.wait_time
