extends Control
@onready var editor = $HBoxContainer/Editor
@onready var label = $HBoxContainer/Label
var path = OS.get_environment("USERPROFILE")
var dir = DirAccess.open(path)

func get_dir_contents():
	var items = []
	if dir:
		dir.list_dir_begin()
		var file_name = dir.get_next()
		while file_name != "":
			if dir.current_is_dir():
				print("Found directory: " + file_name)
			else:
				print("Found file: " + file_name)
			items.append(file_name)
			file_name = dir.get_next()
		return items
	else:
		print("An error has been encountered")
		return null
	
func update_line_numbers():
	var line_count = editor.get_line_count()
	var numbers_text = ""
	for i in range(line_count):
		numbers_text += str(i + 1) + "\n"
	label.text = numbers_text
			
func _process(_delta) -> void:
	update_line_numbers()
