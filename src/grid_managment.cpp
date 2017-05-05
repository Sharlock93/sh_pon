#include "../header/grid_managment.h"
#include <iostream>

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

    return row*grid->width_elem_count + col;
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
    
    ball_obj->vectors = (vec2 *) malloc(sizeof(vec2)); 
    ball_obj->vec_capacity = 1;
    ball_obj->vec_nums = 1;
    ball_obj->vectors->x = 0;
    ball_obj->vectors->y = 0;

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

draw_element sh_draw_rect(vec2 pos, float width, float height, vec4 color) {
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

draw_element sh_draw_circ(vec2 pos, float r, vec4 color) {

    int segments = 32;
    draw_element elem = {};

    elem.points = (vec2 *) malloc(sizeof(vec2)*segments);

    float angle = (6.28)/segments;

    for(int i = 0; i < segments; ++i) {
        elem.points[i] = vec2(cos(angle*i), sin(angle*i))*r;
    }

    elem.point_count = segments;
    elem.command = GL_LINE_LOOP;
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

void push_draw_text(char *text, int font_size, vec2 position) {
    
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
