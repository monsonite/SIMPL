# SIMPL
A serial interpreted minimal programming language toolkit - adapted for various microcontrollers.

SIMPL is about 300 lines of C code that allow you to control the peripheras of a microcontroller from a serial terminal connection.

It's the first thing I try to get working when bringing up any new microcontroller board.

It uses single ascii characters as commands to control the hardware - and commands can be strung together to make new commands - a litte bit like FORTH.

Most recently - runs under Arduino on STM32L433 on the myStorm BlackIce open source FPGA board  - September 2017

Previously coded in under 1024 bytes of MSP430 assembly language - Runs on MSP430G2553 Launchpad
