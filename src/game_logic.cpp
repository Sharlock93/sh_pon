#include "game_logic.h"
#include <iostream>
#define Assert(condtion) if(!(condtion)) {*(int *)0 = 0;}

#define INDEX(row, col, stride) ((row*stride) + col)

void project_on_axis(sh_rect *rect, vec2 axis) {
    
}

void add_force_vector(ball_object *ball, vec2 vector) {

}

float clamp(float value, float min, float max) {
    if(value < min)
        return min;

    if(value > max)
        return max;

    return value;
}

void render_grid_elem(grid_element *grid_elem, int pos_attrib, int color_att) { 
    glBindBuffer(GL_ARRAY_BUFFER, grid_elem->_vbo);  

    glEnableVertexAttribArray(pos_attrib);
    glVertexAttribPointer(pos_attrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glUniform4fv(color_att, 1, &grid_elem->color[0]);
    glDrawArrays(GL_LINE_LOOP, 0, 4);


    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void move_dynamic_elem_to_grid(game_state *gs, game_object *moved_obj, int from_index,
                                                                       int to_index)
{
    dynamic_objects *from = gs->overlord.elements[from_index].dyn_objs;
    dynamic_objects *to = gs->overlord.elements[to_index].dyn_objs;
    game_object *dyn_obj_from = from->root_node;

    //Note(sharo): cannot be null because we are moving an object from it
    Assert(dyn_obj_from != NULL); 
    if(dyn_obj_from == moved_obj) {
        //Note(sharo): is the root node alone, then remove everything. 
        if(dyn_obj_from->next_object == NULL) {
            from->root_node = NULL;
            from->last_node = NULL;
            from->object_counts--;

            add_to_dynamic_obj(to, moved_obj);
        } else {
            from->root_node = dyn_obj_from->next_object;
            from->root_node->previous_object = NULL;
            from->object_counts--;
            std::cout << "Moew" << std::endl;
            //Note(sharo): root node is not alone, thus last node can only point
            //to the last node and not the root node.
            add_to_dynamic_obj(to, moved_obj);
        }
    } else {
        moved_obj->previous_object->next_object = moved_obj->next_object;
        
        if(moved_obj->next_object != nullptr)
            moved_obj->next_object->previous_object = moved_obj->previous_object;

        from->object_counts--;
        add_to_dynamic_obj(to, moved_obj);
    }
}

void add_to_dynamic_obj(dynamic_objects *dyn_obj, game_object *obj) {
    
    obj->next_object = NULL;
    obj->previous_object = NULL;
    if(dyn_obj->root_node == NULL) {
        dyn_obj->root_node = dyn_obj->last_node = obj;
    } else { 
        dyn_obj->last_node->next_object = obj;
        obj->previous_object = dyn_obj->last_node;
        dyn_obj->last_node = obj;
    }

    dyn_obj->object_counts++; 
}

int colision(sh_rect *rect, double x, double y) {
    if((rect->_left <= x) &&
       (rect->_right >= x) &&
       (rect->_top >= y) &&
       (rect->_bottom <= y) ) {
        return 1;
    } 

    return 0;
}

int colision(grid_element *elem, double x, double y) {
    if((elem->left <= x) &&
       (elem->right >= x) &&
       (elem->top >= y) &&
       (elem->bottom <= y) ) {
        return 1;
    } 

    return 0;
}

int colision(grid_element *elem, vec2 *p) {
    if((elem->left <= p->x) &&
       (elem->right >= p->x) &&
       (elem->top >= p->y) &&
       (elem->bottom <= p->y) ) {
        return 1;
    } 

    return 0;
}

int col_cric_rect(rect_object *rect_obj, ball_object *ball_obj, vec2 *result) {
    sh_rect   *rect   = rect_obj->rect;
    sh_circle *circ   = ball_obj->circ;
    
    vec2 line = circ->_position - rect->_position;
    vec2 final = vec2(0, 0);

    final.x = clamp(line.x, -rect->_width/2.0,  rect->_width/2.0);
    final.y = clamp(line.y, -rect->_height/2.0, rect->_height/2.0);
    result->x = final.x;
    result->y = final.y;
    final =  final + rect->_position;
    final = final - circ->_position;

    return (final.x*final.x + final.y*final.y) < (circ->_r*circ->_r);
}

vec2 closes_point_in_rect(sh_circle *circ, sh_rect *rect) {
    vec2 pos = circ->_position;
    float x_left  = pos.x - rect->_left;
    float x_right = rect->_right - pos.x; 
    float y_top    = rect->_top - pos.y;
    float y_bottom = pos.y - rect->_bottom;


    float x_min = (x_left < x_right) ? x_left : x_right;
    float y_min = (y_top < y_bottom) ? y_top : y_bottom;

    if(x_min < y_min) {
        if(x_min == x_left)
            return vec2(rect->_left - circ->_r, pos.y);
        else
            return vec2(rect->_right + circ->_r, pos.y);
    } else {
        if(y_min == y_bottom)
            return vec2(pos.x, rect->_bottom - circ->_r);
        else
            return vec2(pos.x, rect->_top + circ->_r);
    }
}

int find_grid_index(game_grid *grid, vec2 position) {
    float elem_width = grid->elements[0].width;
    float elem_height = grid->elements[0].height;
    float first_elem_left = grid->elements[0].left;
    float first_elem_top = grid->elements[0].top;
    int col = abs(int((first_elem_left - position.x )/elem_width ));
    int row = abs(int((first_elem_top - position.y)/ elem_height));

    return row*grid->width_elem_count + col;
}

extern "C" __declspec(dllexport) GAME_UPDATE_FUNC(update) {

    //Note(sharo): objects that can be addressed and are not fully dynamic
    /* game_object **objs = gamestate->named_objects; */
    /* for(int i = 0; i < gamestate->object_count; ++i) { */
    /*     switch(objs[i]->type) { */
    /*         case BALL: { */
    /*             ball_object *ball = (ball_object *) objs[i]->obj; */ 
    /*             line_object *line = (line_object *)objs[2]->obj; */
    /*             line_object *line2 = (line_object *)objs[3]->obj; */
    /*             rect_object *rect = (rect_object *) objs[0]->obj; */ 
    /*             vec2 pen_vec; */

    /*             ball->previous_pos = ball->circ->get_position(); */
    /*             if(ball->vectors) { */
    /*                 ball->circ->move_position(( ball->circ->_direction*dt*ball->velocity + *ball->vectors*dt )); */
    /*                 *ball->vectors = *ball->vectors - *ball->vectors*dt/2.0; */
    /*             } else { */
    /*                 ball->circ->move_position(( ball->velocity*dt )); */
    /*             } */

    /*             vec2 clos = closes_point_in_rect(ball->circ, rect->rect); */

    /*             int collid = col_cric_rect((rect_object *) objs[0]->obj,ball, &pen_vec); */
                
    /*             line->line->update(ball->circ->_position, clos );//+ ((rect_object *) objs[0]->obj)->rect->_position); */
    /*             line->line->_color = vec4(1, 0, 0, 1); */

    /*             if(collid) { */    
    /*                 ball->circ->move_position((clos - ball->circ->_position)); */
    /*                 ball->circ->_direction = ball->circ->_direction*-1; */
    /*                 *ball->vectors = -(rect->previous_pos - rect->rect->_position)*150; */
    /*             } */
                
    /*             vec2 to_mouse(ball->circ->_position.x - inputs->mouse.mouse_x, ball->circ->_position.y - inputs->mouse.mouse_y); */ 
    /*             to_mouse.x *= to_mouse.x; */
    /*             to_mouse.y *= to_mouse.y; */
                
    /*             if((to_mouse.x + to_mouse.y) <= (ball->circ->_r*ball->circ->_r)) { */
    /*                 inputs->mouse.hot_object = objs[i]; */
    /*             } */

    /*             if(ball->circ->get_position().y < -gamestate->window_width_height.y/2.0) */
    /*                 ball->circ->_direction.y = -1*ball->circ->_direction.y; */ 

    /*             if(ball->circ->get_position().y > gamestate->window_width_height.y/2.0) */
    /*                 ball->circ->_direction.y = -1*ball->circ->_direction.y; */ 

    /*             if(ball->circ->get_position().x < -gamestate->window_width_height.x/2.0) */
    /*                 ball->circ->_direction.x = -1*ball->circ->_direction.x; */ 

    /*             if(ball->circ->get_position().x > gamestate->window_width_height.x/2.0) */
    /*                 ball->circ->_direction.x = -1*ball->circ->_direction.x; */
    /*         } break; */

    /*         case RECTANGLE: { */
    /*             rect_object *rect = (rect_object *) objs[i]->obj; */
    /*             rect->previous_pos = rect->rect->get_position(); */
                
    /*             if(inputs->keyboard.arrow_left) { */
    /*                 rect->rect->move_position(-120*dt, 0); */
    /*             } */

    /*             if(inputs->keyboard.arrow_right) { */
    /*                 rect->rect->move_position(120*dt, 0); */
    /*             } */
    /*         } break; */

    /*         case LINE: { */
    /*             line_object *line_obj = (line_object *) objs[i]->obj; */
    /*         } break; */
    /*     } */ 
    /* } */

    //Todo(sharo): using mouse to move an object
    /* if(inputs->mouse.left_button && inputs->mouse.hot_object) { */
    /*     switch(inputs->mouse.hot_object->type) { */
    /*         case BALL: { */
    /*             ball_object *ball = (ball_object *) inputs->mouse.hot_object->obj; */
    /*             ball->circ->set_position(inputs->mouse.mouse_x, inputs->mouse.mouse_y); */
    /*         } break; */
    /*         case RECTANGLE: { */

    /*         } break; */
    /*     } */
    /* } else { */
    /*     inputs->mouse.hot_object = nullptr; */
    /* } */

    for(int i = 0; i < gamestate->overlord.elem_count;++i) {
        grid_element *elem = &gamestate->overlord.elements[i];
        if(elem == nullptr) continue;
        game_object *dynamic_obj = elem->dyn_objs->root_node;
        while(dynamic_obj) {
            switch(dynamic_obj->type) {
                case BALL: {
                    ball_object *ball = (ball_object *) dynamic_obj->obj;
                    int before_index = find_grid_index(&gamestate->overlord, ball->circ->_position);
                    
                    if(!dynamic_obj->moved) {
                        ball->circ->move_position(ball->velocity*dt);
                    }

                    int after_index = find_grid_index(&gamestate->overlord, ball->circ->_position);
                    
                    if(before_index != after_index) {
                        dynamic_obj->moved = 1;
                        std::cout << gamestate->overlord.elements[before_index].dyn_objs->object_counts;
                        std::cout << gamestate->overlord.elements[after_index].dyn_objs->object_counts << std::endl;
                        move_dynamic_elem_to_grid(gamestate, dynamic_obj, before_index, after_index);
                    } else {
                        dynamic_obj->moved = 0;
                    }
                    
                 } break;
            }

            dynamic_obj = dynamic_obj->next_object;
        }
    }

    for(int i = 0; i < gamestate->overlord.elem_count; ++i) {
        grid_element *elem = &gamestate->overlord.elements[i];
        if(elem == NULL) continue;
        if(elem->dyn_objs->object_counts > 0) {
            elem->color = vec4(1, 0, 0, 1);
        } else {
            elem->color = vec4(1, 1, 1, 1);
        }
    }
}

extern "C" __declspec(dllexport) GAME_RENDER_FUNC(render) { 
    int vpos = gamestate->vpos_attrib_loc;
    int color = gamestate->color_attrib_loc;
    int model = gamestate->model_t_attrib_loc;

/*     game_object *start_node = gamestate->objects_to_update.root_node; */
/*     while(start_node) { */
/*         switch(start_node->type) { */
/*             case BALL: { */
/*                 ball_object *ball = (ball_object *) start_node->obj; */ 
/*                 ball->circ->render(vpos, color, model); */
/*             } break; */

/*             case RECTANGLE: { */
/*                 rect_object *rect = (rect_object *) start_node->obj; */                    
/*                 rect->rect->render(vpos, color, model); */
/*             } break; */
/*             case LINE: { */
/*                 line_object *line_obj = (line_object *) start_node->obj; */               
/*                 line_obj->line->render(vpos, color, model); */
/*             }; */
/*         } */ 

/*         start_node = start_node->next_object; */
/*     } */    


    for (int i = 0; i < gamestate->overlord.elem_count; ++i) {
        dynamic_objects *dynamic_obj = gamestate->overlord.elements[i].dyn_objs;
        if(!dynamic_obj) continue;
        game_object *root_node = dynamic_obj->root_node;
        while(root_node) {
            switch(root_node->type) {
                case BALL: {
                    ball_object *ball = (ball_object *) root_node->obj; 
                    ball->circ->render(vpos, color, model);
                } break;
            }
            root_node = root_node->next_object;
        }
    }

    /* game_object **objs = gamestate->named_objects; */
    /* for(int i = 0; i < gamestate->object_count; ++i) { */

    /*     switch(objs[i]->type) { */
    /*         case BALL: { */
    /*             ball_object *ball = (ball_object *) objs[i]->obj; */ 
    /*             vec2 current_pos = ball->circ->get_position(); */
    /*             ball->circ->set_position(current_pos*alpha + ball->previous_pos*(1.0-alpha)); */
    /*             ball->circ->render(vpos, color, model); */
    /*             ball->circ->set_position(current_pos); */
                
    /*         } break; */

    /*         case RECTANGLE: { */
    /*             rect_object *rect = (rect_object *) objs[i]->obj; */                    
    /*             vec2 current_pos = rect->rect->get_position(); */
    /*             rect->rect->set_position(current_pos*alpha + rect->previous_pos*(1.0-alpha)); */
    /*             rect->rect->render(vpos, color, model); */
    /*             rect->rect->set_position(current_pos); */
    /*         } break; */
    /*         case LINE: { */
    /*             line_object *line_obj = (line_object *) objs[i]->obj; */               
    /*             line_obj->line->render(vpos, color, model); */
    /*         }; */
    /*     } */ 

    /*     start_node++; */
    /* } */


    /* render_grid(&gamestate->overlord, vpos, color); */

    /* for(int i = 0; i < gamestate->overlord.width_elem_count*gamestate->overlord.height_elem_count; ++i) { */
    /*     render_grid_elem(&gamestate->overlord.elements[i], vpos, color); */ 
    /* } */
}

game_object* make_sh_rect(float x, float y, float width, float height, vec4 color) {
    game_object *obj = (game_object *) malloc(sizeof(game_object)); 
    rect_object *rect = (rect_object *) malloc(sizeof(rect_object));

    rect->rect = new sh_rect(x, y, width,height, color);

    obj->type = RECTANGLE;
    obj->next_object = nullptr;
    obj->previous_object = nullptr;
    obj->size = sizeof(rect_object);
    obj->obj = rect;

    return (obj);
}

game_object* make_sh_circle(float x, float y, float radius, float velocity,
                            vec2 direction, vec4 color)
{
    game_object *obj = (game_object *) malloc(sizeof(game_object)); 
    ball_object *ball_obj = (ball_object *) malloc(sizeof(ball_object));

    ball_obj->circ = new sh_circle(x, y, radius, direction, color, vec2(x, y));
    ball_obj->velocity = velocity;
    ball_obj->vectors = (vec2 *) malloc(sizeof(vec2)); 
    ball_obj->vectors->x = 0;
    ball_obj->vectors->y = 0;
    ball_obj->vec_capacity = 0;
    ball_obj->vec_nums = 0;

    obj->type = BALL;
    obj->next_object = nullptr;
    obj->previous_object = nullptr;
    obj->moved = 0;
    obj->size = sizeof(ball_object);
    obj->obj = (void *) ball_obj;

    return (obj);
}

game_object* make_sh_line(dynamic_objects *dyn, vec2 a, vec2 b, vec4 color) {
    game_object *obj  = (game_object *) malloc(sizeof(game_object)); 
    line_object *line = (line_object *) malloc(sizeof(line_object));
    line->line = new sh_line(a, b, color);

    obj->type = LINE;
    obj->next_object = nullptr;
    obj->previous_object = nullptr;
    obj->size = sizeof(line_object);
    obj->obj = line;

    game_object *rect_a = make_sh_rect(a.x, a.y, 5, 5, vec4(1, 0, 0, 1));
    game_object *rect_b = make_sh_rect(b.x, b.y, 5, 5, vec4(1, 0, 0, 1));

    return (obj);
}

game_object* make_sh_line(vec2 a, vec2 b, vec4 color) {
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

void free_objects_all(game_state *gs) {
    for(int i = 0; i < gs->object_count; ++i) {
        delete ( gs->named_objects[i]->obj );
    }

    free(gs->named_objects);
    gs->object_count = 0;
}

void render_grid(game_grid *grid, int pos_attrib, int color_att) {
    glBindBuffer(GL_ARRAY_BUFFER, grid->_vbo);  
    
    glEnableVertexAttribArray(pos_attrib);
    glVertexAttribPointer(pos_attrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
    vec4 color(0, 1, 1, 1);
    glUniform4fv(color_att, 1, &color[0]);
    glDrawArrays(GL_LINE_LOOP, 0, 4);

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
   grid->elements[index].dyn_objs = (dynamic_objects *) calloc(1, sizeof(dynamic_objects));


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

void init_grid(game_grid *grid, float grid_height, float grid_width, int row, int col, vec2 pos) {
    
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

void add_to_grid_dynamic_objects(game_grid *grid, game_object *object) {
    switch (object->type) {
        case BALL: {
            ball_object *ball = (ball_object *)object->obj;
            int index = find_grid_index(grid, ball->circ->_center);
            add_to_dynamic_obj(grid->elements[index].dyn_objs, object);
        } break;
    }
}

extern "C" __declspec (dllexport) GAME_INIT_FUNC(init) {

    if(gamestate->named_objects) {
        free_objects_all(gamestate);
    }

    init_grid(&gamestate->overlord, 500, 500, 3, 3 , vec2(0, 0));

    gamestate->named_objects = (game_object **) malloc(sizeof(game_object **)*10);
    gamestate->named_objects[gamestate->object_count++] = make_sh_rect(0, -220, 10, 100, vec4(1, 1, 1, 1)) ;    
    gamestate->named_objects[gamestate->object_count++] = make_sh_circle(40, 45, 10, 120, vec2(0, 1), vec4(0, 1, 1, 1));
    gamestate->named_objects[gamestate->object_count++] = make_sh_line(vec2(0, 0), vec2(10, 20), vec4(0.5, 1.0, .4, 1));
    gamestate->named_objects[gamestate->object_count++] = make_sh_line(vec2(0, 0), vec2(10, 20), vec4(0, 0.0, 1, 1));

    for(int i = 0; i < gamestate->overlord.width_elem_count*gamestate->overlord.height_elem_count; ++i) {
        init_grid_elem(&gamestate->overlord, i, vec4(1, 1, 1, 1));
    }
    /* game_object *meow = make_sh_circle(0, 0, 10, 10, vec2(0, 1), vec4(1, 1, 1, 1)); */

    /* int from_index = find_grid_index(&gamestate->overlord, meow); */


    for(int i = 355; i < 360; ++i) {
        add_to_grid_dynamic_objects(&gamestate->overlord,
                                     make_sh_circle(50*cos((i)*torad)+20, 50*sin(( i)*torad),
                                                    5, 20,
                                                    vec2(cos(i*torad), sin(i*torad)),
                                                    vec4(1, 1, 1, 1)));
    }
}
