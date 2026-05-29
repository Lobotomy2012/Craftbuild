extends Main

@onready var pause_btn = $ResumeBtn
@onready var leave_btn = $LeaveBtn
@onready var setting_btn = $SettingBtn
@onready var chat_box = $ChatBox

var pausing = true

func _ready() -> void:
	set_seed_and_world_name(Global.world_seed, Global.world_name)
	set_render_distance(int(Global.render_distance))
	init()

func _input(_event):
	if Input.is_action_just_pressed("chat"):
		chat_box.visible = false
		chat(chat_box.text)
		
	if Input.is_action_just_pressed("open_chat"):
		chat_box.visible = true
		chat_box.grab_focus()
		start_chat()
		get_viewport().set_input_as_handled()
		
	if Input.is_action_just_pressed("ui_cancel"):
		if chat_box.visible:
			chat_box.visible = false
			chat("")
		elif not pausing: pause()
		else: resume()

func pause():
	pause_btn.visible = true
	leave_btn.visible = true
	setting_btn.visible = true
	pausing = true
	pause_game()

func resume():
	pause_btn.visible = false
	leave_btn.visible = false
	setting_btn.visible = false
	pausing = false
	resume_game()
