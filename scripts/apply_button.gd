extends Button
@onready var editor = $"../../../../../../Editor_Container/VSplitContainer/VSplitContainer/Editor"

func _on_pressed() -> void:
	editor.setup_theme()
	editor.setup_highlighter()
