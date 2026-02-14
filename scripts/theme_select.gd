extends OptionButton
signal on_theme_change(theme)

func _on_item_selected(index: int) -> void:
	on_theme_change.emit(get_item_text(index))

func get_idx_from_str(value : String):
	for i in range(item_count):
		if get_item_text(i) == value:
			return i
	return -1

func _on_control_on_load_theme(theme_: Variant) -> void:
	select(get_idx_from_str(theme_))

func _on_control_on_load_get_themes(themes: Variant) -> void:
	var cur_theme_selected = get_selected()
	clear()
	for theme_ in themes: add_item(theme_)
	select(cur_theme_selected)
