#define sh_inspect
#define array_count(array) (sizeof((array))/(sizeof(array[0])))

#include <Windows.h>
#include <sharinit.cpp>
#include <sh_circle.h>
#include <sh_line.h> 
#include <sh_rect.h>
#include <intrin.h>
#include <stdint.h>
#include <sh_debug_meta_info.h>
#include <sh_types.h>
#include <sh_fnt_reader.h>
#include <sh_png_reader.h>
#include <game_debug.h>
#include <grid_managment.h>
#include <game_math.h>
#include <game_debug_screen.h>

#include "sh_generated.cpp"
#include "game_math.cpp"
#include "game_debug.cpp"
#include "sh_fnt_reader.cpp"
#include "sh_png_reader.cpp"
#include "grid_managment.cpp"


struct tree {
        char *name;
        bool expanded;
        game_object *obj;
        tree *next;
        tree *prev;
};

tree *main_tree = nullptr;
vec2 dump_game_object(game_object *obj, float font_size, vec2 pos) {
        switch (obj->type) {
                case BALL:
                        ball_object *ball = (ball_object *)obj->obj;
                        dump_struct_to_screen((void *) ball, class_ball_object, array_count(class_ball_object), font_size, &pos);
                        break;
        }

        return pos;
}

void push_draw_tree(tree *passed_tree, input_state *input) {
        vec4 color_tri = vec4(1, 1, 0, 0);
        float height = 10;
        float width  = 10;
        float font_size = 18;
        vec2 pos = vec2(-250 + width/2.0, 230 - height/2.0);
        vec4 color(1, 1, 1, 1);

        pos.x += font_size;

        vec2 mouse_point(input->mouse.mouse_x, input->mouse.mouse_y);

        while(passed_tree) {
                sh_rect_container node_rect = make_text_rect(passed_tree->name, pos, font_size);

                if(point_in_rect(&mouse_point, &node_rect)) {
                        if(input->mouse.left_button) {
                                passed_tree->expanded = !passed_tree->expanded;
                        }
                        color = vec4(1, 0, 0, 1);
                } else {
                        color = vec4(1, 1, 1, 1);
                }

                push_draw_text(gl_game_state, passed_tree->name, font_size, pos, color);
                pos.y -= font_size;

                if(passed_tree->expanded && passed_tree->obj) {
                        passed_tree->name[0] = '-';
                        if(passed_tree->obj) {
                                pos = dump_game_object(passed_tree->obj, font_size, pos);
                        }
                } else {
                        passed_tree->name[0]= '+';
                }

                passed_tree = passed_tree->next;
        }
}

bool object_exists_in_tree(tree *t, game_object *obj) {
        while(t) {

                if(t->obj && (t->obj->id == obj->id)) return true;
                t = t->next;
        }
        return false;
}

void add_element_to_tree(tree *t, game_object *obj) {
        if(object_exists_in_tree(t, obj)) return;
        tree *new_tree = (tree *) malloc(sizeof(tree));
        new_tree->next = nullptr;
        new_tree->prev = nullptr;
        new_tree->name = nullptr;
        new_tree->expanded = false;
        new_tree->obj = obj;


        char buffer[256] = {};
        int length = sprintf_s(buffer, "+ Object id (%d)", obj->id) + 1;
        new_tree->name = (char *) malloc(length );
        strcpy_s(new_tree->name, length, buffer);
        new_tree->name[length] = 0;

        tree *temp_head = main_tree;
        while(temp_head->next) temp_head = temp_head->next;
        
        temp_head->next = new_tree;
        new_tree->prev = temp_head;
}



GAME_RENDER_FUNC(render) { 


    game_state *db = gamestate->debug_state;
    input_state *db_inputs = db->inputs;
    sh_debug_ui_state *db_ui = &db->debug_ui_state;
    game_object *dynamic_object = gamestate->objects_to_update->root_node;

    if(db_ui->hot_object) {
        switch(db_ui->hot_object->type) {
            case BALL:
                ball_object *ball = (ball_object *)db_ui->hot_object->obj;
                push_draw_text(gamestate, sh_vec2tstr(&ball->circ->_position), 20, vec2(0, 0), vec4(1, 1, 0, 1));
                push_draw_text(gamestate, sh_vec2tstr(&ball->circ->_direction), 20, vec2(0, 20), vec4(1, 1, 0, 1));
                push_draw_element(&gamestate->renderstack, sh_gen_draw_circ(ball->circ->_position, ball->circ->_r, ball->circ->_color));
                break;
        }

    }

    if(db_ui->active_object) {
            if(db_inputs->mouse.left_button) {
                    add_element_to_tree(main_tree,  db_ui->active_object);
            }

            switch(db_ui->active_object->type) {
                    case BALL:
                            ball_object *ball = (ball_object *)db_ui->active_object->obj;
                            break;
            }

    }


    char buffer[256];
    sprintf_s(buffer, 64, "%f %f", db_inputs->mouse.mouse_x, db_inputs->mouse.mouse_y);
    push_draw_text(gl_game_state, buffer, 18, vec2(-220, 220), vec4(1, 1, 1, 1));

    push_draw_tree(main_tree, db_inputs);
    render_draw_stack(gamestate);
}

GAME_UPDATE_FUNC(update) {
        game_state *db = gamestate->debug_state;
        input_state *db_inputs = db->inputs;

        if(global_active_variable) {
                switch(global_active_variable_type) {
                        case type_int: {
                                int *t = (int *)global_active_variable;
                                *t = (int)db_inputs->mouse.mouse_x;
                        } break;
                        case type_float: {
                                float *t = (float *)global_active_variable;
                                *t = db_inputs->mouse.mouse_x;
                        } break;
                        case type_vec2: {
                                vec2 p (db_inputs->mouse.mouse_x, db_inputs->mouse.mouse_y);
                                vec2 *vec = (vec2 *) global_active_variable;
                                vec2 diff= p - *vec;
                                *vec = *vec + diff;
                        } break;
                } 
        }
}

GAME_DEBUG_FUNC(clean_up) {
    fclose(gl_log_file);
}

GAME_INIT_FUNC(init) {
    glfwMakeContextCurrent(gamestate->window);
    fopen_s(&gl_log_file, "log.txt", "w");
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
#endif

    char *name = "+root";
    main_tree = (tree *)malloc(sizeof(tree));
    main_tree->name = (char *) malloc(strlen(name) + 1);
    strcpy_s(main_tree->name, strlen(name)+1, name);
    main_tree->name[strlen(name) + 1] = '0';
    main_tree->next = nullptr;
    main_tree->prev = nullptr;
    main_tree->obj = nullptr;

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

    init_grid(&gamestate->overlord, 500, 500, 1, 1 , vec2(0, 0));


    for(int i = 0; i < gamestate->overlord.width_elem_count*gamestate->overlord.height_elem_count; ++i) {
        init_grid_elem(&gamestate->overlord, i, vec4(1, 1, 1, 1));
    }

    for (int i = 0; i < 2; ++i) {
        game_object *new_object = make_sh_circle(i, 20*cos(i*torad), 20*sin(i*torad), 5, 500, vec2(cos(i), sin(i)), vec4(sin(i), sin(cos(i*torad)*cos(i*torad)), cos(i*torad), 1));
        add_to_grid_objects(gamestate, new_object, false); 
    }
}

debug_record records[__COUNTER__ +1];
