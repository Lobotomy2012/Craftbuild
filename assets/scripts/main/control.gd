extends Node

@onready var pause_btn = $"../pause_btn"
@onready var leave_btn = $"../leave_btn"
@onready var world = $"../../main"

func _ready() -> void:
	world.pause.connect(_on_pause)
	world.resume.connect(_on_resume)

func _on_pause():
	pause_btn.visible = true
	leave_btn.visible = true
	world.pause_game()

func _on_resume():
	pause_btn.visible = false
	leave_btn.visible = false
	world.resume_game()
