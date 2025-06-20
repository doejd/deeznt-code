extends Control
@onready var caret_type = get_node("Caret Type")
@onready var caret_blink = get_node("Caret Blink")
@onready var caret_blink_interval_slider = get_node("Caret Blink Interval")
@onready var show_line_number = get_node("Show Line Number")
@onready var code_completion = get_node("Code Completion")
@onready var indent_size = get_node("Indentation Size")
@onready var auto_indent = get_node("Automatic Indentation")
@onready var indent_use_space = get_node("Indentation Use Spaces")
@onready var auto_brace_completion = get_node("Auto Brace Completion")
@onready var highlight_braces = get_node("Highlight Matching Braces")
@onready var scroll_speed = get_node("Scrolling Speed")
@onready var settings : Array = [caret_type, caret_blink, caret_blink_interval_slider, show_line_number, code_completion,
indent_size, auto_indent, indent_use_space, auto_brace_completion, highlight_braces, scroll_speed]

func _ready() -> void:
	get_tree().root.size_changed.connect(resize)
	
func resize() -> void:
	var win_size = DisplayServer.window_get_size()
	var font_scale = 0.02
	for setting in settings:
		var label = setting.get_child(0)
		label.add_theme_font_size_override("font_size", win_size.y * font_scale)
		if setting.get_child(1) is OptionButton:
			var option_button = setting.get_child(1)
			option_button.add_theme_font_size_override("font_size", win_size.y * font_scale)
