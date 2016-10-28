#include <Windows.h>
#include <iostream>
#include <sharinit.h>
#include <sharfunc.h>
#include <GLFW\glfw3.h>
#include <sh_circle.h>
#include <game_logic.h>
#include <sh_rect.h>
#include <sh_line.h>
#include <stdio.h>

//Note(sharo): typedefs
typedef float real32;
typedef int64_t int64;

//Note(Sharo): Globals
#define SCREEN_SIZE_X 500 
#define SCREEN_SIZE_Y 500 

input_state inputs_state = {};

// const double time_change = 0.01;
//
static void toggle_fullscreen(game_state *gm, GLFWwindow *window) {
    if(gm->full_screen) {
        glfwSetWindowMonitor(window, NULL, 0, 0, SCREEN_SIZE_X, SCREEN_SIZE_Y, GLFW_DONT_CARE);
        glViewport(0, 0, SCREEN_SIZE_X, SCREEN_SIZE_Y);
        gm->window_width_height = vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y);
        glUniformMatrix4fv(2, 1, GL_TRUE, shaortho(gm->window_width_height.x/2.0f + 0.5f, -gm->window_width_height.x/2.0f,
                                               gm->window_width_height.y/2.0f, -gm->window_width_height.y/2.0f - 0.5f, -1.0f, 1.0f));
        gm->full_screen = 0;
    } else {
        glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, 1366, 768, GLFW_DONT_CARE);
        glViewport(0, 0, 1366, 768);
        gm->window_width_height = vec2(1366, 768);
        glUniformMatrix4fv(2, 1, GL_TRUE, shaortho(gm->window_width_height.x/2.0f + 0.5f, -gm->window_width_height.x/2.0f,
                                               gm->window_width_height.y/2.0f, -gm->window_width_height.y/2.0f - 0.5f, -1.0f, 1.0f));
        gm->full_screen = 1;
    }

}

static void mouse_pos(GLFWwindow *window, double x, double y) {
    inputs_state.mouse.mouse_x = x - 250;
    inputs_state.mouse.mouse_y = 250 - y;
}

static void keyboard(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if(key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
        inputs_state.keyboard.arrow_left = 1;
    } else if(key == GLFW_KEY_LEFT && action == GLFW_RELEASE) {
        inputs_state.keyboard.arrow_left = 0;
    }
    
    if(key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
        inputs_state.keyboard.arrow_right = 1;
    else if(key == GLFW_KEY_RIGHT && action == GLFW_RELEASE) 
        inputs_state.keyboard.arrow_right = 0;

}

static void mouse_button_call(GLFWwindow *window, int button, int action, int modes) {
    if((button == GLFW_MOUSE_BUTTON_LEFT) && (action == GLFW_PRESS)) {
         inputs_state.mouse.left_button = 1;
    } else if((button == GLFW_MOUSE_BUTTON_LEFT) && (action == GLFW_RELEASE)) {
        inputs_state.mouse.left_button = 0;
        inputs_state.mouse.hot_object = nullptr;
    }
}

GLFWwindow* init(game_state *gamestate) {
    int glw = glfwInit();
    // std::cerr << glw << std::endl;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow *window = glfwCreateWindow(SCREEN_SIZE_X, SCREEN_SIZE_Y, "Shape Test", nullptr, nullptr);
    
    glfwSetCursorPosCallback(window, mouse_pos);
    glfwSetMouseButtonCallback(window, mouse_button_call);
    glfwSetKeyCallback(window, keyboard);

    glfwMakeContextCurrent(window);

    glewExperimental = true;
    GLenum err = glewInit();
    glEnable(GL_MULTISAMPLE);
    // std::cerr << glewGetErrorString(err) << std::endl;
    // std::cerr << GLEW_OK << std::endl;

    GLuint vrt = shamkshader(GL_VERTEX_SHADER, "../shaders/vrt.vert");
    GLuint frg = shamkshader(GL_FRAGMENT_SHADER, "../shaders/fgt.frag");

    GLuint prog = shamkprogram(vrt, frg);
    gamestate->current_program = prog;
    gamestate->full_screen = 0;
    glUseProgram(prog);
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glViewport(0, 0, SCREEN_SIZE_X, SCREEN_SIZE_Y);
    glUniformMatrix4fv(2, 1, GL_TRUE, shaortho(SCREEN_SIZE_X/2.0f + 0.5f, -SCREEN_SIZE_X/2.0f,
                                               SCREEN_SIZE_Y/2.0f, -SCREEN_SIZE_Y/2.0f - 0.5f, -1.0f, 1.0f));
    return window;
}

void load_program_attrib_location(game_state *gamestate) {
    gamestate->vpos_attrib_loc = glGetAttribLocation(gamestate->current_program, "vpos");

    gamestate->color_attrib_loc = glGetUniformLocation(gamestate->current_program, "input_color");
    gamestate->model_t_attrib_loc = glGetUniformLocation(gamestate->current_program, "transform");
}

GAME_UPDATE_FUNC(game_update_stub) {

}

GAME_RENDER_FUNC(game_render_stub) {

}


GAME_INIT_FUNC(game_init_stub) {

}

FILETIME get_last_write_time(char *filename) {
    WIN32_FILE_ATTRIBUTE_DATA fileinfo = {};
    GetFileAttributesEx(filename, GetFileExInfoStandard, (void *) &fileinfo);
    return fileinfo.ftLastWriteTime; 
}

void load_game_dll(game_state *gstate) {
    char *source_dll_name = "game_logic.dll";
    char *temp_dll_name = "game_logic_temp.dll";
    CopyFile(source_dll_name, temp_dll_name, FALSE); 
    gstate->lib = LoadLibrary(temp_dll_name);
    gstate->last_write_time = get_last_write_time(source_dll_name);
    if(gstate->lib) {
        gstate->update = (game_update_func *) GetProcAddress(gstate->lib, "update");
        gstate->render = (game_render_func *) GetProcAddress(gstate->lib, "render");
        gstate->init = (game_init_func *) GetProcAddress(gstate->lib, "init");
        gstate->is_init = 0;
    } else {
        gstate->update = game_update_stub;
        gstate->render = game_render_stub;
        gstate->init = game_init_stub;
    }
}

void unload_game_dll(game_state *gstate) {
    FreeLibrary(gstate->lib);
    gstate->update = game_update_stub;
    gstate->render = game_render_stub;
    gstate->init = game_init_stub;
}

int main(int argc, char ** argv) {

    game_state gamestate = {};
    GLFWwindow *window = init(&gamestate);
    load_program_attrib_location(&gamestate);
    bool run = true;


    int max_frames_to_simulate = 2;
    
    gamestate.update = game_update_stub;
    gamestate.render = game_render_stub;
    gamestate.init = game_init_stub;

    gamestate.window_width_height = vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y);

    load_game_dll(&gamestate);
 
    // glfwSetTime(0);
    double total_time = 0;
    double current_time = glfwGetTime();
    double previous_time = current_time;
    double frame_time = 0;

    // glfwSwapInterval(0);
    sh_circle testing(0, 0, 10, vec2(0, 1), vec4(1, 0, 0, 1));
    double update_start = 0;
    double update_end = 0;
    FILE *file = nullptr;

    fopen_s(&file, "log_time.txt", "w");
    char outputs[] = "ut: %.50lf\n";
    // char buffer[128];
    size_t si = sizeof(outputs)/sizeof(outputs[0]) + 50;

    while(run) {
        current_time = glfwGetTime();
        frame_time = (current_time - previous_time);

        
        //Note(sharo): I don't know what this is. maybe when the difference is
        //too much don't go past 0.25?
        // if(frame_time > 0.25)
        //     frame_time = 0.25;

        total_time += frame_time;
        previous_time = current_time;

        FILETIME last_write_dll = get_last_write_time("game_logic.dll");
        if(CompareFileTime(&gamestate.last_write_time, &last_write_dll) != 0) {
            unload_game_dll(&gamestate); 
            load_game_dll(&gamestate);
        }

        if(!gamestate.is_init) {
            gamestate.init(&gamestate);
            gamestate.is_init = 1;
        }



        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 


        if((glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)) {
                run  = false;
                glfwSetWindowShouldClose(window, true);
        }

        if((glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)) {
            toggle_fullscreen(&gamestate, window);
        }

        // update_start = glfwGetTime();
        while(( total_time >= dt )) {
            gamestate.update(&gamestate, &inputs_state, dt);
            total_time -= dt;       
        }
        // update_end = glfwGetTime();

        // sprintf(buffer, outputs,(update_end-update_start)*1000.0);
        // // printf("%.50f\n", (update_end-update_start));
        // fwrite(buffer, si, 1, file);

        const double time_left_alpha = total_time/dt;
        
        gamestate.render(&gamestate, time_left_alpha);

        
        glfwSwapBuffers(window);
        glfwPollEvents(); 
    }
    fclose(file);


    glfwDestroyWindow(window);
    return 0;
}

