extends Control
@onready var editor = $Editor
@onready var item_list = $ItemList
var is_open_file_picker = false
var dir = DirAccess.open(OS.get_environment("USERPROFILE"))
var cur_ind = 0

func get_extension(stri : String, char_to_split_around : String):
	while true:
		if stri[0] == char_to_split_around:
			stri = stri.erase(0)
			return stri
		stri = stri.erase(0)

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
		
func _ready():
	editor.editable = false
	diplay_items(get_dir_contents())
	item_list.select(cur_ind)

func diplay_items(items: Array) -> void:
	item_list.clear()
	item_list.add_item("..")
	for item in items:
		item_list.add_item(item)

func _input(_event: InputEvent) -> void:
	if Input.is_action_just_pressed("ui_open") and not is_open_file_picker:
		item_list.position.x = 0
		editor.editable = false
		is_open_file_picker = true
	elif Input.is_action_just_pressed("ui_open") and is_open_file_picker:
		item_list.position.x = -162
		is_open_file_picker = false
	elif Input.is_action_pressed("ui_up") and cur_ind > 0:
		cur_ind -= 1
	elif Input.is_action_pressed("ui_down") and cur_ind < item_list.get_item_count() - 1:
		cur_ind += 1
	elif Input.is_action_just_pressed("save"):
		var full_path = dir.get_current_dir().path_join(item_list.get_item_text(cur_ind))
		var file = FileAccess.open(full_path, FileAccess.WRITE)
		if file:
			file.store_string(editor.text)
			file.flush()
			file.close()
	elif Input.is_action_just_pressed("enter") and is_open_file_picker:
		var selected_name = item_list.get_item_text(cur_ind)
		var full_path = dir.get_current_dir().path_join(selected_name)
		if DirAccess.open(full_path):
			dir.change_dir(selected_name)
			diplay_items(get_dir_contents())
			cur_ind = 0
		else:
			var file = FileAccess.open(full_path, FileAccess.READ)
			if file:
				editor.set_up_extensions(get_extension(selected_name, "."))
				editor.setup_highlighter()
				editor.text = file.get_as_text()
				editor.editable = true
				item_list.position.x = -162
				is_open_file_picker = false
			file.close()
	item_list.select(cur_ind)
