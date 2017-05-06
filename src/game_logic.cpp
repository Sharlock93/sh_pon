/* @Todo(sharo): 
 * - take physics steps using verlet or velocity verlet /Note(sharo): for later
 * - make gui elements using immediate mode gui
 * - select object and change their properties using debug menus
 * - textures for objects
 * - renovate the way objects are handled/i.e: make them into meshes or something
 * - particle systems
 * - be able to create and delete objects dynamically
 * - rotation of objects
 * - constrains on objects for physics simulation
 * - mouse and keyboard event handling //almost done
 * - concept of levels
 * - serialization
 * - debug services
 */
#include "../header/sh_types.h"

#include "grid_managment.cpp"
#include "game_math.cpp"
#include "game_debug.cpp"
#include "../header/game_logic.h"
#include "sh_fnt_reader.cpp"
#include "sh_png_reader.cpp"

#include <sharinit.cpp>

#include <iostream>

vec2 vmini(-250, -250);
vec2 vmaxi(250, 250);

void vmax(vec2 *a, vec2 *b) {
    if(b->x > a->x)
        a->x = b->x;

    if(b->y > a->y)
        a->y = b->y;
}

void vmin(vec2 *a, vec2 *b) {
    if(b->x < a->x)
        a->x = b->x;

    if(b->y < a->y)
        a->y = b->y;

}

int gen_id() {
    static int next_id = 0;
    return next_id++;
}

int collision(grid_element *elem, game_object *object) {
    switch(object->type) {
        case BALL: {
            ball_object *ball = (ball_object *) object->obj;
            return col_circ_rect(elem, ball);
        } break;
        
        case RECTANGLE: {
            rect_object *rect = (rect_object *) object->obj;
            return col_rect_rect(elem, rect);
        } break;

        case LINE: {
            line_object *line = (line_object *) object->obj;
            return col_rect_line_LB(elem, line);
        } break;

        default:
            return 0;
    }
}


void update_object_grid_relation(game_grid *grid, game_object *object) {
    for(int i = 0; i < grid->elem_count; ++i) {
        grid_element *elem = &grid->elements[i];
        if(collision(elem, object)) {
            object_bucket *bucket_to_remove = in_grid_element(elem, object);
                if(bucket_to_remove) {
                continue;
            } else {
                add_to_obj_bucket_list(elem->obj_buckets, object);
            }
        } else {
            object_bucket *bucket_to_remove = in_grid_element(elem, object);
            if(bucket_to_remove) {
                if( is_bucket_in_list(elem->obj_buckets, bucket_to_remove) ) {
                    bucket_to_remove->to_delete = 1;
                }
            }
        }
    }
}

//void update(game_state *gamestate, input_state *inputs, double dts, bool without_move) {
GAME_UPDATE_FUNC(update) {
    // TIME_BLOCK;

    //
    if(sh_button(gamestate, 0, vec2(25, 25), "hello world", 40, 20)) {
        std::cout << "it works" << std::endl;
    }
    
    // rect_object *paddle = nullptr;
    //
    // if(gamestate->named_objects[PADDLE]) {
    //     paddle = (rect_object *) gamestate->named_objects[PADDLE]->obj;
    // }
    //
    // if(paddle) {
    //
    //     if(inputs->keyboard.arrow_left) {
    //         paddle->previous_pos = paddle->rect->_position;
    //         paddle->rect->move_position(-paddle->velocity*dt);
    //     }
    //
    //     if(inputs->keyboard.arrow_right) {
    //         paddle->previous_pos = paddle->rect->_position;
    //         paddle->rect->move_position(paddle->velocity*dt);
    //     }
    // }

    objects *dynamic_obj = gamestate->objects_to_update;
    game_object *root_node = dynamic_obj->root_node;
    for(int i = 0; i < dynamic_obj->object_count; ++i) {
        switch(root_node->type) {
            case BALL: {
                ball_object *ball = (ball_object *) root_node->obj; 
                ball->previous_pos = ball->circ->_position;
                ball->circ->move_position(ball->velocity*dt); 
            } break;
        }
        root_node = root_node->next_object;
    }

    if(!without_move) {
        for(int i = 0; i < gamestate->overlord.elem_count;++i) {
            grid_element *elem = &gamestate->overlord.elements[i];

            if(elem == nullptr)
                continue;

            object_bucket *d_o = elem->obj_buckets->root_node;
            while(d_o) {
                Assert(d_o != nullptr);
                if(d_o == nullptr)
                    continue;
                update_object_grid_relation(&gamestate->overlord, d_o->obj);
                d_o = d_o->next;
            }
        }
    }

    if(!without_move) {
        for(int i = 0; i < gamestate->overlord.elem_count;++i) {
            grid_element *elem = &gamestate->overlord.elements[i];
            if(elem == nullptr) continue;

            for( object_bucket *d_o = elem->obj_buckets->root_node;
                d_o ;
                d_o = d_o->next )
            {
                switch(d_o->obj->type) {
                    case BALL: {
                        ball_object *ball = (ball_object *) d_o->obj->obj;
                        vec2 *p = &ball->circ->_position;

                        mouse_state *m = &inputs->mouse;
                        
                        float w = 5;
                        if(m->left_button) {
                            if(p->x < ( m->mouse_x + w) &&
                               p->x > (m->mouse_x - w) &&
                               p->y < (m->mouse_y + w) &&
                               p->y > (m->mouse_y - w) 
                              )
                            {
                                m->hot_object = d_o->obj;
                            }
                        } else {
                            inputs->mouse.hot_object = nullptr;
                        }

                        object_bucket *rest = d_o->next;
                        while(rest) {
                            col_circ(ball, rest->obj, gamestate);
                            rest = rest->next;
                        }
                    } break;

                    case LINE: {
                        line_object *line = (line_object *)d_o->obj->obj;
                        object_bucket *rest = d_o->next;
                        while(rest) {
                            col_line(line, rest->obj, gamestate);
                            rest = rest->next;
                        }
                    } break;
                    case RECTANGLE: {
                        rect_object *rect = (rect_object *) d_o->obj->obj;
                        object_bucket *rest = d_o->next;
                        while(rest) {
                            col_rect(rect, rest->obj, gamestate);
                            rest = rest->next;
                        }
                    } break;
                }
            }
        }

    }

    //Removing elements
    if(!without_move) {
        for(int i = 0; i < gamestate->overlord.elem_count;++i) {
            grid_element *elem = &gamestate->overlord.elements[i];
            if(elem == nullptr)
                continue;
            object_bucket *d_o = elem->obj_buckets->root_node;
            while(d_o) {
                object_bucket *prev = d_o->prev;
                object_bucket *next = d_o->next;

                if(d_o->to_delete) {
                    remove_from_grid_elem(elem->obj_buckets, &d_o);
                    if(prev != nullptr) d_o = prev; 
                    else  d_o = next;
                } else {
                    d_o = d_o->next;
                }
            }
        }

    }

    for(int i = 0; i < gamestate->overlord.elem_count; ++i) {
        grid_element *elem = &gamestate->overlord.elements[i];
        if(elem->obj_buckets->object_count <= 0) {
            elem->color = vec4(1, 1, 1, 1);
        } else if(elem->obj_buckets->object_count >= 5) {
            elem->color = vec4(1, 1, 0, 1);
        } else {
            elem->color = vec4(1, 0, 0, 1);
        }
    }

    if(inputs->mouse.hot_object) {
        game_object *hot = inputs->mouse.hot_object;
        switch(hot->type) {
            case BALL:
                ball_object *ball = (ball_object *) hot->obj;
                ball->previous_pos = ball->circ->_position;
                ball->circ->set_position(inputs->mouse.mouse_x, inputs->mouse.mouse_y);
                break;
        }
    }

}

GAME_RENDER_FUNC(render) {
    int vpos = gamestate->vpos_attrib_loc;
    int color = gamestate->color_attrib_loc;
    int model = gamestate->model_t_attrib_loc;
    input_state *inputs = gamestate->inputs;

    if(inputs->mouse.left_button) {
        gamestate->ui_state.active_object = 0;
        gamestate->ui_state.hot_object = 0;

    }

    objects *dynamic_obj = gamestate->objects_to_update;
    game_object *root_node = dynamic_obj->root_node;
    for(int i = 0; i < dynamic_obj->object_count; ++i) {
        switch(root_node->type) {
            case BALL: {
                ball_object *ball = (ball_object *) root_node->obj; 

                vec2 old_pos = ball->circ->_position;
                vec2 new_pos = ball->circ->_position*alpha + ball->previous_pos*(1.0 - alpha);
                ball->circ->set_position(new_pos);

                ball->circ->render(vpos, color, model);
                ball->circ->set_position(old_pos);
            } break;

            case RECTANGLE: {
                rect_object *rect = (rect_object *) root_node->obj;
                rect->rect->render(vpos, color, model);
            } break;
            case LINE: {
                line_object *line = (line_object *) root_node->obj;
                line->line->render(vpos, color, model);
            } break;

        }
        root_node = root_node->next_object;
    }

    // root_node = gamestate->static_objects->root_node;
     root_node = gamestate->static_objects->root_node;
    for(int i = 0; i < gamestate->static_objects->object_count; ++i) {
        switch(root_node->type) {
            case BALL: {
                ball_object *ball = (ball_object *) root_node->obj; 
                ball->circ->render(vpos, color, model);
            } break;

            case RECTANGLE: {
                rect_object *rect = (rect_object *) root_node->obj;
                rect->rect->render(vpos, color, model);
            } break;
            case LINE: {
                line_object *line = (line_object *) root_node->obj;
                line->line->render(vpos, color, model);
            } break;
        }
        root_node = root_node->next_object;
    }

    if(sh_button(gamestate, __LINE__, vec2(0, 0) , "hello world", 20, 40)) {

    }

    for(int i = 0; i < gamestate->object_count; ++i) {
        game_object *root_node = gamestate->named_objects[i];

        switch(root_node->type) {
            case BALL: {
                ball_object *ball = (ball_object *) root_node->obj; 
                ball->circ->render(vpos, color, model);
           } break;

            case RECTANGLE: {
                rect_object *rect = (rect_object *) root_node->obj;
                rect->rect->render(vpos, color, model);
            } break;
            case LINE: {
                line_object *line = (line_object *) root_node->obj;
                line->line->render(vpos, color, model);
            } break;
        }

    }
    
    for(int i = 0; i < gamestate->overlord.elem_count; ++i) {
        render_grid_elem(&gamestate->overlord.elements[i], vpos, model, color);
    }

    uint32 vbo;
    glGenBuffers(1, &vbo);
    draw_element draw = {};
    
    while(pop_element(&gamestate->renderstack, &draw)) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vec2)*draw.point_count, draw.points, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(vpos, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glUniform4fv(color, 1, &draw.color[0]);
        glDrawArrays(draw.command, 0, draw.point_count);
        free(draw.points);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &vbo);

    
}

void free_objects_all(game_state *gs) {
    for(int i = 0; i < gs->object_count; ++i) {
        delete ( gs->named_objects[i]->obj );
    }

    free(gs->named_objects);
    gs->object_count = 0;
}


GAME_INIT_FUNC(init) {


    int size = 0;
    char *file = shareadfile("sh_font.fnt", &size);

    sh_read_character_descripter_in_memory(&gamestate->font, (uint8 *) file, size);
    free(file);
    
    file = shareadfile("sh_font_0.png", &size);
    
    int32 x = 0;
    int32 y = 0;
    uint8 *image = sh_load_png_mem((uint8 *)file, size, &x, &y);

    free(file);

    glfwMakeContextCurrent(gamestate->window);
    if (gamestate->named_objects) {
        free_objects_all(gamestate);
    }

    gamestate->named_objects = (game_object **) malloc(sizeof(game_object *)*10);
    gamestate->object_count = 0;

    gamestate->objects_to_update = (objects *) malloc(sizeof(objects));
    gamestate->objects_to_update->root_node = NULL;
    gamestate->objects_to_update->last_node = NULL;
    gamestate->objects_to_update->object_count = 0;

    gamestate->static_objects = (objects *) malloc(sizeof(objects));
    gamestate->static_objects->root_node = NULL;
    gamestate->static_objects->last_node = NULL;
    gamestate->static_objects->object_count = 0;

    gamestate->renderstack = {};
    make_stack_capacity(&gamestate->renderstack, 10);

    init_grid(&gamestate->overlord, 500, 500, 9, 9 , vec2(0, 0));

    //
    
    for(int i = 0; i < gamestate->overlord.width_elem_count*gamestate->overlord.height_elem_count; ++i) {
        init_grid_elem(&gamestate->overlord, i, vec4(1, 1, 1, 1));
    }

    // add_to_grid_objects(gamestate, make_sh_line(gen_id(), vec2(-250, -250), vec2(250, 250), vec4(1, 0, 0, 1)), true);
    add_to_grid_objects(gamestate, make_sh_line(gen_id(), vec2(-245, -250), vec2(-245,  250), vec4(1, 0, 0, 1)), true); //Left
    add_to_grid_objects(gamestate, make_sh_line(gen_id(), vec2( 245, -250), vec2( 245,  250), vec4(1, 0, 0, 1)), true); //right
    add_to_grid_objects(gamestate, make_sh_line(gen_id(), vec2(-250,  245), vec2( 250,  245), vec4(1, 0, 0, 1)), true); //top
    add_to_grid_objects(gamestate, make_sh_line(gen_id(), vec2(-250, -245), vec2( 250, -245), vec4(1, 0, 0, 1)), true); //bottom

    // game_object *testing_object = make_sh_circle(gen_id(), 0, -230, 5, 300, vec2(0, 1), vec4(1, 0, 1, 1));
    // ball_object *ball = (ball_object *) testing_object->obj;
    // add_to_grid_objects(gamestate, testing_object , false); 


    // for (int i = 0; i < 100; ++i) {
    //     game_object *new_object = make_sh_circle(gen_id(), 20*cos(i), 20*sin(i), 5, 400, vec2(cos(i), sin(i)), vec4(1, sin(cos(i*torad)*cos(i*torad)), cos(i*torad), 1));
    //     add_to_grid_objects(gamestate, new_object, false); 
    // }

    // gamestate->named_objects[PADDLE] =  make_sh_rect(gen_id(), 0, 0,  250, 15, 80, vec4(1, 1, 1, 1));
    // add_to_grid_objects(gamestate, gamestate->named_objects[PADDLE], false); 
}

debug_record records[__COUNTER__ +1];
GAME_DEBUG_FUNC(debug_func) {
    for(int i = 0; i < ArrayCount(records); ++i) {
        debug_record *record = records+i;
        // if(record->func_name != NULL)
        //     std::cout << record->func_name << " " << record->time << std::endl;
    }
}
