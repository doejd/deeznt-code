extends Button
var color_buttons = []
var set_keywords_list = ["reserved", "string", "binary", "symbol", "variable", "operator", "comments", "error", "function", "member", "import"]


func _ready() -> void:
	for button in $"../..".find_children("*", "ColorPickerButton", true):
		color_buttons.append(button)

func _on_pressed() -> void:
	var Lua_theme_dir : DirAccess = DirAccess.open("user://Lua/themes")
	if Lua_theme_dir == null: return
	var file = FileAccess.open(Lua_theme_dir.get_current_dir() + "/newtheme.lua", FileAccess.WRITE)
	for button in color_buttons:
		if button.key_referring_to in set_keywords_list: file.store_string('set_keywords("%s", "%s")' % [button.key_referring_to, button.color.to_html(false)])
		else: file.store_string('set_gui("%s", "%s")' % [button.key_referring_to, button.color.to_html(false)])
