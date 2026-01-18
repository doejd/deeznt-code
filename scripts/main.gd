extends Control
@onready var intro_wind = $"Intro Window"
@onready var find_replace_wind = $"Find Replace Window"
@onready var editor = $Editor_Container/VSplitContainer/VSplitContainer/Editor
@onready var item_list = $Editor_Container/ItemList
@onready var label = $Editor_Container/VSplitContainer/VSplitContainer/Label
@onready var H_container = $Editor_Container
@onready var V_container = $Editor_Container/VSplitContainer/VSplitContainer
@onready var V_container2 = $Editor_Container/VSplitContainer
@onready var terminal = $Editor_Container/VSplitContainer/Control.get_child(1)
@onready var icons = Icons.new()
var cur_opened_file = ""
var dir = DirAccess.open(OS.get_user_data_dir())
var cur_ind = 0
var cur_ind_focus = 0
var font_size = 16;
var save_file_path = "user://Preferance Data/save_data.cfg"
@onready var map : Dictionary = {
	0 : item_list,
	1 : editor,
	2 : terminal }
signal opened_file(file_name)
signal change_font_size(font_size)
signal on_load_intro_window(show_)
signal on_load_theme(theme_l)


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

func save_preferences():
	var cfg = ConfigFile.new()
	var err = cfg.load(save_file_path)
	if err != OK and err != ERR_FILE_NOT_FOUND:
		print("Failed to initialize %s" % err)
		return
	cfg.set_value("preferences", "font_size", font_size)
	cfg.set_value("preferences", "show", not $"Intro Window/VBoxContainer/Never Show Again Dialog/CheckBox".visible)
	cfg.save(save_file_path)

func _ready() -> void:
	var dir_ : DirAccess = DirAccess.open("user://")
	if not dir_.dir_exists("Preferance Data"): dir_.make_dir("Preferance Data")
	on_load_emit_pref()
	update()
	item_list.select(cur_ind)
	get_tree().root.size_changed.connect(resize)
	get_tree().root.focus_entered.connect(update)
	get_tree().root.close_requested.connect(save_preferences)

func on_load_emit_pref():
	var cfg = ConfigFile.new()
	var err = cfg.load(save_file_path)
	if err != OK: 
		print("Failed to load file %s" % err)
		return
	var show_ = cfg.get_value("preferences", "show", true)
	var theme_ = cfg.get_value("preferences", "theme", "Github Dark")
	font_size = cfg.get_value("preferences", "font_size", 16)
	update_font_size()
	on_load_intro_window.emit(show_)
	on_load_theme.emit(theme_)

func display_items(items: Array) -> void:
	item_list.clear()
	item_list.add_item("..")
	for item in items:
		if DirAccess.open(dir.get_current_dir().path_join(item)): item_list.add_item(" " + item) # If it is a folder
		else: item_list.add_item(icons.get_icon_data(get_extension(item)) + " " + item)

func open() -> void:
	var selected_name = item_list.get_item_text(cur_ind)
	if not selected_name == "..": selected_name = selected_name.erase(0, 2)
	var full_path = dir.get_current_dir().path_join(selected_name)
	if DirAccess.open(full_path):
		cur_ind = 0
		dir.change_dir(selected_name)
		update()
	else:
		save()
		var file = FileAccess.open(full_path, FileAccess.READ)
		if file:
			editor.text = file.get_as_text()
			editor.set_up_extensions(get_extension(selected_name))
			cur_opened_file = str(full_path)
			editor.clear_undo_history()
			file.close()
			call_deferred("_refocus_editor")
			opened_file.emit(selected_name)
		else:
			dir = DirAccess.open(OS.get_user_data_dir())

func _input(_event: InputEvent) -> void:
	if Input.is_action_just_pressed("Change Focus"):
		cur_ind_focus += 1
		if cur_ind_focus >= 3: cur_ind_focus = 0
		map[cur_ind_focus].grab_focus()
	if item_list.has_focus():
		if Input.is_action_pressed("ui_up"): cur_ind -= 1; get_viewport().set_input_as_handled()
		elif Input.is_action_pressed("ui_down"): cur_ind += 1; get_viewport().set_input_as_handled()
		elif Input.is_action_just_pressed("enter"): open(); get_viewport().set_input_as_handled()
		cur_ind = clamp(cur_ind, 0, item_list.get_item_count()-1)
		item_list.select(cur_ind)
		item_list.ensure_current_is_visible()
	if Input.is_action_just_pressed("save"): save(); get_viewport().set_input_as_handled()
	if Input.is_action_just_pressed("Help") and intro_wind.visible == false: intro_wind.show(); get_viewport().set_input_as_handled()
	elif Input.is_action_just_pressed("Help") and intro_wind.visible == true: intro_wind.hide(); get_viewport().set_input_as_handled()
	if Input.is_action_just_pressed("Find") and find_replace_wind.visible == false: find_replace_wind.show(); get_viewport().set_input_as_handled()
	elif Input.is_action_just_pressed("Find") and find_replace_wind.visible == true: find_replace_wind.hide(); get_viewport().set_input_as_handled()
	if Input.is_action_pressed("Increase Font Size") and font_size < 100: font_size += 1; update_font_size(); get_viewport().set_input_as_handled()
	if Input.is_action_pressed("Decrease Font Size") and font_size > 0: font_size -= 1; update_font_size(); get_viewport().set_input_as_handled()

func update() -> void:
	display_items(get_dir_contents())
	
func resize() -> void:
	var win_size = DisplayServer.window_get_size()
	var left_side_spacing = 0.25
	var label_spacing = 0.01
	var console_spacing = 0.1
	H_container.split_offset = win_size.y * left_side_spacing
	V_container.split_offset = win_size.x * label_spacing
	V_container2.split_offset = win_size.x * console_spacing
	
func update_font_size():
	label.add_theme_font_size_override("font_size", font_size)
	editor.add_theme_font_size_override("font_size", font_size)
	item_list.add_theme_font_size_override("font_size", font_size)
	terminal.add_theme_font_size_override("font_size", font_size)
	change_font_size.emit(font_size)
	

func _on_editor_gui_input(_event: InputEvent) -> void:
	if not Input.is_action_just_pressed("ui_open"): return
	editor.accept_event()

func _on_item_list_item_clicked(index: int, _at_position: Vector2, mouse_button_index: int) -> void:
	if index == cur_ind: open()
	if mouse_button_index == 1 and not index == cur_ind: cur_ind = index

func _on_item_list_focus_entered() -> void:
	cur_ind_focus = 0

func _on_editor_focus_entered() -> void:
	cur_ind_focus = 1

func _on_cmd_host_focus_entered() -> void:
	cur_ind_focus = 2
