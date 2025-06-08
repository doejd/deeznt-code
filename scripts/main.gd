extends Control
@onready var editor = $HBoxContainer/VBoxContainer/Editor
@onready var item_list = $HBoxContainer/ItemList
@onready var icons = Icons.new()
var cur_opened_file = ""
var dir = DirAccess.open(OS.get_environment("USERPROFILE"))
var cur_ind = 0
signal opened_file(file_name)

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
	
func save() -> void:
	var file = FileAccess.open(cur_opened_file, FileAccess.WRITE)
	if file:
		file.store_string(editor.text)
		file.flush()
		file.close()
		
func _ready() -> void:
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

func open() -> void:
	var selected_name = item_list.get_item_text(cur_ind)
	if not selected_name == "..": selected_name = selected_name.erase(0, 2)
	var full_path = dir.get_current_dir().path_join(selected_name)
	if DirAccess.open(full_path):
		cur_ind = 0
		dir.change_dir(selected_name)
		diplay_items(get_dir_contents())
	else:
		save()
		var file = FileAccess.open(full_path, FileAccess.READ)
		if file:
			editor.set_up_extensions(get_extension(selected_name))
			cur_opened_file = str(full_path)
			editor.text = file.get_as_text()
		file.close()
		call_deferred("_refocus_editor")
		opened_file.emit(selected_name)
	get_viewport().set_input_as_handled()

func _input(_event: InputEvent) -> void:
	if item_list.has_focus():
		if Input.is_action_pressed("ui_up"):
			cur_ind -= 1
			get_viewport().set_input_as_handled()
		elif Input.is_action_pressed("ui_down"):
			cur_ind += 1
			get_viewport().set_input_as_handled()
		elif Input.is_action_just_pressed("enter"):
			open()
		cur_ind = clamp(cur_ind, 0, item_list.get_item_count()-1)
		item_list.select(cur_ind)
		item_list.ensure_current_is_visible()
	if Input.is_action_just_pressed("save"):
		save()

func _on_editor_gui_input(_event: InputEvent) -> void:
	if not Input.is_action_just_pressed("ui_open"): return
	editor.accept_event()

func _on_item_list_item_clicked(index: int, _at_position: Vector2, mouse_button_index: int) -> void:
	if index == cur_ind:
		open()
	if mouse_button_index == 1 and not index == cur_ind:
		cur_ind = index
