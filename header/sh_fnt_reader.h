#ifndef SH_FNT_READER
#define SH_FNT_READER
#define TOKEN_LEN(token) (token->t_end - token->t_start)
#define PNT2PXL(pt) (pt)

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
    fnt_char characters[256]; //@Note(sharo): cover the first 256 characters
};

float sh_get_scale_for_pixel(sh_fnt *font, int font_size);
uint8* get_nextline(uint8 *current_pos, uint32 mem_size);
uint8* get_line_end(uint8 *current_pos, uint32 mem_size);
uint8* skip_to_str_end(uint8 *quote_start);
uint8* move_to_value(uint8 *cur_point, uint8 seperator);
int32 sh_strlen(uint8 *string);

token_type get_token_type(sh_token *token);
sh_token get_next_token(uint8 *pos, uint8 seperator = ' ');

void fill_fnt_info(uint8 *start_mem, fnt_info *fntinf);
void fill_common_info(uint8 *start_mem, fnt_common *common);
void fill_fnt_page_info(uint8 *mem, fnt_page_inf *pg_inf);
void fill_fnt_chars(uint8 *mem, fnt_chars *fnt_chars);
void fill_fnt_char(uint8 *mem, fnt_char *char_info);
void sh_read_character_descripter_in_memory(sh_fnt *font, uint8 *memory, uint32 size);

#endif
