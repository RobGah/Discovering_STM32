TEMPLATEROOT = ..

# compilation flags for gdb

CFLAGS  = -O1 -g 
ASFLAGS = -g 
LDLIBS += -lm

# FREERTOS 
FreeRTOS = $(CURDIR)/FreeRTOS_Source
CFLAGS += -I$(FreeRTOS)/include -DGCC_ARMCM3
vpath %.c $(FreeRTOS)/
vpath %.c $(FreeRTOS)/include
vpath %.c $(FreeRTOS)/portable/MemMang
vpath %.c $(FreeRTOS)/portable/GCC/ARM_CM3/
vpath %.c $(FreeRTOS)/portable/GCC/ARM_CM3/portmacro.h
# object files

OBJS=  $(STARTUP) main.o
OBJS += spidma.o spi.o uart.o setup_main.o xprintf.o adc.o stm32f10x_dac.o
OBJS += stm32f10x_gpio.o stm32f10x_rcc.o stm32f10x_usart.o misc.o interrupts.o stm32f10x_exti.o
OBJS += stm32f10x_spi.o stm32f10x_dma.o stm32f10x_tim.o timer.o stm32f10x_adc.o dac.o waveform.o
OBJS += tasks.o queue.o list.o timers.o heap_1.o port.o freertos.o semphr.o

# include common make file
include $(TEMPLATEROOT)/Makefile.common


