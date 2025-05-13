extends Control
@onready var player = $AudioStreamPlayer
@onready var timer = $Timer
var sound = preload("res://intro.wav")

func _ready():
	player.stream = sound
	player.play()
	timer.start()

func _on_timer_timeout() -> void:
	get_tree().change_scene_to_file("res://main.tscn")
