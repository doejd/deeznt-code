extends OptionButton
signal on_theme_change(theme)

func _on_item_selected(index: int) -> void:
	on_theme_change.emit(get_item_text(index))

func get_idx_from_str(value : String):
	for i in range(item_count):
		if get_item_text(i) == value:
			return i
	return 0

func _on_control_on_load_theme(theme_: Variant) -> void:
	select(get_idx_from_str(theme_))
