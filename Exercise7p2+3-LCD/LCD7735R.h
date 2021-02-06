//No distinction between '_' and 'R' variants in guard - only 1 driver for the LCD
//Each variant likely to use separate drivers to avoid confusion
#ifndef LCD7735_H 
#define LCD7735_H

#define MADCTLGRAPHICS      0x6
#define MADCTLTEXT          0x07
#define MADCTLBMP           0x2

#define ST7735_WIDTH        128
#define ST7735_HEIGHT       160

//Sample Colors
#define BLACK            0x0000
#define BLUE             0x001F
#define GREEN            0x07E0
#define LBLUE            0x07FF
#define RED              0xF800
#define YELLOW           0xFFE0
#define WHITE            0xFFFF

#include <stdbool.h>

//screen basic stuff
void ST7735_setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t madctl);
void ST7735_pushColor(uint16_t *color, int cnt);
void ST7735_init();
void ST7735_backlight(uint8_t on);
void ST7735_fillScreen(uint16_t color);

//letters and phrases
void ST7735_drawChar(char letter, uint16_t lettercolor, uint16_t bgcolor, 
    uint16_t startx, uint16_t starty);
void ST7735_drawString(char *phrase, uint16_t lettercolor, uint16_t bgcolor, 
uint16_t startx, uint16_t starty);

// lines, 'tangles, circles and shapes
void ST7735_drawRectangle(uint8_t startx, uint8_t starty, uint8_t width, uint8_t height, 
	uint16_t linecolor, uint8_t thickness);
void ST7735_drawCircle(uint8_t centerx, uint8_t centery, uint8_t r,
	uint16_t linecolor);
void ST7735_fillSpace(uint16_t xo, uint16_t yo, uint16_t x1, uint16_t y1, uint16_t color);
void ST7735_drawPixel(uint8_t x, uint8_t y, uint16_t pixelcolor);
void ST7735_drawLine(uint16_t x0, uint16_t y0, uint8_t length, uint8_t thickness,
	uint16_t linecolor, bool linestyle);

//timer stuff
void Delay(uint32_t nTime);
void SysTick_Handler(void);
#endif