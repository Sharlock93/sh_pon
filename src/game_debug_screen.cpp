#include "../header/game_debug_screen.h"
#include "../header/sh_types.h"
#include "grid_managment.cpp"
#include "game_math.cpp"
#include "game_debug.cpp"
#include "sh_fnt_reader.cpp"
#include "sh_png_reader.cpp"

#include <sharinit.cpp>
#include <iostream>

GAME_RENDER_FUNC(render) { 

    game_state *db = gamestate->debug_state;
    input_state *db_inputs = db->inputs;
    sh_debug_ui_state *db_ui = &db->debug_ui_state;
    game_object *dynamic_object = gamestate->objects_to_update->root_node;
    
    while(dynamic_object) {
        render_game_object(gamestate, dynamic_object);
        dynamic_object = dynamic_object->next_object;
    }

    // for(int i = 0; i < gamestate->overlord.elem_count; ++i) {
    //     render_grid_elem(&gamestate->overlord.elements[i], vpos, model, color);
    // }
    //

    if(db_ui->hot_object) {
        switch(db_ui->hot_object->type) {
            case BALL:
                ball_object *ball = (ball_object *)db_ui->hot_object->obj;
                push_draw_text(gamestate, sh_vec2tstr(&ball->circ->_position), 20, vec2(0, 0), vec4(1, 1, 0, 1));
                push_draw_text(gamestate, sh_vec2tstr(&ball->circ->_direction), 20, vec2(0, 20), vec4(1, 1, 0, 1));
                break;
        }

    }

    if(db_ui->active_object) {
        switch(db_ui->active_object->type) {
            case BALL:
                ball_object *ball = (ball_object *)db_ui->active_object->obj;
                push_draw_text(gamestate, sh_vec2tstr(&ball->circ->_position), 20, vec2(0, -40), vec4(1, 0, 0, 1));
                push_draw_text(gamestate, sh_vec2tstr(&ball->circ->_direction), 20, vec2(0, -20), vec4(1, 0, 0, 1));
                write_to_gl_log(sh_vec2tstr(&ball->circ->_direction));
                write_to_gl_log("  ");
                write_to_gl_log(sh_vec2tstr(&ball->circ->_position));
                write_to_gl_log("\n");
                break;
        }

    }


    render_draw_stack(gamestate);
}

GAME_UPDATE_FUNC(update) {

}

GAME_DEBUG_FUNC(clean_up) {
    fclose(gl_log_file);
}

GAME_INIT_FUNC(init) {
    glfwMakeContextCurrent(gamestate->window);
    fopen_s(&gl_log_file, "log.txt", "w");
    
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
