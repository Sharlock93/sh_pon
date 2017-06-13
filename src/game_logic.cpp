/* @Todo(sharo): 
 * - take physics steps using verlet or velocity verlet /Note(sharo): for later
 * - make gui elements using immediate mode gui
 *      - almost done
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

#define STB_IMAGE_IMPLEMENTATION
#include "../header/stb_image.h"


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
    TIME_BLOCK;

    // if(sh_button(gamestate, 0, vec2(25, 25), "hello world", 40, 20)) {
    //     std::cout << "it works" << std::endl;
    // }
    
    if(!inputs->mouse.left_button) {
        gamestate->inputs->mouse.hot_object = nullptr;
    }

    gamestate->debug_ui_state.hot_object = nullptr;

    if(inputs->mouse.right_button) {
        gamestate->debug_ui_state.active_object = nullptr;
    }


    rect_object *paddle = nullptr;

    game_object *b = gamestate->named_objects[MAIN_BALL];//->obj;

    if(gamestate->named_objects[PADDLE]) {
        paddle = (rect_object *) gamestate->named_objects[PADDLE]->obj;
    }

    if(paddle) {
        if(inputs->keyboard.arrow_left) {
            paddle->previous_pos = paddle->rect->_position;
            paddle->rect->move_position(-paddle->velocity*dts);
        }

        if(inputs->keyboard.arrow_right) {
            paddle->previous_pos = paddle->rect->_position;
            paddle->rect->move_position(paddle->velocity*dts);
        }
    }


    if(!without_move) {
        objects *dynamic_obj = gamestate->objects_to_update;
        game_object *root_node = dynamic_obj->root_node;
        for(int i = 0; i < dynamic_obj->object_count; ++i) {
            switch(root_node->type) {
                case BALL: {
                               ball_object *ball = (ball_object *) root_node->obj; 
                               ball->previous_pos = ball->circ->_position;
                               ball->circ->move_position(ball->velocity*dts); 
                           } break;
            }
            root_node = root_node->next_object;
        }
    }

    if(true) {
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

    if(true) {
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

                        vec2 p(inputs->mouse.mouse_x, inputs->mouse.mouse_y);
                        if(point_in_circ(ball, &p)) {
                            gamestate->debug_ui_state.hot_object = d_o->obj;

                            if(inputs->mouse.left_button) {
                                gamestate->debug_ui_state.active_object = d_o->obj;
                            } else {
                                inputs->mouse.hot_object = d_o->obj;
                            }
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
    if(true) {
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

    if(inputs->mouse.left_button && inputs->mouse.hot_object ) {
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
    TIME_BLOCK;
    input_state *inputs = gamestate->inputs;

    if(inputs->mouse.left_button) {
        gamestate->ui_state.active_object = 0;
        gamestate->ui_state.hot_object = 0;

    }

    game_object *root_node = gamestate->objects_to_update->root_node;
    for(int i = 0; i < gamestate->objects_to_update->object_count; ++i) {
        render_game_object(gamestate, root_node);
        root_node = root_node->next_object;
    }

    root_node = gamestate->static_objects->root_node;
    for(int i = 0; i < gamestate->static_objects->object_count; ++i) {
        render_game_object(gamestate, root_node);
        root_node = root_node->next_object;
    }

    for(int i = 0; i < gamestate->object_count; ++i) {
        game_object *root_node = gamestate->named_objects[i];
        render_game_object(gamestate, root_node);
    }
    
    // for(int i = 0; i < gamestate->overlord.elem_count; ++i) {
    //     render_grid_elem(&gamestate->overlord.elements[i], vpos, model, color);
    // }
    // render_grid(gamestate);

    render_draw_stack(gamestate);

}

void free_objects_all(game_state *gs) {
    for(int i = 0; i < gs->object_count; ++i) {
        delete ( gs->named_objects[i]->obj );
    }

    free(gs->named_objects);
    gs->object_count = 0;
}


GAME_INIT_FUNC(init) {

    glfwMakeContextCurrent(gamestate->window);
    gl_game_state = gamestate;

    int size = 0;
    char *file = shareadfile("sh_font.fnt", &size);

    sh_read_character_descripter_in_memory(&gamestate->font, (uint8 *) file, size);
    free(file);
    
    file = shareadfile("sh_font_0.png", &size);
    
#if 1
    int32 x = 0;
    int32 y = 0;
    int32 n = 0;
    uint8 *image = sh_load_png_mem((uint8 *)file, size, &x, &y);
    free(file);
    
    glGenTextures(1, &gamestate->fnt_tex);
    glBindTexture(GL_TEXTURE_2D, gamestate->fnt_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, x, y, 0, GL_RED, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,  GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    if (gamestate->named_objects) {
        free_objects_all(gamestate);
    }
#endif

    gamestate->named_objects = (game_object **) malloc(sizeof(game_object *)*10);
    for(int i = 0; i < 10; ++i) {
        gamestate->named_objects[i] = nullptr;
    }
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

    init_grid(&gamestate->overlord, 500, 500, 1, 1 , vec2(0, 0));

    for(int i = 0; i < gamestate->overlord.width_elem_count*gamestate->overlord.height_elem_count; ++i) {
        init_grid_elem(&gamestate->overlord, i, vec4(1, 1, 1, 1));
    }

    add_to_grid_objects(gamestate, make_sh_line(gen_id(), vec2(-20, 0), vec2(20, 0), vec4(1, 0, 0, 1)), true);
    // add_to_grid_objects(gamestate, make_sh_line(gen_id(), vec2(-230, -250), vec2(250, 250), vec4(1, 0, 0, 1)), true);
    add_to_grid_objects(gamestate, make_sh_line(gen_id(), vec2(-248, -250), vec2(-248,  250), vec4(1, 0, 0, 1)), true); //Left
    add_to_grid_objects(gamestate, make_sh_line(gen_id(), vec2( 248, -250), vec2( 248,  250), vec4(1, 0, 0, 1)), true); //right
    add_to_grid_objects(gamestate, make_sh_line(gen_id(), vec2(-250,  248), vec2( 250,  248), vec4(1, 0, 0, 1)), true); //top
    add_to_grid_objects(gamestate, make_sh_line(gen_id(), vec2(-250, -248), vec2( 250, -248), vec4(1, 0, 0, 1)), true); //bottom
    //
    game_object *testing_object = make_sh_circle(gen_id(), 0, -100, 10, 300, vec2(0, 1), vec4(1, 0, 0, 1));
    ball_object *ball = (ball_object *) testing_object->obj;
    add_to_grid_objects(gamestate, testing_object , false); 

    // for (int i = 0; i < 101; ++i) {
    //     game_object *new_object = make_sh_circle(gen_id(), 20*cos(i*torad), 20*sin(i*torad), 5, 500, vec2(cos(i), sin(i)), vec4(sin(i), sin(cos(i*torad)*cos(i*torad)), cos(i*torad), 1));
    //     add_to_grid_objects(gamestate, new_object, false); 
    // }
    //
    // gamestate->named_objects[MAIN_BALL] =  make_sh_circle(gen_id(), 0, 0, 5, 400, vec2(0, -1), vec4(1, 0, 0, 1));
    // add_to_grid_objects(gamestate, gamestate->named_objects[MAIN_BALL], false); 
    //
    // gamestate->named_objects[2] =  make_sh_circle(gen_id(), 100, -100, 5, 400, vec2(0, 1), vec4(1, 0, 0, 1));
    // add_to_grid_objects(gamestate, gamestate->named_objects[2], false); 


    // gamestate->named_objects[PADDLE] =  make_sh_rect(gen_id(), 0, -240,  250, 15, 80, vec4(1, 1, 1, 1));
    // add_to_grid_objects(gamestate, gamestate->named_objects[PADDLE], false); 
}

debug_record records[__COUNTER__ +1];

GAME_DEBUG_FUNC(debug_func) {
    static char buffer[256] = {};
    int32 debug_fnt_size = PNT2PXL(17);
    vec2 cursor = vec2(-250, 250 - debug_fnt_size);
    float scale = (float)debug_fnt_size/-gamestate->font.info.size;
    for(int i = 0; i < ArrayCount(records); ++i) {
        debug_record *record = records+i;
        if(record->func_name != NULL) {
            sprintf_s(buffer, 256, "%s : %d", record->func_name, record->time);
            push_draw_text(gamestate, buffer, debug_fnt_size, cursor, vec4(0, 1, 0, 1));
            cursor.y -= gamestate->font.common.lineHeight*scale;
        }
    }
}
