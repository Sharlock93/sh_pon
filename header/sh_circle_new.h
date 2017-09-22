#ifndef SEGMENTS
#define SEGMENTS 32
#endif

#include "sh_math.h"

//Todo(sharo): shape has VBO or only data?
//check the size: should only be about 128?
struct sh_circ {
	float vel; //4
	float r;  //4
	sh_vec2 pos; //8
	sh_vec2 direction; //8
	mat4 transformation;
	sh_vec2 _data_points[SEGMENTS];
}; 
