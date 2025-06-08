extends VBoxContainer
@onready var label = $RichTextLabel

func _ready() -> void:
	label.custom_minimum_size = Vector2(get_viewport().get_visible_rect().size.x, get_viewport().get_visible_rect().size.x / 20)


func _on_control_opened_file(file_name) -> void:
	label.text = str(file_name)
