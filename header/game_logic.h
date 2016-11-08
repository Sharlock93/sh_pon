#include <shar.h>
#include <sharfunc.h>
#include <sh_circle.h>
#include <sh_line.h>

#include <Windows.h>
#ifndef SH_SHAPES
#include <sh_rect.h>
#endif

#define dt 0.01666666667
/* #define dt 0.0g */

enum object_type {
    BALL,
    RECTANGLE,
    LINE
};

struct game_object {
    object_type type;
    game_object *next_object;
    game_object *previous_object;
    int size; 
    int moved;
    void *obj;
};

struct ball_object {
    sh_circle *circ;
    float velocity;
    vec2 *vectors;
    int vec_nums;
    int vec_capacity;
    vec2 previous_pos;
};


struct rect_object {
    sh_rect *rect;
    float velocity;
    vec2 previous_pos;
};

struct line_object {
    sh_line *line;       
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
    game_object *hot_object;
};

struct keyboard_state {
    int arrow_left;
    int arrow_right;
};


struct input_state {
    mouse_state mouse;
    keyboard_state keyboard;
};

struct object_bucket {
    game_object *obj;
    int object_count;
};


struct grid_element {
    //Todo(sharo): should they know about their parent?
    // - their index inside the grid 
    dynamic_objects *dyn_objs;

    int index_grid;
    int index_row;
    int index_column;

    float width;
    float height;

    unsigned int _vbo;
    vec2 data[4];
    vec4 color;

    float left;
    float right;
    float top;
    float bottom;
};

struct game_grid {
    grid_element *elements;
    int width_elem_count;
    int height_elem_count;
    int elem_count;
    int grid_width;
    int grid_height;

    float center_x;
    float center_y;
    unsigned int _vbo;
    vec2 data[4];
};

struct game_state;

#define GAME_UPDATE_FUNC(name) void name(game_state *gamestate, input_state *inputs, double dts, bool without_move) 
typedef GAME_UPDATE_FUNC(game_update_func);

#define GAME_RENDER_FUNC(name) void name(game_state *gamestate, double alpha)
typedef GAME_RENDER_FUNC(game_render_func);

#define GAME_INIT_FUNC(name) void name(game_state *gamestate)
typedef GAME_INIT_FUNC(game_init_func);


void add_to_dynamic_obj(dynamic_objects *dyn_obj, game_object *obj);
void init_grid(game_grid *grid, float grid_height, float grid_width, vec2 pos);
void free_objects_all(game_state *gs);

game_object* make_sh_line(dynamic_objects *dyn, vec2 a, vec2 b, vec4 color);
game_object* make_sh_line(vec2 a, vec2 b, vec4 color);
game_object* make_sh_circle(float x, float y, float radius, float velocity, vec2 direction, vec4 color);
game_object* make_sh_rect(float x, float y, float width, float height, vec4 color);
int col_cric_rect(rect_object *rect_obj, ball_object *ball_obj, vec2 *result);
void render_grid(game_grid *grid, int pos_attrib, int transform_att, int color_attrib);

void move_dynamic_elem_to_grid(game_state *gamestate, game_object *obj,
                              int from_index, int to_index);

struct game_state {
    dynamic_objects objects_to_update;
    vec2 window_width_height;
    HMODULE lib;
    FILETIME last_write_time;
    game_update_func *update;
    game_render_func *render;
    game_init_func *init;

    game_object **named_objects;
    game_grid overlord;
    GLFWwindow *window;
    unsigned int current_vao;
    int object_count;

    int current_program;
    int vpos_attrib_loc;
    int color_attrib_loc;
    int model_t_attrib_loc;
    int is_init;
    int full_screen;
};
