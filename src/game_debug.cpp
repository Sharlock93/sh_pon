void sh_memcpy(uint8 *dest_mem, uint8 *source, uint32 bytes_to_cpy) {
    while(bytes_to_cpy--) {
        *( dest_mem + bytes_to_cpy) = *(source + bytes_to_cpy);
    }
}

void sh_memset(uint8 *mem, uint8 data, uint32 byte_size) {
    while(byte_size-- > 0){
        *mem++ = data;
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

uint8* sh_cpy_str(uint8 *start, uint32 length) {
    uint8 *new_str = (uint8 *) malloc(length + 1); 
    sh_memcpy(new_str, start, length);
    *( new_str + length) = '\0';
    
    return new_str;
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

int32 sh_abs(int32 number) {
    return ( number > 0) ? number : -number;
}

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


int sh_button(game_state *gs, unsigned int id, vec2 position, char *text, vec4 color) {
    int32 text_length = sh_strlen((uint8 *)text)-1;
    float width = 0;
    float height = 20;

    float scale = sh_get_scale_for_pixel(&gs->font, 20);

    for(int32 i = 0; i < text_length; ++i) {
        fnt_char *ch = gs->font.characters + text[i];
        width += ch->xadvance*scale;
    }

    input_state *inputs = gs->inputs;
    sh_ui_state *state = &gs->ui_state;
    vec2 mouse = vec2(inputs->mouse.mouse_x, inputs->mouse.mouse_y);
    draw_element rect = sh_gen_draw_rect_fill(position, width + 10, height + 10, color);

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

    push_draw_text(gs, text, height, vec2(position.x - width/2.0, position.y - height/2.0), vec4(0, 0, 0, 1));
    push_draw_element(&gs->renderstack, rect);

    return clicked;
}

int sh_button_circ(game_state *gs, unsigned int id, vec2 position, char *text, float r) {
    input_state *inputs = gs->inputs;
    sh_ui_state *state = &gs->ui_state;

    vec2 mouse = vec2(inputs->mouse.mouse_x, inputs->mouse.mouse_y);
    draw_element circ = sh_gen_draw_circ(position, r, vec4(1, 1, 1, 1));

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


char* sh_inttstr(int32 val) {
    static char buffer[12] = {};

    char *result = buffer + 11;
    do {
        result--;
        int8 curr_dig = val%10;
        *result = '0' + curr_dig; 
        val /= 10;
    } while(val);

    return result;
}

char* sh_flttstr(float val) {
    //Todo(sharo): implement this
    static char buffer[64] = {};
    // int32 significant = *(int32 *) &val & (( 1 << 23 ) - 1);
    // int32 exponent = ( *(int32 *) &val & ( (1 << 31) - 1 ) ) >> 23;
    // int32 sign = *(int32 *) &val >> 30;
    //
    // exponent -= 127;
    sprintf_s(buffer, 64, "%.3f", val);
    
    return buffer;
}

char* sh_vec2tstr(vec2 *vec) {
    static char buffer[64] = {};
    sprintf_s(buffer, 64, "vec2(%.3f, %.3f)", vec->x, vec->y);
    return buffer;
}

void write_to_log(game_state *gs, char *str) {
    fputs(str, gs->log_file);
}

void write_to_gl_log(char *str) {
    fputs(str, gl_log_file);
}

sh_rect_container make_text_rect(char *text, vec2 pos, int font_size) {
        size_t length = strlen(text);

        float width = 0;
        float height = font_size;

        float scale = sh_get_scale_for_pixel(&gl_game_state->font, font_size);

        for(int32 i = 0; i < length; ++i) {
                fnt_char *ch = gl_game_state->font.characters + text[i];
                width += ch->xadvance*scale;
        }

        pos.x += width/2.0;
        pos.y += height/2.0;

        sh_rect_container rect = {pos, width, height};
        push_draw_element(&gl_game_state->renderstack, sh_gen_draw_rect(pos, width, height, vec4(1, 0, 0, 1)));
        return rect;
}


sh_rect_container get_text_rect(sh_fnt *fnt, int font_size, char *text, vec2 pos) {
        size_t length = strlen(text);
        float width = 0;
        float height = font_size;
        float scale = sh_get_scale_for_pixel(&gl_game_state->font, font_size);

        for(int32 i = 0; i < length; ++i) {
                fnt_char *ch = fnt->characters + text[i];
                width += ch->xadvance*scale;
        }

        pos.x += width/2.0;
        pos.y += height/2.0;

        sh_rect_container rect = {pos, width, height};
        return rect;
}

int32 has_mouse_hovered_text(sh_fnt *fnt, input_state *input, char *text, int font_size, vec2 pos) {
        vec2 mouse_point(input->mouse.mouse_x, input->mouse.mouse_y);
        sh_rect_container rect = get_text_rect(fnt, font_size, text, pos);
        return point_in_rect(&mouse_point, &rect);
}

int32 has_mouse_clicked_text(sh_fnt *fnt, input_state *input, char *text, int font_size, vec2 pos) {
        vec2 mouse_point(input->mouse.mouse_x, input->mouse.mouse_y);
        sh_rect_container rect = get_text_rect(fnt, font_size, text, pos);
        return point_in_rect(&mouse_point, &rect) && input->mouse.left_button;
}

#ifdef GAME_DEBUG_SCREEN_H

int32 is_left_mouse_button_down() {
        input_state *input = gl_game_state->debug_state->inputs;
        return input->mouse.left_button;
}

void dump_struct_to_screen(void *data, struct_meta_info *meta, int meta_count, int font_size, vec2 *pos) {
        input_state *input = gl_game_state->debug_state->inputs;
        if(!is_left_mouse_button_down() && interacting_with_variable) {
                interacting_with_variable = false;
                global_active_variable = nullptr;
        }

        for(int i = 0; i < meta_count; ++i) {
                vec4 color(1, 1, 1, 1);
                char buffer[256] = {};
                struct_meta_info member = meta[i];
                char *m = static_cast<char *>(data) + member.offset;
                switch(member.type) {
                        case type_uint32:
                                sprintf_s(buffer, "%s : %d", member.name, *reinterpret_cast<uint32 *>(m));
                                break;

                        case type_int:
                                sprintf_s(buffer, "%s : %d", member.name, *reinterpret_cast<int *>(m));
                                break;

                        case type_float:
                                sprintf_s(buffer, "%s : %f", member.name, *reinterpret_cast<float *>(m));
                                break;

                        case type_vec2: {
                                vec2 *vec = reinterpret_cast<vec2 *>(m);
                                sprintf_s(buffer, "%s : (x: %f, y: %f)", member.name, vec->x, vec->y);
                        } break; 

                        case type_sh_circle: {
                                sprintf_s(buffer, "%s: ", member.name);
                                push_draw_text(gl_game_state, buffer, font_size, *pos, color);
                                buffer[0] = 0;
                                pos->y -= font_size;

                                pos->x += font_size*2;
                                
                                sh_circle *circ = *reinterpret_cast<sh_circle **>(m);

                                if(member.flags & meta_pointer) {
                                        dump_struct_to_screen(static_cast<void *>(circ), class_sh_circle , array_count(class_sh_circle), font_size, pos);
                                }
                                pos->x -= font_size*2;
                        } break;
                }

                if(member.flags & meta_pointer) {
                        // sprintf_s(buffer, "%s is a pointer %d", member.name, member.type);
                }

                int focused = glfwGetWindowAttrib(gl_game_state->window, GLFW_FOCUSED);

                if(buffer[0] != 0) {
                        if((focused) && has_mouse_hovered_text(&gl_game_state->font, input, buffer, font_size, *pos) && !interacting_with_variable) {
                                global_hot_variable = static_cast<void *>(m);
                                if(is_left_mouse_button_down() && !interacting_with_variable) {
                                        global_active_variable = static_cast<void *>(m);
                                        global_active_variable_type = member.type;
                                        interacting_with_variable = true;
                                }
                        } 

                        if(global_hot_variable == static_cast<void *>(m)) color = vec4(1, 0, 0, 1);
                        if(global_active_variable == (void *)m) color = vec4(1, 1, 0, 1);

                        push_draw_text(gl_game_state, buffer, font_size, *pos, color);
                        global_hot_variable = nullptr;
                        pos->y -= font_size;
                }
        }
}



#endif

void dump_object_log(void* data, struct_meta_info* meta, int meta_count, char* var_name) {

	char buffer[256] = {};
	char* buffer_pointer = buffer;
	int size_buffer = 256;
	int written = sprintf_s(buffer_pointer, size_buffer, "%s:\n", var_name);
	buffer_pointer += written;
	size_buffer -= written;

	for(int i = 0; i < meta_count; ++i) {
		struct_meta_info member = meta[i];
		char *m = static_cast<char *>(data) + member.offset;
		switch(member.type) {
			case type_uint32:
				written = sprintf_s(buffer_pointer, size_buffer, "%s : %d\n", member.name, *reinterpret_cast<uint32 *>(m));
				break;
			case type_int:
				written = sprintf_s(buffer_pointer, size_buffer, "%s : %d\n", member.name, *reinterpret_cast<int *>(m));
				break;
			case type_float:
				written = sprintf_s(buffer_pointer, size_buffer, "%s : %f\n", member.name, *reinterpret_cast<float *>(m));
				break;
			case type_vec2: {
				vec2 *vec = reinterpret_cast<vec2 *>(m);
				written = sprintf_s(buffer_pointer, size_buffer, "%s : (x: %f, y: %f)\n", member.name, vec->x, vec->y);
			} break; 

			case type_vec4: {
				vec4 *vec = reinterpret_cast<vec4 *>(m);
				written = sprintf_s(buffer_pointer, size_buffer, "%s : (x: %f, y: %f, z: %f, w:%f)\n", member.name, vec->x, vec->y, vec->z, vec->w);
			} break; 

			case type_sh_circle: {
				// written = sprintf_s(buffer_pointer, size_buffer, "%s: ", member.name);
				write_to_gl_log(buffer);
				sh_circle *circ = *reinterpret_cast<sh_circle **>(m);

				if(member.flags & meta_pointer) {
					DUMP_SH_CIRCLE(circ);

				}


				buffer_pointer = buffer;
				buffer_pointer[0] = 0;
				written = 0;
				size_buffer = 256;
			} break;

			case type_sh_line: {
				// written = sprintf_s(buffer_pointer, size_buffer, "%s: ", member.name);
				write_to_gl_log(buffer);
				sh_line *line = *reinterpret_cast<sh_line **>(m);
				if(member.flags & meta_pointer) {
					DUMP_SH_LINE(line);
				}

				buffer_pointer = buffer;
				buffer_pointer[0] = 0;
				written = 0;
				size_buffer = 256;

			} break;

		}

		buffer_pointer += written;
		size_buffer -= written;
		written = 0;
	}


	write_to_gl_log(buffer);
}

void dump_log_stamp(char* text, int line_number, char* file_name) {
	char buffer[128];
	sprintf_s(buffer, 128, "#%s:%d %s", file_name, line_number, text);
	write_to_gl_log(buffer);
}

