extends Node

var world_name = ""
var world_seed = 0
var world_saves = "user://game/saves"
var render_distance = 32
var sensitivity = 0.0043

var stack_scene = []

func go_to(path: String):
	stack_scene.push_back(get_tree().current_scene.scene_file_path)
	get_tree().change_scene_to_file(path)

func ret_last_scene():
	if stack_scene.is_empty(): return
	get_tree().change_scene_to_file(stack_scene.pop_back())
