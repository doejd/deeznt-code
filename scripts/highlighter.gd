class_name MyHighlight 
extends CodeHighlighter 
const keywords = ["import", "and", "as", "assert", "break", "class", "continue", "def", 
"del", "elif", "else", "except", "False", "finally", "for", "from", "global", "if", 
"in", "is", "lambda", "None", "nonlocal", "not", "or", "pass", "raise", "return", "True", 
"try", "while", "with", "yield"]
const color = Color("#C586C0")
func _init() -> void:
	number_color = Color("#81ffbc")
	symbol_color = Color("#ffffff")
	function_color = Color("#fff277")
	member_variable_color = Color("#a5f4ff")
	for keyword in keywords:
		add_keyword_color(keyword, color)
	add_member_keyword_color("self", Color("#56B6C2"))
