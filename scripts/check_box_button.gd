extends CheckBox
@export var modifies_property : String = ""
@onready var editor : CodeEdit = $"../../../../../Editor_Container/VSplitContainer/VSplitContainer/Editor"
@onready var control : Control = $"../../../../.."

func _ready() -> void:
	var prop = editor.get(modifies_property)
	if prop == null: return
	set("button_pressed", prop)

func _on_pressed() -> void:
	if modifies_property == "show_intro_wind":
		control.intro_wind_popup = button_pressed
		return
	var prop = editor.get(modifies_property)
	if prop == null: return
	editor.set(modifies_property, button_pressed)

func _on_control_on_load_intro_window(show_: Variant) -> void:
	if modifies_property == "show_intro_wind": button_pressed =  show_
	return
