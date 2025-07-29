extends Control
@onready var intro_wind = $"Intro Window"
@onready var find_replace_wind = $"Find Replace Window"
@onready var editor = $Editor_Container/VSplitContainer2/VSplitContainer/Editor
@onready var item_list = $Editor_Container/ItemList
@onready var label = $Editor_Container/VSplitContainer2/VSplitContainer/RichTextLabel
@onready var H_container = $Editor_Container
@onready var V_container = $Editor_Container/VSplitContainer2/VSplitContainer
@onready var V_container2 = $Editor_Container/VSplitContainer2
@onready var cmdhost = $Editor_Container/VSplitContainer2/CmdHost
@onready var icons = Icons.new()
var cur_opened_file = ""
var dir = DirAccess.open(OS.get_user_data_dir())
var cur_ind = 0
var cur_ind_focus = 0
@onready var intro_wind_open = intro_wind.visible
@onready var find_replace_wind_open = find_replace_wind.visible
@onready var map : Dictionary = {
	0 : item_list,
	1 : editor,
	2 : cmdhost }
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
	cur_ind_focus = 1
	
func save() -> void:
	var file = FileAccess.open(cur_opened_file, FileAccess.WRITE)
	if file:
		file.store_string(editor.text.replace("\t", "    "))
		file.flush()
		file.close()
		
func _ready() -> void:
	diplay_items(get_dir_contents())
	item_list.select(cur_ind)
	get_tree().root.size_changed.connect(resize)

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
			editor.text = file.get_as_text()
			editor.set_up_extensions(get_extension(selected_name))
			cur_opened_file = str(full_path)
		file.close()
		call_deferred("_refocus_editor")
		opened_file.emit(selected_name)
	get_viewport().set_input_as_handled()

func _input(_event: InputEvent) -> void:
	if Input.is_action_just_pressed("Change Focus"):
		cur_ind_focus += 1
		if cur_ind_focus >= 3:
			cur_ind_focus = 0
		map[cur_ind_focus].grab_focus()
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
	if Input.is_action_just_pressed("Help") and not intro_wind_open:
		intro_wind.show()
		intro_wind_open = true
	elif Input.is_action_just_pressed("Help") and intro_wind_open:
		intro_wind.hide()
		intro_wind_open = false
	if Input.is_action_just_pressed("Find") and not find_replace_wind_open:
		find_replace_wind.show()
		find_replace_wind_open = true
	elif Input.is_action_just_pressed("Find") and find_replace_wind_open:
		find_replace_wind.hide()
		find_replace_wind_open = false

func resize() -> void:
	var win_size = DisplayServer.window_get_size()
	var left_side_spacing = 0.25
	var label_spacing = 0.01
	var console_spacing = 0.1
	H_container.split_offset = win_size.y * left_side_spacing
	V_container.split_offset = win_size.x * label_spacing
	V_container2.split_offset = win_size.x * console_spacing
	label.add_theme_font_size_override("normal_font_size", win_size.y * label_spacing * 1.5)
	editor.add_theme_font_size_override("font_size", win_size.y * label_spacing * 1.5)
	item_list.add_theme_font_size_override("font_size", win_size.y * label_spacing * 1.5)
	cmdhost.add_theme_font_size_override("font_size", win_size.y * label_spacing * 1.5)

func _on_editor_gui_input(_event: InputEvent) -> void:
	if not Input.is_action_just_pressed("ui_open"): return
	editor.accept_event()

func _on_item_list_item_clicked(index: int, _at_position: Vector2, mouse_button_index: int) -> void:
	if index == cur_ind:
		open()
	if mouse_button_index == 1 and not index == cur_ind:
		cur_ind = index

func _on_cmd_host_focus_entered() -> void:
	cur_ind_focus = 2

func _on_item_list_focus_entered() -> void:
	cur_ind_focus = 0

func _on_editor_focus_entered() -> void:
	cur_ind_focus = 1
