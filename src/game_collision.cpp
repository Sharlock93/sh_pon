#include "../header/game_math.h"


sh_col_info sh_col_circ_line(ball_object* ball, line_object* line) {

    sh_col_info col_info = {};
    
    vec2 q1 = ball->previous_pos;
    vec2 q2 = ball->circ->_position;

    vec2 p1 = line->line->point[0];
    vec2 p2 = line->line->point[1];

    float sq2 = sh_sqdist_seg_seg(q1, q2, p1, p2, &col_info.dt1, &col_info.dt2);
    float rad = ball->circ->_r*ball->circ->_r;

    col_info.collied = ( rad ) > sq2;
    col_info.type = COL_SWP_CIRC_LINE;
    
    return col_info;
}

void sh_col_res(ball_object* ball, line_object* line) {
    vec2 q1 = ball->previous_pos;
    vec2 q2 = ball->circ->_position;

    vec2 p1 = line->line->point[0];
    vec2 p2 = line->line->point[1];

    float t = sh_circ_line_toi(q1, q2, p1, line->normal, ball->circ->_r);
    vec2 pos = q1 + t*((q2 - q1));
    printf("%f: %f\n", t, 1 - t);


    ball->circ->set_position(pos);

    push_draw_element(&gl_game_state->renderstack, sh_gen_draw_circ(pos, ball->circ->_r, vec4(0, 1, 0, 1)));
}

sh_col_info col_circ(ball_object *ball, game_object *object, game_state *state) {
    switch(object->type) {
	case BALL: {
	    ball_object *ball2 = (ball_object *) object->obj;

	} break;

	case LINE: {
	    line_object* line = (line_object*) object->obj;
	    return sh_col_circ_line(ball, line);;
	} break;

	case RECTANGLE: {
	    rect_object *rect = (rect_object * ) object->obj;

	} break;
    }
}

void col_rect(rect_object *rect, game_object *object, game_state *state) {
    switch(object->type) {
	case BALL: {
	    ball_object *ball = (ball_object * ) object->obj;

	} break;

	case LINE: {

	} break;

	case RECTANGLE: {

	} break;
    }

}

sh_col_info col_line(line_object *line, game_object *object, game_state *state) {

    switch(object->type) {
	case BALL: {
	    ball_object *ball = (ball_object *) object->obj;
	    return sh_col_circ_line(ball, line);;
	} break;
    }
}


//@Note(sharo): collisions of rectangles(AABB)
//
//

int col_rect_rect(grid_element *elem, rect_object *rect) {
    if(elem->left > rect->rect->_right || 
	    elem->right < rect->rect->_left ||
	    elem->top  < rect->rect->_bottom || 
	    elem->bottom > rect->rect->_top) return 0;
    return 1;
}

//@Todo(sharo): implement this roobustly
int col_rect_line_LB(grid_element *elem, line_object *line) {
    vec2 delta = line->line->point[1] - line->line->point[0];
    vec2 a = line->line->point[0];
    vec2 b = line->line->point[1];
    float q[4] = { 
	a.x - elem->left,
	elem->right - a.x,
	a.y - elem->bottom,
	elem->top - a.y
    };

    float p[4] = {
	-delta.x, //left
	delta.x, //right
	-delta.y, //bottom
	delta.y //top
    };

    float umin = 0;
    float umax = 1;

    for(int i = 0; i < 4; ++i) {
	if(p[i] == 0 && q[i] < 0) return 0;

	if(p[i] < 0)  {
	    float u = q[i]/p[i];
	    umin = max(umin, u);
	}

	if(p[i] > 0) {
	    float u = q[i]/p[i];
	    umax = min(umax, u);
	}
    }

    return umin < umax;
}


int col_rect_line(grid_element *elem, line_object *line) {
    for(int i = 0; i < 3; ++i) {
	if(col_line_line(line->line->point[0], line->line->point[1], elem->data[i], elem->data[i+1])) return 1;
    }

    return 0;
}


//@Note(sharo): Line Collisions
//
//
//

int col_line_line(vec2 a, vec2 b, vec2 c, vec2 d, vec2 *result ) {

    double A1 = b.y - a.y;
    double B1 = a.x - b.x;
    double C1 = A1*a.x + B1*a.y;

    double A2 = d.y - c.y;
    double B2 = c.x - d.x;
    double C2 = A2*c.x + B2*c.y;

    double det = A1*B2 - A2*B1;

    if (det == 0) {
	return 0;
    } else {
	double x = (B2*C1 - B1*C2)/det;
	double y = (A1*C2 - A2*C1)/det;

	float xmin = min(a.x, b.x);
	float xmax = max(a.x, b.x);
	float ymin = min(a.y, b.y);
	float ymax = max(a.y, b.y);

	if(x < xmin) return 0;
	if(x > xmax) return 0;
	if(y < ymin) return 0;
	if(y > ymax) return 0;

	xmin = min(c.x, d.x);
	xmax = max(c.x, d.x);
	ymin = min(c.y, d.y);
	ymax = max(c.y, d.y);

	if(x < xmin) return 0;
	if(x > xmax) return 0;
	if(y < ymin) return 0;
	if(y > ymax) return 0;

	result->x = x;
	result->y = y;
	return 1;
    }

}

