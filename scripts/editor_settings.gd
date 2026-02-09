extends VBoxContainer
@onready var editor : CodeEdit = $"../../../Editor_Container/VSplitContainer/VSplitContainer/Editor"
var theme_nodes = []

func _ready() -> void:
	var setting_window = $"../.."
	setting_window.connect("close_requested", setting_window.hide)

func _on_general_button_pressed() -> void:
	for pannel in get_child_count() - 1: get_child(pannel).visible = false
	find_child("General Settings", false).visible = true

func _on_editor_button_pressed() -> void:
	for pannel in get_child_count() - 1: get_child(pannel).visible = false
	find_child("Editor Settings", false).visible = true

func _on_themes_button_pressed() -> void:
	for pannel in get_child_count() - 1: get_child(pannel).visible = false
	find_child("Theme Settings", false).visible = true

func _on_advanced_button_pressed() -> void:
	for pannel in get_child_count() - 1: get_child(pannel).visible = false
	find_child("Advanced Settings", false).visible = true

func _on_reset_to_defaults_button_pressed() -> void:
	pass # Replace with function body.
