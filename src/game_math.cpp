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
            if(col_line_capsuleSAT(ball, line, &result)) {
                ball->circ->set_position(result);
                vec2 line_vec = normalize(line->line->point[1] - line->line->point[0]);
                vec2 line_normal = vec2(-line_vec.y, line_vec.x);
                ball->circ->_direction = reflect(ball->circ->_direction, line_normal);
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
            if(col_line_capsuleSAT(ball, line, &result)) {
                ball->circ->set_position(result);
                vec2 line_vec = normalize(line->line->point[1] - line->line->point[0]);
                vec2 line_normal = vec2(-line_vec.y, line_vec.x);
                ball->circ->_direction = reflect(ball->circ->_direction, line_normal);
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


int col_line_capsuleSAT(ball_object *ball, line_object *line, vec2 *result) {
    TIME_BLOCK;

    vec2 *ball_dir = &ball->circ->_direction;
    int col = col_line_line(line->line->point[0], line->line->point[1], ball->previous_pos - *ball_dir*ball->circ->_r , ball->circ->_position + *ball_dir*ball->circ->_r , result);
    
    if(col) {
        *result = *result - *ball_dir*ball->circ->_r*1.2;
    }

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

