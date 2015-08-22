#define GLEW_STATIC
// 1 rad  = 180.0/M_PI degree
// 1 degree = M_PI/180.0 rads

#include "..\headers\sh_circle.h"
#include <iostream>
#include <cmath>


sh_circle::sh_circle() {
    _position = vec2(0, 0);
    _r  = 5;
    _color = vec4(1, 1, 1, 1);
    _center = vec2(0, 0);

    float step = 360.0/CIRCLE_SEGMENTS;

    for(int i = 0; i < CIRCLE_SEGMENTS; i++) {
        _data[i] = vec2(_r*cos(i*step*torad), _r*sin(i*step*torad));
    }

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(_data), _data, GL_DYNAMIC_DRAW);
}


sh_circle::sh_circle(float x, float y, float r, vec4 color, vec2 center) {
    //make a circle from cos and sin, they both take radians as argument
    _position = vec2(x, y);
    _r  = r;
    _color = color;
    _center = center;

    float step = 360.0/CIRCLE_SEGMENTS; //CIRCLE_SEGMENTS segments to make a circle

    for(int i = 0; i < CIRCLE_SEGMENTS; i++) {
        _data[i] = vec2(r*cos(i*step*torad) + x, r*sin(i*step*torad) + y);
    }

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(_data), _data, GL_DYNAMIC_DRAW);

}

void sh_circle::render() {
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glUniform4fv(1, 1, (float *)&_color);
    glDrawArrays(GL_TRIANGLE_FAN, 0, CIRCLE_SEGMENTS);
}


void sh_circle::set_position(float x, float y) {
    vec2 diff = vec2(x - _position.x, y - _position.y);
    _position = vec2(x, y);

    for(int i = 0; i < CIRCLE_SEGMENTS; i++) {
        _data[i] = _data[i] + diff;
    }

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(_data), _data, GL_DYNAMIC_DRAW);
}

void sh_circle::set_position(vec2 pos) {
    vec2 diff = pos - _position;
    _position = pos;

    for(int i = 0; i < CIRCLE_SEGMENTS; i++) {
        _data[i] = _data[i] + diff;
    }

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(_data), _data, GL_DYNAMIC_DRAW);
}

vec2 sh_circle::get_position() {
    return _position;
}

void sh_circle::move_position(float x, float y) {
    vec2 diff(x, y);
    _position = _position + diff;
    _center = _center + diff;
    for(int i = 0; i < CIRCLE_SEGMENTS; i++) {
        _data[i] = _data[i] + diff;
    }

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(_data), _data, GL_DYNAMIC_DRAW);

}

void sh_circle::move_position(vec2 vel) {
    _position = _position + vel;
    _center = _center + vel;

    for(int i = 0; i < CIRCLE_SEGMENTS; i++) {
        _data[i] = _data[i] + vel;
    }

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(_data), _data, GL_DYNAMIC_DRAW);

}

void sh_circle::set_size(float r) {
    // negative radius handle
    if(r < 0)
        r *= -1;

    float ratio = r/_r;

    _r = r;

    for(int i = 0; i < CIRCLE_SEGMENTS; i++) {
        _data[i] = _data[i]*ratio;
    }

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(_data), _data, GL_DYNAMIC_DRAW);
}


float sh_circle::get_size() {
    return _r;
}


void sh_circle::set_color(vec4 color) {
    _color = color;
}

vec4 sh_circle::get_color() {
    return _color;
}

void sh_circle::rotate(float angle) {
    angle *= torad;
    mat2x2 rot(cos(angle), -sin(angle),
               sin(angle), cos(angle));

    vec2 cur_center = _center;
    move_position(-_center);
    
    _position = _position * rot;

    for(int i = 0; i < CIRCLE_SEGMENTS; i++) {
        _data[i] = _data[i]*rot;
    }

    move_position(cur_center);
    

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(_data), _data, GL_DYNAMIC_DRAW);

}

void sh_circle::set_center(float x, float y) {
    _center = vec2(x, y);
}

void sh_circle::set_center(vec2 center) {
    _center = center;
}

sh_circle::~sh_circle() {
    // std::cout << "good bye world" << std::endl;
}
