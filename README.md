Discovering the STM32 Microcontroller - Worked Problems
==============

A repo of worked examples from "Discovering the STM32 Microcontroller by Geoffery Brown

Main project folder contains makefile.common, the STM32 Standard Peripheral Library and necessary startup code, while subfolders contain the exercise source code that I've created and the resulting flashable .elf file.

I'm using an STM32 "Blue Pill" w/ STM32f103c8t6 onboard. Author makes use of STM32VL Discovery Board. Any hardware peripherals required that are native to the STM32VL Discovery have been added externally to my setup (i.e. buttons and LEDs, STLink/V2 debugger). 

Minor edits to the author provided makefile and example code in the book (i.e. user LED is PC13 vs PA8) enable the use of the Blue Pill in lieu of the STM32VL discovery board. Highly recommended as I had quite a bit of trouble with setting up the STM32VL Discovery Board and its STLink v1 debugger with the open source STLink recommended by the author. 

To be explicit, change 

PTYPE = STM32F10X_MD_VL
to
PTYPE = STM32F10X_MD

in makefile.common in the author-provided code. Or just clone my repo!

I might try working in this repo with an STM32L100 Discovery board as it hosts an STLink v2 debugger onboard and is quite similar. If I do, I'll report back. No garuntees!

