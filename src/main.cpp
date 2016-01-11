#define GLEW_STATIC
#define PONG_DEBUG 0
#define PONG_EXTRA 0
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
    // std::cerr << glw << std::endl;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);



    GLFWwindow *window = glfwCreateWindow(SCREEN_SIZE_X, SCREEN_SIZE_Y, "Shape Test", nullptr, nullptr);

    glfwMakeContextCurrent(window);

    glewExperimental = true;
    GLenum err = glewInit();
    // std::cerr << glewGetErrorString(err) << std::endl;
    // std::cerr << GLEW_OK << std::endl;

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

vec2 collision_inverse(char colision_result) {
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




static bool collide_with_rectangle(vec2 point, vec4 rectangle) { 
   

    if((point.x >= rectangle.x) && //left
       (point.x  <= rectangle.z) && //right
       (point.y <= rectangle.y) && // up
       (point.y >= rectangle.w)) //down
    {
        return true;
    }

    return false;
}

void static collision_ball_paddle(sh_circle &ball, sh_rect &paddle) {
   //Note(sharo): divide the paddle into 3 sections, left, middle, right
    // vec4 ball_left = vec4(paddle._left + ball._r, paddle._top-ball._r, paddle._left + paddle._width/2.0f - ball._r, paddle._bottom + ball._r);
    float width_divid = paddle._width/3.0f;
    vec4 ball_left = vec4(paddle._left - ball._r , paddle._top + ball._r, paddle._left + width_divid + ball._r, paddle._bottom - ball._r);
    
    // if(((ball._position.x - ball._r) >= paddle._left) && ((ball._position.x + ball._r) <= paddle._right)) {
#if 1 
        if(collide_with_rectangle(ball._position, ball_left))
        {
            ball.set_color(vec4(1, 0, 0, 1));
            ball._direction = normalize((vec2(cos(torad*135), sin(torad*135))));
        }

        // std::cerr << "Left: " << ball_left << std::endl;

        ball_left.x = ball_left.z;
        ball_left.z = ball_left.x + width_divid;
        
        if(collide_with_rectangle(ball._position, ball_left))
        {
            ball.set_color(vec4(0, 0, 1, 1));
            ball._direction = normalize((vec2(cos(torad*ball._position.x), sin(torad*ball._position.y))));
        }
        // std::cerr << "Center: " << ball_left << std::endl;


        ball_left.x = ball_left.z;
        ball_left.z = ball_left.x + width_divid;


        if(collide_with_rectangle(ball._position, ball_left))
        {
            ball.set_color(vec4(0, 1, 0, 1));
            ball._direction = normalize((vec2(cos(torad*45), sin(torad*45))));
        }
        

#endif
        ball_left.x = ball_left.z;
        ball_left.z = ball_left.x + width_divid;
        
    // }
}


bool collision_rect(sh_circle &circle, sh_rect &rect) {
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
    return (v2*alpha)  + (v1*(1.0 - alpha));
}

char collision_inner_rect(sh_circle &circle, vec4 &rectangle) {
    // 0000 4, bits, left, top, right, bottom, colid => set bit to 1;
    char colid = 0;
    

    if(circle._position.x <= (rectangle.z + circle._r)) { //left
        // circle.set_position(rectanlge.x, circle.get_position().y);
        colid |= (1 << 3);
    }

    if(circle._position.x >= (rectangle.x - circle._r)) { //right
        // circle.set_position(rectanlge.z, circle.get_position().y);
        // circle.set_size(10);
        colid |= (1 << 1);
    }

    if(circle._position.y >= (rectangle.y - circle._r)){ //top
        // circle.set_position(circle.get_position().x, rectanlge.y );
        colid |= (1 << 2);
    }

    if(circle._position.y <= (rectangle.w + circle._r)) { //bottom
        // circle.set_position(circle.get_position().x, rectanlge.w );
        colid |= 1;
    }


    return colid;
}

static void stall_program(real32 target_frame_time, real32 time_elapsed_for_frame, LARGE_INTEGER last_counter) {
    if(time_elapsed_for_frame < target_frame_time) {


        DWORD sleep_ms =  (DWORD)((target_frame_time - time_elapsed_for_frame)*1000);

        std::cout << "TE: " << time_elapsed_for_frame*1000 << "\t";
        std::cout << "TF: " << target_frame_time*1000 << "\t";
        std::cout << "TS: " << sleep_ms << "\t";

        Sleep(1);

        real32 after_sleep_time_pass = win32_get_seconds_elapsed(last_counter, win32_get_wall_clock());

        std::cout << "ASTP: " << after_sleep_time_pass*1000 << std::endl;
        // int64 loop_count = 0;
        // while(time_elapsed_for_frame < target_frame_time) {
        //     loop_count++;
        //     time_elapsed_for_frame = win32_get_seconds_elapsed(last_counter, win32_get_wall_clock());
        // }
    }
}

int main(int argc, char ** argv) {
    timeBeginPeriod(1);
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);

    gl_perf_count_frequency = freq.QuadPart;

    std::cout << "pref " << freq.QuadPart << "   " << gl_perf_count_frequency << std::endl;



#if 1
    GLFWwindow *window = init();
    glfwSwapInterval(0);
    bool run = true;
    
    vec4 walls(SCREEN_SIZE_X/2.0f, SCREEN_SIZE_Y/2.0f, -SCREEN_SIZE_X/2.0f, -SCREEN_SIZE_Y/2.0f);
    sh_circle ball(0, 0, 5, vec2(0 ,  -1), vec4(0.5, 0.2, 0.1, 1));
    sh_rect window_rect;

    sh_rect paddle(0, -SCREEN_SIZE_Y/2.0 + 10, 10,  100, vec4(0.1, 0.2, 0.5, 1));
    
    window_rect.set_size(SCREEN_SIZE_X, SCREEN_SIZE_Y);

    ball.set_size(5);
    ball.set_color(vec4(1, 0, 0, 1));
    ball.set_position(0, 0);
    //
    
    

    float ball_velocity = 400;
    float paddle_velocity = 350;
    int frames_per_second = 60;
    int size_change_velocity = 15;
    int size_after_collision = 15;
    double physics_dt_ms = 1000.0f/60.0f;

    float target_frame_time = 1.0f/(float)frames_per_second;
    double total_time = 0;


    vec2 previous_position = ball.get_position();
    vec2 current_positon = ball.get_position();
    double alpha = 0;
    int frame_done = 2;

    LARGE_INTEGER last_counter = win32_get_wall_clock();

    double previous_frame_time = glfwGetTime()*1000;
    double current_frame_time = glfwGetTime()*1000;
    double frame_time = current_frame_time - previous_frame_time;

    while(run) {

        glfwPollEvents();
#if 1
        total_time += (frame_time > physics_dt_ms) ? physics_dt_ms : frame_time;
#endif

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        if((glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)) {
                run  = false;
                glfwSetWindowShouldClose(window, true);
        }

#if 1
        while((total_time >= physics_dt_ms)) {
            ball.move_position(ball_velocity*physics_dt_ms/1000.0f);
            if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
                if(!((paddle.get_position().x + paddle.get_size().x/2.0f) >= SCREEN_SIZE_X/2.0)) {
                    paddle.move_position(paddle_velocity*physics_dt_ms/1000.0f, 0);
                }
            }

            if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
                if(!((paddle.get_position().x - paddle.get_size().x/2.0f) <= -SCREEN_SIZE_X/2.0)) {
                    paddle.move_position(-paddle_velocity*physics_dt_ms/1000.0f, 0);
                }
            }

            previous_position = current_positon;
            
            current_positon = ball.get_position();


            vec2 collision_result = collision_inverse(collision_inner_rect(ball, walls));
            ball._direction = collision_result*ball._direction;
            
            if(ball.get_size() > 5) {
                // std::cerr << "pos b: " << ball._position;
                ball.change_size_by(size_change_velocity*physics_dt_ms/1000.0f);
                // std::cout << "pos a: " << ball._position << std::endl;

            }

            collision_ball_paddle(ball, paddle);

            total_time -= physics_dt_ms;
        }



#endif

#if PONG_EXTRA
        for(int i = tail_size-1; i > -1; --i) {
            tail[i].set_position(tail[i-1].get_position().x , tail[i-1].get_position().y);
            tail[i].set_size(tail[i-1].get_size().x, tail[i-1].get_size().x);
        }

        tail[0].set_position(previous_position);
        tail[0].set_size(ball._r, ball._r);

        for(int i = 0; i < tail_size; ++i) {
            tail[i].render();
        }
#endif

#if 0
        //Todo(sharo): understand interplation and fix this
        alpha = total_time / physics_dt_ms;
        // std::cerr << "A: " << alpha << '\t';
        // std::cerr << "PP: " << previous_position << '\t';
        // std::cerr << "BI: " << current_positon << '\t';
        // std::cerr << "TT: " << total_time << '\t';
        vec2 in_between_position = interpolate_vec2(previous_position, current_positon, alpha);
        ball.set_position(in_between_position);
        // std::cerr << "AI: " << ball.get_position() << std::endl;
        // std::cerr << in_between_position << std::endl;
#endif //interpolation stuff


        ball.render();
        paddle.render();

        glfwSwapBuffers(window);

        previous_frame_time = current_frame_time;
        current_frame_time = glfwGetTime()*1000;

        frame_time = current_frame_time - previous_frame_time;
    }


    timeEndPeriod(1);

#endif

    glfwDestroyWindow(window);
    // glfwSetWindowShouldClose(window, true);

    system("pause");
    return 0;
}

