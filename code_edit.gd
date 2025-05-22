extends CodeEdit
var lua = LuaAPI.new()

func _ready() -> void:
	self.syntax_highlighter = SyntaxHighlighter.new()
	lua.do_file("res://langs/python.lua")
	var keywords = lua.get_global("highlight_rules")
	var regions = lua.get_global("region_rules")
	for kw in keywords:
		self.syntax_highlighter.add_keyword_color(kw, keywords[kw])
	for region in regions:
		self.syntax_highlighter.add_color_region(region[0], region[1], region[2], false)
	self.syntax_highlighter.force_update_all()
