TEMPLATEROOT = ..

# compilation flags for gdb

CFLAGS  = -O1 -g 
ASFLAGS = -g 
LDLIBS += -lm

# object files

OBJS=  $(STARTUP) main.o
OBJS += spidma.o spi.o uart.o setup_main.o xprintf.o adc.o stm32f10x_dac.o
OBJS += stm32f10x_gpio.o stm32f10x_rcc.o stm32f10x_usart.o misc.o interrupts.o stm32f10x_exti.o
OBJS += stm32f10x_spi.o stm32f10x_dma.o stm32f10x_tim.o timers.o stm32f10x_adc.o dac.o waveform.o

# include common make file
include $(TEMPLATEROOT)/Makefile.common


