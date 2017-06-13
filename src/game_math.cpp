#include "../header/game_math.h"

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

float line_to_pnt_thresh = 0.0001; //@Note(sharo): when should a line be considered a point

double closest_point_seg_seg(vec2 p1, vec2 q1, vec2 p2, vec2 q2, vec2 *res) {
    vec2 d1 = q1 - p1;
    vec2 d2 = q2 - p2;

    float sq_len_d1 = dot(d1, d1);
    float sq_len_d2 = dot(d2, d2);

    if(sq_len_d1 < line_to_pnt_thresh && sq_len_d2 < line_to_pnt_thresh) {
        vec2 p = p2 - p1;
        *res = p;
        return dot(p, p);
    }

    if(sq_len_d1 < line_to_pnt_thresh) {
        float t = dot(d2, p1 - p2)/sq_len_d2;
        clamp(t, 0.0, 1.0);
        vec2 p = p1 - p2 - t*d2;
        *res = p;
        return dot(p, p);
    }

    if(sq_len_d2 < line_to_pnt_thresh) {
        float s = dot(d1, p2 - p1)/sq_len_d1;
        clamp(s, 0.0, 1.0);
        vec2 p = p2 - p1 - s*d1;
        *res = p;
        return dot(p, p);
    }

    float a = dot(d1, d2);
    float mat_determ = a*a - sq_len_d2*sq_len_d1;

    //@Note(sharo): floating point is iffy, so check if its not zero is better
    //than checking for zero
    float s = 0.0;
    vec2 d = p1 - p2;
    float f = dot(d2, d);
    float e = dot(d1, d);
    if(mat_determ != 0.0) {
        s =  -a*f + e*sq_len_d2;
        s = s/mat_determ;
        s = clamp(s, 0.0, 1.0);
    }

    float t = ( a*s + f )/sq_len_d2;

    if(t < 0.0) {
        t = 0;
        s = (-e)/sq_len_d1;
    } else if(t > 1.0) {
        t = 1.0;
        s = ( a - e )/sq_len_d1;
    }

    s = clamp(s, 0.0, 1.0);

    vec2 r1 = p1 + s*d1;
    vec2 r2 = p2 + t*d2;

    vec2 p = r1 - r2;
    *res = p;
    float result = dot(p, p);
    
#if 0
    if(gl_game_state) {
        push_draw_text(gl_game_state, sh_flttstr(result), 20, vec2(100, 120), vec4(1, 1, 1, 1));
        push_draw_text(gl_game_state, sh_vec2tstr(&p), 20, vec2(-100, 100), vec4(1, 1, 1, 1));
        // push_draw_element(&gl_game_state->renderstack, sh_gen_draw_circ(ball->circ->_position, ball->circ->_r, vec4(1, 1, 0, 1)));
        push_draw_element(&gl_game_state->renderstack, sh_gen_draw_line(r1, r2, vec4(1, 1, 0, 1)));
    }
#endif

    return result;
}

float book_closest_seg_seg(vec2 p1, vec2 q1, vec2 p2, vec2 q2) {
    vec2 d1 = q1 - p1;
    vec2 d2 = q2 - p2;
    vec2 r = p1 - p2;
    float a = dot(d1, d1);
    float e = dot(d2, d2);
    float f = dot(d2, r);
    float s;
    float t;
    vec2 c1;
    vec2 c2;
    if (a <= line_to_pnt_thresh && e <= line_to_pnt_thresh) {
        s = t = 0.0f;
        c1 = p1;
        c2 = p2;
        return dot(c1 - c2, c1 - c2);
    }

    if(a <= line_to_pnt_thresh) {
        s = 0.0f;
        t = f/e;
        t = clamp(t, 0.0f, 1.0f);
    } else {
        float c = dot(d1, r);
        if(e <= line_to_pnt_thresh) {
            t = 0.0f;
            s = clamp(-c/a, 0.0f, 1.0f);
        } else {
            float b = dot(d1, d2);
            float denom = a*e - b*b;
            
            if(denom != 0.0) {
                s = clamp((b*f - c*e)/denom, 0.0f, 1.0f);
            } else s = 0.0f;

            t = (b*s + f)/e;

            if(t < 0.0f) {
                t = 0.0;
                s = clamp(-c/a, 0.0f, 1.0);
            } else if(t > 1.0f) {
                t = 1.0f;
                s = clamp((b-c)/a, 0.0f, 1.0f);
            }
        }
    }

    c1 = p1 + d1*s;
    c2 = p2 + d2*t;
    return dot(c1 - c2, c1 - c2);
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



void col_circ(ball_object *ball, game_object *object, game_state *state) {

    switch(object->type) {
        case BALL: {
            ball_object *ball2 = (ball_object *) object->obj;
        } break;

        case LINE: {
            line_object *line = (line_object *) object->obj;
            vec2 result;
            if(col_dot_line_norm(ball, line, &result)) {
                ball->circ->set_position(result);
                ball->circ->_direction = reflect(ball->circ->_direction, line->normal);
            }
        } break;
        
        case RECTANGLE: {
            rect_object *rect = (rect_object * ) object->obj;
            vec2 result;
            
            int col = col_circ_rect(ball, rect, &result);

            if(col) {
                ball->previous_pos = ball->circ->_position;
                result = result + normalize(ball->circ->_position - result)*ball->circ->_r;
                ball->circ->set_position(result);
                vec2 normal = get_normal_voroni_region(&ball->circ->_position, rect);
                ball->circ->_direction = normalize(reflect(ball->circ->_direction, normal));
            }
        } break;
    }
}

void col_rect(rect_object *rect, game_object *object, game_state *state) {
    switch(object->type) {
        case BALL: {
            ball_object *ball = (ball_object * ) object->obj;
            vec2 result;
            int col = col_circ_rect(ball, rect, &result);

            if(col) {
                ball->previous_pos = ball->circ->_position;
                result = result + normalize(ball->circ->_position - result)*ball->circ->_r;
                ball->circ->set_position(result);
                vec2 normal = get_normal_voroni_region(&ball->circ->_position, rect);
                ball->circ->_direction = normalize(reflect(ball->circ->_direction, normal));
            }
        } break;

        case LINE: {

        } break;
        
        case RECTANGLE: {
       
        } break;
    }

}

void col_line(line_object *line, game_object *object, game_state *state) {
    switch(object->type) {
        case BALL: {
            ball_object *ball = (ball_object *) object->obj;
            vec2 result;
            
            if(col_dot_line_norm(ball, line, &result)) {
                ball->circ->set_position(result);
                ball->circ->_direction = reflect(ball->circ->_direction, line->normal);
            }
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

int col_seg_seg(ball_object *ball, line_object *line, vec2 *result) {
    TIME_BLOCK;
    float dist = closest_point_seg_seg(
            line->line->point[0], line->line->point[1],
            ball->previous_pos, ball->circ->_position, result);

    return (dist) <= (ball->circ->_r*ball->circ->_r);
}

int col_dot_line_norm(ball_object *ball, line_object *line, vec2 *result) {
    TIME_BLOCK;
    float d1 = abs(dot(ball->previous_pos - line->line->point[0], line->normal));
    float d2 = abs( dot(ball->circ->_position - line->line->point[1], line->normal) );

    // float r2 = ball->circ->_r;
    // r2 *= r2;

    float t = (ball->circ->_r + 1 - d1)/(d2 - d1);
    vec2 vec_dir = ball->circ->_position - ball->previous_pos;

    int col = (t >= 0.0 && t <= 1.0);

    if(!col) {
        if(d1 < ball->circ->_r) col = 1;

        if(d2 < ball->circ->_r) col = 1;
    }


#if 0
    push_draw_element(&gl_game_state->renderstack, sh_gen_draw_circ(ball->circ->_position, ball->circ->_r, vec4(1, 1, 0, 1)));
    push_draw_element(&gl_game_state->renderstack, sh_gen_draw_circ(ball->previous_pos, ball->circ->_r, vec4(0, 1, 0, 1)));
    push_draw_text(gl_game_state, sh_flttstr(t), 20, vec2(100, 0), vec4(1, 1, 1, 1));
    push_draw_text(gl_game_state, sh_flttstr(d1), 20, vec2(100, -20), vec4(1, 1, 1, 1));
    push_draw_text(gl_game_state, sh_flttstr(d2), 20, vec2(100, -40), vec4(1, 1, 1, 1));
    push_draw_text(gl_game_state, sh_inttstr(col), 20, vec2(100, 25), vec4(1, 1, 1, 1));
    // push_draw_text(gl_game_state, sh_inttstr(col_seg_seg(ball, line, result)), 20, vec2(100, 50), vec4(1, 1, 1, 1));
#endif
    if(col) {
        t -= 0.1;
        
        *result = ball->previous_pos + vec_dir*t;
        // if(d1 <= r2 && d2 <= r2) { //ball inside the line before and after the frame
        //     
        // // push_draw_element(&gl_game_state->renderstack, sh_gen_draw_circ(ball->circ->_position, ball->circ->_r, vec4(1, 1, 0, 1)));
        // }
    }

    return col;

}


int col_line_capsuleSAT(ball_object *ball, line_object *line, vec2 *result) {
    int col = col_seg_seg(ball, line, result);
    col = col_dot_line_norm(ball, line, result);
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
    // if(col) {
    //     if(result != nullptr) {
    //
    //         result->x = -linevec.y*abs(length_to_ball - ball->circ->_r);
    //         result->y =  linevec.x*abs(length_to_ball - ball->circ->_r);
    //     }
    // }

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
    vec2 line = circ->_position - rect_center;
    vec2 final = vec2(0, 0);

    final.x = clamp(line.x, -elem->width/2.0,  elem->width/2.0);
    final.y = clamp(line.y, -elem->height/2.0, elem->height/2.0);
    final =  final + rect_center;
    final = final - circ->_position;

    return (final.x*final.x + final.y*final.y) < (circ->_r*circ->_r);
}

