#include "drawing_utils.h"
#include "mzapo_parlcd.h"


void draw_panel(int x, int y, int width, int height, unsigned short color) {
    for (int j=0; j<height; j++) { // panel dimensions
        for (int i=0; i<width; i++) {
            draw_pixel(i+x,j+y,color);
        }
    }
}

void draw_pixel(int x, int y, unsigned short color) {
    if (y>=0 && y<LCD_PANEL_HEIGHT && x>=0 && x<LCD_PANEL_WIDTH) {
        fb[x+LCD_PANEL_WIDTH*(LCD_PANEL_HEIGHT - y)] = color;
    }
}

void draw_boxes(box **boxes, unsigned char *parlcd_mem_base){
    
    for (int y = 0; y < BOXES_IN_COL; y++){
        for (int x = 0; x < BOXES_IN_ROW; x++){
            box* current_box = boxes[x + y*BOXES_IN_ROW]; 
            
            if (current_box->lives == 0){
                int ptr=current_box->y*LCD_PANEL_WIDTH + current_box->x;
                for (int i = 0; i < BOX_OFFSET_X ; i++) {
                    ptr = (i + current_box->y)*LCD_PANEL_WIDTH + current_box->x;
                    for (int j = 0; j < BOX_WIDTH; j++) {
                        fb[ptr]=0;
                    }
                }
                continue;
            }

            unsigned short color = RED;
            if (current_box->lives == 1){
                color = GREEN;
            }

            else if (current_box->lives == 2){
                color = YELLOW;
            }
        
            draw_panel(current_box->x, current_box->y, BOX_WIDTH, BOX_HEIGHT, color);
        }
    }
}


void draw_ball(int x, int y) {
    int ball_row_lengths[BALL_SIZE] = {4, 8, 8, 10, 10, 10, 10, 8, 8, 4};
    for (int i = 0; i < BALL_SIZE; i++) {
        for (int j = 0; j <BALL_SIZE; j++) {
            if(j >= (BALL_SIZE - ball_row_lengths[i])/2 && j < BALL_SIZE - (BALL_SIZE - ball_row_lengths[i])/2) {
                draw_pixel(x + j, y + i, BLUE);
            }
        }
    }
}

int char_width(int ch) {
    int width;
    if (!fdes->width) {
        width = fdes->maxwidth;
    } else {
        width = fdes->width[ch-fdes->firstchar];
    }
    return width;
}

void draw_pixel_big(int x, int y, unsigned short color, int size) {
    int i,j;
    for (i=0; i<size; i++) {
        for (j=0; j<size; j++) {
            draw_pixel(x+i, y+j, color);
        }
    }
}

void draw_char(int x, int y, char ch, unsigned short color, int size) {
    int w = char_width(ch);
    const font_bits_t *ptr;
    
    if ((ch >= fdes->firstchar) && (ch-fdes->firstchar < fdes->size)) {
        if (fdes->offset) {
            ptr = &fdes->bits[fdes->offset[ch-fdes->firstchar]];
        } else {
            int bw = (fdes->maxwidth+15)/16;
            ptr = &fdes->bits[(ch-fdes->firstchar)*bw*fdes->height];
        }
    
        int i, j;
        for (i=0; i<fdes->height; i++) {
            font_bits_t val = *ptr;
            for (j=0; j<w; j++) {
                if ((val&0x8000)!=0) {
                    draw_pixel_big(x+size*j, y+size*(fdes->height - 1- i), color, size);
                }
                val<<=1;
            }
            ptr++;
        }
    }
}


void draw_text(int x, int y, char *text, unsigned short color, int size){

    int counter = 0;
    char cur_char = text[0];
 
    while (cur_char != '\0'){
        draw_char(x + ((size * CHAR_PIXEL_SIZE)*counter), y, cur_char, color, size);
        counter++;
        cur_char = text[counter];
    }
}

void update_score(int score) {
    char str[5];
    sprintf(str, "%d", score);
    char score_text[20] = "SCORE: \0";
    strcat(score_text, str);
    draw_text(320, 290, score_text, WHITE, 1);
}

void update_lives(int lives) {
    char str[2];
    sprintf(str, "%d", lives);
    char lives_text[20] = "LIVES: \0";
    strcat(lives_text, str);
    draw_text(30, 290, lives_text, WHITE, 1);
}

void display_best_score(int new_best){
    char str[5];
    sprintf(str, "%d", new_best);
    char score_text[20] = "BEST SCORE: \0";
    strcat(score_text, str);
    draw_text(60, 80, score_text, WHITE, 1);
}

unsigned getbits(unsigned value, unsigned offset, unsigned n){
    const unsigned max_n = CHAR_BIT * sizeof(unsigned);
    if (offset >= max_n){
        return 0;
    }
    
    value >>= offset; 
    if (n >= max_n){
        return value;
    }

    const unsigned mask = (1u << n) - 1; 
    return value & mask;
}


box** initialize_boxes(box** boxes) {
    int box_lives = 3;
    for (size_t i = 0; i < BOXES_IN_ROW * BOXES_IN_COL; i++){
        if (boxes[i] == NULL) {
            boxes[i] = malloc(sizeof(box*));
        }
    }
    
    for (int y = 0; y < BOXES_IN_COL; y++){
        box_lives = y != 0 && y % 2 == 0 ? box_lives - 1 : box_lives;
        for (int x = 0; x < BOXES_IN_ROW; x++){  
            boxes[y* BOXES_IN_ROW + x]->x = BOX_OFFSET_X + (x * (BOX_WIDTH + BOX_OFFSET_X));
            boxes[y* BOXES_IN_ROW + x]->y = LCD_PANEL_HEIGHT - BOX_CEILING_OFFSET - (BOX_OFFSET_Y + ((BOX_OFFSET_Y + BOX_HEIGHT)*y));
            boxes[y* BOXES_IN_ROW + x]->lives = box_lives;
        }
    }
    return boxes;
}

void free_boxes(box **boxes){
    for (size_t i = 0; i < BOXES_IN_COL * BOXES_IN_ROW; i++){
        free(boxes[i]);
    }
    free(boxes);
}

void reset_peripherals(unsigned char * parlcd_mem_base, unsigned char* spiled_membase) {
    parlcd_write_cmd(parlcd_mem_base, 0x2c);
    for (int i = 0; i < LCD_PANEL_HEIGHT ; i++) {
        for (int j = 0; j < LCD_PANEL_WIDTH ; j++) {
            fb[i * LCD_PANEL_WIDTH + j]=0;
            parlcd_write_data(parlcd_mem_base, fb[i * LCD_PANEL_WIDTH + j]);
        }
    }
    
    *(volatile uint32_t*)(spiled_membase + SPILED_REG_LED_RGB1_o) = 0x000000;
    *(volatile uint32_t*)(spiled_membase + SPILED_REG_LED_RGB2_o) = 0x000000;
    *(volatile uint32_t*)(spiled_membase + SPILED_REG_LED_LINE_o) = 0x000000;
}

void refresh_lcd(unsigned char* parlcd_mem_base) {
    parlcd_write_cmd(parlcd_mem_base, 0x2c); // display changes
    for (int i = 0; i < LCD_PANEL_WIDTH*LCD_PANEL_HEIGHT ; i++) {
        parlcd_write_data(parlcd_mem_base, fb[i]);
    }
}

volatile unsigned int get_knobs_value(unsigned char* spiled_membase) {
    return *(volatile uint32_t*)(spiled_membase + SPILED_REG_KNOBS_8BIT_o);
}

int wait_for_button_input(unsigned char* spiled_membase) {
    while (true){
        unsigned int rgb_knobs_value = get_knobs_value(spiled_membase);
        unsigned int buttons_val = getbits(rgb_knobs_value, 24, 8);
      
        if (buttons_val != 0) {
            return buttons_val;
        }
    }
}

int display_start_menu(unsigned char* parlcd_mem_base, unsigned char* spiled_membase) {
    draw_text(90, 150, "BREAKOUT\0", WHITE, 3);
    draw_text(90, 120, "PRESS ANY KNOB TO START\0", 0xFFFF, 1);
    
    draw_text(400, 20, "[EASY]\0", WHITE, 1);
    draw_text(260, 20, "[MEDIUM]\0", WHITE, 1);
    draw_text(150, 20, "[HARD]\0", WHITE, 1);
    refresh_lcd(parlcd_mem_base);

    int button_input = wait_for_button_input(spiled_membase);

    return button_input;
}

bool display_end_screen(unsigned char* parlcd_mem_base, unsigned char* spiled_membase, char* result){
    draw_text(60, 150, result, WHITE, 3);
    draw_text(60, 120, "PRESS GREEN KNOB TO RESTART\0", WHITE, 1);
    draw_text(60, 100, "PRESS RED KNOB TO QUIT\0", WHITE, 1);
    refresh_lcd(parlcd_mem_base);
    
    int button_input = wait_for_button_input(spiled_membase);
    while (button_input != GREEN_BUTTON && button_input != RED_BUTTON) {
        button_input = wait_for_button_input(spiled_membase);
        printf("Button input: %d.\n", button_input);
    }
    
    if (button_input == GREEN_BUTTON) {
        return true;
    }
    return false;
}

void empty_frame_buffer() {
    for (int ptr = 0; ptr < LCD_PANEL_HEIGHT*LCD_PANEL_WIDTH ; ptr++) {
        fb[ptr]=0u;
    }
}
