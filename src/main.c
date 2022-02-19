#define _POSIX_C_SOURCE 200112L
#define SCORE_MAX 960
#define LED_STARTING_POSITION 0xF0000 >> 2 
#define VELOCITY_MEDIUM -10
#define VELOCITY_HARD -15
#define VELOCITY_EASY -7
#define PADDEL_COLOR 0x7ff
#define LED_RED_COLOR 0xF80000 
#define KNOB_TURN_THRESHOLD 127

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <limits.h>   
#include <termios.h>        

#include "mzapo_parlcd.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"
#include "drawing_utils.h"
#include "collision_utils.h"

void run_game_loop(unsigned char *parlcd_mem_base, unsigned char *spiled_membase, box **boxes, int velocity_x, int velocity_y);

bool knob_moved_left(int p_x, uint8_t blue_knob_val, uint8_t prev_blue_knob_val);

bool knob_moved_right(int p_x, uint8_t blue_knob_val, uint8_t prev_blue_knob_val);

int main(int argc, char const *argv[]){
    box **boxes = malloc(sizeof(box*) * BOXES_IN_COL * BOXES_IN_ROW);
    unsigned char *parlcd_mem_base;
    unsigned char *spiled_membase;
    fb  = (unsigned short *)malloc(LCD_PANEL_HEIGHT*LCD_PANEL_WIDTH*2);
    fdes = &font_winFreeSystem14x16;
    parlcd_mem_base = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0);
    spiled_membase = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);
    if (parlcd_mem_base == NULL || spiled_membase == NULL)
        exit(1);

    static struct termios oldt, newt;

    tcgetattr( STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON);
    newt.c_cc[VMIN] = 0;
    newt.c_cc[VTIME] = 0;

    tcsetattr( STDIN_FILENO, TCSANOW, &newt);
    reset_peripherals(parlcd_mem_base, spiled_membase);
    
    //int difficulty = display_start_menu(parlcd_mem_base, spiled_membase);
    int velocity_x;
    int velocity_y;

    //if (difficulty == RED_BUTTON) {
    //    velocity_x = velocity_y = VELOCITY_HARD;
    //} else if (difficulty == GREEN_BUTTON) {
    //    velocity_y = velocity_x = VELOCITY_MEDIUM;
    //} else {
    //    velocity_y = velocity_x = VELOCITY_EASY;
    //}

    velocity_x = velocity_y = VELOCITY_MEDIUM;
    printf("User has chosen to play Breakout with velocity: %d\n", velocity_y);

    run_game_loop(parlcd_mem_base, spiled_membase, boxes, velocity_x, velocity_y);

    reset_peripherals(parlcd_mem_base, spiled_membase);
    
    free_boxes(boxes);
    printf("Program has been terminated\n");
    return EXIT_SUCCESS;
}

void run_game_loop(unsigned char *parlcd_mem_base, unsigned char *spiled_membase, box **boxes, int velocity_x, int velocity_y) {
    struct timespec loop_delay;
    loop_delay.tv_sec = 0;
    loop_delay.tv_nsec = 150 * 1000 * 1000;
    int ball_x = LCD_PANEL_WIDTH / 2 - BALL_SIZE/2;
    int ball_y = LCD_PANEL_HEIGHT / 2 - BALL_SIZE/2;
    int bounce_angle_value = 0; 
    int lives = INIT_LIVES;
    int score = 0;
    int best_score = 0;
    uint32_t led = LED_STARTING_POSITION;
    paddle p;
    p.x = INIT_PADDLE_X;
    p.y = PANEL_GROUND_OFFSET;
    boxes = initialize_boxes(boxes);
    
    uint8_t blue_knob_val = getbits(get_knobs_value(spiled_membase), 0, 8);
    uint8_t prev_blue_knob_val;

    // main loop
    while (lives > 0){
        empty_frame_buffer();


        prev_blue_knob_val = blue_knob_val;
        blue_knob_val = getbits(get_knobs_value(spiled_membase), 0, 8);

        draw_panel (p.x, p.y, PANEL_WIDTH, PANEL_HEIGHT, PADDEL_COLOR); //draw paddel
        draw_boxes (boxes, parlcd_mem_base); //draw colorful boxes

        // ball animation
        int dir_x = velocity_x > 0 ? 1 : -1;
        int dir_y = velocity_y > 0 ? 1 : -1;
        for (int i = 0; i < abs(velocity_y); i++) {
      
            ball_x += dir_x*bounce_angle_value;
            ball_y += dir_y;
      
            if (is_box_collision_x(ball_x, ball_y, boxes, velocity_x)) {
                if (bounce_angle_value != 0) {
                    bounce_angle_value = 1;
                }
                velocity_x *= -1;
                score += 10;
                break;
            } 
          
            else if (is_box_collision_y(ball_x, ball_y, boxes)) {
                if (bounce_angle_value != 0) {
                    bounce_angle_value = 1;
                }
                velocity_y *= -1;
                score += 10;
                break;
            } 
      
            else if (is_wall_collision(ball_x, ball_y)) {// is wall collision
                if (ball_x <= 0) {
                    ball_x = 0;
                } 
                
                else if (ball_x >= LCD_PANEL_WIDTH - BALL_SIZE) {
                    ball_x = LCD_PANEL_WIDTH - BALL_SIZE;
                }

                if (bounce_angle_value != 0) {
                    bounce_angle_value = 1;
                }
            
                break;
            }

      
            else if ((ball_x >= p.x && ball_x < p.x + PANEL_WIDTH && ball_y == p.y + PANEL_HEIGHT)) { // paddle collision
              bounce_angle_value = get_bounce_angle(ball_x, p.x);
              velocity_y *= -1;
              break;
            } else if (ball_y + BALL_SIZE <= PANEL_GROUND_OFFSET + PANEL_HEIGHT) {
                break;
            }
        
        }

        draw_ball(ball_x, ball_y);
    
        if (ball_x == 0 || ball_x >= LCD_PANEL_WIDTH - BALL_SIZE) { // wall collision
            velocity_x *= -1;
        } 
        
        if (ball_y >= LCD_PANEL_HEIGHT - BALL_SIZE) { // ceiling collision
            velocity_y *= -1;
        }

        if (ball_y + BALL_SIZE <= PANEL_GROUND_OFFSET + PANEL_HEIGHT) { // under paddle
            
            if (lives == 1) {
                *(volatile uint32_t*)(spiled_membase + SPILED_REG_LED_RGB1_o) = LED_RED_COLOR;
                *(volatile uint32_t*)(spiled_membase + SPILED_REG_LED_RGB2_o) = LED_RED_COLOR;
                if (score > best_score){
                    best_score = score;
                }
                display_best_score(best_score);
        
                
                if (display_end_screen(parlcd_mem_base, spiled_membase, "GAME OVER\0")) {
                    // restart
                    reset_peripherals(parlcd_mem_base, spiled_membase);
                    lives = 3;
                    score = 0;
                    ball_x = 240 - BALL_SIZE/2;
                    ball_y = 160 - BALL_SIZE/2;
                    p.x = 480/2 - PANEL_WIDTH/2;
                    p.y = PANEL_GROUND_OFFSET;
                    bounce_angle_value = 0;
                    initialize_boxes(boxes);
                    led = LED_STARTING_POSITION;
                    continue;
                } else {
                    //end
                    break;
                }
    
                *(volatile uint32_t*)(spiled_membase + SPILED_REG_LED_RGB1_o) = 0x000000;
                *(volatile uint32_t*)(spiled_membase + SPILED_REG_LED_RGB2_o) = 0x000000;
                break;
            } else { // lives != 1
                lives--;
                ball_x= 240 - BALL_SIZE/2;
                p.x = 240 - PANEL_WIDTH/2;
                ball_y = 160 - BALL_SIZE/2;
                bounce_angle_value = 0;
                led = 0xF0000;
            } 
        }

        //solve uint going over zero
        int knob_turn = abs(prev_blue_knob_val - blue_knob_val);
        if (knob_turn > KNOB_TURN_THRESHOLD) {
            if (blue_knob_val > KNOB_TURN_THRESHOLD) {
                prev_blue_knob_val = 255;
            } else {
                prev_blue_knob_val = 0;
            }
        }
    
        //Moved left
        if (knob_moved_left(p.x, blue_knob_val, prev_blue_knob_val)) {
            p.x +=20;
            led = led >> 1;
        } 
        
        //Moved right
        else if ((knob_moved_right(p.x, blue_knob_val, prev_blue_knob_val))) {
            p.x-=20;
            led = led << 1;
        }

        char ch1=' ';
        int r = read(0, &ch1, 1);
        while (r==1) {
            if (ch1=='d') {
                p.x += 20;
                led = led >> 1;
            } else if (ch1=='a') {
                p.x -= 20;
                led = led << 1;
            }
            r = read(0, &ch1, 1);
        }
        update_score(score);
        update_lives(lives);

        if (score == SCORE_MAX) {
            display_end_screen(parlcd_mem_base, spiled_membase, "YOU WON!\0");
        }


        refresh_lcd(parlcd_mem_base);
        *(volatile uint32_t*)(spiled_membase + SPILED_REG_LED_LINE_o) = led;
        clock_nanosleep(CLOCK_MONOTONIC, 0, &loop_delay, NULL);
    }
}

bool knob_moved_left(int p_x, uint8_t blue_knob_val, uint8_t prev_blue_knob_val) {
    return (prev_blue_knob_val > blue_knob_val && p_x < 480 - PANEL_WIDTH - 10);
}

bool knob_moved_right(int p_x, uint8_t blue_knob_val, uint8_t prev_blue_knob_val) {
    return (prev_blue_knob_val < blue_knob_val && p_x > 10) ||(blue_knob_val > 230 && prev_blue_knob_val < 25);
}
