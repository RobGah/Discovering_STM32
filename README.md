Discovering the STM32 Microcontroller - Worked Problems using the STM32 "Blue Pill" PCBA
==============
A repo of worked examples from "Discovering the STM32 Microcontroller by Geoffery Brown.
This repo is a work-in-progress. My goal is to create a solution for every example in the book using the STM32F193C8T6 "Blue Pill" board commonly found online. 

The author uses the STM32 VL series discovery board in the book but thus far I've found it easy and relatively equivalent and painless to use the Blue Pill board. Any hardware peripherals required that are native to the STM32VL Discovery have been added externally to my setup (i.e. buttons and LEDs, STLink/V2 debugger). 

Main project folder contains makefile.common, the STM32 Standard Peripheral Library and necessary startup code, while subfolders contain the exercise source code that I've created and the resulting flashable .elf file. I admit that my subfolders are a tad messy as I did not separate them into src and inc files. I might go back and do so. Additionally, there are likely (definitely) unused modules that have gotten included. I likely will, when this project is complete, go back and do some code clean-up. 

Minor edits to the author provided makefile and example code in the book (e.g. user LED is PC13 vs PA8) enable the use of the Blue Pill in lieu of the STM32VL discovery board. Highly recommended as I had quite a bit of trouble with setting up the STM32VL Discovery Board and its STLink v1 debugger with the open source STLink recommended by the author. 

To be explicit, to use the BluePill board, change 

PTYPE = STM32F10X_MD_VL

to

PTYPE = STM32F10X_MD

in makefile.common in the author-provided code. Or just clone my repo!

