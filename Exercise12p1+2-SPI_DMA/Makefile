TEMPLATEROOT = ..

# compilation flags for gdb

CFLAGS  = -O1 -g 
ASFLAGS = -g 
LDLIBS += -lm

# object files

OBJS=  $(STARTUP) main.o
OBJS += ff.o spidma.o spi.o uart.o sdmm.o setup_main.o ffsystem.o ffunicode.o LCD7735R.o xprintf.o
OBJS += stm32f10x_gpio.o stm32f10x_rcc.o stm32f10x_usart.o misc.o 
OBJS += stm32f10x_spi.o stm32f10x_dma.o core_cm3.o sdcard.o

# include common make file
include $(TEMPLATEROOT)/Makefile.common


