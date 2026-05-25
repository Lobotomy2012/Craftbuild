extends VBoxContainer

func _ready() -> void:
	load_folders("user://game/saves")
	
func create_button(world_name: String) -> void:
	var btn = Button.new()
	btn.text = world_name
	btn.pressed.connect(func(): 
		Global.world_name = world_name
		get_tree().change_scene_to_file("res://scenes/Game.tscn")
	)
	add_child(btn)

func load_folders(path: String) -> void:
	var dir = DirAccess.open(path)
	if dir == null:
		print("Cannot open dir:", path)
		return

	dir.list_dir_begin()
	var folder_name = dir.get_next()

	while folder_name != "":
		if dir.current_is_dir() and folder_name != "." and folder_name != "..":
			create_button(folder_name)
		folder_name = dir.get_next()
	dir.list_dir_end()
