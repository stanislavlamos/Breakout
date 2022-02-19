#include <stdbool.h>
#include "drawing_utils.h"
#include "collision_utils.h"

bool is_box_collision_y(int ball_x, int ball_y, box** boxes) {
    int x = (ball_x) / (BOX_WIDTH + BOX_OFFSET_X);
    for (int i = 0; i < BOXES_IN_COL; i++) {
        if (boxes[i * BOXES_IN_ROW + x]->lives == 0) {
            continue;
        }

        if (ball_y + BALL_SIZE == boxes[i * BOXES_IN_ROW + x]->y) { // bottom collision
            printf("Bottom collision: Box x: %d y: %d\n", x, i);
            boxes[x + i*BOXES_IN_ROW]->lives--;
           
            return true;
        }

        if (ball_y == boxes[i * BOXES_IN_ROW + x]->y + BOX_HEIGHT) { // top collision
            printf("Top collision: Box x: %d y: %d\n", x, i);
            boxes[x + i*BOXES_IN_ROW]->lives--;

            return true;
        }
    }

    return false;
}

bool is_box_collision_x(int ball_x, int ball_y, box** boxes, int velocity) {
    

    int dir_x = velocity > 0 ? 1 : -1;
    int x = (((ball_x) / (BOX_WIDTH + BOX_OFFSET_X)) + dir_x);
    if (x > 7 || x < 0) { // check index out of bounds
        return false;
    }
    for (int i = 0; i < 6; i++) {
        if (boxes[i * BOXES_IN_ROW + x]->lives == 0) {
            continue;
        }


        if (ball_x <= boxes[i * BOXES_IN_ROW + x]->x + BOX_WIDTH && ball_x >= boxes[i * BOXES_IN_ROW + x]->x 
            && ball_y >= boxes[i * BOXES_IN_ROW + x]->y && ball_y  <= boxes[i * BOXES_IN_ROW + x]->y + BOX_HEIGHT) { // left collision
            printf("X collision: Box x: %d y: %d\n", x, i);
            boxes[x + i*BOXES_IN_ROW]->lives--;
          
            return true;
        }

        if (ball_x + BALL_SIZE >= boxes[i * BOXES_IN_ROW + x]->x && ball_x <= boxes[i * BOXES_IN_ROW + x]->x 
            && ball_y >= boxes[i * BOXES_IN_ROW + x]->y && ball_y <= boxes[i * BOXES_IN_ROW + x]->y + BOX_HEIGHT) { // right collision
            printf("X collision: Box x: %d y: %d\n", x, i);
            boxes[x + i*BOXES_IN_ROW]->lives--;

            return true;
        }
    }

    return false;
}

bool is_wall_collision(int ball_x, int ball_y){
    bool ret_val = false;
    if (ball_x <= 0 || ball_x >= LCD_PANEL_WIDTH - BALL_SIZE || ball_y == 0 || ball_y == 320 - BALL_SIZE){
        ret_val = true;
    }
    return ret_val;
}

int get_bounce_angle(int ball_x, int paddle_x){
    int ball_x_centre = ball_x + (BALL_SIZE / 2);
    if ((ball_x_centre >= paddle_x && ball_x_centre < paddle_x + STEEP_ANGLE_LIMIT_LEFT) 
        || (ball_x_centre >= paddle_x + STEEP_ANGLE_LIMIT_RIGHT && ball_x_centre < paddle_x + PANEL_WIDTH)){
        return 2;
    }

    else if ((ball_x_centre >= paddle_x + STEEP_ANGLE_LIMIT_LEFT-1 && ball_x_centre < paddle_x + MID_ANGLE_LIMIT_LEFT) 
        || (ball_x_centre >= paddle_x + MID_ANGLE_LIMIT_RIGHT && ball_x_centre < paddle_x + STEEP_ANGLE_LIMIT_RIGHT + 1)){
        return 1;
    }
    return 0;
}
