extends HSlider
@export var modifies_property : String = ""
@onready var editor = $"../../../../../../Editor_Container/VSplitContainer/VSplitContainer/Editor"
@onready var control = $"../../../../../.."

func _on_drag_ended(_value_changed: bool) -> void:
	if not _value_changed: return
	if modifies_property == "font_size": 
		control.font_size = value
		control.update_font_size()
	else: editor.set(modifies_property, value)

func _on_control_on_startup(_should_load_last_project: Variant) -> void:
	if modifies_property == "font_size":
		var property = editor.get_theme_font_size("font_size")
		if property == null: return
		set("value", property)
	elif modifies_property == "indent_size":
		var property = editor.get_tab_size()
		if property == null: return
		set("value", property)
