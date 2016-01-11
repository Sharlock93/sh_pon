//TODO:
/*
- mapping to viewport? internal or Uniform?...maybe both?
- colors ?  per-vertex, or per-draw call? uniforms?
- color: (0 - 255) or (0 - 1)?

- size: handle negative numbers.

--
- a rectangle using two triangles, 6 vertexes.
- "_position" variable is the center of the rectangle
- "_center" variable is the center of rotation of the rectangle
- "_color" is a 4 float struct ranging from [0-1], with format RGBA
   the same way its laid-out in the shader.
*/

#define GLEW_STATIC
#include "..\header\sh_shapes.h"
#include "..\..\GL\glew.h" 
#include "..\..\Shar\headers\shar.h"
#include "..\..\Shar\headers\sharfun.h"
#include <iostream>

sh_rect::sh_rect(float x, float y, float rect_height, float rect_width, vec4 color) {


    _position = vec2(x, y);

    _height = rect_height;
    _width = rect_width;
    _color = color;
    _center = _position;


    data[0] = vec2(x - _width/2.0, y + _height/2.0);
    data[1] = vec2(x + _width/2.0, y + _height/2.0);
    data[2] = vec2(x + _width/2.0, y - _height/2.0);
    
    data[3] = vec2(x + _width/2.0, y - _height/2.0);
    data[4] = vec2(x - _width/2.0, y - _height/2.0);
    data[5] = vec2(x - _width/2.0, y + _height/2.0);

    _left = (_position.x - _width/2.0f);
    _right = (_position.x + _width/2.0f);
    _top = (_position.y + _height/2.0f);
    _bottom = (_position.y - _height/2.0f);

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // dirty = false;
}

sh_rect::sh_rect() {
    _position = vec2(0, 0);
    _center = _position;
    _height = 5;
    _width = 5;
    _color = vec4(1, 1, 1, 1);

    _left = (_position.x - _width/2.0f);
    _right = (_position.x + _width/2.0f);
    _top = (_position.y + _height/2.0f);
    _bottom = (_position.y - _height/2.0f);
    // _rotation = mat2x2(1);
    
    data[0] = vec2(- _width/2.0, + _height/2.0);
    data[1] = vec2(+ _width/2.0, + _height/2.0);
    data[2] = vec2(+ _width/2.0, - _height/2.0);
    data[3] = vec2(+ _width/2.0, - _height/2.0);
    data[4] = vec2(- _width/2.0, - _height/2.0);
    data[5] = vec2(- _width/2.0, + _height/2.0);

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // dirty = false;
}

sh_rect::~sh_rect() {
    // std::cout << "bye" << std::endl;
}

void sh_rect::render() {

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glUniform4fv(1, 1, (float*)&_color);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void sh_rect::set_position(float x, float y) {
    vec2 diff = vec2(-_position.x + x, -_position.y + y);
    _position = vec2(x, y);
    
    _left = (_position.x - _width/2.0f);
    _right = (_position.x + _width/2.0f);
    _top = (_position.y + _height/2.0f);
    _bottom = (_position.y - _height/2.0f);

    for(int i = 0; i < 6; i++) {
        data[i] = data[i] + diff;
    }

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(data), data);

}

void sh_rect::set_position(vec2 addvec) {
    vec2 diff = vec2(-_position.x + addvec.x, -_position.y + addvec.y);
    _position = addvec;
    for(int i = 0; i < 6; i++) {
        data[i] = data[i] + diff;
    }
    
    _left = (_position.x - _width/2.0f);
    _right = (_position.x + _width/2.0f);
    _top = (_position.y + _height/2.0f);
    _bottom = (_position.y - _height/2.0f);
    
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(data), data);
}

void sh_rect::move_position(float x, float y) {
    _position = _position + vec2(x, y);
    _center = _center + vec2(x, y);
    
    _left = (_position.x - _width/2.0f);
    _right = (_position.x + _width/2.0f);
    _top = (_position.y + _height/2.0f);
    _bottom = (_position.y - _height/2.0f);
    
    for(int i = 0; i < 6; i++) {
        data[i].x = data[i].x + x;
        data[i].y = data[i].y + y;
    }


    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(data), data);
    
}

void sh_rect::move_position(vec2 addvec) {
    _position = _position + addvec;
    _center = _center + addvec;
    
    _left = (_position.x - _width/2.0f);
    _right = (_position.x + _width/2.0f);
    _top = (_position.y + _height/2.0f);
    _bottom = (_position.y - _height/2.0f);

    for(int i = 0; i < 6; i++) {
        data[i] = data[i] + addvec;
    }

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(data), data);
}


vec2 sh_rect::get_position() {
    return _position;
}

vec4 sh_rect::get_color() {
    return _color;
}

void sh_rect::set_color(vec4 color) {
    _color = color;
}

vec2 sh_rect::get_size() {
    return vec2(_width, _height);
}

void sh_rect::set_size(float width, float height) {
    float diffx = width - _width;
    float diffy = height - _height;
    
    _width = width;
    _height = height;

    data[0] = vec2(data[0].x - diffx/2.0, data[0].y + diffy/2.0);
    data[1] = vec2(data[1].x + diffx/2.0, data[1].y + diffy/2.0);
    data[2] = vec2(data[2].x + diffx/2.0, data[2].y - diffy/2.0);
    data[3] = vec2(data[3].x + diffx/2.0, data[3].y - diffy/2.0);
    data[4] = vec2(data[4].x - diffx/2.0, data[4].y - diffy/2.0);
    data[5] = vec2(data[5].x - diffx/2.0, data[5].y + diffy/2.0);
    
    _left = (_position.x - _width/2.0f);
    _right = (_position.x + _width/2.0f);
    _top = (_position.y + _height/2.0f);
    _bottom = (_position.y - _height/2.0f);

    glGenBuffers(1, &_vbo);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void sh_rect::set_size(vec2 size) {
    _width = size.x;
    _height = size.y;
    // dirty = true;
}

void sh_rect::rotate(float angle) {
    angle *= torad;
    mat2x2 rot = mat2x2( cos(angle), -sin(angle),
                         sin(angle), cos(angle) );
    
    vec2 cur_cen = _center;

    move_position(-_center.x, -_center.y);
    
   
    _position = _position*rot;
    for(int i = 0; i < 6; i++) {
       data[i] = data[i]*rot;
    }

    move_position(cur_cen);

    _left = (_position.x - _width/2.0f);
    _right = (_position.x + _width/2.0f);
    _top = (_position.y + _height/2.0f);
    _bottom = (_position.y - _height/2.0f);

    // _rotation = _rotation*rot;

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(data), data);
}

vec2 sh_rect::get_center() {
    return _center;
}

void sh_rect::set_center(float x, float y) {
    _center.x = x;
    _center.y = y;
}

void sh_rect::set_center(vec2 center) {
    _center = center;
}
