
float sh_get_scale_for_pixel(sh_fnt *font, int font_size) {
    return (float)font_size/-font->info.size; 
}

uint8* get_line_end(uint8 *current_pos, uint32 mem_size) {
    while(*current_pos != '\n' && *current_pos != '\r' && ( mem_size--  >= 0)) current_pos++;
    return current_pos;
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


uint8* skip_to_str_end(uint8 *quote_start) {
    char quote_type = *quote_start;
    quote_start++;
    while(*quote_start != quote_type) quote_start++;
    return quote_start;
}

uint8* move_to_value(uint8 *cur_point, uint8 seperator) {
    while(*cur_point != '\0' &&  *cur_point != seperator) {
        ++cur_point;
    }
    ++cur_point;

    return cur_point;
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


int32 sh_strlen(uint8 *string) {
    uint8 *start = string;
    while(*string++ != '\0');
    return (int32) ( string - start );
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
    int id = sh_parse_int(tok.t_start, TOKEN_LEN(( &tok )));
    char_info = char_info + id;
    char_info->id = id;

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

    // font->characters = (fnt_char *)(malloc(sizeof(fnt_char)*font->chars.count));
    
    for(uint32 i = 0; i < font->chars.count; ++i) {
        line_start = get_nextline(line_start, size);
        line_end = get_line_end(line_start, size);
        token = get_next_token(line_start);

        fill_fnt_char(token.t_end, font->characters);
    }
}



