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

//screen basic stuff
void ST7735_setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t madctl);
void ST7735_pushColor(uint16_t *color, int cnt);
void ST7735_init();
void ST7735_backlight(uint8_t on);
void ST7735_fillScreen(uint16_t color);

//letters and phrases
//void ST7735_writeChar(char letter, uint16_t lettercolor, uint16_t bgcolor, 
    //uint16_t startx, uint16_t starty);0
    
void ST7735_writeChar(uint16_t x0, uint16_t y0,
		unsigned char c, uint16_t textColor, uint16_t bgColor);
//timer stuff
void Delay(uint32_t nTime);
void SysTick_Handler(void);
#endif