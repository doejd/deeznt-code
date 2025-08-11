extends OptionButton
signal on_theme_change(theme)

func _ready() -> void:
	var cfg = ConfigFile.new()
	var path = "user://Preferance Data/save_data.cfg"
	var err = cfg.load(path)
	if err != OK:
		print("Failed to load file: %s" % err)
	select(get_idx_from_str(cfg.get_value("preferences", "theme", "Github Dark")))

func _on_item_selected(index: int) -> void:
	on_theme_change.emit(get_item_text(index))

func get_idx_from_str(value : String):
	for i in range(item_count):
		if get_item_text(i) == value:
			return i
	return 0
