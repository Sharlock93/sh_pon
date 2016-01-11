// #include "..\..\GL\glew.h"
// #include "..\..\GLFW\glfw3.h"
#include <GL\glew.h>
#include <GLFW\glfw3.h>
// #include <Shar\sharinclude.h>

#include <Shar\headers\shar.h>

#ifndef SH_SHAPES
#define SH_SHAPES

class sh_rect {
public:
    sh_rect();
    sh_rect(float x, float y, float rect_height, float rect_width,
                              vec4 color = vec4(1, 1, 1, 1));
    ~sh_rect();
    
    vec2 get_position();
    void set_position(float x, float y);
    void set_position(vec2 addvec);
    
    void move_position(float x, float y);
    void move_position(vec2 addvec);


    vec4 get_color();
    void set_color(vec4 color);

    vec2 get_size();
    void set_size(float height, float width);
    void set_size(vec2 size);

    void rotate(float angle);

    vec2 get_center();
    void set_center(float x, float y);
    void set_center(vec2 center);
    
    void render();

    GLuint _vbo;
    vec2 _position;
    vec2 _center;
    vec4 _color;

    float _height;
    float _width;
    
    union {
        struct {
            float _left;
            float _right;
            float _top;
            float _bottom;
        };
        
        vec4 walls;
    };
    vec2 data[6];

    //possible other thingy to do.
    // bool dirty;
    // mat2x2 _rotation;
    // GLFWwindow* _window;
};

#endif
