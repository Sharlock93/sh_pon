#ifndef GAME_DEBUG_H
#define GAME_DEBUG_H

#define TIME_BLOCK__(number) debug_timed_block block##number(__COUNTER__, __FILE__,  __FUNCTION__, __LINE__)
#define TIME_BLOCK_(number) TIME_BLOCK__(number)
#define TIME_BLOCK TIME_BLOCK_(__LINE__)
#define sh_assert(con) if(!(con)) __debugbreak(); 

struct game_object;

struct debug_record {
    int line;
    char *filename;
    char *func_name;
    int hitcount;
    int time; 
};

debug_record records[];

struct debug_timed_block {
    debug_record *record;

    debug_timed_block(int counter, char *filename, char *func_name, int line) {
        record = records + counter;
        record->filename = filename;
        record->func_name = func_name;
        record->hitcount++;
        record->time = __rdtsc();
    }

    ~debug_timed_block() {
        record->time = __rdtsc() - record->time;
    }
};

struct sh_ui_state { unsigned int hot_object;
    unsigned int active_object;
};

struct sh_debug_ui_state { 
    game_object *active_object; //@Todo(sharo): this should have the ability to contain multiple objects
    game_object *hot_object; //@Todo(sharo): this should have the ability to contain multiple objects
};
 
struct sh_rect_container {
    vec2 pos;
    float width;
    float height;
};

struct sh_circ_container {
    vec2 pos;
    float r;
};


void sh_memcpy(uint8 *dest_mem, uint8 *source, uint32 bytes_to_cpy);
void sh_memset(uint8 *mem, uint8 data, uint32 byte_size);
uint32 sh_streq(const char *str1, const char *str2, uint32 bytes_to_cmp);
uint8* sh_cpy_str(uint8 *start, uint32 length);

int32 sh_parse_int(uint8 *mem, uint8 str_len);
vec4 sh_parse_vec4(uint8 *start, uint32 len);//format: x, y, z, w
vec2 sh_parse_vec2(uint8 *start, uint32 len);//format: x, y
int32 sh_abs(int32 number);
char* sh_inttstr(int32 val);
char* sh_flttstr(float val);
char* sh_vec2tstr(vec2 *vec);

struct game_state;
struct input_state;

void render_rect(vec2 pos, float width, float height, vec4 color, int vpos_attrib, int color_attrib);
int sh_button(game_state *gs, unsigned int id, vec2 position, char *text, vec4 color);
void write_to_log(game_state *gs, char *str);
void write_to_gl_log(char *str);
void dump_struct_to_screen(void *data, struct_meta_info *meta, int meta_count, int font_size, vec2 *pos);

void dump_object_log(void* data, struct_meta_info* met, int meta_count, char* var_name);
void dump_log_stamp(char* text, int line_number, char* file_name);

int32 has_mouse_clicked_text(sh_fnt *fnt, input_state *input, char *text, int font_size, vec2 pos);
int32 has_mouse_hovered_text(sh_fnt *fnt, input_state *input, char *text, int font_size, vec2 pos);
sh_rect_container get_text_rect(sh_fnt *fnt, int font_size, char *text, vec2 pos);

#define DUMP_STAMP(text) dump_log_stamp(text, __LINE__, __FILE__);
#endif
