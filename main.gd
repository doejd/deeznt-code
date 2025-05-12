extends Control
@onready var editor = $HBoxContainer/Editor
@onready var label = $HBoxContainer/Label

func _ready():
	load_code()
	
func update_line_numbers():
	var line_count = editor.get_line_count()
	var numbers_text = ""
	for i in range(line_count):
		numbers_text += str(i + 1) + "\n"
	label.text = numbers_text
	
func load_code():
	var file = FileAccess.open("user://code.txt", FileAccess.READ)
	if file:
		editor.text = file.get_as_text()
		file.close()
		
func save_code():
	var file = FileAccess.open("user://code.txt", FileAccess.WRITE)
	file.store_string(editor.text)
	file.close()

func _unhandled_input(event: InputEvent) -> void:
	if event is InputEventKey and event.ctrl_pressed:
		if event.keycode == KEY_S:
			save_code()
			
func _process(_delta) -> void:
		update_line_numbers()
			
