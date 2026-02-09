extends Window
@onready var labels : Array = [$"VBoxContainer/Never Show Again Dialog/CheckBox"]

func _ready() -> void:
	grab_focus()
	connect("close_requested", hide)
	for child in find_children("*", "Label", true): labels.append(child)

func _input(_event : InputEvent):
	if not Input.is_action_pressed("Increase Font Size") or not Input.is_action_pressed("Decrease Font Size"): return;

func _on_control_on_load_intro_window(show_: Variant) -> void:
	if not show_: hide()
	else: show()
