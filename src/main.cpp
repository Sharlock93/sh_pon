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

#define SCREEN_SIZE_X 500
#define SCREEN_SIZE_Y 500


bool colision_rect(sh_circle &circle, sh_rect &rect);
char colision_rect_inner(sh_circle &circle, sh_rect &rect);
vec2 colision_inverse(char colision_result);

struct game_state {
    bool running;
    GLFWwindow *window;
    void game_loop();
    game_state(GLFWwindow *wi);
};


struct timer {
    double dt;
    double prev_time;
    timer();
    void update_dt();
};

timer::timer() {
    dt = 0;
    prev_time = glfwGetTime();
}

void timer::update_dt() {
    dt = glfwGetTime() - this->prev_time;
    this->prev_time = glfwGetTime();
}

game_state::game_state(GLFWwindow *win) {
    running = false;
    this->window = win;
}

GLFWwindow* init();

std::random_device rd;
std::mt19937 mt(rd());
std::uniform_real_distribution<float> dist(-10, 10);

vec2 buffer[SCREEN_SIZE_X*SCREEN_SIZE_Y];

int main(int argc, char ** argv) {
    game_state game(init());   
    game.running = true;
        
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(buffer), buffer, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    
    for(int i = 0; i <= SCREEN_SIZE_X; i++) {
        for(int j = 0; j <= SCREEN_SIZE_Y; j++) {
            // std::cout << i*SCREEN_SIZE_X  + j<< std::endl;
            buffer[i*SCREEN_SIZE_X + j] = vec2(i - SCREEN_SIZE_X/2.0, j-SCREEN_SIZE_Y/2.0);
        } 
    }    
    glBufferData(GL_ARRAY_BUFFER, sizeof(buffer), buffer, GL_DYNAMIC_DRAW);
    /*
    {
    
        sh_circle test(0, 0, 5, vec4(1, 0, 0, 1), vec2(0, 0));
        sh_rect paddle(0, -200, 10, 70);  
        sh_rect win(0, 0, 500, 500); 
        vec2 ball_vel(290, -280); 
        vec2 after_tast(10, 0);
        vec2 colision_change(1, 1);
        float paddle_vel = 300;
        
        vec2 amama(10, 0);
        
        vec2 prev_pos = paddle.get_position(); 
        float meow = 0;
     
        timer la_time;

    }
    */
    
    while(game.running) {
        
            
            // la_time.update_dt();
    
            if(glfwGetKey(game.window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                game.running = false; 
            }
                
        
            
            /* { 
            if(glfwGetKey(game.window, GLFW_KEY_A) == GLFW_PRESS) { 
                paddle.move_position(-paddle_vel*la_time.dt, 0);
            }
    
            if(glfwGetKey(game.window, GLFW_KEY_D) == GLFW_PRESS) { 
                paddle.move_position(paddle_vel*la_time.dt, 0);
            }
    
            glClear(GL_COLOR_BUFFER_BIT);  
            colision_change = colision_inverse(colision_rect_inner(test, win));
            if(colision_change.y < 0) {
                ball_vel.x += dist(mt);
            } else if(colision_change.x < 0) {
                std::cout << "kladsjfjasdf" << std::endl;
                ball_vel.y += dist(mt);
            }
    
            ball_vel = vec2(ball_vel.x*colision_change.x, ball_vel.y*colision_change.y);
            test.move_position(ball_vel.x*la_time.dt , ball_vel.y*la_time.dt); 
            // std::cout << colision_change << std::endl;    
            if(colision_rect(test, paddle))  { 
                meow = (paddle.get_position() - prev_pos).x;
                if(meow)
                    ball_vel.x = -ball_vel.x*(meow/abs(meow)); 
                ball_vel.y = -ball_vel.y;
            }
            
            if((paddle.get_position().x-paddle.get_size().x/2.0) < -SCREEN_SIZE_X/2)
                paddle.set_position(-250+paddle.get_size().x/2.0, paddle.get_position().y);
            
            if((paddle.get_position().x+paddle.get_size().x/2.0) > SCREEN_SIZE_X/2) 
                paddle.set_position(250-paddle.get_size().x/2.0, paddle.get_position().y);
    
            paddle.render(); 
    
    
            // win.render();     
            test.render();
            prev_pos = paddle.get_position(); 
            // glDrawArrays(GL_POINTS, 0, 1);
        } */

        glDrawArrays(GL_POINTS, 0, SCREEN_SIZE_X*SCREEN_SIZE_Y);
        glfwSwapBuffers(game.window);
        glfwPollEvents();
    }

    return 0;
}

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

    glUniformMatrix4fv(2, 1, GL_TRUE, shaortho(SCREEN_SIZE_X/2, -SCREEN_SIZE_X/2,
                                               SCREEN_SIZE_Y/2, -SCREEN_SIZE_Y/2, -1, 1));
    return window;
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

char colision_rect_inner(sh_circle &circle, sh_rect &rect) {
    // 0000 4, bits, left, top, right, bottom, colid => set bit to 1;
    char colid = 0;
    vec2 rect_size = rect.get_size();
    vec2 pos = rect.get_position();

    vec4 rectanlge(pos.x - rect_size.x/2.0 + circle.get_size(), pos.y + rect_size.y/2.0 - circle.get_size(),
                   pos.x + rect_size.x/2.0 - circle.get_size(), pos.y - rect_size.y/2.0 + circle.get_size());

    if(circle.get_position().x <= rectanlge.x) { //left
        colid |= (1 << 3);
    }

    if(circle.get_position().x >= rectanlge.z) //right
        colid |= (1 << 1);

    if(circle.get_position().y >= rectanlge.y) //top
        colid |= (1 << 2);

    if(circle.get_position().y <= rectanlge.w) //bottom
        colid |= 1;


    return colid;
}

