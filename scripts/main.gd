extends Control
@onready var intro_wind = $"Intro Window"
@onready var find_replace_wind = $"Find Replace Window"
@onready var setting_wind = $"Settings Window"
@onready var editor = $Editor_Container/VSplitContainer/VSplitContainer/Editor
@onready var item_list = $Editor_Container/ItemList
@onready var H_container = $Editor_Container
@onready var V_container = $Editor_Container/VSplitContainer/VSplitContainer
@onready var V_container2 = $Editor_Container/VSplitContainer
@onready var terminal = $Editor_Container/VSplitContainer/Control.get_child(0)
@onready var tab_bar = $Editor_Container/VSplitContainer/VSplitContainer/TabBar
@onready var timer : Timer = $"Timer"
@onready var reload_timer : Timer = $"Reload Timer"
@onready var icons = Icons.new()
var dir = DirAccess.open(OS.get_user_data_dir())
var cur_opened_file = ""
var cur_ind = 0
var cur_ind_focus = 0
var font_size = 16
var from_idx = -1
var save_file_path = "user://Preferance Data/save_data.cfg"
var intro_wind_popup : bool = true
var open_last_project_on_startup : bool = true
@onready var map : Dictionary = {
	0 : item_list,
	1 : editor,
	2 : terminal }
var tab_path_arr = []
signal opened_file(file_name, file_path)
signal on_load_intro_window(show_)
signal on_load_theme(theme_l)
signal on_load_get_themes(themes)
signal on_startup(should_load_last_project)

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
	
func save() -> void:
	if cur_opened_file == "": return
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
	cfg.set_value("preferences", "open_tabs", tab_path_arr)
	cfg.set_value("preferences", "font_size", font_size)
	cfg.set_value("preferences", "show", intro_wind_popup)
	cfg.set_value("preferences", "open_last_project_on_startup", open_last_project_on_startup)
	cfg.set_value("preferences", "save_timer_delay", timer.wait_time)
	cfg.set_value("preferences", "reload_timer_delay", reload_timer.wait_time)
	cfg.save(save_file_path)

func _ready() -> void:
	var dir_ : DirAccess = DirAccess.open("user://")
	if not dir_.dir_exists("Preferance Data"): dir_.make_dir("Preferance Data")
	on_load_emit_pref()
	update()
	item_list.select(cur_ind)
	get_tree().root.focus_entered.connect(update)
	get_tree().root.close_requested.connect(save_preferences)

func on_load_emit_pref():
	var cfg = ConfigFile.new()
	var err = cfg.load(save_file_path)
	if err != OK: print("Failed to load file %s" % err); return
	intro_wind_popup = cfg.get_value("preferences", "show", true)
	font_size = cfg.get_value("preferences", "font_size", 16)
	open_last_project_on_startup = cfg.get_value("preferences", "open_last_project_on_startup", true)
	timer.wait_time = cfg.get_value("preferences", "save_timer_delay", 3)
	reload_timer.wait_time = cfg.get_value("preferences", "reload_timer_delay", 3)
	var theme_ = cfg.get_value("preferences", "theme", "Github Dark")
	load_tabs(cfg)
	load_themes()
	update_font_size()
	on_load_intro_window.emit(intro_wind_popup)
	on_load_theme.emit(theme_)
	on_startup.emit(open_last_project_on_startup)
	editor.setup_theme()

func load_tabs(cfg : ConfigFile) -> void:
	if not open_last_project_on_startup: return
	var tmp_arr = cfg.get_value("preferences", "open_tabs", [])
	for tab in tmp_arr: open_file_dir(tab, tab.get_file())
	
func load_themes() -> void:
	var themes_ : Array = []
	var Lua_dir : DirAccess = DirAccess.open("user://Lua/themes")
	if !Lua_dir: return
	for file in Lua_dir.get_files(): themes_.append(file.get_basename())
	on_load_get_themes.emit(themes_)

func display_items(items: Array) -> void:
	item_list.clear()
	item_list.add_item("..")
	for item in items:
		if DirAccess.open(dir.get_current_dir().path_join(item)): item_list.add_item(" " + item) # If it is a folder
		else: item_list.add_item(icons.get_icon_data(get_extension(item)) + " " + item)

func open_file_dir(full_path : String, selected_name : String) -> void:
	if DirAccess.open(full_path):
		cur_ind = 0
		dir.change_dir(selected_name)
		update()
	else:
		save()
		var file = FileAccess.open(full_path, FileAccess.READ)
		if file:
			editor.text = file.get_as_text()
			file.close()
			editor.set_up_extensions(get_extension(selected_name))
			editor.setup_highlighter()
			cur_opened_file = file.get_path_absolute()
			editor.clear_undo_history()
			timer.start()
			opened_file.emit(selected_name, full_path)
		else:
			return

func open_from_file_explorer():
	var selected_name = item_list.get_item_text(cur_ind)
	if not selected_name == "..": selected_name = selected_name.erase(0, 2)
	var full_path = dir.get_current_dir().path_join(selected_name)
	open_file_dir(full_path, selected_name)

func _input(_event: InputEvent) -> void:
	if Input.is_action_just_pressed("Change Focus"):
		cur_ind_focus += 1
		if cur_ind_focus >= 3: cur_ind_focus = 0
		map[cur_ind_focus].grab_focus()
	if item_list.has_focus():
		if Input.is_action_pressed("ui_up"): cur_ind -= 1; get_viewport().set_input_as_handled()
		elif Input.is_action_pressed("ui_down"): cur_ind += 1; get_viewport().set_input_as_handled()
		elif Input.is_action_just_pressed("enter"): open_from_file_explorer(); get_viewport().set_input_as_handled()
		cur_ind = clamp(cur_ind, 0, item_list.get_item_count()-1)
		item_list.select(cur_ind)
		item_list.ensure_current_is_visible()
	if Input.is_action_just_pressed("save"): save(); timer.start(); get_viewport().set_input_as_handled()
	if Input.is_action_just_pressed("settings") and setting_wind.visible: setting_wind.hide(); get_viewport().set_input_as_handled()
	elif Input.is_action_just_pressed("settings") and not setting_wind.visible: setting_wind.show(); get_viewport().set_input_as_handled()
	if Input.is_action_just_pressed("Help") and not intro_wind.visible: intro_wind.show(); get_viewport().set_input_as_handled()
	elif Input.is_action_just_pressed("Help") and intro_wind.visible: intro_wind.hide(); get_viewport().set_input_as_handled()
	if Input.is_action_just_pressed("Find") and not find_replace_wind.visible: find_replace_wind.show(); get_viewport().set_input_as_handled()
	elif Input.is_action_just_pressed("Find") and find_replace_wind.visible: find_replace_wind.hide(); get_viewport().set_input_as_handled()
	if Input.is_action_pressed("Increase Font Size") and font_size < 100: font_size += 1; update_font_size(); get_viewport().set_input_as_handled()
	if Input.is_action_pressed("Decrease Font Size") and font_size > 0: font_size -= 1; update_font_size(); get_viewport().set_input_as_handled()
	if Input.is_action_just_pressed("Tab Switch") and not tab_bar.current_tab == -1 and not tab_path_arr.is_empty():
		if tab_bar.current_tab + 1 >= tab_bar.get_tab_count(): tab_bar.current_tab = 0
		else: tab_bar.current_tab += 1
		open_file_dir(tab_path_arr[tab_bar.current_tab], tab_bar.get_tab_title(tab_bar.current_tab)) 
		get_viewport().set_input_as_handled()

func update() -> void:
	display_items(get_dir_contents())

func update_font_size():
	tab_bar.add_theme_font_size_override("font_size", font_size)
	editor.add_theme_font_size_override("font_size", font_size)
	item_list.add_theme_font_size_override("font_size", font_size)
	terminal.add_theme_font_size_override("font_size", font_size)
	theme.default_font_size = font_size
	
func remove_tab(tab : int):
	tab_bar.remove_tab(tab)
	tab_path_arr.remove_at(tab)
	
	if tab_bar.get_tab_count() == 0:
		editor.text = ""
		cur_opened_file = ""
		editor.clear_undo_history()
		tab_path_arr.clear()
		return
		
	var new_idx = clamp(tab - 1, 0, tab_bar.get_tab_count() - 1)
	tab_bar.current_tab = new_idx
	open_file_dir(tab_path_arr[new_idx], tab_bar.get_tab_title(new_idx))

func _on_editor_gui_input(_event: InputEvent) -> void:
	if not Input.is_action_just_pressed("ui_open"): return
	editor.accept_event()

func _on_item_list_item_clicked(index: int, _at_position: Vector2, mouse_button_index: int) -> void:
	if index == cur_ind: open_from_file_explorer()
	if mouse_button_index == 1 and not index == cur_ind: cur_ind = index

func _on_item_list_focus_entered() -> void:
	cur_ind_focus = 0

func _on_editor_focus_entered() -> void:
	cur_ind_focus = 1

func _on_cmd_host_focus_entered() -> void:
	cur_ind_focus = 2

func _on_opened_file(file_name: Variant, file_path : Variant) -> void:
	if tab_path_arr.find(file_path) != -1: return;
	tab_bar.add_tab(file_name)
	tab_bar.current_tab = tab_bar.get_tab_count() - 1
	tab_path_arr.push_back(file_path)

func _on_tab_bar_tab_clicked(tab: int) -> void:
	from_idx = tab_bar.current_tab
	if cur_opened_file == tab_path_arr[tab]: return
	open_file_dir(tab_path_arr[tab], tab_bar.get_tab_title(tab))
	editor.set_up_extensions(tab_bar.get_tab_title(tab))

func _on_tab_bar_active_tab_rearranged(idx_to: int) -> void:
	if from_idx == -1 or from_idx == idx_to: return
	tab_path_arr.insert(idx_to, tab_path_arr.pop_at(from_idx))
	from_idx = -1

func _on_timer_timeout() -> void:
	save()

func _on_reload_timer_timeout() -> void:
	update()
