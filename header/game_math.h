#ifndef GAME_MATH_H
#define GAME_MATH_H

float clamp(float value, float min, float max);
vec2 closest_point_in_rect(sh_circle *circ, sh_rect *rect);
vec2 reflect(vec2 &incoming_ray, vec2 &normal);

vec2 get_normal_voroni_region(const vec2 *point, const rect_object *rect);

void col_circ(ball_object *ball, game_object *object, game_state *state);
void col_rect(rect_object *rect, game_object *object, game_state *state);
void col_line(line_object *line, game_object *object, game_state *state);

int point_in_rect(vec2 *point, sh_rect_container *rect);
int point_in_circ(vec2 *point, sh_circ_container *circ);
int point_in_circ(ball_object *ball, vec2 *p);

double closest_point_seg_seg(vec2 p1, vec2 q1, vec2 p2, vec2 q2);

//collisions to implement
//@Todo(sharo): these just do checks, possibly return more info from them
int col_rect_rect(grid_element *elem, rect_object *rect);
int col_rect_rect(rect_object *recta, rect_object *rectb); //@Implement

int col_rect_line_LB(rect_object *rect, line_object *line); //@Implement
int col_rect_line_LB(grid_element *elem, line_object *line);
int col_rect_line(grid_element *elem, line_object *line);

int col_line_line(sh_line *line1, sh_line *line2); //@Implement
int col_line_line(vec2 a, vec2 b, vec2 c, vec2 d, vec2 *result = nullptr);
int col_line_capsuleSAT(ball_object *ball, line_object *line, vec2 *result);
int col_dot_line_norm(ball_object *ball, line_object *line, vec2 *result);

int col_circ_circ(ball_object *ball1, ball_object *ball2, vec2 *result = nullptr);
int col_circ_line(ball_object *ball, line_object *line, vec2 *result = nullptr);
int col_circ_rect(ball_object *ball_obj, rect_object *rect_obj, vec2 *result);
int col_circ_rect(grid_element *elem, ball_object *ball_obj);

#endif
