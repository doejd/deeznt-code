extends Window
@onready var labels = [$VBoxContainer/Find/Label, $"VBoxContainer/Replace All/Label2", $"VBoxContainer/Replace selected/Label"]
@onready var find_input = $VBoxContainer/Find/HBoxContainer/LineEdit
@onready var replace_selected_input = $"VBoxContainer/Replace selected/LineEdit"
@onready var replace_input = $"VBoxContainer/Replace All/LineEdit2"
@onready var editor = get_node("../Editor_Container/VSplitContainer2/VSplitContainer/Editor")
@onready var main = get_node("..")
@onready var buttons = [$"VBoxContainer/Find/HBoxContainer/Up Arrow", $"VBoxContainer/Find/HBoxContainer/Down Arrow"]
var all_matches = []
var cur_selected_match = 0

func _ready() -> void:
	get_tree().root.size_changed.connect(resize)
	hide()

func _on_close_requested() -> void:
	hide()
	main.find_replace_wind_open = false

func resize() -> void:
	var screen_size = DisplayServer.window_get_size()
	for label in labels:
		label.add_theme_font_size_override("font_size", screen_size.y * 0.015)
	for button in buttons:
		button.add_theme_font_size_override("font_size", screen_size.y * 0.015)
	add_theme_font_size_override("title_font_size", screen_size.y * 0.015)
	size = Vector2i(screen_size.x / 4, screen_size.y / 7)
	position = (screen_size - size) / 2
	find_input.add_theme_font_size_override("font_size", screen_size.y * 0.01)
	replace_selected_input.add_theme_font_size_override("font_size", screen_size.y * 0.01)
	replace_input.add_theme_font_size_override("font_size", screen_size.y * 0.01)

func _on_line_edit_text_submitted(new_text: String) -> void:
	all_matches.clear()
	cur_selected_match = 0
	var lines = editor.get_text().split("\n", true)
	for line_ind in lines.size():
		var col = lines[line_ind].find(new_text)
		if col != -1:
			all_matches.append(Vector3i(line_ind, col, new_text.length()))
	if all_matches.is_empty(): return
	editor.grab_focus()
	editor.set_caret_line(all_matches[cur_selected_match].x)
	editor.set_caret_column(all_matches[cur_selected_match].y)
	editor.select(all_matches[0].x, all_matches[0].y, all_matches[0].x, all_matches[0].y + all_matches[0].z, 0)

func _on_line_edit_2_text_submitted(new_text: String) -> void:
	if find_input.get_text() == "": return 
	var full_text = editor.get_text()
	full_text = full_text.replace(find_input.get_text(), new_text)
	editor.set_text(full_text)
	
func _on_line_edit3_text_submitted(new_text: String) -> void:
	if editor.has_selection():
		editor.delete_selection()
		editor.insert_text_at_caret(new_text)
	
func _on_up_arrow_pressed() -> void:
	if all_matches.is_empty(): return
	cur_selected_match -= 1
	cur_selected_match = clamp(cur_selected_match, 0, all_matches.size() - 1)
	editor.grab_focus()
	editor.set_caret_line(all_matches[cur_selected_match].x)
	editor.set_caret_column(all_matches[cur_selected_match].y)
	editor.select(all_matches[cur_selected_match].x, all_matches[cur_selected_match].y, all_matches[cur_selected_match].x, all_matches[cur_selected_match].y + all_matches[cur_selected_match].z, 0)

func _on_down_arrow_pressed() -> void:
	if all_matches.is_empty(): return
	cur_selected_match += 1
	cur_selected_match = clamp(cur_selected_match, 0, all_matches.size() - 1)
	editor.grab_focus()
	editor.set_caret_line(all_matches[cur_selected_match].x)
	editor.set_caret_column(all_matches[cur_selected_match].y)
	editor.select(all_matches[cur_selected_match].x, all_matches[cur_selected_match].y, all_matches[cur_selected_match].x, all_matches[cur_selected_match].y + all_matches[cur_selected_match].z, 0)
