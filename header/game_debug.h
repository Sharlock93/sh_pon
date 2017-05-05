#ifndef GAME_DEBUG_H
#define GAME_DEBUG_H
#include <intrin.h>
#include <stdint.h>



#define TIME_BLOCK__(number) debug_timed_block block##number(__COUNTER__, __FILE__,  __FUNCTION__, __LINE__)
#define TIME_BLOCK_(number) TIME_BLOCK__(number)
#define TIME_BLOCK TIME_BLOCK_(__LINE__)

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

struct sh_ui_state {
    unsigned int hot_object;
    unsigned int active_object;
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

typedef enum {
    NOTHING,
    INFO,
    COMMON,
    PAGE,
    CHAR_ID,
    CHAR_FNT_INFO
} token_type;

struct sh_token {
    uint8 *t_start;
    uint8 *t_end;
    token_type type;
};

struct fnt_info {
    char *face;
    int32 size;
    uint8 bold;
    uint8 italic;
    char *charset;
    uint8 unicode;
    int32 stretchH;
    uint8 smooth;
    uint8 aa;
    vec4 padding;
    vec2 spacing;
    uint8 outline;
};

struct fnt_common {
    uint32 lineHeight;
    uint32 base;
    uint32 scaleW;
    uint32 scaleH;
    uint32 pages;
    uint32 packed;
    uint32 alphaChnl;
    uint32 redChnl;
    uint32 greenChnl;
    uint32 blueChnl;
};

struct fnt_page_inf {
    uint32 id;
    char *file;
};

struct fnt_chars {
    uint32 count;
};

struct fnt_char {
    uint32 id;
    uint32 x;
    uint32 y;
    uint32 width;
    uint32 height;
    uint32 xoffset;
    uint32 yoffset;
    uint32 xadvance;
    uint32 page;
    uint32 chnl;
};

struct sh_fnt {
    fnt_info info;
    fnt_common common;
    fnt_page_inf page;
    fnt_chars chars;
    fnt_char *characters;
};

void sh_memcpy(uint8 *dest_mem, uint8 *source, uint32 bytes_to_cpy);
uint32 sh_streq(const char *str1, const char *str2, uint32 bytes_to_cmp);

#define TOKEN_LEN(token) (token->t_end - token->t_start)
#define TOKEN_LEN(token) (token->t_end - token->t_start)

void sh_load_png_mem(uint8 *mem, uint32 size);

void sh_read_character_descripter_in_memory(sh_fnt *font, uint8 *memory, uint32 size);
uint32 number_of_lines(uint8 *str, uint32 size);
uint8* get_nextline(uint8 *current_pos, uint32 mem_size);
uint8* get_line_end(uint8 *current_pos, uint32 mem_size);
uint8* skip_to_str_end(uint8 *quote_start);
token_type get_token_type(sh_token *token);
sh_token get_next_token(uint8 *pos, uint8 seperator = ' ');

#endif
