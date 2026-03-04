extends Button
@onready var editor = $"../../../../../../Editor_Container".find_child("Editor")

func _on_pressed() -> void:
	editor.setup_theme()
	editor.setup_highlighter()
