#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "font_types.h"
#include "mzapo_regs.h"


#define BOX_WIDTH 50
#define BOX_HEIGHT 9
#define BOX_OFFSET_X 9
#define BOX_OFFSET_Y 6
#define BOX_CEILING_OFFSET 40
#define PANEL_WIDTH 60
#define PANEL_HEIGHT 10
#define PANEL_GROUND_OFFSET 20
#define VELOCITY_ABS 15
#define LCD_PANEL_WIDTH 480
#define LCD_PANEL_HEIGHT 320
#define INIT_LIVES 3
#define INIT_PADDLE_X 480/2 - PANEL_WIDTH/2;
#define BALL_SIZE 10

#define BLUE_BUTTON 1
#define GREEN_BUTTON 2
#define RED_BUTTON 4

#define RED 0xF800 
#define YELLOW 0xFFE0 
#define GREEN 0x07E0
#define WHITE 0xFFFF
#define BLUE 0x0000FF

#define BOXES_IN_COL 6
#define BOXES_IN_ROW 8

#define CHAR_PIXEL_SIZE 14


typedef struct paddle {
    int x;
    int y;
} paddle;


typedef struct box {
    int x;
    int y;
    int lives;
} box;

unsigned short *fb;
font_descriptor_t *fdes;

void draw_panel(int x, int y, int width, int height, unsigned short color);
 
void draw_pixel(int x, int y, unsigned short color) ;

void draw_boxes(box **boxes, unsigned char *parlcd_mem_base);
  
void draw_ball(int x, int y);

void draw_text(int x, int y, char *text, unsigned short color, int size);

void draw_char(int x, int y, char ch, unsigned short color, int size);

void update_score(int score);

void update_lives(int lives);

void display_best_score(int new_best);

unsigned getbits(unsigned value, unsigned offset, unsigned n);

box** initialize_boxes(box** boxes);

void free_boxes(box **boxes);

void reset_peripherals(unsigned char * parlcd_mem_base, unsigned char* spiled_membase);

int display_start_menu(unsigned char* parlcd_mem_base, unsigned char* spiled_membase);

volatile unsigned int get_knobs_value(unsigned char* spiled_membase);

bool display_end_screen(unsigned char* parlcd_mem_base, unsigned char* spiled_membase, char* result);

void refresh_lcd(unsigned char* parlcd_mem_base);

void empty_frame_buffer();