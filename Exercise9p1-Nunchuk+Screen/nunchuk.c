/****Driver for I2C Wii Nunchuk***/


#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_spi.h>
#include <stm32f10x_usart.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "uart.h"
#include "spi.h"
#include "LCD7735R.h"
#include "setup_main.h"
#include "xprintf.h"
//#include "i2c.h"
#include "I2CRoutines.h"
//#include "I2CHandsOn.h"

#define ACCEL_LOWER_BOUND 0
#define ACCEL_UPPER_BOUND 1023

#define MILLIG_LOWER_BOUND -2000
#define MILLIG_UPPER_BOUND 2000

//initial writes to start comms w/ nunchuk
uint8_t buf_init1[2] = {0xF0, 0x55}; 
uint8_t buf_init2[2] = {0xFB, 0x00};

uint8_t raw_data_buffer[6];
uint8_t joystick_x;
uint8_t joystick_y;
uint16_t accel_x_raw;
uint16_t accel_y_raw;
uint16_t accel_z_raw;
uint16_t accel_x_millig;
uint16_t accel_y_millig;
uint16_t accel_z_millig;
bool c_button;
bool z_button;

void nunchuk_init(I2C_TypeDef *I2C, int I2C_Speed, uint8_t I2C_address)
{
    Status status;
    //Initialize 'chuk
    xprintf("Initializing I2C w/ Nunchuk...\r\n");
    Delay(1000);
    I2C_LowLevel_Init(I2C);
    Delay(20); 
    status = I2C_Master_BufferWrite(I2C , buf_init1 , 2, Polling, I2C_address);
    Delay(20); //slight delay?
    xprintf("Write of %x and %x to Nunchuk returned %d.\r\n", buf_init1[0],buf_init1[1],status);
    status = I2C_Master_BufferWrite(I2C , buf_init2 , 2, Polling, I2C_address);
    xprintf("Write of %x and %x to Nunchuk returned %d.\r\n", buf_init2[0],buf_init2[1],status);
    Delay(20);
}

// void nunchuk_init(uint8_t I2C_address)
// {
//     //Initialize 'chuk
//     xprintf("Initializing I2C w/ Nunchuk...\r\n");
//     Delay(1000);
//     i2c_init();
//     Delay(20); 
//     i2c_write(I2C_address,buf_init1);
//     Delay(20); //slight delay?
//     xprintf("Write of %x and %x to Nunchuk.\r\n", buf_init1[0],buf_init1[1]);
//     i2c_write(I2C_address,buf_init2);
//     xprintf("Write of %x and %x to Nunchuk.\r\n", buf_init2[0],buf_init2[1]);
//     Delay(20);
// }

void read_raw_nunchuk_data(I2C_TypeDef *I2C, uint8_t I2C_address, uint8_t *data)
{
   //I2C_Master_BufferWrite(I2C,0x00,1,Polling,I2C_address);//0x00 written to start comms
    I2C_Master_BufferRead(I2C,data,6,Polling,I2C_address);//scoop up 6 bytes
}

// static void read_raw_nunchuk_data(uint8_t I2C_address, uint8_t *data)
// {
//     i2c_read(I2C_address, &data);
// }

static void parse_raw_nunchuk_data(uint8_t *data)
{
    //the easy stuff
    joystick_x = data[0];
    joystick_y = data[1];

    /*Tedious bit shifting and OR-ING
    1) grab the raw 8 bit portion of the accel data and
    store it as a 16 bit value. 
    2) Left shift 2 bits to make it a 10 bit value.
    3) Grab the last 2 accel x/y/z bits from the last byte
        a) grab the last byte
        b) mask out all other bits but the bits of interest by ANDing
            w/ the appropriate hex value (i.e. for last 2 accel bits 
            its 0x0C or 00001100)
        c) right shift those selected bits to the 2 least significant bits places
        d) OR the shifted 8-bit accel value to the 2 least significant bits. 
        e) congrats, we have a 10 bit value!
    */
    accel_x_raw = (data[2]<<2) | ((data[5]&0x0C)>>2);
    accel_y_raw = (data[3]<<2) | ((data[5]&0x30)>>4);
    accel_z_raw = (data[4]<<2) | ((data[5]&0xC0)>>6);

    //Same deal, mask (AND) out all other bits except the one of interest
    //then do a cute inline if
    //if the value is 1, bool is true, else is false
    c_button = (((data[5]&0x02)>>1) ? true:false); 
    z_button = ((data[5]&0x01) ? true:false);
}

 static int map(uint16_t x, uint16_t in_min, uint16_t in_max, 
    int out_min, int out_max) 
    {
    /*
    Takes in some value within some range and 
    maps it proportionally to a new range.
    Here we use it to convert 0-1023 to -2000 to 2000millig's

    adapted from:
    //https://www.arduino.cc/reference/en/language/functions/math/map/
    */
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }   

static void convert_raw_accel_to_millig(uint8_t accel_x ,uint8_t accel_y,uint8_t accel_z)
{
    accel_x_millig = map(accel_x, ACCEL_LOWER_BOUND, ACCEL_UPPER_BOUND, 
    MILLIG_LOWER_BOUND, MILLIG_UPPER_BOUND);

    accel_y_millig = map(accel_y, ACCEL_LOWER_BOUND, ACCEL_UPPER_BOUND, 
    MILLIG_LOWER_BOUND, MILLIG_UPPER_BOUND);

    accel_z_millig = map(accel_z, ACCEL_LOWER_BOUND, ACCEL_UPPER_BOUND, 
    MILLIG_LOWER_BOUND, MILLIG_UPPER_BOUND);
}

void report_nunchuk_data(I2C_TypeDef * I2C, uint8_t I2C_address, uint8_t *data)
{
    read_raw_nunchuk_data(I2C, I2C_address, data);
    parse_raw_nunchuk_data(data);
    convert_raw_accel_to_millig(accel_x_raw, accel_y_raw, accel_z_raw);

    xprintf("\r\nNunchuk reads... \r\n\r\n");
    xprintf("Accel x: %d milliG's.\r\n", accel_x_millig);
    xprintf("Accel y: %d milliG's.\r\n", accel_y_millig);
    xprintf("Accel z: %d milliG's.\r\n", accel_z_millig);
    xprintf("Joystick x: %d ticks.\r\n", joystick_x);
    xprintf("Joystick y: %d ticks.\r\n",  joystick_y);
    xprintf("C Button: %s.\r\n", c_button ? "TRUE":"FALSE");
    xprintf("Z Button: %s.\r\n", z_button ? "TRUE":"FALSE");
}




