extends VBoxContainer

var game = "res://scenes/Game.tscn"

func _ready() -> void:
	load_folders()
	
func create_button(world_name: String) -> void:
	var btn = Button.new()
	btn.text = world_name
	btn.pressed.connect(func(): 
		Global.world_name = world_name
		get_tree().change_scene_to_file(game)
	)
	add_child(btn)

func load_folders() -> void:
	var dir = DirAccess.open(Global.world_saves)
	if dir == null:
		get_tree().change_scene_to_file(Global.world_saves)
		return

	dir.list_dir_begin()
	var folder_name = dir.get_next()
	if folder_name == "": get_tree().change_scene_to_file(game)
	
	while folder_name != "":
		if dir.current_is_dir() and folder_name != "." and folder_name != "..":
			create_button(folder_name)
		folder_name = dir.get_next()
	dir.list_dir_end()
