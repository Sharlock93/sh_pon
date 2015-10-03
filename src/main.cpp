#define GLEW_STATIC
#include <iostream>
#include <Shar\headers\sharinit.h>
#include <Shar\headers\sharfun.h>
#include <GLFW\glfw3.h>
#include <windows.h>
#include "..\header\sh_shapes.h"
#include "..\header\sh_circle.h"
#include <random>
#include <cmath>
#include <bitset>
#include <stdio.h>
#include <stdint.h>

//Note(sharo): typedefs
typedef float real32;
typedef int64_t int64;

//Note(Sharo): Globals
#define SCREEN_SIZE_X 500
#define SCREEN_SIZE_Y 500
static int64 gl_perf_count_frequency;


GLFWwindow* init() {
    int glw = glfwInit();
    std::cout << glw << std::endl;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);



    GLFWwindow *window = glfwCreateWindow(SCREEN_SIZE_X, SCREEN_SIZE_Y, "Shape Test", nullptr, nullptr);

    glfwMakeContextCurrent(window);

    glewExperimental = true;
    GLenum err = glewInit();
    std::cout << glewGetErrorString(err) << std::endl;
    std::cout << GLEW_OK << std::endl; 

    GLuint vrt = shamkshader(GL_VERTEX_SHADER, "vrt.vert");
    GLuint frg = shamkshader(GL_FRAGMENT_SHADER, "fgt.frag");

    GLuint prog = shamkprogram(vrt, frg);
    glUseProgram(prog);
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glUniformMatrix4fv(2, 1, GL_TRUE, shaortho(SCREEN_SIZE_X/2.0f + 0.5f, -SCREEN_SIZE_X/2.0f,
                                               SCREEN_SIZE_Y/2.0f, -SCREEN_SIZE_Y/2.0f - 0.5f, -1.0f, 1.0f));
    return window;
}

static inline LARGE_INTEGER win32_get_wall_clock() {
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);
    return result;
} 

static inline real32 win32_get_seconds_elapsed(LARGE_INTEGER start, LARGE_INTEGER end) {
    real32 result = ((real32)(end.QuadPart - start.QuadPart) / (real32)gl_perf_count_frequency);
    return result;
}

vec2 colision_inverse(char colision_result) {
    vec2 ret(1, 1);
    //left (8), right(2), top(4), bottom(1),
    //top-left(12), top-right(6), bottom-left(9), bottom-right(3) 
    switch(colision_result) {
        case 8: case 2: {
            ret.x = -ret.x;             
        } break;

        case 1: case 4: {
            ret.y = -ret.y;             
        } break;

        case 12: case 9: case 6: case 3: {
            ret.y = -ret.y;
            ret.x = -ret.x;
        } break;
    } 

    return ret;
}

bool colision_rect(sh_circle &circle, sh_rect &rect) {
    vec2 rect_size = rect.get_size();
    vec2 pos = rect.get_position();
    vec4 rectanlge(pos.x-rect_size.x/2.0 - circle.get_size(), pos.y + rect_size.y/2.0 + circle.get_size(),
                   pos.x+rect_size.x/2.0 + circle.get_size(), pos.y - rect_size.y/2.0 - circle.get_size());

    if(circle.get_position().x >= rectanlge.x &&
       circle.get_position().x  <= rectanlge.z &&
       circle.get_position().y <= rectanlge.y &&
       circle.get_position().y >= rectanlge.w) {
        return true;
    } 

    return false;
}

inline static vec2 interpolate_vec2(vec2 v1, vec2 v2, float alpha) {
    return (v2*alpha  + v1*(1.0 - alpha));
}

char collision_inner_rect(sh_circle &circle, sh_rect &rect) {
    // 0000 4, bits, left, top, right, bottom, colid => set bit to 1;
    char colid = 0;
    vec2 rect_size = rect.get_size();
    vec2 pos = rect.get_position();

    vec4 rectanlge(pos.x - rect_size.x/2.0 + circle.get_size(), pos.y + rect_size.y/2.0 - circle.get_size(),
                   pos.x + rect_size.x/2.0 - circle.get_size(), pos.y - rect_size.y/2.0 + circle.get_size());

    if(circle.get_position().x <= rectanlge.x) { //left
        circle.set_position(rectanlge.x, circle.get_position().y);
        colid |= (1 << 3);
    }

    if(circle.get_position().x >= rectanlge.z) { //right
        circle.set_position(rectanlge.z, circle.get_position().y);
        colid |= (1 << 1);
    }

    if(circle.get_position().y >= rectanlge.y){ //top
        circle.set_position(circle.get_position().x, rectanlge.y );
        colid |= (1 << 2);
    }

    if(circle.get_position().y <= rectanlge.w) { //bottom 
        circle.set_position(circle.get_position().x, rectanlge.w );
        colid |= 1;
    }


    return colid;
}

static void stall_program(real32 target_frame_time, real32 time_elapsed_for_frame, LARGE_INTEGER last_counter) {


    if(time_elapsed_for_frame < target_frame_time) {
        DWORD sleep_ms =  (DWORD)((target_frame_time - time_elapsed_for_frame)*1000);

        Sleep(sleep_ms);
        int64 loop_count = 0; 
        while(time_elapsed_for_frame < target_frame_time) { 
            loop_count++; 
            std::cout << "Inside before loop TF: " << target_frame_time << " TE: " << time_elapsed_for_frame*1000;
            std::cout << " sleep for: " << sleep_ms << " Sleep Sec: ";
            std::cout << (DWORD)((target_frame_time - time_elapsed_for_frame)*1000);
            std::cout << " loop_count: " << loop_count << std::endl;
            time_elapsed_for_frame = win32_get_seconds_elapsed(last_counter, win32_get_wall_clock());  
            std::cout << "inside after before loop TF: " << target_frame_time << " TE: " << time_elapsed_for_frame*1000;
            std::cout << " sleep for: " << sleep_ms << " Sleep Sec: ";
            std::cout << (DWORD)((target_frame_time - time_elapsed_for_frame)*1000);
            std::cout << " loop_count: " << loop_count << std::endl;
        }
    }
}

int main(int argc, char ** argv) {
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);

    gl_perf_count_frequency = freq.QuadPart;

    std::cout << "pref " << freq.QuadPart << "   " << gl_perf_count_frequency << std::endl;

#if 1
    GLFWwindow *window = init();
    glfwSwapInterval(0);
    
    TIMECAPS time_res;
    
    timeGetDevCaps(&time_res, sizeof(time_res)); 
    timeBeginPeriod(time_res.wPeriodMin);

    std::cout << "hello " << time_res.wPeriodMax<< std::endl;
    bool run = true;
    
    sh_circle ball;
    sh_rect window_rect;
    sh_rect flash;

    flash.set_size(5, 5);
    flash.set_color(vec4(0, 1, 0, 1));
    flash.set_position(0, 0);
    

    window_rect.set_size(500, 500);

    ball.set_size(50); 
    ball.set_color(vec4(1, 0, 0, 1));
    ball.set_position(3, 6);

    double current_time = glfwGetTime();

    float ball_velocity = 50;
    int frames_per_second = 60;
    int size_change_velocity = 50;
    float physics_dt = 1.0f/60.0f; 
    
    float target_frame_time = 1.0f/(float)frames_per_second;  
    double total_time = 0;


    vec2 previous_position = ball.get_position();
    vec2 current_positon = ball.get_position();
    double alpha = 0;
    int frame_done = 2;

    
    LARGE_INTEGER last_counter = win32_get_wall_clock(); 

    while(run) {
        glfwPollEvents();
        
        double new_time = glfwGetTime();
        double frame_time = new_time - current_time;

       
        total_time += (frame_time > physics_dt) ? physics_dt : frame_time;
        current_time = new_time;

       
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if((glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)) {
                run  = false;
        } 
        
        
        flash.render();

        while((total_time >= physics_dt)) {


            previous_position = current_positon;
            ball.move_position(ball_velocity*physics_dt, 0);
            current_positon = ball.get_position(); 

            vec2 collision_result = colision_inverse(collision_inner_rect(ball, window_rect)); 
            ball_velocity = ball_velocity*collision_result.x;



            total_time -= physics_dt;
            frame_done--;
        }

        alpha = total_time / physics_dt;

        frame_done = 2;

        vec2 in_between_position = interpolate_vec2(previous_position, current_positon, alpha); 
        ball.set_position(in_between_position);
        // std::cout << in_between_position << std::endl;
        

        ball.render();
        
       

        glfwSwapBuffers(window);
        LARGE_INTEGER work_time = win32_get_wall_clock();
        real32 work_time_for_frame = win32_get_seconds_elapsed(last_counter, work_time);
        
        stall_program(target_frame_time, work_time_for_frame, last_counter);
        
        last_counter = win32_get_wall_clock(); 
    }


    timeEndPeriod(time_res.wPeriodMin);
#endif
    
    
    glfwSetWindowShouldClose(window, true);

    system("pause");
    return 0;
}

