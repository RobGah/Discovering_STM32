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
#include <math.h>
#include "spi.h"
#include "LCD7735R.h"
#include "setup_main.h"
#include "xprintf.h"
#include "i2c.h"
#include "nunchuk.h"
//#include "I2CRoutines.h" <-This is what the book's i2c.c/h is based on

//initial writes to start comms w/ nunchuk
static const uint8_t buf_init1[2] = {0xF0, 0x55}; 
static const uint8_t buf_init2[2] = {0xFB, 0x00};
Status status; //generic retval from i2c operations

/*math stuff*/
#define RATIO_BITS_TO_GS ((float)MILLIG_DYNAMIC_RANGE/(float)ACCEL_BITS)

//nunchuk variables
int accel_x_millig;
int accel_y_millig;
int accel_z_millig;

//public 'chuk init function.
void nunchuk_init(I2C_TypeDef *I2C, int I2CClockSpeed, uint8_t I2C_address)
{
    //Initialize 'chuk
    xprintf("Initializing I2C w/ Nunchuk...\r\n");
    Delay(1000);
    I2C_LowLevel_Init(I2C,I2CClockSpeed, 0x00);
    I2C_StretchClockCmd(I2C,ENABLE);//give this a shot!
    Delay(1000); //Give everything a literal second to get its act together!
    status = I2C_Write(I2C , buf_init1 , 2, I2C_address);
    xprintf("Write of %x and %x to Nunchuk returned %d.\r\n", buf_init1[0],buf_init1[1],status);
    status = I2C_Write(I2C , buf_init2 , 2, I2C_address);
    xprintf("Write of %x and %x to Nunchuk returned %d.\r\n", buf_init2[0],buf_init2[1],status);
    Delay(20);
}


//private nunchuk data packet read function
static void read_raw_nunchuk_data(I2C_TypeDef *I2C, uint8_t I2C_address, uint8_t *data)
{
    status = I2C_Write(I2C,0x00,1,I2C_address);//0x00 written to start comms
    xprintf("I2C Read initiation write %d returned %d\r\n", 0,status);
    Delay(5); //give it some time to get its act together
    status = I2C_Read(I2C,data,6,I2C_address);//scoop up 6 bytes
    xprintf("I2C Read routine returned %d\r\n", status);
}

//private parsing function for nunchuk data
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
    accel_x_raw = (data[2]<<2) | ((data[5]&0x0C)>>2); //00001100 mask
    accel_y_raw = (data[3]<<2) | ((data[5]&0x30)>>4); //00110000 mask
    accel_z_raw = (data[4]<<2) | ((data[5]&0xC0)>>6); //11000000 mask

    //Same deal, mask (AND) out all other bits except the one of interest
    //then do a cute inline if
    //if the value is 1, bool is false, else is true (this is how my chuk behaves, YMMV)
    c_button = (((data[5]&0x02)>>1) ? false:true); 
    z_button = ((data[5]&0x01) ? false:true);
}
 
//private conversion function of bits ->mG's
static void convert_raw_accel_to_millig(uint16_t accel_x ,uint16_t accel_y,uint16_t accel_z)
{
    /* This is a scale and offset operation - not crazy necessary for this project but its cool to see
    1. Scale the raw accel value by the ratio of dynamic range/bits or "Bits to G's" conversion ratio
    2. Offset (i.e. subtract) half the total dynamic range of the accelerometer to get +/-2g value
    3. Round to the nearest whole number and cast as an int to make life easy
    */

    accel_x_millig = (int)round(RATIO_BITS_TO_GS*accel_x_raw)-(MILLIG_DYNAMIC_RANGE/2);
    accel_y_millig = (int)round(RATIO_BITS_TO_GS*accel_y_raw)-(MILLIG_DYNAMIC_RANGE/2);
    accel_z_millig = (int)round(RATIO_BITS_TO_GS*accel_z_raw)-(MILLIG_DYNAMIC_RANGE/2);

}

//public function to update nunchuk data in the variables
void update_nunchuck_data(I2C_TypeDef *I2C, uint8_t I2C_address,uint8_t *data)
{
    read_raw_nunchuk_data(I2C, I2C_address, data);
    parse_raw_nunchuk_data(data);
}

//public function to print out nunchuk data over UART
//expresses accel in mG's but easy to express it in 0-1023 (10bits) w/ tweaking if needed
void report_nunchuk_data(I2C_TypeDef * I2C, uint8_t I2C_address, uint8_t *data)
{
    update_nunchuck_data(I2C,I2C_address,data);

    //do the conversion of bits -> mG's
    convert_raw_accel_to_millig(accel_x_raw, accel_y_raw, accel_z_raw);

    //print out all nunchuck variables
    xprintf("\r\nNunchuk reads... \r\n\r\n");
    xprintf("Accel x: %d in mG.\r\n", accel_z_millig);
    xprintf("Accel y: %d in mG.\r\n", accel_y_millig);
    xprintf("Accel z: %d in mG.\r\n", accel_z_millig);
    xprintf("Joystick x: %d ticks.\r\n", joystick_x);
    xprintf("Joystick y: %d ticks.\r\n", joystick_y);
    xprintf("C Button: %s.\r\n", c_button ? "TRUE":"FALSE");
    xprintf("Z Button: %s.\r\n", z_button ? "TRUE":"FALSE");
}





