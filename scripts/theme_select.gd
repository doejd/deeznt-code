extends OptionButton
signal on_theme_change(theme)

func _ready() -> void:
	grab_focus()
	select(0)


func _on_item_selected(index: int) -> void:
	on_theme_change.emit(get_item_text(index))
