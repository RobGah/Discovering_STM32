TEMPLATEROOT = ..

# compilation flags for gdb

CFLAGS  = -O1 -g 
ASFLAGS = -g 
LDLIBS += -lm

# object files

OBJS=  $(STARTUP) main.o
OBJS += spi.o uart.o I2CRoutines.o setup_main.o LCD7735R.o xprintf.o nunchuk.o glcdfont.o
OBJS += stm32f10x_gpio.o stm32f10x_rcc.o stm32f10x_usart.o stm32f10x_i2c.o
OBJS += stm32f10x_dma.o stm32f10x_spi.o

# include common make file
include $(TEMPLATEROOT)/Makefile.common


