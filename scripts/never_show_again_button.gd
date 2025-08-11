extends CheckBox

func _ready() -> void:
	var cfg = ConfigFile.new()
	var path = "user://Preferance Data/save_data.cfg"
	var err = cfg.load(path)
	if err != OK:
		print("Failed to intialize file")
	button_pressed = not cfg.get_value("preferences", "show", true)

func _on_toggled(toggled_on: bool) -> void:
	var cfg = ConfigFile.new()
	var path = "user://Preferance Data/save_data.cfg"
	var err = cfg.load(path)
	if err != OK and err != ERR_FILE_NOT_FOUND:
		print("Failed to initialize %s" % err)
		return
	cfg.set_value("preferences", "show", not toggled_on)
	cfg.save(path)
