TEMPLATEROOT = ..

# compilation flags for gdb

CFLAGS  = -O1 -g
ASFLAGS = -g 

# object files

OBJS=  $(STARTUP) main.o
OBJS += spi.o uart.o stm32f10x_spi.o stm32f10x_gpio.o stm32f10x_rcc.o stm32f10x_usart.o LCD7735R.o glcdfont.o
OBJS += ff.o ffconf.o ffunicode.o ffsystem.o diskio.o mmc_stm32f1_spi.o
# include common make file

include $(TEMPLATEROOT)/Makefile.common


