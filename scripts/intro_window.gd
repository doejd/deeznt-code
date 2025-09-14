extends Window
@onready var labels : Array = [$"VBoxContainer/Label", $"VBoxContainer/Shrotcut Section/Label", 
$"VBoxContainer/Shrotcut Section/Shortcut Item/Label", $"VBoxContainer/Shrotcut Section/Shortcut Item/Label2",
$"VBoxContainer/Shrotcut Section/Shortcut Item2/Label", $"VBoxContainer/Shrotcut Section/Shortcut Item2/Label2", 
$"VBoxContainer/Shrotcut Section/Shortcut Item3/Label", $"VBoxContainer/Shrotcut Section/Shortcut Item3/Label2",
$"VBoxContainer/Shrotcut Section2/Label", $"VBoxContainer/Shrotcut Section2/Shortcut Item/Label", $"VBoxContainer/Shrotcut Section2/Shortcut Item/Label2",
$"VBoxContainer/Shrotcut Section2/Shortcut Item2/Label", $"VBoxContainer/Shrotcut Section2/Shortcut Item2/Label2", 
$"VBoxContainer/Shrotcut Section2/Shortcut Item3/Label", $"VBoxContainer/Shrotcut Section2/Shortcut Item3/Label2",
$"VBoxContainer/Shrotcut Section3/Label", $"VBoxContainer/Shrotcut Section3/Shortcut Item/Label", $"VBoxContainer/Shrotcut Section3/Shortcut Item/Label2",
$"VBoxContainer/Shrotcut Section3/Shortcut Item2/Label", $"VBoxContainer/Shrotcut Section3/Shortcut Item2/Label2",
$"VBoxContainer/Never Show Again Dialog/CheckBox"]
@onready var main = get_node("..")


func _ready() -> void:
	grab_focus()
	get_tree().root.size_changed.connect(center_resize)
	var cfg = ConfigFile.new()
	var err = cfg.load(main.save_file_path)
	if err != OK: 
		print("Failed to load file %s" % err)
		return
	var show_ = cfg.get_value("preferences", "show", true)
	if not show_:
		hide()
	
func center_resize() -> void:
	var screen_size = DisplayServer.window_get_size()
	size = Vector2i(screen_size.x / 3, screen_size.y / 3)
	position = (screen_size - size) / 2
	for label in labels:
		label.add_theme_font_size_override("font_size", screen_size.y * 0.015)
	add_theme_font_size_override("title_font_size", screen_size.y * 0.015)

func _on_close_requested() -> void:
	hide()
	main.intro_wind_open = false
