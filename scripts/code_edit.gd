extends CodeEdit
var lua : LuaAPI = LuaAPI.new()
var keywords_to_highlight: Dictionary = {}
var color_regions_to_highlight: Array = []
var keywords: Dictionary = {
	"reserved":   str_to_clr("c678cc"),
	"annotation": str_to_clr("a2b429"),
	"string":     str_to_clr("98c379"),
	"binary":     str_to_clr("d19a66"),
	"symbol":     str_to_clr("839fb6"),
	"variable":   str_to_clr("e5c07b"),
	"operator":   str_to_clr("56b6c2"),
	"comments":   str_to_clr("7f848e"),
	"error":      str_to_clr("d31820"),
	"function":   str_to_clr("437ed9"),
	"member":     str_to_clr("e06c75")
}


func str_to_clr(string: String) -> Color:
	return Color.from_string(string, "#ff0000")
	
	
func highlight(kword : String, color : String):
	if not (color in keywords.keys()):
		print("Dumbass")
		return
		
	keywords_to_highlight[kword] = color
	
func highlight_region(start : String, end : String, color : String, single_line : bool = false):
	if not (color in keywords.keys()):
		print("Lol, that ain't there")
		return
		
	color_regions_to_highlight.append([start, end, color, single_line])
	
func set_up_extensions(extension):
	lua.bind_libraries(["base", "table", "string"])
	lua.push_variant("highlight", highlight)
	lua.push_variant("highlight_region", highlight_region)
	var error = lua.do_file("res://langs" + extension + ".lua")
	if error is LuaError:
		print("ERROR %d: %s" % [error.type, error.message])
		return
		
func setup_highlighter() -> void:
	var CH: CodeHighlighter = CodeHighlighter.new();

	syntax_highlighter = CH;

	CH.number_color = keywords.binary;
	CH.symbol_color = keywords.symbol;
	CH.function_color = keywords.function;
	CH.member_variable_color = keywords.member;

	var kth = keywords_to_highlight;
	var crth = color_regions_to_highlight;

	for key in kth:
		CH.add_keyword_color(key, keywords[kth[key]])

	for entry in crth:
		if CH.has_color_region(entry[0]): continue

		CH.add_color_region(entry[0], entry[1], keywords[entry[2]], entry[3])
		
func _ready() -> void:
	setup_highlighter()
