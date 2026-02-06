extends Window
@onready var labels : Array = [$"VBoxContainer/Never Show Again Dialog/CheckBox"]

func _ready() -> void:
	grab_focus()
	get_tree().root.size_changed.connect(center_resize)
	for child in find_children("*", "Label", true): labels.append(child)

func center_resize() -> void:
	var screen_size = DisplayServer.window_get_size()
	size = Vector2i(round(screen_size.x) / 3, round(screen_size.y) / 3)
	position = (round(screen_size - size)) / 2

func _input(_event : InputEvent):
	if not Input.is_action_pressed("Increase Font Size") or not Input.is_action_pressed("Decrease Font Size"): return;

func _on_close_requested() -> void:
	hide()

func _on_control_change_font_size(font_size: Variant) -> void:
	for label in labels: label.add_theme_font_size_override("font_size", font_size)
	add_theme_font_size_override("title_font_size", font_size)

func _on_control_on_load_intro_window(show_: Variant) -> void:
	if not show_: hide()
