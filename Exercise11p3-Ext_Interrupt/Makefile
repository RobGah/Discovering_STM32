TEMPLATEROOT = ..

# compilation flags for gdb

CFLAGS  = -O1 -g 
ASFLAGS = -g 
LDLIBS += -lm

# object files

OBJS =  $(STARTUP) main.o
OBJS += uartfc.o setup_main.o
OBJS += misc.o interrupts.o stm32f10x_exti.o
OBJS += stm32f10x_gpio.o stm32f10x_rcc.o stm32f10x_usart.o
OBJS += stm32f10x_dma.o stm32f10x_spi.o stm32f10x_tim.o

# include common make file
include $(TEMPLATEROOT)/Makefile.common


