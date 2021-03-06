#ifndef GRID_MANAGMENT_H
#define GRID_MANAGMENT_H

#define HELLO
#undef SH_API
#define SH_API __declspec(dllexport)
//Note(sharo): some defines 
#define dt (double)(1.0/60.0)
#define Assert(condtion) if(!(condtion)) {*(int *)0 = 0;}
#define INDEX(row, col, stride) ((row*stride) + col)

struct game_object;
struct ball_object;
struct rect_object;
struct line_object;
struct objects;
struct mouse_state;
struct keyboard_state;
struct input_state;
struct object_bucket;
struct object_bucket_list;
struct grid_element;
struct game_grid;
struct game_state;
struct draw_stack;
struct draw_element;
struct sh_ui_state;
struct sh_phy_force;
struct sh_col_queue;
struct sh_col_info;

game_state *gl_game_state = nullptr;
FILE *gl_log_file = nullptr;

#define GAME_UPDATE_FUNC(name) void name(game_state *gamestate, input_state *inputs, double dts, int32 without_move) 
typedef GAME_UPDATE_FUNC(game_update_func);

#define GAME_RENDER_FUNC(name) void name(game_state *gamestate, double alpha)
typedef GAME_RENDER_FUNC(game_render_func);

#define GAME_INIT_FUNC(name) void name(game_state *gamestate)
typedef GAME_INIT_FUNC(game_init_func);

#define GAME_DEBUG_FUNC(name) void name(game_state *gamestate)
typedef GAME_DEBUG_FUNC(game_debug_func);


SH_API void init_grid(game_grid *grid, float grid_width, float grid_height, int row, int col, vec2 pos);
SH_API void init_grid_elem(game_grid *grid, int index, vec4 color);
SH_API void add_to_obj_bucket_list(object_bucket_list *obj_bckt, game_object *obj);
SH_API void add_to_objects(objects *dyn_obj, game_object *obj);
SH_API void add_to_grid_objects(game_state *gs, game_object *bucket, int staic_obj);

SH_API void add_to_named_objects(game_state *gs, game_object *object);

SH_API void remove_from_grid_elem(object_bucket_list *list, object_bucket **bucket_ptr);
SH_API int find_grid_index(game_grid *grid, vec2 position);

SH_API void render_grid(game_state *gs);
SH_API void render_grid_elem(grid_element *grid_elem, int pos_attrib, int transform_att, int color_att);
SH_API void render_draw_stack(game_state *gamestate);
SH_API void render_draw_stack(game_state *gs, draw_stack *stack);
SH_API void render_game_object(game_state *gs, game_object *gm_obj);

SH_API void move_bucket_elem_to_grid(game_state *gs, object_bucket *mvd_obj_bucket, int from_index, int to_index);

SH_API object_bucket* in_grid_element(grid_element *elem, game_object *object);
SH_API int is_bucket_in_list(object_bucket_list *list, object_bucket *bucket);

SH_API game_object* make_sh_line(objects *dyn, vec2 a, vec2 b, vec4 color);
SH_API game_object* make_sh_line(int id, vec2 a, vec2 b, vec4 color);
SH_API game_object* make_sh_circle(int id, float x, float y, float radius, float velocity, vec2 direction, vec4 color);
SH_API game_object* make_sh_rect(int id, float x, float y, float velocity, float width, float height, vec4 color);

draw_element sh_gen_draw_rect(vec2 pos, float width, float height, vec4 color);
draw_element sh_gen_draw_rect_fill(vec2 pos, float width, float height, vec4 color);
draw_element sh_gen_draw_circ(vec2 pos, float r, vec4 color);
draw_element sh_gen_draw_line(vec2 a, vec2 b, vec4 color);
draw_element sh_gen_draw_text(sh_fnt *fnt, char *text, int font_size, vec2 position, vec4 color);
draw_element sh_gen_triangle(vec2 pos, float height, float width, int triangle_poining, vec4 color);
draw_element sh_gen_draw_triangle(vec2 pos, float height, float width, int triangle_poining, vec4 color);

void push_draw_element(draw_stack *stack, draw_element elem);
void push_draw_text(game_state *gs, char *text, int font_size, vec2 position, vec4 color);
int pop_element(draw_stack *stack, draw_element *draw);
void make_stack_capacity(draw_stack *stack, int new_capacity); 

void sh_col_q_init(sh_col_queue* q, int init_capacity);
void sh_col_q_push(sh_col_queue* q, sh_col_info sh_col_info);
sh_col_info sh_col_q_pop(sh_col_queue* q);


#undef SH_API
#define SH_API
enum  object_type {
    BALL,
    RECTANGLE,
    LINE,
    WALL
};

struct  mouse_state {
    int          left_button;
    int          right_button;
    double       mouse_x;
    double       mouse_y;
    game_object *hot_object;
};

struct  keyboard_state {
    int arrow_left;
    int arrow_right;
};

struct  input_state {
    mouse_state    mouse;
    keyboard_state keyboard;
};


struct  game_object {
    object_type type;
    int           id = -1;
    int           size; 
    int           moved;
    void         *obj;
    game_object  *next_object;
    game_object  *previous_object;
    grid_element *in_grids;

};

sh_inspect struct  ball_object {
    int        vec_nums;
    int        vec_capacity;
    float      velocity;
    vec2       previous_pos;
    sh_circle* circ;
    sh_phy_force* forces;
};


struct rect_object {
    vec2    previous_pos;
    float   velocity;
    sh_rect* rect;
};

sh_inspect struct line_object {
    sh_line* line;       
    vec2 normal;
};

struct objects {
     game_object* root_node;
     game_object* last_node;
     int         object_count;
}; 

struct object_bucket {
    object_bucket *next;
    object_bucket *prev;
    int           to_delete;
    game_object   *obj;
    int           object_count;
};

struct object_bucket_list {
    object_bucket *root_node;
    object_bucket *last_node;
    int            object_count;
};

struct grid_element {
    object_bucket_list *obj_buckets;

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


struct draw_element {
    vec2 *points;
    vec4  color;
    int   point_count;
    int   command; //OpenGL command
    int   has_texture;
};

struct draw_stack {
    draw_element *elements;
    int elem_count;
    int capacity;
};

typedef enum {
    COL_SWP_CIRC_LINE
} col_type;

struct sh_col_info {
    bool collied;
    col_type type;
    float dt1; // t on which the first object is closest to object 2
    float dt2; // t on which the second object is closest to object 1

    game_object* o1;
    game_object* o2;
};

struct sh_col_queue {
    int size;
    int capacity;

    int head;
    int tail;
    sh_col_info *q;
};


sh_inspect struct game_state {
    HMODULE lib;
    FILETIME last_write_time;

    vec2 window_width_height;
    
    uint32 current_vao;
    int object_count;
    int current_program;
    int vpos_attrib_loc;
    int color_attrib_loc;
    int model_t_attrib_loc;
    int tex_coord;
    int has_texture_attrib;
    int is_init;
    int full_screen;

    game_grid        overlord;
    game_object      **named_objects;
    objects          *objects_to_update;
    objects          *static_objects;
    GLFWwindow       *window;

    game_update_func *update;
    game_render_func *render;
    game_init_func   *init;
    game_debug_func  *debug_func;
    game_debug_func  *clean_up_func;
    sh_ui_state      ui_state;
    sh_debug_ui_state debug_ui_state;
    draw_stack       renderstack;
    sh_col_queue     col_queue;
    sh_fnt           font;
    uint32           fnt_tex;
    FILE             *log_file;

    input_state      *inputs;
    game_state       *debug_state; //Note(sharo): a gamestate would debug this state
};
#undef SH_API
#define SH_API __declspec(dllexport)
#endif
