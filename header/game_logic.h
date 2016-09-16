#include <shar.h>
#include <sharfunc.h>
#include <sh_circle.h>
#include <sh_line.h>
#include <Windows.h>
#ifndef SH_SHAPES
#include <sh_rect.h>
#endif

#define dt 0.01666666667

enum object_type {
    BALL,
    RECTANGLE,
    LINE
};

struct game_object {
    object_type type;
    game_object *next_object;
    int size; 
    void *obj;
};

struct ball_object {
    sh_circle *circ;
    float velocity;
    vec2 *vectors;
    vec2 previous_pos;
};


struct rect_object {
    sh_rect *rect;
    float velocity;
};

struct line_object {
    sh_line line;       
};

struct dynamic_objects {
    game_object *root_node;
    game_object *last_node;
    int object_counts; 
};

struct mouse_state {
    int left_button;
    int right_button;
    double mouse_x;
    double mouse_y;
    game_object *obj;
};

struct game_state;

#define GAME_UPDATE_FUNC(name) void name(game_state *gamestate, mouse_state *mouse) 
typedef GAME_UPDATE_FUNC(game_update_func);

#define GAME_RENDER_FUNC(name) void name(game_state *gamestate, double alpha)
typedef GAME_RENDER_FUNC(game_render_func);

#define GAME_INIT_FUNC(name) void name(game_state *gamestate)
typedef GAME_INIT_FUNC(game_init_func);


void add_to_dynamic_obj(dynamic_objects *dyn_obj, game_object *obj);

struct game_state {
    dynamic_objects objects_to_update;
    vec2 window_width_height;
    HMODULE lib;
    FILETIME last_write_time;
    game_update_func *update;
    game_render_func *render;
    game_init_func *init;

    game_object **named_objects;
    int object_count;

    int current_program;
    int vpos_attrib_loc;
    int color_attrib_loc;
    int model_t_attrib_loc;
    int is_init;
};
