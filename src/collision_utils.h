#include <stdbool.h>

#define BOX_COLLISION_START_Y 196
#define BOX_COLLISION_END_Y 280

#define STEEP_ANGLE_LIMIT_LEFT 12
#define STEEP_ANGLE_LIMIT_RIGHT 47

#define MID_ANGLE_LIMIT_LEFT 26
#define MID_ANGLE_LIMIT_RIGHT 33

bool is_box_collision_y(int ball_x, int ball_y, box** boxes);

bool is_box_collision_x(int ball_x, int ball_y, box** boxes, int velocity);

bool is_wall_collision(int ball_x, int ball_y);

int get_bounce_angle(int ball_x, int paddle_x);