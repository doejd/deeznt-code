extends Window
@onready var labels = [$VBoxContainer/Find/Label, $VBoxContainer/Replace/Label2]
@onready var find_input = $VBoxContainer/Find/LineEdit
@onready var replace_input = $VBoxContainer/Replace/LineEdit2
@onready var editor = get_node("../Editor_Container/VSplitContainer2/VSplitContainer/Editor")

func _ready() -> void:
	get_tree().root.size_changed.connect(resize)
	hide()

func _on_close_requested() -> void:
	hide()

func resize() -> void:
	var screen_size = DisplayServer.window_get_size()
	for label in labels:
		label.add_theme_font_size_override("font_size", screen_size.y * 0.015)
	add_theme_font_size_override("title_font_size", screen_size.y * 0.015)
	size = Vector2i(screen_size.x / 4, screen_size.y / 10)
	find_input.add_theme_font_size_override("font_size", screen_size.y * 0.01)
	replace_input.add_theme_font_size_override("font_size", screen_size.y * 0.01)

func _on_line_edit_text_submitted(new_text: String) -> void:
	var res = []
	var lines = editor.get_text().split("\n", true)
	for line_ind in lines.size():
		var col = lines[line_ind].find(new_text)
		if col != -1:
			res.append(Vector2i(line_ind, col))
	if res.is_empty(): return
	editor.grab_focus()
	editor.set_caret_line(res[0].x + 1)
	editor.set_caret_column(res[0].y)
	editor.select(res[0].x, res[0].y, res[0].x, res[0].y + new_text.length(), 0)

func _on_line_edit_2_text_submitted(_new_text: String) -> void:
	pass
