//@Note(sharo): general math
float clamp(float value, float min, float max) {
	if(value < min)
		return min;

	if(value > max)
		return max;

	return value;
}

//@Note(sharo): other math
vec2 reflect(vec2 &incoming_ray, vec2 &normal) {
	return incoming_ray - 2*dot(incoming_ray, normal)*normal;
}

//@Note(sharo): Cohen-Sutherland outcode based normal for a rectangle
//based on the algorithm, compute outcode, switch case it, the simplified version
//becomes this
vec2 get_normal_voroni_region(const vec2 *point, const rect_object *rect) {

	vec2 normal(0, 0);

	if(point->x <= rect->rect->_left) {
		normal.x = -1;
	} else if(point->x >= rect->rect->_right) {
		normal.x = 1;
	}

	if(point->y >= rect->rect->_top) {
		normal.y = 1;
	} else if(point->y <= rect->rect->_bottom) {
		normal.y = -1;
	}

	return (normal);
}

int point_in_rect(vec2 *p, sh_rect_container *r) {
	float hw = r->width/2.0;
	float hh = r->height/2.0;
	if(p->x >= (r->pos.x + hw)) return false;
	if(p->x <= (r->pos.x - hw)) return false;
	if(p->y >= (r->pos.y + hh)) return false;
	if(p->y <= (r->pos.y - hh)) return false;

	return true;
}

int point_in_circ(vec2 *p, sh_circ_container *c) {
	vec2 d2 = c->pos - *p;
	return (d2.x*d2.x + d2.y*d2.y) < c->r*c->r;
}

int point_in_circ(ball_object *ball, vec2 *p) {
	vec2 d2 = ball->circ->_position - *p;
	return (d2.x*d2.x + d2.y*d2.y) < ball->circ->_r*ball->circ->_r;
}

float point_on_line(vec2 p, vec2 a, vec2 b) {
	vec2 d = b - a;
	vec2 pa = p - a;

	float s = dot(d, pa)/dot(d, d);

	return s;
}

vec2 line_seg_voroni_region(vec2 *point, line_object *line) {
	float s = point_on_line(*point, line->line->point[0], line->line->point[1]);
	float t = clamp(s, 0.0, 1.0);

	vec2 end_to_point = ( line->line->point[0] + t*(line->line->point[1] - line->line->point[0]) );
	return end_to_point;
}

float line_to_pnt_thresh = 0.0001; //@Note(sharo): when should a line be considered a point

double closest_point_seg_seg(vec2 p1, vec2 q1, vec2 p2, vec2 q2, vec2 *res, float *s, float *t, vec2 *p_1, vec2 *p_2) {
	vec2 d1 = q1 - p1;
	vec2 d2 = q2 - p2;

	float sq_len_d1 = dot(d1, d1);
	float sq_len_d2 = dot(d2, d2);

	if(sq_len_d1 < line_to_pnt_thresh && sq_len_d2 < line_to_pnt_thresh) {
		vec2 p = p2 - p1;
		*s = *t = 0;
		*p_1 = p1;
		*p_2 = p2;
		*res = p;
		return dot(p, p);
	}

	if(sq_len_d1 < line_to_pnt_thresh) {
		*t = dot(d2, p1 - p2)/sq_len_d2;
		*s = 0;
		*t = clamp(*t, 0.0, 1.0);
		*p_1 = p1;
		*p_2 = p2 + *t*d2;
		vec2 p = *p_1 - *p_2;
		*res = p;
		return dot(p, p);
	}

	if(sq_len_d2 < line_to_pnt_thresh) {
		*s = dot(d1, p2 - p1)/sq_len_d1;
		*s = clamp(*s, 0.0, 1.0);
		*t = 0;
		*p_1 = p1 + *s*d1;
		*p_2 = p2;
		vec2 p = *p_1 - *p_2;
		*res = p;
		return dot(p, p);
	}

	float a = dot(d1, d2);
	float mat_determ = a*a - sq_len_d2*sq_len_d1;

	//@Note(sharo): floating point is iffy, so check if its not zero is better
	//than checking for zero
	*s = 0.0;
	vec2 d = p1 - p2;
	float f = dot(d2, d);
	float e = dot(d1, d);
	if(mat_determ != 0.0) {
		*s =  -a*f + e*sq_len_d2;
		*s = *s/mat_determ;
		*s = clamp(*s, 0.0, 1.0);
	}

	*t = ( a*(*s)+ f )/sq_len_d2;

	if(*t < 0.0) {
		*t = 0;
		*s = (-e)/sq_len_d1;
	} else if(*t > 1.0) {
		*t = 1.0;
		*s = ( a - e )/sq_len_d1;
	}

	*s = clamp(*s, 0.0, 1.0);

	*p_1 = p1 + *s*d1;
	*p_2 = p2 + *t*d2;

	vec2 p = *p_1 - *p_2;
	*res = p;
	float result = dot(p, p);

#if 0
	if(gl_game_state) {
		// push_draw_element(&gl_game_state->renderstack, sh_gen_draw_circ(ball->circ->_position, ball->circ->_r, vec4(1, 1, 0, 1)));
		push_draw_element(&gl_game_state->renderstack, sh_gen_draw_line(*p_1, *p_2, vec4(1, 1, 0, 1)));
	}
#endif

	return result;
}

float book_closest_seg_seg(vec2 p1, vec2 q1, vec2 p2, vec2 q2) {
    return 0;
}



vec2 closes_point_in_rect(sh_circle *circ, sh_rect *rect) {
	vec2 pos = circ->_position;
	float x_left  = pos.x - rect->_left;
	float x_right = rect->_right - pos.x; 
	float y_top    = rect->_top - pos.y;
	float y_bottom = pos.y - rect->_bottom;


	float x_min = (x_left < x_right) ? x_left : x_right;
	float y_min = (y_top < y_bottom) ? y_top : y_bottom;

	if(x_min < y_min) {
		if(x_min == x_left)
			return vec2(rect->_left - circ->_r, pos.y);
		else
			return vec2(rect->_right + circ->_r, pos.y);
	} else {
		if(y_min == y_bottom)
			return vec2(pos.x, rect->_bottom - circ->_r);
		else
			return vec2(pos.x, rect->_top + circ->_r);
	}
}

int col_seg_seg(ball_object *ball, line_object *line, vec2 *result) {
	TIME_BLOCK;
	float s = 0;
	float t = 0;
	float dist = 0;// closest_point_seg_seg(
	// line->line->point[0], line->line->point[1],
	// ball->previous_pos, ball->circ->_position, result, 
	// s, t);

	return (dist) <= (ball->circ->_r*ball->circ->_r);
}

vec2 point_on_line_vec(vec2 p, vec2 a, vec2 b) {
	vec2 d = b - a;
	vec2 pa = p - a;
	float s = dot(d, pa)/dot(d,d);

	s = clamp(s, 0.0, 1.0);

	vec2 on_line = a + s*d;

	return on_line;
}

float sq_dist_point_line(vec2 p, vec2 a, vec2 b) {
	vec2 d = b - a;
	vec2 pa = p - a;
	float s = dot(d, pa)/dot(d,d);

	s = clamp(s, 0.0, 1.0);

	vec2 on_line = a + s*d;

	return dot(on_line - p, on_line - p);
}

int col_dot_line_norm(ball_object *ball, line_object *line, vec2 *result, float *remaining) {
	TIME_BLOCK;
	DUMP_STAMP("col_dot_normal\n");
	DUMP_BALL_OBJECT(ball);
	DUMP_LINE_OBJECT(line);
	vec2 vec_dir = ball->circ->_position - ball->previous_pos;

	vec2 p1(0, 0);
	vec2 p2(0, 0);
	float a = 0;
	float b = 0;

	float dist = closest_point_seg_seg(ball->previous_pos, ball->circ->_position,
			line->line->point[0], line->line->point[1], result, &a, &b, &p1, &p2);

	int col = dist < ball->circ->_r*ball->circ->_r;

	if(col) {
		line->line->_color = vec4(1, 1, 1, 1);
	} else {
		line->line->_color = vec4(1, 0, 0, 1);
	}

	if(col) {

		float t = 0;
		//end points?
		if( b == 0 || b == 1 ) {
			vec2 Q = p2 - ball->previous_pos;
			float _a = dot(vec_dir, vec_dir);
			float _b = -2*(dot(Q, vec_dir));
			float r = ball->circ->_r + 1;
			float _c = dot(Q, Q) - r*r;

			float _under_sqr = _b*_b - 4*_a*_c;
			float _sqr_undr = sqrt(_under_sqr);
			float x = ( -_b - _sqr_undr)/(2*_a);
			float x2 = ( -_b + _sqr_undr)/(2*_a);

			t = min(x, x2);

		} else {
			float d1 = dot(ball->previous_pos - line->line->point[0], line->normal);
			float d2 = dot(ball->circ->_position - line->line->point[0], line->normal);

			if(d1*d2 > 0) {
				if(d1 < 0.0) {
					d1 = dot(ball->previous_pos - line->line->point[0], -line->normal);
				}

				if(d2 < 0.0) {
					d2 = dot(ball->circ->_position - line->line->point[0], -line->normal);
				}
			}

			char buffer[64];
			sprintf_s(buffer, 64, "d1: %f d2:%f\n", d1, d2);
			// write_to_gl_log(buffer);


			// DUMP_VEC2(&ball->circ->_position);
			// DUMP_VEC2(&ball->previous_pos);

			int sign_d1 = (d1 >= 0) ? 1 : -1;
			if((d2 - d1) == 0) {
				t = 0;
			} else {

				t = abs((ball->circ->_r - sign_d1*d1)/(d2 - d1));
			}
			// if(isinf(t)) __debugbreak();
		}

		char buffer[64];
		sprintf_s(buffer, 64, "t remaining: %f\n", t);
		// write_to_gl_log(buffer);


		*result = ball->previous_pos + t*(vec_dir);
		*remaining = 1 - t;
	}

	return col;
}


int col_line_capsuleSAT(ball_object *ball, line_object *line, vec2 *result) {
	int col = col_seg_seg(ball, line, result);
	// col = col_dot_line_norm(ball, line, result);
	return col;
}


//@Note(sharo): circle collisions
//
//

int col_circ_circ(ball_object *ball1, ball_object *ball2, vec2 *result) {
	vec2 p = ball2->circ->_position - ball1->circ->_position;
	float sum_r = ball2->circ->_r + ball1->circ->_r;

	int overlap = (p.x*p.x + p.y*p.y) < sum_r*sum_r;
	if(result != nullptr && overlap == 1) {
		float len  = length(p) - sum_r;
		*result = normalize(p)*len;
	}

	return overlap;
}

int col_circ_line(ball_object *ball, line_object *line, vec2 *result) {
	// TIME_BLOCK;
	vec2 linevec = normalize(line->line->point[1] - line->line->point[0]);
	vec2 ball_line = ball->circ->_position - line->line->point[0];
	vec2 point_to_circ = line->line->point[0] + dot(ball_line, linevec)*linevec - ball->circ->_position;

	float length_to_ball = length(point_to_circ);
	int col = length_to_ball < ball->circ->_r;
	return col;
}

int col_circ_rect(ball_object *ball_obj, rect_object *rect_obj, vec2 *result) {
	sh_rect   *rect   = rect_obj->rect;
	sh_circle *circ   = ball_obj->circ;

	vec2 line = circ->_position - rect->_position;
	vec2 final = vec2(0, 0);

	final.x = clamp(line.x, -rect->_width/2.0,  rect->_width/2.0);
	final.y = clamp(line.y, -rect->_height/2.0, rect->_height/2.0);

	result->x = final.x;
	result->y = final.y;
	*result = *result + rect->_position;

	final = circ->_position - (final + rect->_position);

	return (final.x*final.x + final.y*final.y) < (circ->_r*circ->_r);
}

int col_circ_rect(grid_element *elem, ball_object *ball_obj) {
	sh_circle *circ   = ball_obj->circ;

	vec2 rect_center = vec2(elem->left + elem->width/2.0, elem->top - elem->height/2.0 );
	vec2 line = ball_obj->previous_pos - rect_center;
	vec2 final = vec2(0, 0);

	final.x = clamp(line.x, -elem->width/2.0,  elem->width/2.0);
	final.y = clamp(line.y, -elem->height/2.0, elem->height/2.0);
	final =  final + rect_center;
	final = final - ball_obj->previous_pos;

	return (final.x*final.x + final.y*final.y) < (circ->_r*circ->_r);
}


int sh_sqdist_seg_seg(vec2 p1, vec2 q1, vec2 p2, vec2 q2, float* s, float* t) {

    vec2 d1 = q1 - p1;
    vec2 d2 = q2 - p2;
    vec2 r = p1 - p2;
    float a = dot(d1, d1);
    float e = dot(d2, d2);
    float f = dot(d2, r);
    vec2 c1;
    vec2 c2;
    if (a <= line_to_pnt_thresh && e <= line_to_pnt_thresh) {
	*s = *t = 0.0f;
	c1 = p1;
	c2 = p2;
	return dot(c1 - c2, c1 - c2);
    }

    if(a <= line_to_pnt_thresh) {
	*s = 0.0f;
	*t = f/e;
	*t = clamp(*t, 0.0f, 1.0f);
    } else {
	float c = dot(d1, r);
	if(e <= line_to_pnt_thresh) {
	    *t = 0.0f;
	    *s = clamp(-c/a, 0.0f, 1.0f);
	} else {
	    float b = dot(d1, d2);
	    float denom = a*e - b*b;

	    if(denom != 0.0) {
		*s = clamp((b*f - c*e)/denom, 0.0f, 1.0f);
	    } else *s = 0.0f;

	    *t = (b*(*s) + f)/e;

	    if(*t < 0.0f) {
		*t = 0.0;
		*s = clamp(-c/a, 0.0f, 1.0);
	    } else if(*t > 1.0f) {
		*t = 1.0f;
		*s = clamp((b-c)/a, 0.0f, 1.0f);
	    }
	}
    }


    c1 = p1 + d1*(*s);
    c2 = p2 + d2*(*t);
    
    push_draw_element(&gl_game_state->renderstack, sh_gen_draw_line(p1, q1, vec4(.5, 1, 1, 1)));
    push_draw_element(&gl_game_state->renderstack, sh_gen_draw_line(c1, c2, vec4(0, 1, 1, 1)));

    float sq2 = dot(c1 - c2, c1 - c2);


    return sq2;
}


float sh_circ_line_toi(vec2 p1, vec2 q1, vec2 line_start, vec2 line_norm, float d) {
    vec2 qp1 = (q1 - p1);
    vec2 pp = (p1 - line_start);

    float num = (d - dot(pp, line_norm));
    float denom = (dot(qp1, line_norm));
    float t = 0;

    if(denom == 0) {
	//note(sharo): probably a striaght line
    } else {
	t = num/denom;
    }

    vec2 pos = p1 + t*qp1;
    push_draw_element(&gl_game_state->renderstack, sh_gen_draw_circ(pos, d, vec4(0, 1, 1, 1)));
    t = clamp(t, 0, 1);

    return t;
}



