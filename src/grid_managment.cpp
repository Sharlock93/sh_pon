
void init_grid(game_grid *grid, float grid_width, float grid_height, int row, int col, vec2 pos) {

    grid->grid_width = grid_width;
    grid->grid_height = grid_height;

    grid->center_x = pos.x;
    grid->center_y = pos.y;
    grid->width_elem_count = col;
    grid->height_elem_count = row;
    grid->elem_count = row*col;

    grid->elements = (grid_element *)malloc(sizeof(grid_element)*row*col);

    grid->data[0] = vec2(pos.x - grid_width/2.0, pos.y + grid_height/2.0);
    grid->data[1] = vec2(pos.x + grid_width/2.0, pos.y + grid_height/2.0);
    grid->data[2] = vec2(pos.x + grid_width/2.0, pos.y - grid_height/2.0);
    grid->data[3] = vec2(pos.x - grid_width/2.0, pos.y - grid_height/2.0);

    glGenBuffers(1, &grid->_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, grid->_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec2)*4, &grid->data, GL_STATIC_DRAW); 
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void init_grid_elem(game_grid *grid, int index, vec4 color) {
    int col = index%(grid->width_elem_count);
    int row = index/grid->height_elem_count;

    float height_elem = grid->grid_height/float(grid->height_elem_count);
    float width_elem = grid->grid_width/float(grid->width_elem_count);

    //Note(sharo): this assumes the grid is like a mathematical 
    //coordinate system and not a screen one.

    float top_left_x = -grid->grid_width/2.0;
    float top_left_y =  grid->grid_height/2.0;

    grid->elements[index] = {};

    //Note(sharo): assign memory to the buckets.
    grid->elements[index].obj_buckets = (object_bucket_list *) calloc(1, sizeof(object_bucket_list));


    grid->elements[index].index_grid   = index;
    grid->elements[index].index_row    = row;
    grid->elements[index].index_column = col;
    grid->elements[index].width   = width_elem;
    grid->elements[index].height  = height_elem;
    grid->elements[index].data[0] = vec2(top_left_x+width_elem*col, top_left_y-height_elem*row);
    grid->elements[index].data[1] = vec2(top_left_x+width_elem*(col+1), top_left_y-height_elem*row);
    grid->elements[index].data[2] = vec2(top_left_x+width_elem*(col+1), top_left_y-height_elem*(row+1));
    grid->elements[index].data[3] = vec2(top_left_x+width_elem*col, top_left_y-height_elem*(row+1));

    grid->elements[index].left   = grid->elements[index].data[0].x;
    grid->elements[index].top    = grid->elements[index].data[0].y;
    grid->elements[index].right  = grid->elements[index].data[2].x;
    grid->elements[index].bottom = grid->elements[index].data[2].y;

    grid->elements[index].color = color;

    glGenBuffers(1, &grid->elements[index]._vbo);
    glBindBuffer(GL_ARRAY_BUFFER, grid->elements[index]._vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec2)*4, &grid->elements[index].data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void add_to_obj_bucket_list(object_bucket_list *obj_list, game_object *gobj) {
    Assert(gobj->id >= 0);
    Assert(gobj->id < 1000);
    object_bucket *obj_bucket = (object_bucket *) malloc(sizeof(object_bucket));

    obj_bucket->next = NULL;
    obj_bucket->prev = NULL;
    obj_bucket->obj = gobj;
    obj_bucket->to_delete = 0;
    obj_bucket->object_count = 0;

    if(obj_list->root_node == NULL) {
        obj_list->root_node = obj_list->last_node = obj_bucket;
    } else { 
        obj_bucket->prev = obj_list->last_node;
        obj_list->last_node->next = obj_bucket;
        obj_list->last_node = obj_bucket;
    }

    obj_list->object_count++; 
}

void remove_from_grid_elem(object_bucket_list *list, object_bucket **bucket_ptr) {


    object_bucket *bucket = *bucket_ptr;


    if(list->root_node == bucket) {

        if(list->root_node->next != NULL) {
            list->root_node = list->root_node->next;
            list->root_node->prev = nullptr;
        } else {
            list->root_node = nullptr;
            list->last_node = nullptr;
        }
    } else {
        if(list->last_node == bucket) {
            list->last_node = list->last_node->prev;
            list->last_node->next = nullptr;
            bucket->prev->next = nullptr;
        } else {
            bucket->prev->next = bucket->next;
            bucket->next->prev = bucket->prev;
        }
    }

    bucket->next = nullptr;
    bucket->prev = nullptr;

    list->object_count--;
    free(*bucket_ptr);
    *bucket_ptr = nullptr;
}

void add_to_objects(objects *objects, game_object *obj) {
    obj->next_object = NULL;
    obj->previous_object = NULL;

    if(objects->root_node == NULL) {
        objects->root_node = objects->last_node = obj;
    } else { 
        obj->previous_object = objects->last_node;
        objects->last_node->next_object = obj;
        objects->last_node = obj;
    }

    objects->object_count++; 
}

void add_to_grid_objects(game_state *gs, game_object *gobj, int static_obj) {
    game_grid *grid = &gs->overlord;
    switch (gobj->type) {
        case BALL: {
                       ball_object *ball = (ball_object *)gobj->obj;
                       int index = find_grid_index(grid, ball->circ->_position);
                       add_to_obj_bucket_list(grid->elements[index].obj_buckets, gobj);
                   } break;

        case RECTANGLE: {
                            rect_object *rect = (rect_object *) gobj->obj;
                            int index = find_grid_index(grid, rect->rect->_position);
                            add_to_obj_bucket_list(grid->elements[index].obj_buckets, gobj);
                        } break;
        case LINE: {
                       line_object *line = (line_object *) gobj->obj;
                       vec2 center = (line->line->point[1] + line->line->point[0])/2.0;
                       int index = find_grid_index(grid, center);
                       add_to_obj_bucket_list(grid->elements[index].obj_buckets, gobj);
                   } break;
    }


    if(static_obj) {
        add_to_objects(gs->static_objects, gobj);
    } else {
        add_to_objects(gs->objects_to_update, gobj);
    }
}

int find_grid_index(game_grid *grid, vec2 position) {
    float elem_width = grid->elements[0].width;
    float elem_height = grid->elements[0].height;
    float first_elem_left = grid->elements[0].left;
    float first_elem_top = grid->elements[0].top;
    int col = abs(int((first_elem_left - position.x )/elem_width ));
    int row = abs(int((first_elem_top - position.y)/ elem_height));

    if(col > grid->height_elem_count - 1) col = grid->height_elem_count - 1;
    if(col < 0) col = 0;

    if(row > grid->width_elem_count - 1) row = grid->width_elem_count - 1;
    if(row < 0) row = 0;

    return row*grid->width_elem_count + col;
}

void render_grid(game_state *gs) {
    uint32 vpos = gs->vpos_attrib_loc;
    uint32 model = gs->model_t_attrib_loc;
    uint32 color_att = gs->color_attrib_loc;
     
    game_grid *grid = &gs->overlord;

    glBindBuffer(GL_ARRAY_BUFFER, grid->_vbo);  
    //
    glEnableVertexAttribArray(vpos);
    glVertexAttribPointer(vpos, 2, GL_FLOAT, GL_FALSE, 0, 0);
    vec4 color(0, 1, 1, 1);
    glUniform4fv(color_att, 1, &color[0]);
    glDrawArrays(GL_LINE_LOOP, 0, 4);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    for(int i = 0; i < grid->elem_count; ++i) {
        render_grid_elem(&grid->elements[i], vpos, model, color_att);
    }
}

void render_grid_elem(grid_element *grid_elem, int pos_attrib, int transform_att, int color_att) { 
    glBindBuffer(GL_ARRAY_BUFFER, grid_elem->_vbo);  

    glEnableVertexAttribArray(pos_attrib);
    glVertexAttribPointer(pos_attrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
    mat4 mat(1);
    glUniformMatrix4fv(transform_att, 1, GL_TRUE, &mat[0][0]);
    glUniform4fv(color_att, 1, &grid_elem->color[0]);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void render_game_object(game_state *gs, game_object *gm_obj) {
    int vpos = gs->vpos_attrib_loc;
    int color = gs->color_attrib_loc;
    int model = gs->model_t_attrib_loc;
    int tex_coord = gs->tex_coord;
    int has_tex = gs->has_texture_attrib;

    switch(gm_obj->type) {
        case BALL: {
            ball_object *ball = (ball_object *) gm_obj->obj; 
            ball->circ->render(vpos, color, model);
        } break;
        case RECTANGLE: {
            rect_object *rect = (rect_object *) gm_obj->obj;
            rect->rect->render(vpos, color, model);
        } break;
        case LINE: {
            line_object *line = (line_object *) gm_obj->obj;
            line->line->render(vpos, color, model);
        } break;
    }
}


void render_draw_stack(game_state *gs) {
    int vpos = gs->vpos_attrib_loc;
    int color = gs->color_attrib_loc;
    int model = gs->model_t_attrib_loc;
    int tex_coord = gs->tex_coord;
    int has_tex = gs->has_texture_attrib;
    int fnt_tex = gs->fnt_tex;
    draw_stack *stack = &gs->renderstack;

    uint32 vbo;
    glGenBuffers(1, &vbo);
    draw_element draw = {};

    while(pop_element(stack, &draw)) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vec2)*(draw.point_count*(1 + draw.has_texture)), draw.points, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(vpos);
        glVertexAttribPointer(vpos, 2, GL_FLOAT, GL_FALSE, 0, 0);

        if(draw.has_texture) {
            glEnableVertexAttribArray(tex_coord);
            glBindTexture(GL_TEXTURE_2D, fnt_tex);
            glVertexAttribPointer(tex_coord, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(sizeof(vec2)*draw.point_count));
        }

        glUniform1i(has_tex, draw.has_texture);
        glUniform4fv(color, 1, &draw.color[0]);
        glDrawArrays(draw.command, 0, draw.point_count);
        glDisableVertexAttribArray(tex_coord);
        free(draw.points);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &vbo);

    glUniform1i(has_tex, 0);
}

void render_draw_stack(game_state *gs, draw_stack *stack) {
    int vpos = gs->vpos_attrib_loc;
    int color = gs->color_attrib_loc;
    int model = gs->model_t_attrib_loc;
    int tex_coord = gs->tex_coord;
    int has_tex = gs->has_texture_attrib;
    int fnt_tex = gs->fnt_tex;
    // draw_stack *stack = &gs->renderstack;

    uint32 vbo;
    glGenBuffers(1, &vbo);
    draw_element draw = {};

    while(pop_element(stack, &draw)) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vec2)*(draw.point_count*(1 + draw.has_texture)), draw.points, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(vpos, 2, GL_FLOAT, GL_FALSE, 0, 0);

        if(draw.has_texture) {
            glEnableVertexAttribArray(tex_coord);
            glBindTexture(GL_TEXTURE_2D, fnt_tex);
            glVertexAttribPointer(tex_coord, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(sizeof(vec2)*draw.point_count));
        }

        glUniform1i(has_tex, draw.has_texture);
        glUniform4fv(color, 1, &draw.color[0]);
        glDrawArrays(draw.command, 0, draw.point_count);
        glDisableVertexAttribArray(tex_coord);
        free(draw.points);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &vbo);

    glUniform1i(has_tex, 0);
}


void move_bucket_elem_to_grid(game_state *gs,
        object_bucket *mvd_obj_bucket,
        int from_index,
        int to_index)
{

    object_bucket_list *from = gs->overlord.elements[from_index].obj_buckets;
    object_bucket_list *to = gs->overlord.elements[to_index].obj_buckets;

    //Note(sharo): cannot be null because we are moving an object from it
    // game_object *dyn_obj_from = from->root_node;
    // Assert(dyn_obj_from != NULL); 
    //
    if(mvd_obj_bucket->prev == NULL ) {
        //Note(sharo): this is the assumption that the object is the root of from
        if(mvd_obj_bucket->next == NULL) {
            from->root_node = NULL;
            from->last_node = NULL;
        } else {
            from->root_node = from->root_node->next;
            from->root_node->prev = NULL;
        }

    } else {
        //Note(sharo): are we the last node?
        if(mvd_obj_bucket->next == NULL) {
            from->last_node = from->last_node->prev; //Move one up
            from->last_node->next = NULL;
        } else {
            mvd_obj_bucket->prev->next = mvd_obj_bucket->next;
        }

    }

    from->object_count--;
    add_to_obj_bucket_list(gs->overlord.elements[to_index].obj_buckets, mvd_obj_bucket->obj);
    remove_from_grid_elem(gs->overlord.elements[to_index].obj_buckets, &mvd_obj_bucket);
}


object_bucket* in_grid_element(grid_element *elem, game_object *object) {
    object_bucket_list *list = elem->obj_buckets;
    for( object_bucket *bucket = list->root_node;
            bucket;
            bucket = bucket->next)
    {
        if(bucket->obj->id == object->id) return bucket;
    }

    return nullptr;
}

int is_bucket_in_list(object_bucket_list *list, object_bucket *bucket) {
    object_bucket *root_node = list->root_node;

    while(root_node) {
        if(root_node == bucket) return true;
        root_node = root_node->next;
    }

    return false;
}


game_object* make_sh_line(int id, vec2 a, vec2 b, vec4 color) {
    game_object *obj  = (game_object *) malloc(sizeof(game_object)); 
    line_object *line = (line_object *) malloc(sizeof(line_object));
    line->line = new sh_line(a, b, color);
    
    vec2 line_vec = b - a;
    vec2 line_vec_norm = normalize(line_vec);
    line->normal = vec2(line_vec_norm.y, -line_vec_norm.x);

    obj->id = id;
    obj->type = LINE;
    obj->next_object = nullptr;
    obj->previous_object = nullptr;
    obj->size = sizeof(line_object);
    obj->obj = line;

    return (obj);
}

game_object* make_sh_line(objects *dyn, vec2 a, vec2 b, vec4 color) {
    game_object *obj  = (game_object *) malloc(sizeof(game_object)); 
    line_object *line = (line_object *) malloc(sizeof(line_object));
    line->line = new sh_line(a, b, color);

    obj->type = LINE;
    obj->next_object = nullptr;
    obj->previous_object = nullptr;
    obj->size = sizeof(line_object);
    obj->obj = line;

    return (obj);
}



game_object* make_sh_rect(int id, float x, float y, float velocity, float width, float height, vec4 color) {
    game_object *obj = (game_object *) malloc(sizeof(game_object)); 
    rect_object *rect = (rect_object *) malloc(sizeof(rect_object));
    rect->velocity = velocity;

    rect->rect = new sh_rect(x, y, width, height, color);

    obj->id = id;
    obj->moved = 0;
    obj->type = RECTANGLE;
    obj->next_object = nullptr;
    obj->previous_object = nullptr;
    obj->in_grids = nullptr;
    obj->size = sizeof(rect_object);
    obj->obj = rect;

    return (obj);
}

game_object* make_sh_circle(int id, float x, float y, float radius,
        float velocity, vec2 direction, vec4 color)
{
    game_object *obj = (game_object *) malloc(sizeof(game_object)); 
    ball_object *ball_obj = (ball_object *) malloc(sizeof(ball_object));

    ball_obj->circ = new sh_circle(x, y, radius, direction, color, vec2(x, y));
    ball_obj->velocity = velocity;

    ball_obj->forces = nullptr;//(vec2 *) malloc(sizeof(vec2)); 
    ball_obj->vec_capacity = 1;
    ball_obj->vec_nums = 1;

    ball_obj->previous_pos = vec2(x, y) - velocity*direction*dt; //@Todo(sharo): pass in dt?

    obj->id = id;
    obj->type = BALL;
    obj->next_object = nullptr;
    obj->previous_object = nullptr;
    obj->moved = 0;
    obj->size = sizeof(ball_object);
    obj->obj = (void *) ball_obj;
    return (obj);
}

draw_element sh_gen_draw_rect(vec2 pos, float width, float height, vec4 color) {
    draw_element elem = {};
    elem.points = (vec2 *) malloc(sizeof(vec2) * 4);

    elem.points[0]  = vec2(pos.x - width/2.0, pos.y + height/2.0);
    elem.points[1]  = vec2(pos.x + width/2.0, pos.y + height/2.0);
    elem.points[2]  = vec2(pos.x + width/2.0, pos.y - height/2.0);
    elem.points[3]  = vec2(pos.x - width/2.0, pos.y - height/2.0);

    elem.point_count = 4;
    elem.command = GL_LINE_LOOP;
    elem.color = color;

    return (elem);
}


draw_element sh_gen_draw_rect_fill(vec2 pos, float width, float height, vec4 color) {
    draw_element elem = {};
    elem.points = (vec2 *) malloc(sizeof(vec2) * 6);

    elem.points[0]  = vec2(pos.x - width/2.0, pos.y - height/2.0);
    elem.points[1]  = vec2(pos.x - width/2.0, pos.y + height/2.0);
    elem.points[2]  = vec2(pos.x + width/2.0, pos.y + height/2.0);

    elem.points[3]  = vec2(pos.x - width/2.0, pos.y - height/2.0);
    elem.points[4]  = vec2(pos.x + width/2.0, pos.y + height/2.0);
    elem.points[5]  = vec2(pos.x + width/2.0, pos.y - height/2.0);

    elem.point_count = 6;
    elem.command = GL_TRIANGLES;
    elem.color = color;

    return (elem);
}


draw_element sh_gen_draw_circ(vec2 pos, float r, vec4 color) {

    int segments = 32;
    draw_element elem = {};

    elem.points = (vec2 *) malloc(sizeof(vec2)*segments);

    float angle = (6.28)/segments;

    for(int i = 0; i < segments; ++i) {
        elem.points[i] = vec2(cos(angle*i)*r + pos.x, sin(angle*i)*r + pos.y);
    }

    elem.point_count = segments;
    elem.command = GL_LINE_LOOP;
    elem.color = color;

    return elem;
}


draw_element sh_gen_draw_triangle(vec2 pos, float height, float width, int triangle_poining, vec4 color) {
        draw_element elem = {};
        elem.points = (vec2 *) malloc(sizeof(vec2)*3);
        elem.point_count = 3;
        elem.command = GL_TRIANGLES;
        elem.color = color;

        vec2 base1(0, 0);
        vec2 base2(0, 0);
        vec2 tip(0, 0);

        switch(triangle_poining) {
                case 0:
                        base1 = vec2(pos.x - width/2.0, pos.y - height/2.0);
                        base2 = vec2(pos.x + width/2.0, pos.y - height/2.0);
                        tip = vec2(pos.x, pos.y + height/2.0);
                        break;
                case 1:
                        base1 = vec2(pos.x - width/2.0, pos.y + height/2.0);
                        base2 = vec2(pos.x - width/2.0, pos.y - height/2.0);
                        tip = vec2(pos.x + width/2.0, pos.y);
                        break;
                case 2:
                        base1 = vec2(pos.x + width/2.0, pos.y + height/2.0);
                        base2 = vec2(pos.x - width/2.0, pos.y + height/2.0);
                        tip = vec2(pos.x, pos.y - height/2.0);
                        break;
                case 3:
                        base1 = vec2(pos.x + width/2.0, pos.y - height/2.0);
                        base2 = vec2(pos.x + width/2.0, pos.y + height/2.0);
                        tip = vec2(pos.x - width/2.0, pos.y);
                        break;
        };

        elem.points[0] = base1;
        elem.points[1] = tip; 
        elem.points[2] = base2;

        return elem;
}

draw_element sh_gen_draw_line(vec2 a, vec2 b, vec4 color) {
    draw_element elem = {};
    elem.points = (vec2 *) malloc(sizeof(vec2)*2);
    elem.points[0] = a;
    elem.points[1] = b;
    elem.point_count = 2;
    elem.command = GL_LINES;
    elem.color = color;
    return elem;
}

void push_draw_element(draw_stack *stack, draw_element elem) {

    if(elem.point_count == 0) return;

    if(stack->elem_count >= stack->capacity) {
        make_stack_capacity(stack, floor(stack->capacity*1.5));
    }

    stack->elements[stack->elem_count++] = elem;
}

draw_element draw_single_char(sh_fnt *fnt, char c,  int font_size, vec2 pos) {
    draw_element elem = {};
    elem.point_count = 4;
    elem.points = (vec2 *) malloc(sizeof(vec2)*8);
    elem.has_texture = 1;
    elem.command = GL_TRIANGLE_FAN;
    elem.color = vec4(1, 1, 1, 1);
    fnt_char *ch = fnt->characters + c;
    float w = ch->width;
    float h = ch->height;

    float fh = fnt->common.scaleH;
    float fw = fnt->common.scaleW;


    elem.points[0] = vec2(pos.x, pos.y);
    elem.points[1] = vec2(pos.x, pos.y + h );
    elem.points[2] = vec2(pos.x + w, pos.y + h );
    elem.points[3] = vec2(pos.x + w, pos.y);

    elem.points[4] = vec2((ch->x )/fw, (ch->y + h)/fh);
    elem.points[5] = vec2(ch->x/fw, ch->y/fh);
    elem.points[6] = vec2((ch->x + w)/fw, (ch->y)/fh);
    elem.points[7] = vec2((ch->x + w)/fw, (ch->y + h)/fh);
    return elem;
}

void draw_char_into_buffer(vec2 *quad_buff, vec2 *tex_buff, sh_fnt *fnt, char c, int font_size, vec2 pos) {
    fnt_char *ch = fnt->characters + c;
    float scale = sh_get_scale_for_pixel(fnt, font_size);
    float h = ch->height*scale;
    float w = ch->width*scale;;
    float fh = fnt->common.scaleH;
    float fw = fnt->common.scaleW;
    pos.x += ch->xoffset*scale;
    quad_buff[0] = vec2(pos.x, pos.y);
    quad_buff[1] = vec2(pos.x, pos.y + h);
    quad_buff[2] = vec2(pos.x + w, pos.y + h);

    quad_buff[3] = vec2(pos.x, pos.y);
    quad_buff[4] = vec2(pos.x + w, pos.y + h);
    quad_buff[5] = vec2(pos.x + w, pos.y);

    tex_buff[0] = vec2((ch->x )/fw, (ch->y + ch->height)/fh);
    tex_buff[1] = vec2(ch->x/fw, ch->y/fh);
    tex_buff[2] = vec2((ch->x + ch->width)/fw, (ch->y)/fh);

    tex_buff[3] = vec2((ch->x )/fw, (ch->y + ch->height)/fh);
    tex_buff[4] = vec2((ch->x + ch->width)/fw, (ch->y)/fh);
    tex_buff[5] = vec2((ch->x + ch->width)/fw, (ch->y + ch->height)/fh);
}

draw_element sh_gen_draw_text(sh_fnt *fnt, char *text, int font_size, vec2 position, vec4 color) {
    TIME_BLOCK;
    int32 length = sh_strlen((uint8 *)text) - 1;
    draw_element elem = {};
    vec2 *points = (vec2 *)malloc(sizeof(vec2)*12*(length));
    elem.points = points;
    elem.has_texture = 1;
    elem.color = color;

    elem.point_count = 6*(length);
    elem.command = GL_TRIANGLES;
    vec2 cursor_pos = position;
    float scale = sh_get_scale_for_pixel(fnt, font_size);
    for(int32 i = 0; i < length; ++i) {
        fnt_char *ch = fnt->characters + text[i];
        draw_char_into_buffer(points + i*6, points + length*6 + i*6, fnt, text[i], font_size, cursor_pos);
        cursor_pos.x += ch->xadvance*scale;
    }

    return elem;
}


void push_draw_text(game_state *gs, char *text, int font_size, vec2 position, vec4 color) {
    push_draw_element(&gs->renderstack, sh_gen_draw_text(&gs->font, text, font_size, position, color));
}

void make_stack_capacity(draw_stack *stack, int new_capacity) {

    if(stack->capacity == 0) {
        stack->elements = (draw_element *) malloc(sizeof(draw_element)*new_capacity);

        if(stack->elements) {
            stack->capacity = 10;
            stack->elem_count = 0;
        } else {
            std::cout << "shit happened" << std::endl;
        }
    } else {
        if(stack->capacity == stack->elem_count) {
            draw_element *elements = (draw_element *)realloc(stack->elements, sizeof(draw_element)*new_capacity);

            if(elements) {
                stack->elements = elements;
                stack->capacity = new_capacity;
            }
        }
    }
}

int pop_element(draw_stack *stack, draw_element *elem) {
    if(stack->elem_count == 0) {
        return 0;
    }

    *elem = stack->elements[--stack->elem_count];
    return 1;
}

void sh_col_q_init(sh_col_queue* q, int init_capacity) {
    q->q = (sh_col_info*) malloc(sizeof(sh_col_info)*init_capacity);
    q->capacity = init_capacity;
    q->size = 0;
}

void sh_col_q_push(sh_col_queue* q, sh_col_info col_pair) {
    if(q->size >= q->capacity) {
	//need to grow
    }


    //need to check tail and head places
    // if(( q->head - q->tail) < 0 ) {
    //
    // }

    q->q[q->head] = col_pair;
    q->head = ++q->head % q->capacity;
}

sh_col_info sh_col_q_pop(sh_col_queue* q) {
    int cur_index = q->tail;
    
    q->tail = ++q->tail%q->capacity;

    return q->q[cur_index];
}


