extends CheckBox
var dir : DirAccess = DirAccess.open("user://")

func _on_toggled(toggled_on: bool) -> void:
	if not dir.dir_exists("Preferance Data"):
		var err = dir.make_dir("Preferance Data")
	var file : FileAccess = FileAccess.open("user://Preferance Data/save_data.json", FileAccess.WRITE)
	if file:
		var shown = not toggled_on
		file.store_string(JSON.stringify({ "show" : shown}))
		file.close()
