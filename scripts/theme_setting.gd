extends ColorPickerButton
@export var key_referring_to : String = ""
@onready var editor = $"../../../../../../Editor_Container/VSplitContainer/VSplitContainer/Editor"

func _ready() -> void:
	var merged : Dictionary = editor.keywords.duplicate()
	for key in editor.GUI.keys(): merged[key] = editor.GUI[key]
	if !merged.has(key_referring_to): return
	color = merged[key_referring_to]
	editor.setup_highlighter()

func _on_color_changed(new_color: Color) -> void:
	if editor.keywords.has(key_referring_to):
		editor.keywords[key_referring_to] = new_color
	elif editor.GUI.has(key_referring_to):
		editor.GUI[key_referring_to] = new_color
	else: return

func _on_theme_change(_theme: Variant) -> void:
	var merged = editor.keywords.duplicate()
	for key in editor.GUI.keys(): merged[key] = editor.GUI[key]
	if !merged.has(key_referring_to): return
	color = merged[key_referring_to]
