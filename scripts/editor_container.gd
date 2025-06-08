extends HBoxContainer
@onready var item_list = $ItemList
@onready var animation_player = $AnimationPlayer
var open_theme_select = false

func _ready() -> void:
	item_list.custom_minimum_size = Vector2(get_viewport().get_visible_rect().size.x * 2 / 5, get_viewport().get_visible_rect().size.y)
	
func _input(_event: InputEvent) -> void:
	if Input.is_action_just_pressed("theme_switch") and not open_theme_select:
		open_theme_select = true
		animation_player.play("open_theme_select")
		get_viewport().set_input_as_handled()
	elif Input.is_action_just_pressed("theme_switch") and open_theme_select:
		open_theme_select = false
		animation_player.play("close_theme_select")
		get_viewport().set_input_as_handled()
