#include "LCD7735R.h"
#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_spi.h>

#define LCD_PORT 1 //tbd
#define GPIO_PIN_DC 1 //tbd
#define GPIO_PIN_SCE 1 //tbd
void ST7735_fillScreen(uint16_t color)
{
    uint8_t x,y;

    ST7735_setAddrWindow(0,0,ST7735_WIDTH-1, ST7735_HEIGHT-1, MADCTLGRAPHICS);

    for(x=0; x<ST7735_WIDTH; x++)
    {
        for(y=0; y<ST7735_HEIGHT; y++)
        {
            ST7735_pushColor(&color, 1);
        }
    }
}

static void LcdWrite(char dc, const char *data, int nbytes)
{
    //dc 1 = data; dc 0 = control
    GPIO_WriteBit(LCD_PORT, GPIO_PIN_DC, dc);
    GPIO_ResetBits(LCD_PORT,GPIO_PIN_SCE);
}