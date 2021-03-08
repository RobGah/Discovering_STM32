TEMPLATEROOT = ..

# compilation flags for gdb

CFLAGS  = -O1 -g 
ASFLAGS = -g 
LDLIBS += -lm

# object files

OBJS=  $(STARTUP) main.o
OBJS += spi.o uart.o setup_main.o timers.o 
OBJS += xprintf.o HC-SR04.o
OBJS += stm32f10x_gpio.o stm32f10x_rcc.o stm32f10x_usart.o 
OBJS += stm32f10x_dma.o stm32f10x_spi.o stm32f10x_tim.o

# include common make file
include $(TEMPLATEROOT)/Makefile.common


