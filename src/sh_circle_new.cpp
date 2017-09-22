#include "../header/sh_circle_new.h"

sh_circ* make_circ(vec2 pos, float r, vec2 direction, float vel) {
	sh_circ *circ = (sh_circ *) malloc(sizeof(sh_circ));
	circ->pos = pos;
	circ->vel = vel;
	circ->r   = r;
	circ->direction = direction;

	float angle_step = 360f/SEGMENTS;
	for(int i = 0; i < SEGMENTS; ++i) {
		for(int i = 0; i < CIRCLE_SEGMENTS; i++) {
			circ->_data_points[i] = vec2( (float)(r*cos(i*step*torad)), (float) (r*sin(i*step*torad))); 
		}

	}	
};
