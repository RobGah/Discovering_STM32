Discovering the STM32 Microcontroller - Worked Problems using the STM32 "Blue Pill" PCBA
==============
A repo of worked examples from "Discovering the STM32 Microcontroller by Geoffery Brown.
This repo is a work-in-progress. My goal is to create a solution for every example in the book using the STM32F193C8T6 "Blue Pill" board commonly found online. 

The author uses the STM32 VL series discovery board in the book but thus far I've found it easy and relatively equivalent and painless to use the Blue Pill board w/ an external STLink V2 dongle. Any hardware peripherals required that are native to the STM32VL Discovery have been added externally to my setup (i.e. buttons and LEDs, STLink/V2 debugger). There are minor differences in clock speed (Blue pill's STMF103C8T6 clock is 72MHz vs 24MHz on STM32VL) and peripherals (e.g. blue pill's STMF103C8T6 doesn't have a DAC onboard). *When building using the Blue Pill, change*:

PTYPE = STM32F10X_MD_VL

to

PTYPE = STM32F10X_MD

or vise-versa

in makefile.common in the author-provided code. Or just clone my repo.

Main project folder contains makefile.common, the STM32 Standard Peripheral Library and necessary startup code, while subfolders contain the exercise source code that I've created and the resulting flashable .elf file. I admit that my subfolders are a tad messy as I did not separate them into src and inc files. I might go back and do so. Additionally, there are likely (definitely) unused modules that have gotten included. I likely will, when this project is complete, go back and do some code clean-up. 

Minor edits to the author provided makefile and example code in the book (e.g. user LED is PC13 vs PA8) enable the use of the Blue Pill in lieu of the STM32VL discovery board. Highly recommended as I had quite a bit of trouble with setting up the STM32VL Discovery Board and its STLink v1 debugger with the open source STLink recommended by the author. 

*EDIT*: Late in the game I got the Discovery board to work with the STLink open-source software(https://github.com/stlink-org/stlink). I'm using an external STLink V2 dongle (They are ubiquitous and easily found online). I've tried dearly to try to get the onboard Disco board STLINK to work to absolutely no avail. I think its an ~advanced~ driver incompatiblity issue.

Wherever I've used the Disco board in an exercise, I will make a note of it in main.c for that exercise. (Most notably, see the DAC exercises).

To program a Disco board w/ the STLink Dongle:
-Remove the CN3 jumpers (this disconnects the onboard STLink from the STM32VL IC)
-disconnect the USB cable 
-dongle SWDIO -> disco PA13
-dongle SWCLK -> disco PA14
-dongle GND -> disco GND
-dongle 3V3 -> disco 3V3/VDD

N.B. that because 2 different boards were used that particulars when building may arise depending on the target board.

Also late in the game but very much worth mentioning. This guy 
https://www.youtube.com/watch?v=PxQw5_7yI8Q&t=720s (part 1 of 3 linked here)
gives an EXCELLENT toolchain overview for the STM32. This project makes absolutely no use of STM32CubeMX (which looks like an AWESOME way to config the peripherals automatically vs doing it manually in this project suite) but every other tool in his chain is VERY much applicable and in use in one way or another. 

Debugging with openocd is massively advantageous over gdb and the ability to do it in VSCode w/ the Cortex Debug extension is very helpful.