extends Button

@onready var world = $"../../Main"

func _ready() -> void:
	pressed.connect(_on_resume)

func _on_resume():
	world.emit_signal("resume")
