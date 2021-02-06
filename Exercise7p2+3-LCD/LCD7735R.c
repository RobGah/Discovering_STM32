#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_spi.h>
#include "spi.h"
#include "LCD7735R.h"
#include "glcdfont.h"
#include <string.h>
#include <stdbool.h>

/*

***REG 1/19/21***

This driver was initially gotten from the book's author who adapted it from the Adafruit 1.8" TFT driver 
and is based heavily on (and in places like in init() copy-pasted from): 

https://github.com/StuartHa/DisoveringSTM32Solutions/blob/master/LCD/ST7735.c 
https://github.com/s1512783/DTSTM32Sols/blob/master/7.1_LCDintro/7735lcd.c <-- this one especially!

I've found slight differences in other's init functions, which is probably due to using a ST7735-based LCD 
and not an ST7735R like I have here. 

NOTES:
-Uses a Blue Pill STM32 Board and an ST7735R screen (https://www.adafruit.com/product/358)
-I first tried GPIOB port pins 3,4,5,6 for control of the LCD. DIDN'T WORK. Switched over to GPIOA. 
That worked for some reason that's unclear to me. I dug a bit into the pin's and their capabilities but couldn't find anything
conclusive. It could be a "me problem" but try GPIOB for yourself and let me know if you get it to work.
-Author's backlight function seems to have on/off reversed. I changed it here and it works for me but may depend on your screen. 
-I moved my timer code from main.c to here based on what I saw in the above 2 links. I doubt it makes a difference but 
I did it while debugging my setup. I have ideas of making a driver for strictly "setup" stuff like timers but haven't gotten to it yet.

*/

// pin definitions
#define LCD_PORT GPIOA
#define LCD_PORT_BKL GPIOA
#define GPIO_PIN_DC GPIO_Pin_4 // on port GPIOA
#define GPIO_PIN_RST GPIO_Pin_3 // on port GPIOA
#define GPIO_PIN_SCE GPIO_Pin_5 // on port GPIOA
#define GPIO_PIN_SD_CS GPIO_Pin_6 // on port GPIOA
#define GPIO_PIN_BKL GPIO_Pin_1 // on port GPIOA
#define SPILCD SPI2
#define LCDSPEED SPI_FAST

// LCD control
#define LOW 0
#define HIGH 1
#define LCD_C LOW
#define LCD_D HIGH
#define ST7735_CASET 0x2A
#define ST7735_RASET 0x2B
#define ST7735_MADCTL 0x36
#define ST7735_RAMWR 0x2C
#define ST7735_RAMRD 0x2E
#define ST7735_COLMOD 0x3A

// Timer code
static __IO uint32_t TimingDelay;

void Delay(uint32_t nTime)
{
    TimingDelay = nTime;
    while(TimingDelay !=0);
}

void SysTick_Handler(void)
{
    if (TimingDelay != 0x00)
    {
        TimingDelay--;
    }
}


#define MADVAL(x) (((x) << 5) | 8) //Works with my 7735R but others say to remove | 8
//static uint8_t madctlcurrent = MADVAL(MADCTLGRAPHICS);

struct ST7735_cmdBuf
{
	uint8_t command; //ST7735 command byte
	uint8_t delay; // ms delay after
	uint8_t len; // length of parameter data
	uint8_t data [16]; //parameter data
};


static const struct ST7735_cmdBuf initializers[] = {
  // SWRESET Software reset 
  { 0x01, 150, 0, 0},
  // SLPOUT Leave sleep mode
  { 0x11,  150, 0, 0},
  // FRMCTR1, FRMCTR2 Frame Rate configuration -- Normal mode, idle
  // frame rate = fosc / (1 x 2 + 40) * (LINE + 2C + 2D) 
  { 0xB1, 0, 3, { 0x01, 0x2C, 0x2D }},
  { 0xB2, 0, 3, { 0x01, 0x2C, 0x2D }},
  // FRMCTR3 Frame Rate configureation -- partial mode
  { 0xB3, 0, 6, { 0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D }},
  // INVCTR Display inversion (no inversion)
  { 0xB4,  0, 1, { 0x07 }},
  // PWCTR1 Power control -4.6V, Auto mode
  { 0xC0,  0, 3, { 0xA2, 0x02, 0x84}},
  // PWCTR2 Power control VGH25 2.4C, VGSEL -10, VGH = 3 * AVDD
  { 0xC1,  0, 1, { 0xC5}},
  // PWCTR3 Power control, opamp current smal, boost frequency
  { 0xC2,  0, 2, { 0x0A, 0x00 }},
  // PWCTR4 Power control, BLK/2, opamp current small and medium low
  { 0xC3,  0, 2, { 0x8A, 0x2A}},
  // PWRCTR5, VMCTR1 Power control
  { 0xC4,  0, 2, { 0x8A, 0xEE}},
  { 0xC5,  0, 1, { 0x0E }},
  // INVOFF Don't invert display
  { 0x20,  0, 0, 0},
  // Memory access directions. row address/col address, bottom to top refesh (10.1.27)
  { ST7735_MADCTL,  0, 1, {MADVAL(MADCTLGRAPHICS)}},
  // Color mode 16 bit (10.1.30
  { ST7735_COLMOD,   0, 1, {0x05}},
  // Column address set 0..127 
  { ST7735_CASET,   0, 4, {0x00, 0x00, 0x00, 0x7F }},
  // Row address set 0..159
  { ST7735_RASET,   0, 4, {0x00, 0x00, 0x00, 0x9F }},
  // GMCTRP1 Gamma correction
  { 0xE0, 0, 16, {0x02, 0x1C, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2D,
			    0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10 }},
  // GMCTRP2 Gamma Polarity corrction
  { 0xE1, 0, 16, {0x03, 0x1d, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D,
			    0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10 }},
  // DISPON Display on
  { 0x29, 100, 0, 0},
  // NORON Normal on
  { 0x13,  10, 0, 0},
  // End
  { 0, 0, 0, 0}
};


// local routines
/* LcdWrite - send 8-bit data to LCD over SPI*/
static void LcdWrite(char dc, const char *data, int cnt)
{
	GPIO_WriteBit(LCD_PORT, GPIO_PIN_DC, dc); // we use the DC pin to distinguish between data and control sequences
	GPIO_ResetBits(LCD_PORT, GPIO_PIN_SCE); // select LCD on SPI by pulling its pin low
	spiReadWrite(SPILCD, 0, data, cnt, LCDSPEED); // send SPI data
	GPIO_SetBits(LCD_PORT, GPIO_PIN_SCE); // disassert LCD on SPI
}

/* LcdWrite16 - send 16-bit data to LCD over SPI*/
static void LcdWrite16(char dc, const uint16_t *data, int cnt)
{
	GPIO_WriteBit(LCD_PORT, GPIO_PIN_DC, dc);
	GPIO_ResetBits(LCD_PORT, GPIO_PIN_SCE);
	spiReadWrite16(SPILCD, 0, data, cnt, LCDSPEED);
	GPIO_SetBits(LCD_PORT, GPIO_PIN_SCE);
}

/* ST7735_writeCmd - send a command to the ST7735 chip*/
static void ST7735_writeCmd(uint8_t c)
{
	LcdWrite(LCD_C, &c, 1);
}

//global routines

static uint8_t madctlcurrent = MADVAL(MADCTLGRAPHICS); // for storing current control sequence

/* ST7735_setAddrWindow - send window in which we'll be drawing pixels */
void ST7735_setAddrWindow(uint16_t x0, uint16_t y0,
		uint16_t x1, uint16_t y1, uint8_t madctl)
{
	madctl = MADVAL(madctl);
	if (madctl != madctlcurrent){
		ST7735_writeCmd(ST7735_MADCTL);
		LcdWrite(LCD_D, &madctl, 1);
		madctlcurrent = madctl;
	}

	// set column boundaries
	ST7735_writeCmd(ST7735_CASET);
	LcdWrite16(LCD_D, &x0, 1);
	LcdWrite16(LCD_D, &x1, 1);

	// set row boundaries
	ST7735_writeCmd(ST7735_RASET);
	LcdWrite16(LCD_D, &y0, 1);
	LcdWrite16(LCD_D, &y1, 1);

	// send write command to start writing
	ST7735_writeCmd(ST7735_RAMWR);
}

/*  ST7735_init - initialize the LCD */
void ST7735_init()
{
	const struct ST7735_cmdBuf *cmd;

	// if (SysTick_Config(SystemCoreClock/1000))
	// 	while (1);

	/* Set up pins */
	// enable clock to GPIO
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	/* Set up GPIO */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	// GPIOA LCD control (was GPIOB but didn't work for B port/pins)
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_DC | GPIO_PIN_RST | GPIO_PIN_SCE;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(LCD_PORT, &GPIO_InitStructure);
	// GPIOA LCD Backlight
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_BKL;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(LCD_PORT_BKL, &GPIO_InitStructure);

	/*Initialize SPI*/
	spiInit(SPILCD); 

	SysTick_Config(SystemCoreClock / 1000);
    
    GPIO_WriteBit(LCD_PORT, GPIO_PIN_SCE, HIGH);
	GPIO_WriteBit(LCD_PORT, GPIO_PIN_RST, HIGH);
	Delay(10);
	GPIO_WriteBit(LCD_PORT, GPIO_PIN_RST, LOW);
	Delay(10);
	GPIO_WriteBit(LCD_PORT, GPIO_PIN_RST, HIGH);
	Delay(10);

	// Send initialization commands to ST7735
	for (cmd = initializers; cmd->command; cmd ++){
		LcdWrite(LCD_C, &(cmd->command), 1);
		if (cmd->len)
			LcdWrite(LCD_D, cmd->data , cmd->len);
		if (cmd->delay)
			Delay(cmd->delay);
	}
}


/* ST7735_pushColor - write pixel color data to LCD RAM */
void ST7735_pushColor(uint16_t *color, int cnt)
{
	LcdWrite16(LCD_D, color, cnt);
}

/* ST7735_backlight - turn LCD backlight on and off */
void ST7735_backlight(uint8_t on)
{
	// high and low were swapped here - works for me
	// you won't see anything w/o backlight being on!
	if(on)
		GPIO_WriteBit(LCD_PORT_BKL, GPIO_PIN_BKL, HIGH);
	else
		GPIO_WriteBit (LCD_PORT_BKL, GPIO_PIN_BKL, LOW);
}

void ST7735_fillScreen(uint16_t color)
{
    uint8_t x, y;

    ST7735_setAddrWindow(0, 0, ST7735_WIDTH - 1, ST7735_HEIGHT - 1, MADCTLGRAPHICS);

    for (x = 0; x < ST7735_WIDTH; x++)
    {
        for (y = 0; y < ST7735_HEIGHT; y++)
        {
            ST7735_pushColor(&color, 1);
        }
    }
}

void ST7735_drawChar(char letter, uint16_t lettercolor, uint16_t bgcolor, 
	uint16_t startx, uint16_t starty)
{
/*This drawChar function is based on: 
https://github.com/s1512783/DTSTM32Sols/blob/master/7.2_and_7.3_LCDtext_graphics/7735lcd.h
	Almost had it - M. Minkowski's implementation got me over the hump

	inputs: 
	letter: e.g. 'A'
	lettercolor - ad. oculos
	bgcolor - background color
	startx, starty - starting pos. of letter

	UPDATE 2/4/21: 
	-Got my homegrown function working after studying M. Minkowski's implementation.
	-The letters appear in "landscape(ish) mode" if you go with 0x06 for setting address window
	-Use 0x07 for portait mode, and think of it as 10x7 and not 7x10
	-X is top-down, Y is left-right and writes "column by column" visually on the screen
	*/
	
	//Step 1: determine where we are in the font array
	uint16_t location_in_font_array = 5*letter; //converts to a number and 5x to get pos. in font array
	
	//Step 2: set our window - start x,y + room as described in the book
	//each character is placed in a 10x7 (x is top down, y is left right) rectangle leaving
	//space between lines (3 pixels) and characters (1 pixel)
	ST7735_setAddrWindow(startx, starty, startx+9, starty+6, MADCTLTEXT);

	//Step 3:
	//for all 5 bytes that compose the letter
	for(uint8_t i=0; i<5; i++) 
	//each loop is another defining of a "column" by the 8bit hex value
	{
		//get the hex value in font 
		uint8_t font_hex = font[location_in_font_array +i];
		
		//for each bit in the hex value byte
		for(uint8_t j=0; j<8; j++)
		{
			if(font_hex & 1) //if the last bit is a 1
			{
				ST7735_pushColor(&lettercolor,1); //its part of the letter
			}
			else
			{
				ST7735_pushColor(&bgcolor,1); //otherwise, its background.
			}

			font_hex = font_hex >> 1; //right shift out the used bit
		}
		
		// draw line spacing
		for(uint8_t k=0;k<2;k++) //row 9 and 10 are blank
		{
			ST7735_pushColor(&bgcolor,1);
		}

		
	}
		//write same 2 bytes  10 times for the spacing.
		//Based on M. minkowski's implentation,
		//I dont trust setting cnt in pushColor to 10
		//Change loop count to get sanity check for how it writes top-down
		for(int k = 0; k<10;k++)
		{
			ST7735_pushColor(&bgcolor,1); //write background to the 5th column
		}
}

void ST7735_drawString(char *phrase, uint16_t lettercolor, uint16_t bgcolor, 
	uint16_t startx, uint16_t starty)
{
/* writes a character-based phrase to the screen!

inputs: 
-pointer to a phrase to be written
-letter color for text
-background color for text
-starting position of phrase in x and y

***Horizontal short side of screen  = y
***Vertical long side of screen = x 
*/

//track our position on screen 
uint16_t current_position_x = startx;
uint16_t current_position_y = starty;

//For each char in the phrase
	for(uint8_t i=0; i<strlen(phrase); i++)
	{
		//if a new line - UNTESTED
		if(phrase[i]=='\n')
		{
			current_position_x += 10; //start a new line 10 rows down
			current_position_y = starty; //return to the starting y position
		}
	
		//handling wrap and end of screen issues
		if(current_position_y + 7 > ST7735_WIDTH) 
		{
			current_position_x += 10;
			current_position_y = starty;
		}

		if(current_position_x + 10 > ST7735_HEIGHT - 10)
		{
			return; //terminate function if we exceed writable space on the screen
		}
	
		else ST7735_drawChar(phrase[i],lettercolor,bgcolor,current_position_x, current_position_y);

		//increment current position horizontally
		current_position_y +=6;
	}
}

void ST7735_drawPixel(uint8_t x, uint8_t y, uint16_t pixelcolor)
{
	/*draws a single pixel

	inputs: x,y: pixel location to draw.
	pixelcolor: ad oculos
	*/

	//set window to any window size - it doesnt matter
	ST7735_setAddrWindow(x, y, x+1, y+1,MADCTLGRAPHICS); //might not even need to +1 these?
	ST7735_pushColor(&pixelcolor,1); //single pushColor command
}


void ST7735_fillSpace(uint16_t xo, uint16_t yo, uint16_t x1, uint16_t y1, uint16_t color)
{
	/* fills a rectangular section of the screen with a color
	inputs:
	x0,y0,x1,y1: coordinates of the start and end points of the rectangular screen
	color: ad oculos
	*/

	//just like fill screen but less!
	ST7735_setAddrWindow(xo,yo,x1-1,y1-1,MADCTLGRAPHICS);

	//ruthless copy-paste of fillScreen guts
	for (uint8_t x = 0; x < x1; x++)
    {
        for (uint8_t y = 0; y < y1; y++)
        {
            ST7735_pushColor(&color, 1);
        }
    }
}
void ST7735_drawLine(uint16_t x0, uint16_t y0, uint8_t length, uint8_t thickness,
	uint16_t linecolor, bool linestyle)
	{
		/* Draws a line either vertically or horizontally
		inputs: 
		-x0, y0: starting point of line
		-length: line length - goes top-down and left-right
		-thickness: line thickness
		-linecolor: ad oculos
		-linestyle: horizontal if FALSE or 0 , vertical if 1 or TRUE
		*/
		if (linestyle == 1)
		{
			ST7735_fillSpace(x0, y0, x0+thickness, y0+length, linecolor);
		}
		
		else ST7735_fillSpace(x0, y0, x0+length, y0+thickness,linecolor);

	}

void ST7735_drawRectangle(uint8_t startx, uint8_t starty, uint8_t width, uint8_t height, 
	uint16_t linecolor, uint8_t thickness)
{
/* draws a rectangle of fixed height/width
input: 
-startx, starty is the location of the first corner 
-width and height are left to right x and y top to bottom properties as defined
*/

//Top horizontal
ST7735_drawLine(startx,starty,width,thickness,linecolor,0);
//Left vertical
ST7735_drawLine(startx,starty,height,thickness,linecolor,1);
//Right vertical
ST7735_drawLine(startx+width,starty,height+thickness,thickness,linecolor,1);
//Bottom horizontal
ST7735_drawLine(startx,starty+height,width,thickness,linecolor,0);

}

void ST7735_drawCircle(uint8_t centerx, uint8_t centery, uint8_t radius,
	uint16_t linecolor, uint16_t bgcolor)
{
/* draws a circle of fixed radius
input:
-centerx and center y is the location of the circle center
-radius ad oculos
*/

}

