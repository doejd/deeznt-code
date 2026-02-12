extends HSlider
@export var modifies_property : String = ""
@onready var editor = $"../../../../../../Editor_Container/VSplitContainer/VSplitContainer/Editor"
@onready var control = $"../../../../../.."

func _ready() -> void:
	var property = editor.get_theme_font_size("font_size")
	if property == null: return
	set("value", property)

func _on_drag_ended(_value_changed: bool) -> void:
	if not _value_changed: return
	if modifies_property == "font_size": 
		control.font_size = value
		control.update_font_size()
	else: editor.set(modifies_property, value)


func _on_control_startup(should_load_last_project: Variant) -> void:
	pass # Replace with function body.
