extends Control
@onready var editor = $Editor
@onready var item_list = $ItemList
@onready var icons = Icons.new()
var is_open_file_picker = false
var cur_opened_file = ""
var dir = DirAccess.open(OS.get_environment("USERPROFILE"))
var cur_ind = 0

func get_extension(stri : String) -> String:
	var dot_index = stri.rfind(".")
	if dot_index != -1 and dot_index < stri.length() - 1:
		return stri.substr(dot_index + 1)
	return ""

func get_dir_contents() -> Array:
	var items = []
	if dir:
		dir.list_dir_begin()
		var file_name = dir.get_next()
		while file_name != "":
			items.append(file_name)
			file_name = dir.get_next()
		return items
	else:
		print("An error has been encountered")
		return []
		
func _refocus_editor():
	editor.grab_focus()
	editor.set_caret_line(0)
	editor.set_caret_column(0)
	editor.set_caret_blink_enabled(true)
		
func _ready():
	editor.editable = false
	diplay_items(get_dir_contents())
	item_list.select(cur_ind)

func diplay_items(items: Array) -> void:
	item_list.clear()
	item_list.add_item("..")
	for item in items:
		if DirAccess.open(dir.get_current_dir().path_join(item)):
			item_list.add_item(" " + item)
		else:
			item_list.add_item(icons.get_icon_data(get_extension(item)) + " " + item)

func _input(_event: InputEvent) -> void:
	if Input.is_action_just_pressed("ui_open") and not is_open_file_picker:
		item_list.visible = true
		editor.editable = false
		is_open_file_picker = true
	elif Input.is_action_just_pressed("ui_open") and is_open_file_picker:
		item_list.visible = false
		editor.editable = true
		is_open_file_picker = false
	elif Input.is_action_pressed("ui_up") and cur_ind > 0 and is_open_file_picker:
		cur_ind -= 1
	elif Input.is_action_pressed("ui_down") and cur_ind < item_list.get_item_count() - 1 and is_open_file_picker:
		cur_ind += 1
	elif Input.is_action_just_pressed("save"):
		var file = FileAccess.open(cur_opened_file, FileAccess.WRITE)
		if file:
			file.store_string(editor.text)
			file.flush()
			file.close()
	elif Input.is_action_just_pressed("enter") and is_open_file_picker:
		var selected_name = item_list.get_item_text(cur_ind)
		if not selected_name == "..": selected_name = selected_name.erase(0, 2)
		var full_path = dir.get_current_dir().path_join(selected_name)
		if DirAccess.open(full_path):
			dir.change_dir(selected_name)
			diplay_items(get_dir_contents())
			cur_ind = 0
		else:
			var file = FileAccess.open(full_path, FileAccess.READ)
			if file:
				editor.show()
				editor.set_up_extensions(get_extension(selected_name))
				item_list.visible = false
				editor.editable = true
				is_open_file_picker = false
				cur_opened_file = str(full_path)
				editor.text = file.get_as_text()
			file.close()
			call_deferred("_refocus_editor")
		get_viewport().set_input_as_handled()
	item_list.select(cur_ind)


func _on_editor_gui_input(_event: InputEvent) -> void:
	if not Input.is_action_just_pressed("ui_open"): return
	editor.accept_event()
