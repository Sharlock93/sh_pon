#include "game_logic.h"
#include <iostream>

void project_on_axis(sh_rect *rect, vec2 axis) {
    
}

void add_to_dynamic_obj(dynamic_objects *dyn_obj, game_object *obj) {
    
    obj->next_object = NULL;
    if(dyn_obj->root_node == NULL) {
        dyn_obj->root_node = dyn_obj->last_node = obj;
    } else { 
        dyn_obj->last_node->next_object = obj;
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

extern "C" __declspec(dllexport) GAME_UPDATE_FUNC(update) {

    game_object *start_node = gamestate->objects_to_update.root_node;
     
    while(start_node) {
        switch(start_node->type) {
            case BALL: {

                ball_object *ball = (ball_object *) start_node->obj; 
                ball->circ->move_position((float)( ball->velocity*dt ));

                if(ball->circ->get_position().y < -gamestate->window_width_height.y/2.0)
                    ball->circ->_direction.y = 1; 

                if(ball->circ->get_position().y > gamestate->window_width_height.y/2.0)
                    ball->circ->_direction.y = -1; 

                if(ball->circ->get_position().x < -gamestate->window_width_height.x/2.0)
                    ball->circ->_direction.x = 1; 

                if(ball->circ->get_position().x > gamestate->window_width_height.x/2.0)
                    ball->circ->_direction.x = -1;

            } break;

            case RECTANGLE: {

                rect_object *rect = (rect_object *) start_node->obj;
                if(mouse->left_button &&
                   colision(rect->rect, mouse->mouse_x, mouse->mouse_y) &&
                   !mouse->obj)
                {
                    mouse->obj = start_node;
                }

            } break;

        }  

        start_node = start_node->next_object;
    }


    game_object **objs = gamestate->named_objects;
    for(int i = 0; i < gamestate->object_count; ++i) {

        switch(objs[i]->type) {
            case BALL: {
                ball_object *ball = (ball_object *) objs[i]->obj; 

                if(ball->vectors) {
                    vec2 result_vec = vec2(0, 0);
                    for(int i = 0; i < 2; ++i) {
                        result_vec = result_vec + ball->vectors[i];
                    }
                    ball->circ->_direction = result_vec + ball->circ->_direction;
                }

                ball->previous_pos = ball->circ->get_position();
                ball->circ->move_position(( ball->velocity*dt ));

                if(ball->circ->get_position().y < -gamestate->window_width_height.y/2.0)
                    ball->circ->_direction.y = -1*ball->circ->_direction.y; 

                if(ball->circ->get_position().y > gamestate->window_width_height.y/2.0)
                    ball->circ->_direction.y = -1*ball->circ->_direction.y; 

                if(ball->circ->get_position().x < -gamestate->window_width_height.x/2.0)
                    ball->circ->_direction.x = -1*ball->circ->_direction.x; 

                if(ball->circ->get_position().x > gamestate->window_width_height.x/2.0)
                    ball->circ->_direction.x = -1*ball->circ->_direction.x;
            } break;

            case RECTANGLE: {
                rect_object *rect = (rect_object *) objs[i]->obj;
            } break;

            case LINE: {
                line_object *line_obj = (line_object *) objs[i]->obj;
            } break;
        } 

        start_node++;
    }
    //Todo(sharo): using mouse to move an object
/*
    if(mouse->left_button && mouse->obj) {
        switch(mouse->obj->type) {
            case RECTANGLE: {
                rect_object *rect = (rect_object *) mouse->obj->obj;
                rect->rect->set_position( mouse->mouse_x, mouse->mouse_y);
            } break;
        }
    } else {
        mouse->obj = nullptr;
    }
*/ 
}

extern "C" __declspec(dllexport) GAME_RENDER_FUNC(render) { 
    int vpos = gamestate->vpos_attrib_loc;
    int color = gamestate->color_attrib_loc;
    int model = gamestate->model_t_attrib_loc;

    game_object *start_node = gamestate->objects_to_update.root_node;
    while(start_node) {
        switch(start_node->type) {
            case BALL: {
                ball_object *ball = (ball_object *) start_node->obj; 
                vec2 current_pos = ball->circ->get_position();
                ball->circ->set_position(ball->circ->get_position()*alpha + ball->previous_pos*(1.0-alpha));
                ball->circ->render(vpos, color, model);
                ball->circ->set_position(current_pos);
            } break;

            case RECTANGLE: {
                rect_object *rect = (rect_object *) start_node->obj;                    
                rect->rect->render(vpos, color, model);
            } break;
            case LINE: {
                line_object *line_obj = (line_object *) start_node->obj;               
                line_obj->line.render(vpos, color, model);
            };
        } 

        start_node = start_node->next_object;
    }    

    game_object **objs = gamestate->named_objects;
    for(int i = 0; i < gamestate->object_count; ++i) {

        switch(objs[i]->type) {
            case BALL: {
                ball_object *ball = (ball_object *) objs[i]->obj; 
                // std::cout << ball->circ->get_position()*alpha + ball->previous_pos*(1.0-alpha) << std::endl;
                // ball->circ->set_position(0, 0);
                // ball->circ->set_position(ball->circ->get_position()*alpha + ball->previous_pos*(1.0-alpha));
                ball->circ->render(vpos, color, model);
            } break;

            case RECTANGLE: {
                rect_object *rect = (rect_object *) objs[i]->obj;                    
                rect->rect->render(vpos, color, model);
            } break;
            case LINE: {
                // line_object *line_obj = (line_object *) start_node->obj;               
                // line_obj->line.render(vpos, color, model);
            };
        } 

        start_node++;
    }
}

game_object* make_sh_rect(float x, float y, float width, float height, vec4 color) {
    game_object *obj = (game_object *) malloc(sizeof(game_object)); 
    rect_object *rect = (rect_object *) malloc(sizeof(rect_object));

    rect->rect = new sh_rect(x, y, width,height, color);

    obj->type = RECTANGLE;
    obj->next_object = nullptr;
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
    ball_obj->vectors = nullptr; 

    obj->type = BALL;
    obj->next_object = nullptr;
    obj->size = sizeof(ball_object);
    obj->obj = (void *) ball_obj;

    return (obj);
}

game_object* make_sh_line(dynamic_objects *dyn, vec2 a, vec2 b, vec4 color) {
    game_object *obj  = (game_object *) malloc(sizeof(game_object)); 
    line_object *line = (line_object *) malloc(sizeof(line_object));
    line->line = sh_line(a, b, color);

    obj->type = LINE;
    obj->next_object = nullptr;
    obj->size = sizeof(line_object);
    obj->obj = line;

    game_object *rect_a = make_sh_rect(a.x, a.y, 5, 5, vec4(1, 0, 0, 1));
    game_object *rect_b = make_sh_rect(b.x, b.y, 5, 5, vec4(1, 0, 0, 1));

    add_to_dynamic_obj(dyn, rect_a);
    add_to_dynamic_obj(dyn, rect_b);

    return (obj);
}

void free_objects_all(game_state *gs) {
    for(int i = 0; i < gs->object_count; ++i) {
        free ( gs->named_objects[i]->obj );
    }

    free(gs->named_objects);
    gs->object_count = 0;
}

extern "C" __declspec (dllexport) GAME_INIT_FUNC(init) {

    if(gamestate->named_objects) {
        free_objects_all(gamestate);
    }

    gamestate->named_objects = (game_object **) malloc(sizeof(game_object **)*10);
    gamestate->named_objects[0] = ( make_sh_rect(10, 0, 10, 10, vec4(1, 1, 1, 1)) );
    gamestate->object_count++; 

    gamestate->named_objects[1] = ( make_sh_rect(1, 10, 10, 10, vec4(0, 1, 1, 1)) );
    gamestate->object_count++;  

    
    gamestate->named_objects[gamestate->object_count++] = (make_sh_circle(0, 0, 10, 1, vec2(0, 1), vec4(1, 1, 1, 1)));

    // vec2 window_x_y = gamestate->window_width_height;
    //
    // vec2 test_up_a = vproj(square._position - projected_on.point[0], normalize( projected_on.point[1] - projected_on.point[0] ));
    // vec2 test_up_b = vproj(vec2(square._position.x, square._position.y - square._height/2.0) - projected_on.point[0],
    //         normalize(projected_on.point[1] - projected_on.point[0]));
    //
    // to_center.update(projected_on.point[0] + test_up_a, square._position);
    // to_bottom.update(projected_on.point[0] + test_up_b, vec2(square._position.x, square._position.y - square._height/2.0));
    //
    // bottom_vec.update(projected_on.point[0] + test_up_a , projected_on.point[0] + test_up_b);
    //
    //
    // a.render(gamestate.vpos_attrib_loc, gamestate.color_attrib_loc, gamestate.model_t_attrib_loc);
    // square.render(gamestate.vpos_attrib_loc, gamestate.color_attrib_loc, gamestate.model_t_attrib_loc);
    //
    //
    // projected_on.render(gamestate.vpos_attrib_loc, gamestate.color_attrib_loc, gamestate.model_t_attrib_loc);
    //
    // bottom_vec.render(gamestate.vpos_attrib_loc, gamestate.color_attrib_loc, gamestate.model_t_attrib_loc);
    // anchor_b.render(gamestate.vpos_attrib_loc, gamestate.color_attrib_loc, gamestate.model_t_attrib_loc);
    // anchor_a.render(gamestate.vpos_attrib_loc, gamestate.color_attrib_loc, gamestate.model_t_attrib_loc);
    // to_center.render(gamestate.vpos_attrib_loc, gamestate.color_attrib_loc, gamestate.model_t_attrib_loc);
    // to_bottom.render(gamestate.vpos_attrib_loc, gamestate.color_attrib_loc, gamestate.model_t_attrib_loc);   
}
