#include <game_debug_screen.h>
#include <iostream>

GAME_RENDER_FUNC(render) { 
    int vpos = gamestate->vpos_attrib_loc;
    int color = gamestate->color_attrib_loc;
    int model = gamestate->model_t_attrib_loc;


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

    for (int i = 0; i < gamestate->overlord.elem_count; ++i) {
        dynamic_objects *dynamic_obj = gamestate->overlord.elements[i].dyn_objs;
        if(!dynamic_obj) continue;
        game_object *root_node = dynamic_obj->root_node;
        while(root_node) {
            switch(root_node->type) {
                case BALL: {
                    ball_object *ball = (ball_object *) root_node->obj; 
                } break;
            }
            game_object *prev = root_node;
            root_node = root_node->next_object;

        }
    }

    for(int i = 0; i < gamestate->overlord.elem_count; ++i) {
        render_grid_elem(&gamestate->overlord.elements[i], vpos, model, color);
    }
}

GAME_UPDATE_FUNC(update) {

    glfwMakeContextCurrent(gamestate->window);

    for (int i = 0; i < gamestate->overlord.elem_count; ++i) {
        dynamic_objects *dynamic_obj = gamestate->overlord.elements[i].dyn_objs;
        if(!dynamic_obj) continue;
        game_object *root_node = dynamic_obj->root_node;
        while(root_node) {
            switch(root_node->type) {
                case BALL: {
                    ball_object *ball = (ball_object *) root_node->obj; 
                    ball->circ->_color = ball->circ->_color*0.9;
                } break;
            }
            root_node = root_node->next_object;
        }
    }


    game_state *state_debug = gamestate->debug_state;
    int counter = 0;

    float x = -gamestate->overlord.grid_width/2.0;
    float y = gamestate->overlord.grid_height/2.0 - 5;
    float xstep = gamestate->overlord.grid_width/state_debug->overlord.elem_count;
    float ystep = 10;//gamestate->overlord.grid_height/state_debug->overlord.elem_count;
    

    for(int i = 0; i < state_debug->overlord.elem_count; ++i) {
        dynamic_objects *de_dyn_obj = state_debug->overlord.elements[i].dyn_objs;
        dynamic_objects *dyn_obj = gamestate->overlord.elements[0].dyn_objs;
        game_object *root_object = de_dyn_obj->root_node;
        if(root_object == nullptr) continue;
        int j = 0;
        float radius = 5;
        while(root_object) {
            add_to_dynamic_obj(
                    dyn_obj,
                    make_sh_circle(
                        x+i*xstep, y - j*ystep,
                        radius,
                        0,
                        vec2(0,0),
                        vec4(1, 0, 0, 1)
                        )
                    );
            j++;
            root_object = root_object->next_object;
        }
    }

    // for (int i = 0; i < gamestate->overlord.elem_count; ++i) {
    //     dynamic_objects *dynamic_obj = gamestate->overlord.elements[i].dyn_objs;
    //     if(!dynamic_obj) continue;
    //     game_object *root_node = dynamic_obj->root_node;
    //     while(root_node) {
    //         switch(root_node->type) {
    //             case BALL: {
    //                 ball_object *ball = (ball_object *) root_node->obj; 
    //                 ball->circ->move_position(10*dt);
    //             } break;
    //         }
    //         root_node = root_node->next_object;
    //     }
    // }
}

GAME_INIT_FUNC(init) {
    glfwMakeContextCurrent(gamestate->window);
    init_grid(&gamestate->overlord, 500, 700, 1, 1 , vec2(0, 0));

    for(int i = 0; i < gamestate->overlord.width_elem_count*gamestate->overlord.height_elem_count; ++i) {
        init_grid_elem(&gamestate->overlord, i, vec4(1, 1, 1, 1));
    }


    
    gamestate->objects_to_update = (dynamic_objects *) malloc(sizeof(dynamic_objects));

    game_state *state_debug = gamestate->debug_state;
    int counter = 0;

    int x = -240;
    int y = 340;
    float xstep = gamestate->overlord.grid_width/state_debug->overlord.elem_count;
    float ystep = gamestate->overlord.grid_height/state_debug->overlord.elem_count;
    

    for(int i = 0; i < state_debug->overlord.elem_count; ++i) {
        dynamic_objects *de_dyn_obj = state_debug->overlord.elements[i].dyn_objs;
        dynamic_objects *dyn_obj = gamestate->overlord.elements[0].dyn_objs;
        game_object *root_object = de_dyn_obj->root_node;
        if(root_object == nullptr) continue;
        int j = 0;
        float radius = 10.0/de_dyn_obj->object_counts;
        while(root_object) {
            add_to_dynamic_obj(dyn_obj, make_sh_circle(x+i*xstep, y - j*ystep, radius, 0, vec2(0,0), vec4(1, 0, 0, 1)));
            j++;
            root_object = root_object->next_object;
        }
    }
}
