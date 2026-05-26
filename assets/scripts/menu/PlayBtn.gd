extends Button

func _ready() -> void:
	pressed.connect(_on_pressed)

func _on_pressed():
	Global.go_to("res://scenes/world_list.tscn")
