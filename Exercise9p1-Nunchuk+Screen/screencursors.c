
#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_spi.h>
#include <stm32f10x_usart.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "uart.h"
#include "spi.h"
#include "LCD7735R.h"
#include "nunchuk.h"
#include "setup_main.h"
#include "xprintf.h"
#include "i2c.h"
#include "screencursors.h"

/*
Module draws a cursor on the LCD screen based on joystick and accel reads from the nunchuk

 Methodology (Thinking out loud) 
-One could take an approach where this module the is strictly the highest level layer for this project
and have a single init function for both the chuk and the screen. That's probably the most
"principled" approach.
*/

//save last joyx/y and accelx/y reads to clear previous cursor position from screen
//save last char cursor too
static uint8_t prev_joyx = 0;
static uint8_t prev_joyy = 0;
static uint16_t prev_accelx = 0;
static uint16_t prev_accely = 0;
static char prev_char_c = 'C';
static char prev_char_z = 'Z';
int scaled_cursor_data[4];

#define JOY_CURSOR_COLOR        WHITE
#define ACCEL_CURSOR_COLOR      GREEN
#define BGCOLOR                 BLACK

#define SCALE_X_JOY     ((float)255/(float)ST7735_HEIGHT)
#define SCALE_Y_JOY     ((float)255/(float)ST7735_WIDTH)
#define SCALE_X_ACCEL   ((float)1023/(float)ST7735_HEIGHT)
#define SCALE_Y_ACCEL   ((float)1023/(float)ST7735_WIDTH)

static void convert_nunchuk_input_to_screen_position(void)
{
    /* 
    joystick x and y comes in at 0-255
    accel x and y comes in at 0-1023
    4-part scaling and offset function maps these inputs to 128x160 screen

    offset -7 in width and -10 in height to prevent edgy cursor start values

    first two retval array positions = joy x/y
    last two retval array positions = accel x/y
    */
    scaled_cursor_data[0] = (int)(round(((float)joystick_x/SCALE_X_JOY)-10));
    scaled_cursor_data[1] = (int)(round(((float)joystick_y/SCALE_Y_JOY)-7));
    scaled_cursor_data[2] = (int)(round(((float)accel_x_raw/SCALE_X_ACCEL)-10));
    scaled_cursor_data[3] = (int)(round(((float)accel_y_raw/SCALE_Y_ACCEL)-7));

    //prevent negative screen positions from occurring
    for(int i=0; i<4;i++)
    {
        if(scaled_cursor_data[i]<0) 
        {
            scaled_cursor_data[i] = 0; 
        }
    }
    //debug!
    xprintf("Raw joyx is %d\r\n", joystick_x);
    xprintf("Raw joyy is %d\r\n", joystick_y);
    xprintf("Raw accelx is %d\r\n", accel_x_raw);
    xprintf("Raw accely is %d\r\n", accel_y_raw);

    xprintf("Scaled joyx is %d\r\n",scaled_cursor_data[0]);
    xprintf("Scaled joyy is %d\r\n",scaled_cursor_data[1]);
    xprintf("Scaled accelx is %d\r\n",scaled_cursor_data[2]);
    xprintf("Scaled accely is %d\r\n",scaled_cursor_data[3]);

}

void init_screencursor_peripherals(I2C_TypeDef *I2C, int I2CClockSpeed, uint8_t I2C_address)
{
    ST7735_init();
    xprintf("ST7735 Initialized.\r\n");
    ST7735_backlight(1);
    ST7735_fillScreen(BLACK);
    xprintf("LCD Backlight ON.\r\n");
    nunchuk_init(I2C, I2CClockSpeed,I2C_address);
    xprintf("Nunchuk Initialized\r\n");
}

void update_cursors_on_screen(I2C_TypeDef *I2C, uint8_t I2C_address,uint8_t *data)
{
    //Step 1 - get nunchuk data
    update_nunchuck_data(I2C,I2C_address,data);

    //Step 2 - scale nunchuk data to screen proportions
    convert_nunchuk_input_to_screen_position();
    
    //Step 3 - determine cursor char for each cursor
    char cursor_char_c;
    char cursor_char_z;
    //state of button determines the cursor character
    cursor_char_c = c_button ? 'C':'c';
    cursor_char_z = z_button ? 'Z' : 'z'; 


    //Step 4 - erase the previous char on the screen 
    //Just write the previous char in the BG color to the prev starting positions!
    if((cursor_char_c !=prev_char_c) | (scaled_cursor_data[0] != prev_joyx) 
        | (scaled_cursor_data[1] != prev_joyy))
        {
            ST7735_drawChar(prev_char_c,BGCOLOR, BGCOLOR,prev_joyx,prev_joyy);
            ST7735_drawChar(cursor_char_c,JOY_CURSOR_COLOR,BGCOLOR,scaled_cursor_data[0],scaled_cursor_data[1]);

            //update prev vals to current vals
            prev_joyx = scaled_cursor_data[0];
            prev_joyy = scaled_cursor_data[1];
            prev_char_c = cursor_char_c;
        }
    if((cursor_char_z !=prev_char_z) | (scaled_cursor_data[2] != prev_accelx) 
        | (scaled_cursor_data[3] != prev_accely))
        {
            ST7735_drawChar(prev_char_z,BGCOLOR, BGCOLOR,prev_accelx,prev_accely);
            ST7735_drawChar(cursor_char_z,ACCEL_CURSOR_COLOR,BGCOLOR,scaled_cursor_data[2],scaled_cursor_data[3]);
            prev_accelx = scaled_cursor_data[2];
            prev_accely = scaled_cursor_data[3];
            prev_char_z = cursor_char_z;
        }
}