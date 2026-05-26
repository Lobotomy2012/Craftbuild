extends HSlider

func _ready() -> void:
	value = Global.render_distance
	value_changed.connect(_on_slider_changed)

func _on_slider_changed(slider_value) -> void:
	$Name.text = "Render distance: " + str(int(slider_value))
	Global.render_distance = slider_value
