#ifndef SH_META_TYPES_H
#define SH_META_TYPES_H
enum member_type {
	type_mat4,
	type_vec4,
	type_GLuint,
	type_game_state,
	type_input_state,
	type_FILE,
	type_sh_fnt,
	type_sh_col_queue,
	type_draw_stack,
	type_sh_debug_ui_state,
	type_sh_ui_state,
	type_game_debug_func,
	type_game_init_func,
	type_game_render_func,
	type_game_update_func,
	type_GLFWwindow,
	type_objects,
	type_game_object,
	type_game_grid,
	type_uint32,
	type_FILETIME,
	type_HMODULE,
	type_sh_line,
	type_sh_phy_force,
	type_sh_circle,
	type_vec2,
	type_float,
	type_int,
};


#define DUMP_VEC4(t)		dump_object_log((void*)t, class_vec4, array_count(class_vec4), #t)
#define DUMP_VEC2(t)		dump_object_log((void*)t, class_vec2, array_count(class_vec2), #t)
#define DUMP_SH_LINE(t)		dump_object_log((void*)t, class_sh_line, array_count(class_sh_line), #t)
#define DUMP_SH_CIRCLE(t)		dump_object_log((void*)t, class_sh_circle, array_count(class_sh_circle), #t)
#define DUMP_GAME_STATE(t)		dump_object_log((void*)t, class_game_state, array_count(class_game_state), #t)
#define DUMP_LINE_OBJECT(t)		dump_object_log((void*)t, class_line_object, array_count(class_line_object), #t)
#define DUMP_BALL_OBJECT(t)		dump_object_log((void*)t, class_ball_object, array_count(class_ball_object), #t)
#endif SH_META_TYPES_H
