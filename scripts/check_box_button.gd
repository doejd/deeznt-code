extends CheckBox
@export var modifies_property : String = ""
@onready var editor : CodeEdit = $"../../../../../Editor_Container/VSplitContainer/VSplitContainer/Editor"

func _ready() -> void:
	var prop = editor.get(modifies_property)
	if prop == null: return
	set("button_pressed", prop)


func _on_pressed() -> void:
	var prop = editor.get(modifies_property)
	if prop == null: return
	editor.set(modifies_property, button_pressed)
