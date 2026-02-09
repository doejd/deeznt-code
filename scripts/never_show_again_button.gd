extends CheckBox
@onready var control : Control = $"../../../.."

func _on_control_on_load_intro_window(show_: Variant) -> void:
	button_pressed = not show_

func _on_pressed() -> void:
	control.intro_wind_popup = not pressed
