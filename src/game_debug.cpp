#include "../header/game_debug.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../header/stb_image.h"

void render_rect(vec2 pos, float width, float height, vec4 color, int vpos_attrib, int color_attrib) {
    vec2 vertex[] = {
        vec2(pos.x - width/2.0, pos.y + height/2.0), 
        vec2(pos.x + width/2.0, pos.y + height/2.0), 
        vec2(pos.x + width/2.0, pos.y - height/2.0), 
        vec2(pos.x - width/2.0, pos.y - height/2.0)
    };

    unsigned int vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec2)*4, vertex, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(vpos_attrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glUniform4fv(color_attrib, 1, &color[0]);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &vbo);
    
}


int sh_button(game_state *gs, unsigned int id, vec2 position, char *text, float width, float height)
{
    input_state *inputs = gs->inputs;
    sh_ui_state *state = &gs->ui_state;
    vec2 mouse = vec2(inputs->mouse.mouse_x, inputs->mouse.mouse_y);
    draw_element rect = sh_draw_rect(position, width, height, vec4(1, 1, 1, 1));

    int clicked = 0;

    sh_rect_container rect_cont = {position, width, height};

    if(state->active_object == id) {
        rect.color = vec4(0, 1, 1, 1);
    }

    if(point_in_rect(&mouse, &rect_cont)) {
        state->hot_object = id;
        if(inputs->mouse.left_button) {
            state->active_object = id;
            rect.color = vec4(1, 0, 0, 1);
            clicked = 1;
        } else {
            rect.color = vec4(1, 1, 0, 1);
        }
    }

    push_draw_text(text, 12, position);
    push_draw_element(&gs->renderstack, rect);

    return clicked;
}

int sh_button_circ(game_state *gs, unsigned int id, vec2 position, char *text, float r) {
    input_state *inputs = gs->inputs;
    sh_ui_state *state = &gs->ui_state;

    vec2 mouse = vec2(inputs->mouse.mouse_x, inputs->mouse.mouse_y);
    draw_element circ = sh_draw_circ(position, r, vec4(1, 1, 1, 1));

    int clicked = 0;
    sh_circ_container circ_cont = {position, r};

    if(state->active_object == id) {
        circ.color = vec4(0, 1, 1, 1);
    }

    if(point_in_circ(&mouse, &circ_cont)) {
        state->hot_object = id;
        if(inputs->mouse.left_button) {
            state->active_object = id;
            circ.color = vec4(1, 0, 0, 1);
            clicked = 1;
        } else {
            circ.color = vec4(1, 1, 0, 1);
        }
    }

    push_draw_element(&gs->renderstack, circ);

    return clicked;
}

void sh_memcpy(uint8 *dest_mem, uint8 *source, uint32 bytes_to_cpy) {
    while(bytes_to_cpy--) {
        *( dest_mem + bytes_to_cpy) = *(source + bytes_to_cpy);
    }
}

uint32 sh_streq(const char *str1, const char *str2, uint32 bytes_to_cmp) {
    while(( bytes_to_cmp > 0 ) && ( *str1 != '\0' ) && ( *str2  != '\0' ) && (*str1 == *str2)) {
        ++str1;
        ++str2;
        --bytes_to_cmp;
    }

    return bytes_to_cmp == 0;
}

int32 sh_parse_int(uint8 *mem, uint8 str_len) {
    int32 number = 0;
    int32 sign = 1;
    if(*mem == '-') {
        sign = -1;
        str_len--;
        mem++;
    }
    
    while(str_len-- > 0) {
        number = number*10 + (*mem - '0');
        mem++;
    }

    return sign*number;

}

vec4 sh_parse_vec4(uint8 *start, uint32 len) {
    vec4 vec;
    uint8 index = 0;
    uint8 *val_start = start;
    uint8 *val_end = start;
    while(val_end != (start + len)) {
        while(*val_end != ','  && val_end != (start + len)) val_end++;
        vec[index++] = sh_parse_int(val_start, val_end - val_start);
        if(val_end == (start + len)) break;
        val_end++;
        val_start = val_end; 
    }

    return vec;
}

vec2 sh_parse_vec2(uint8 *start, uint32 len) {
    vec2 vec;
    uint8 index = 0;
    uint8 *val_start = start;
    uint8 *val_end = start;
    while(val_end != (start + len)) {
        while(*val_end != ','  && val_end != (start + len)) val_end++;
        vec[index++] = sh_parse_int(val_start, val_end - val_start);
        if(val_end == (start + len)) break;
        val_end++;
        val_start = val_end; 
    }

    return vec;
}

uint8* sh_cpy_str(uint8 *start, uint32 length) {
    uint8 *new_str = (uint8 *) malloc(length + 1); 
    sh_memcpy(new_str, start, length);
    *( new_str + length) = '\0';
    
    return new_str;
}


uint32 number_of_lines(uint8 *str, uint32 size) {
    uint32 count = 0;
    while(*str != '\0' && size >= 0) {
        if(*str == '\n') count++;
        str++;
        size--;
    }

    return count;
}


uint8* get_nextline(uint8 *current_pos, uint32 mem_size) {
    while(( mem_size--  >= 0)) {
        if(( *current_pos == '\r' && *( current_pos + 1 ) == '\n' ) || ( *current_pos == '\n' )) {
            if(*current_pos == '\n'){
                current_pos++;
                break;
            }

            current_pos += 2;
            break;
        }
        current_pos++;
    }
    return current_pos;
}

uint8* get_line_end(uint8 *current_pos, uint32 mem_size) {
    while(*current_pos != '\n' && *current_pos != '\r' && ( mem_size--  >= 0)) current_pos++;
    return current_pos;
}

uint8* skip_to_str_end(uint8 *quote_start) {
    char quote_type = *quote_start;
    quote_start++;
    while(*quote_start != quote_type) quote_start++;
    return quote_start;
}

token_type get_token_type(sh_token *token) {

    if(sh_streq((const char *) token->t_start, "char", TOKEN_LEN(token))) {
        return CHAR_FNT_INFO;
    }

    if(sh_streq((const char *)token->t_start, "info", TOKEN_LEN(token))) {
        return INFO;
    }

    if(sh_streq((const char *) token->t_start, "common", TOKEN_LEN(token))) {
        return COMMON; 
    }

    if(sh_streq((const char *)token->t_start, "page", TOKEN_LEN(token))) {
        return PAGE;
    }

    if(sh_streq((const char *) token->t_start, "chars", TOKEN_LEN(token))) {
        return CHAR_ID; 
    }

    return NOTHING;
}

sh_token get_next_token(uint8 *pos, uint8 seperator) {
    sh_token token = {};
    while(*pos == ' ') pos++;
    token.t_start = pos;
    while(*pos != seperator && *pos != '\n' && *pos != '\r') {
        if(*pos == '\"' || *pos == '\'') {
            pos = skip_to_str_end(pos);
        }

        pos++;
    }
    token.t_end = pos;

    token.type = get_token_type(&token);

    return token;
}

uint8* move_to_value(uint8 *cur_point, uint8 seperator) {
    while(*cur_point != '\0' &&  *cur_point != seperator) {
        ++cur_point;
    }
    ++cur_point;

    return cur_point;
}

void fill_fnt_info(uint8 *start_mem, fnt_info *fntinf) {
    sh_token tok = get_next_token(start_mem); 
    uint8 sep = '=';
    //face info
    tok.t_start = move_to_value(tok.t_start, sep);
    uint8 face_name_len = (uint8) (TOKEN_LEN((&tok)));
    fntinf->face = (char *)sh_cpy_str(tok.t_start, face_name_len);
    
    //size
    tok = get_next_token(tok.t_end);
    tok.t_start = move_to_value(tok.t_start, sep);
    fntinf->size = sh_parse_int(tok.t_start, TOKEN_LEN((&tok)));

    //bold
    tok = get_next_token(tok.t_end);
    tok.t_start = move_to_value(tok.t_start, sep);
    fntinf->bold = sh_parse_int(tok.t_start, TOKEN_LEN((&tok)));

    tok = get_next_token(tok.t_end);
    tok.t_start = move_to_value(tok.t_start, sep);
    fntinf->italic = sh_parse_int(tok.t_start, TOKEN_LEN((&tok)));

    tok = get_next_token(tok.t_end);
    tok.t_start = move_to_value(tok.t_start, sep);
    fntinf->charset = (char *) sh_cpy_str(tok.t_start, TOKEN_LEN((&tok)));

    tok = get_next_token(tok.t_end);
    tok.t_start = move_to_value(tok.t_start, sep);
    fntinf->unicode = sh_parse_int(tok.t_start, TOKEN_LEN((&tok)));

    tok = get_next_token(tok.t_end);
    tok.t_start = move_to_value(tok.t_start, sep);
    fntinf->stretchH = sh_parse_int(tok.t_start, TOKEN_LEN((&tok)));

    tok = get_next_token(tok.t_end);
    tok.t_start = move_to_value(tok.t_start, sep);
    fntinf->smooth = sh_parse_int(tok.t_start, TOKEN_LEN((&tok)));

    tok = get_next_token(tok.t_end);
    tok.t_start = move_to_value(tok.t_start, sep);
    fntinf->aa = sh_parse_int(tok.t_start, TOKEN_LEN((&tok)));

    tok = get_next_token(tok.t_end);
    tok.t_start = move_to_value(tok.t_start, sep);
    fntinf->padding = sh_parse_vec4(tok.t_start, TOKEN_LEN((&tok)));

    tok = get_next_token(tok.t_end);
    tok.t_start = move_to_value(tok.t_start, sep);
    fntinf->spacing = sh_parse_vec2(tok.t_start, TOKEN_LEN((&tok)));

    tok = get_next_token(tok.t_end);
    tok.t_start = move_to_value(tok.t_start, sep);
    fntinf->outline = sh_parse_int(tok.t_start, TOKEN_LEN((&tok)));

}

void fill_common_info(uint8 *start_mem, fnt_common *common) {
    uint8 sep = '=';
    sh_token tok = get_next_token(start_mem); 
    tok.t_start = move_to_value(tok.t_start, sep);
    common->lineHeight = sh_parse_int(tok.t_start, TOKEN_LEN((&tok)));
    
    tok = get_next_token(tok.t_end);
    tok.t_start = move_to_value(tok.t_start, sep);
    common->base = sh_parse_int(tok.t_start, TOKEN_LEN((&tok)));

    tok = get_next_token(tok.t_end);
    tok.t_start = move_to_value(tok.t_start, sep);
    common->scaleW = sh_parse_int(tok.t_start, TOKEN_LEN((&tok)));

    tok = get_next_token(tok.t_end);
    tok.t_start = move_to_value(tok.t_start, sep);
    common->scaleH = sh_parse_int(tok.t_start, TOKEN_LEN((&tok)));

    tok = get_next_token(tok.t_end);
    tok.t_start = move_to_value(tok.t_start, sep);
    common->pages = sh_parse_int(tok.t_start, TOKEN_LEN((&tok)));

    tok = get_next_token(tok.t_end);
    tok.t_start = move_to_value(tok.t_start, sep);
    common->packed = sh_parse_int(tok.t_start, TOKEN_LEN((&tok)));

    tok = get_next_token(tok.t_end);
    tok.t_start = move_to_value(tok.t_start, sep);
    common->alphaChnl = sh_parse_int(tok.t_start, TOKEN_LEN((&tok)));

    tok = get_next_token(tok.t_end);
    tok.t_start = move_to_value(tok.t_start, sep);
    common->redChnl = sh_parse_int(tok.t_start, TOKEN_LEN((&tok)));

    tok = get_next_token(tok.t_end);
    tok.t_start = move_to_value(tok.t_start, sep);
    common->greenChnl = sh_parse_int(tok.t_start, TOKEN_LEN((&tok)));

    tok = get_next_token(tok.t_end);
    tok.t_start = move_to_value(tok.t_start, sep);
    common->blueChnl = sh_parse_int(tok.t_start, TOKEN_LEN((&tok)));
}

void fill_fnt_page_info(uint8 *mem, fnt_page_inf *pg_inf) {
    uint8 sep = '=';
    
    sh_token tok = get_next_token(mem);
    tok.t_start = move_to_value(tok.t_start, sep);
    pg_inf->id = sh_parse_int(tok.t_start, TOKEN_LEN(( &tok )));

    tok = get_next_token(tok.t_end);
    tok.t_start = move_to_value(tok.t_start, sep);
    pg_inf->file = (char *) sh_cpy_str(tok.t_start, TOKEN_LEN((&tok)));
}

void fill_fnt_chars(uint8 *mem, fnt_chars *fnt_chars) {
    uint8 sep = '=';
    
    sh_token tok = get_next_token(mem);
    tok.t_start = move_to_value(tok.t_start, sep);
    fnt_chars->count = sh_parse_int(tok.t_start, TOKEN_LEN(( &tok )));
}

void fill_fnt_char(uint8 *mem, fnt_char *char_info) {
    uint8 sep = '=';

    sh_token tok = get_next_token(mem);
    tok.t_start = move_to_value(tok.t_start, sep);
    char_info->id = sh_parse_int(tok.t_start, TOKEN_LEN(( &tok )));

    tok = get_next_token(tok.t_end);
    tok.t_start = move_to_value(tok.t_start, sep);
    char_info->x = sh_parse_int(tok.t_start, TOKEN_LEN(( &tok )));

    tok = get_next_token(tok.t_end);
    tok.t_start = move_to_value(tok.t_start, sep);
    char_info->y = sh_parse_int(tok.t_start, TOKEN_LEN(( &tok )));

    tok = get_next_token(tok.t_end);
    tok.t_start = move_to_value(tok.t_start, sep);
    char_info->width = sh_parse_int(tok.t_start, TOKEN_LEN(( &tok )));

    tok = get_next_token(tok.t_end);
    tok.t_start = move_to_value(tok.t_start, sep);
    char_info->height = sh_parse_int(tok.t_start, TOKEN_LEN(( &tok )));

    tok = get_next_token(tok.t_end);
    tok.t_start = move_to_value(tok.t_start, sep);
    char_info->xoffset = sh_parse_int(tok.t_start, TOKEN_LEN(( &tok )));

    tok = get_next_token(tok.t_end);
    tok.t_start = move_to_value(tok.t_start, sep);
    char_info->yoffset = sh_parse_int(tok.t_start, TOKEN_LEN(( &tok )));

    tok = get_next_token(tok.t_end);
    tok.t_start = move_to_value(tok.t_start, sep);
    char_info->xadvance = sh_parse_int(tok.t_start, TOKEN_LEN(( &tok )));

    tok = get_next_token(tok.t_end);
    tok.t_start = move_to_value(tok.t_start, sep);
    char_info->page = sh_parse_int(tok.t_start, TOKEN_LEN(( &tok )));

    tok = get_next_token(tok.t_end);
    tok.t_start = move_to_value(tok.t_start, sep);
    char_info->chnl = sh_parse_int(tok.t_start, TOKEN_LEN(( &tok )));

}

void sh_read_character_descripter_in_memory(sh_fnt *font, uint8 *memory, uint32 size) {
    uint32 lines = number_of_lines(memory, size);

    uint8 *line_start = memory;
    uint8 *line_end = get_line_end(line_start, size);
    
    
    sh_token token = get_next_token(line_start);

    fill_fnt_info(token.t_end, &font->info);
    
    line_start = get_nextline(line_start, size);
    line_end = get_line_end(line_start, size);
    token = get_next_token(line_start);

    fill_common_info(token.t_end, &font->common);

    line_start = get_nextline(line_start, size);
    line_end = get_line_end(line_start, size);
    token = get_next_token(line_start);
    
    fill_fnt_page_info(token.t_end, &font->page);

    line_start = get_nextline(line_start, size);
    line_end = get_line_end(line_start, size);
    token = get_next_token(line_start);

    fill_fnt_chars(token.t_end, &font->chars);


    font->characters = (fnt_char *)(malloc(sizeof(fnt_char)*font->chars.count));
    
    for(uint32 i = 0; i < font->chars.count; ++i) {
        line_start = get_nextline(line_start, size);
        line_end = get_line_end(line_start, size);
        token = get_next_token(line_start);

        fill_fnt_char(token.t_end, font->characters + i);
    }

}

void sh_memset(uint8 *mem, uint8 data, uint32 byte_size) {
    while(byte_size-- > 0){
        *mem++ = data;
    }
}


int32 sh_abs(int32 number) {
    return ( number > 0) ? number : -number;
}

uint32 sh_png_paeth_defilter(int32 a, int32 b, int32 c) {
    int32 p = a + b - c;
    int32 pa = sh_abs( p - a );
    int32 pb = sh_abs( p - b );
    int32 pc = sh_abs( p - c );

    if(pa <= pb && pa <= pc) return a;
    if(pb <= pc) return b;
    return c;
}

uint8* sh_defilter_png(uint8 *decompressed_png, sh_png_ihdr *ihdr) {
    uint32 x = ihdr->x;
    uint32 y = ihdr->y;
    uint32 byte_per_pixel = 1;//( ihdr->bit_depth );
    uint32 stride = x*byte_per_pixel;
    uint8 *row = decompressed_png;
    uint8 *unfiltered = (uint8 *) malloc(x*y*byte_per_pixel);
    uint8 *current_working = unfiltered;
    for(uint32 i = 0; i < y; ++i) {
        uint8 filter = *row++; //@Note(sharo): filter is one byte before the scanline
        current_working = unfiltered + i*stride;
        
        switch(filter) {
            case sh_no_filter: {
                for(uint32 j = 0; j < stride; ++j) {
                    current_working[j] = row[j];
                }
            } break;

            case sh_sub_filter: {
                for(uint32 j = 0; j < stride; ++j) {
                    uint8 a = 0;
                    if(j != 0) {
                        a = current_working[j-1];
                    }
                    uint8 value = row[j] + (a);
                    current_working[j] = value;
                }
            } break;
            
            case sh_up_filter: {
                uint8 *prev_scanline = ( current_working - stride );
                for(uint32 j = 0; j < stride; ++j) {
                    uint8 b = prev_scanline[j];
                    uint8 value = row[j] + (b);
                    current_working[j] = value;
                }
            } break;

            case sh_avg_filter: {
                uint8 *prev_scanline = ( current_working - stride );
                for(uint32 j = 0; j < stride; ++j) {
                    uint8 a = 0;
                    uint8 b = prev_scanline[j];
                    
                    if(j != 0) {
                        a = current_working[j-1];
                    }

                    uint8 value = row[j] + ((a + b) >> 1);
                    current_working[j] = value;
                }
            } break;

            case sh_paeth_filter: {
                uint8 *prev_scanline = ( current_working - stride );
                for(uint32 j = 0; j < stride; ++j) {
                    int a = 0;
                    int b = prev_scanline[j];
                    int c = 0;
                    
                    if(j != 0) {
                        a = current_working[j-1];
                        c = prev_scanline[j-1];
                    }
                    
                    uint8 value = ( row[j] + sh_png_paeth_defilter(a, b, c) ) & 255;
                    current_working[j] = value;
                }

            } break;
        } 

        row += (stride);
        // current_working += (stride);
    }

    return unfiltered;
}

void sh_load_png_mem(uint8 *mem, uint32 mem_size) {
   
    uint8 *memory = mem;
    // sh_png_img image = {};
    sh_png_sig signature = {};
    sh_png_read_sig(memory, &signature);
    SKIP_BYTES(memory, 8);

    sh_png_chunk *chunks = (sh_png_chunk *) malloc(sizeof(sh_png_chunk)*2);

    int i = 0;
    sh_png_ihdr ihdr;
    uint8 *image = nullptr;

    for(;;) { 
        chunks[i] = sh_png_read_chunk(memory);
        
        if(sh_streq((char *)&chunks[i].head , "IHDR", 4)) {
            ihdr = sh_png_read_head(chunks + i); 
        }

        if(sh_streq((char *) &chunks[i].head, "IDAT", 4)) {
            sh_png_zlib zlib_block = sh_png_read_zlib_block(chunks[i].data, chunks[i].len);
            uint8 *data = sh_decompress_png_deflate(&zlib_block);
            image = sh_defilter_png(data, &ihdr);
        }

        if(sh_streq((char *) &chunks[i].head , "IEND", 4)) {
            break;
        }

        SKIP_BYTES(memory, chunks[i].len + 4 + 4 + 4);
        i++;
    }

}
