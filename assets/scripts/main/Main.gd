extends Main

@onready var pause_btn = $ResumeBtn
@onready var leave_btn = $LeaveBtn

func _ready() -> void:
	set_seed_and_world_name(Global.world_seed, Global.world_name)
	pause.connect(_on_pause)
	resume.connect(_on_resume)
	init()

func _on_pause():
	pause_btn.visible = true
	leave_btn.visible = true
	pause_game()

func _on_resume():
	pause_btn.visible = false
	leave_btn.visible = false
	resume_game()
