//No distinction between '_' and 'R' variants in guard - only 1 driver for the LCD
//Each variant likely to use separate drivers to avoid confusion
#ifndef LCD7735_H 
#define LCD7735_H

#define MADCTLGRAPHICS      0x6
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

void ST7735_setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t madctl);
void ST7735_pushColor(uint16_t *color, int cnt);
void ST7735_init();
void ST7735_backlight(uint8_t on);
void ST7735_fillScreen(uint16_t color);

#endif