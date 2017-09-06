struct_meta_info class_ball_object[] = {
	{0, type_int, "vec_nums", offsetof(ball_object, vec_nums)},
	{0, type_int, "vec_capacity", offsetof(ball_object, vec_capacity)},
	{0, type_float, "velocity", offsetof(ball_object, velocity)},
	{0, type_vec2, "previous_pos", offsetof(ball_object, previous_pos)},
};
struct_meta_info class_game_state[] = {
	{0, type_HMODULE, "lib", offsetof(game_state, lib)},
	{0, type_FILETIME, "last_write_time", offsetof(game_state, last_write_time)},
	{0, type_vec2, "window_width_height", offsetof(game_state, window_width_height)},
	{0, type_uint32, "current_vao", offsetof(game_state, current_vao)},
	{0, type_int, "object_count", offsetof(game_state, object_count)},
	{0, type_int, "current_program", offsetof(game_state, current_program)},
	{0, type_int, "vpos_attrib_loc", offsetof(game_state, vpos_attrib_loc)},
	{0, type_int, "color_attrib_loc", offsetof(game_state, color_attrib_loc)},
	{0, type_int, "model_t_attrib_loc", offsetof(game_state, model_t_attrib_loc)},
	{0, type_int, "tex_coord", offsetof(game_state, tex_coord)},
	{0, type_int, "has_texture_attrib", offsetof(game_state, has_texture_attrib)},
	{0, type_int, "is_init", offsetof(game_state, is_init)},
	{0, type_int, "full_screen", offsetof(game_state, full_screen)},
	{0, type_game_grid, "overlord", offsetof(game_state, overlord)},
	{0, type_sh_ui_state, "ui_state", offsetof(game_state, ui_state)},
	{0, type_sh_debug_ui_state, "debug_ui_state", offsetof(game_state, debug_ui_state)},
	{0, type_draw_stack, "renderstack", offsetof(game_state, renderstack)},
	{0, type_sh_fnt, "font", offsetof(game_state, font)},
	{0, type_uint32, "fnt_tex", offsetof(game_state, fnt_tex)},
};
struct_meta_info class_sh_circle[] = {
	{0, type_vec2, "_direction", offsetof(sh_circle, _direction)},
	{0, type_GLuint, "_vbo", offsetof(sh_circle, _vbo)},
	{0, type_vec2, "_position", offsetof(sh_circle, _position)},
	{0, type_vec2, "_center", offsetof(sh_circle, _center)},
	{0, type_vec4, "_color", offsetof(sh_circle, _color)},
	{0, type_mat4, "_transform", offsetof(sh_circle, _transform)},
	{0, type_float, "_r", offsetof(sh_circle, _r)},
};
struct_meta_info class_vec2[] = {
	{0, type_float, "x", offsetof(vec2, x)},
	{0, type_float, "y", offsetof(vec2, y)},
};
