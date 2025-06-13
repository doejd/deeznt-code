extends VSplitContainer

@onready var input_box = $LineEdit
@onready var output_box = $TextEdit
var current_dir = OS.get_environment("USERPROFILE")

func _ready():
	input_box.grab_focus()
	input_box.connect("text_submitted", Callable(self, "_on_input_submitted"))
	append_to_output("Terminal started in: " + current_dir)

func _on_input_submitted(command: String) -> void:
	var trimmed = command.strip_edges()
	input_box.text = ""
	append_to_output("$ " + trimmed)

	if trimmed.begins_with("cd "):
		handle_cd(trimmed)
	elif trimmed == "cd":
		append_to_output(current_dir)
	else:
		run_cmd(trimmed)

func handle_cd(command: String) -> void:
	var path = command.substr(3).strip_edges()
	var target = ""

	if path.begins_with("\\") or path.match("^[A-Za-z]:.*"):
		target = path
	else:
		target = current_dir.path_join(path)

	var dir = DirAccess.open(target)
	if dir:
		current_dir = dir.get_current_dir()
		append_to_output("Directory changed to: " + current_dir)
	else:
		append_to_output("The system cannot find the path specified.")

func run_cmd(command: String) -> void:
	var output := []
	command = "cd " + current_dir + " && " + command
	var exit_code = OS.execute("cmd.exe", ["/C", command], output, true)

	if exit_code != 0:
		append_to_output("Error: Exit code " + str(exit_code))

	for line in output:
		append_to_output(line)

func append_to_output(text: String) -> void:
	output_box.text += text + "\n"
	output_box.scroll_vertical = output_box.get_line_count()
