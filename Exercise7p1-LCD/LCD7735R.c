#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_spi.h>
#include "spi.h"
#include "LCD7735R.h"

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