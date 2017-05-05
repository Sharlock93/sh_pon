#include <Windows.h>
#include <iostream>
#include <sharinit.h>
#include <sharfunc.h>
#include <GLFW\glfw3.h>
#include <sh_circle.h>
#include "game_logic.cpp"
#include <sh_rect.h>
#include <sh_line.h>
#include <stdio.h>


//Note(Sharo): Globals
#define SCREEN_SIZE_X 500 
#define SCREEN_SIZE_Y 500 

extern debug_record records[];
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

GLFWwindow* init(game_state *gamestate, int screen_width, int screen_height,
                 char *title, int pos_x, int pos_y)
{
    int glw = glfwInit();
    /* std::cerr << glw << std::endl; */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow *window = glfwCreateWindow(screen_width, screen_height, title, nullptr, nullptr);
    glfwSetWindowPos(window, pos_x, pos_y);
    
    glfwSetCursorPosCallback(window, mouse_pos);
    glfwSetMouseButtonCallback(window, mouse_button_call);
    glfwSetKeyCallback(window, keyboard);

    glfwMakeContextCurrent(window);

    glewExperimental = true;
    GLenum err = glewInit();
    glEnable(GL_MULTISAMPLE);

    GLuint vrt = shamkshader(GL_VERTEX_SHADER, "../shaders/vrt.vert");
    GLuint frg = shamkshader(GL_FRAGMENT_SHADER, "../shaders/fgt.frag");

    GLuint prog = shamkprogram(vrt, frg);
    gamestate->current_program = prog;
    gamestate->full_screen = 0;
    gamestate->window = window;

    glUseProgram(prog);
    GLuint vao;
    glGenVertexArrays(1, &vao);
    gamestate->current_vao = vao;
    glBindVertexArray(vao);

    glViewport(0, 0, screen_width, screen_height);
    glUniformMatrix4fv(2, 1, GL_TRUE, shaortho(screen_width/2.0f + 0.5f, -screen_width/2.0f,
                                               screen_height/2.0f, -screen_height/2.0f - 0.5f, -1.0f, 1.0f));
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

GAME_DEBUG_FUNC(game_debug_stub) {

}

FILETIME get_last_write_time(char *filename) {
    WIN32_FILE_ATTRIBUTE_DATA fileinfo = {};
    GetFileAttributesEx(filename, GetFileExInfoStandard, (void *) &fileinfo);
    return fileinfo.ftLastWriteTime; 
}

//Note(sharo): this counts the null terminator
int sh_strlen(char *str) {
    char *first_letter = str;
    while(*str++ != '\0');
    return (int)( str - first_letter);
}

//Note(sharo): concatinates two file names, while preserving str one extenstion
char* concat_preserve_extension(char *str1, char *str2) {
    int str1_len = sh_strlen(str1);
    int str2_len = sh_strlen(str2);
    
    //Note(sharo): count both nulls, sub one 
    char *output = (char *) calloc(1, sizeof(char)*(str1_len + str2_len -1));

    char *changer_head = output;
    char *extension = nullptr; 

    for(int i = 0; i < str1_len; ++i) {
        *(changer_head++) = *(str1 + i);
        if(*(str1 + i + 1) == '.' ) {
            extension = str1+i+1;
            break;
        } 
    }

    for(int i = 0; i < str2_len; ++i) {
        *(changer_head++) = *(str2 + i);
        if(*(str2 + i + 1) == '.' ) {
            break;
        } 
    }

    for(int i = 0; i < sh_strlen(extension); ++i) {
        *(changer_head++) = *(extension + i);
        if(*(extension+i) == '\0') break;
    }

    return output;
}

void load_game_dll(game_state *gstate, char *source_name, char *temp_name) {
    char *temp_dll_name = concat_preserve_extension(source_name, temp_name);
    CopyFile(source_name, temp_dll_name, FALSE); 
    gstate->lib = LoadLibrary(temp_dll_name);
    gstate->last_write_time = get_last_write_time(source_name);

    free(temp_dll_name);

    if(gstate->lib) {
        gstate->update = (game_update_func *) GetProcAddress(gstate->lib, "update");
        gstate->render = (game_render_func *) GetProcAddress(gstate->lib, "render");
        gstate->init = (game_init_func *) GetProcAddress(gstate->lib, "init");
        gstate->debug_func = (game_debug_func *) GetProcAddress(gstate->lib, "debug_func");
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
    gstate->debug_func = game_debug_stub;
}

void render_screen(game_state *gs, double alpha) {
    //Todo(sharo): what exactly needs to be rebinded for proper context switch
    glfwMakeContextCurrent(gs->window);
    glUseProgram(gs->current_program);
    gs->render(gs, alpha);
    glfwSwapBuffers(gs->window);
}



int main(int argc, char ** argv) {


    game_state gamestate = {};
    // game_state gamestate2 = {};

    gamestate.inputs = &inputs_state;
    GLFWwindow *window   = init(&gamestate, SCREEN_SIZE_X, SCREEN_SIZE_Y, "Test", 100, 100);
    load_program_attrib_location(&gamestate);

    

    char *source_name = "game_logic.dll";
    char *temp_name =  "_temp.dll";
    char *source_name2 = "game_debug_screen.dll";
    load_game_dll(&gamestate,  source_name, temp_name);
    // load_game_dll(&gamestate2, source_name2, temp_name);
    // gamestate2.debug_state = &gamestate;

    bool run = true;
    glfwSetTime(0);
    glfwSwapInterval(0);
    glfwMakeContextCurrent(gamestate.window);
    glUseProgram(gamestate.current_program);

    double current_time = glfwGetTime();
    double previous_time = current_time;
    double frame_time = 0;
    double total_time = 0;

    while(run) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
        previous_time = current_time; 
        current_time = glfwGetTime();

        frame_time = current_time - previous_time;
        total_time += frame_time;
        
        //Note(sharo): I don't know what this is. maybe when the difference is
        //too much don't go past 0.25?
        // if(frame_time > 0.25)
        //     frame_time = 0.25;

        FILETIME last_write_dll = get_last_write_time("game_logic.dll");
        if(CompareFileTime(&gamestate.last_write_time, &last_write_dll) != 0) {
            unload_game_dll(&gamestate); 
            load_game_dll(&gamestate, source_name, temp_name);
        }

        if(!gamestate.is_init) {
            gamestate.init(&gamestate);
            gamestate.is_init = 1;
        }

        //Todo(sharo): this is only window1 must be looped
        if((glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)) {
                run  = false;
                glfwSetWindowShouldClose(window, true);
        }

        //Todo(sharo): this needs to be added to a loop for all the screens
        while(( total_time >= dt ) ) {
            double passed_in = min(frame_time, dt);
            gamestate.update(&gamestate, &inputs_state, frame_time, false);

            total_time -= dt;
        }

        double time_left_alpha = total_time/dt;
        render_screen(&gamestate, time_left_alpha);
        glfwPollEvents(); 
    }
    
    glfwDestroyWindow(window);
    return 0;
}

