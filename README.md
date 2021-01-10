# SIMPL

Introduction.

A serial interpreted minimal programming language toolkit - adapted for various microcontrollers including Arduino, ARM and MSP430.

SIMPL sets up a minimal serial shell which allows you to control a microcontroller from a few ascii characters typed from a serial terminal.

Numbers 0-9  are processed as a 16-bit integer and used as numerical input (literals) or as control parameters

lowercase alpha a-z  Usually used to access a predefined routine stored in ROM  eg. p is printnumber, u is microsecond delay

Uppercase alpha A-Z  Frequently used to define User Commands or parameters, created to meet the requirements of the application

Symbols Used for arithmetic, logic, loop control and program flow purposes.

This convention is suggested as it gives each character a strong mnemonic value and makes the code more readable. However any of the 96 printable ascii characters could be used to initiate any function in code.

Each ascii character acts as an instruction for a command, forcing a jump to a routine that executes the command, before returning to the interpreter to process the next serial character.

The code required to do this is very straightforward and can be implemented as a large switch-case statement or look up table contained within a while loop.

The serial interpreter was first conceived as a means to access and control the Arduino language functions from a serial link, such as digitalWrite, digitalRead, analog, delay, delaymicroseconds, serialPrint etc.

Later arithmetic, logical, looping and program flow operations were added to extend the functionality.

I describe it as a toolkit, because it has the means to customise it to your own application. Every ascii character, except numerals is treated as a bytecode command. Numeric strings are converted into 16-bit intergers and placed on the stack. They are used as parameters to control the operation of the command.  

For example 1000m will invoke a delay of 1000 milliseconds - ideal for timing operations, flashing LEDs, generating audio tones etc.

SIMPL appears as a series of short ascii phrases For example:

Make a LED on pin 13 flash 10 times, 100mS on and 1000mS off - just 20 characters terminated by a CRLF

13d10{1o100m0o1000m}

13d  - select digital pin 13

10{  - set the loop counter to 10 and begin the loop

1o   - output 1 (HIGH) to the selected pin

100m - delay for 100 milliseconds

0o   - output 0 (LOW) to the pin

1000m  delay for 1000 milliseconds

}    - loop back to the beginning until the loop counter = 0

If you want to keep this code snippet, you can allocate it to a User Command, normally an uppercase character A-Z. Here we use F for flash

:F13d10{1o100m0o1000m}

Every time you type F the mcu will flash the LED 10 times. The code behind F can be redefined at any time to change the flash behaviour

FFFF will execute the code 4 times - flashing the LED 40 times.

SIMPL code is very compact. A lot can be performed using short snippets, taking up very small memory resources. It is the equivalent of a programming "Shorthand" - designed to be quick and concise but still remain human readable.

It gives you the ability to define your own instuction set to suit your application - whether you want to exercise hardware to flash a few LEDs or control an X-Y positioning system.

Scripted commands can be typed in directly followed by a CRLF, or you can send a sequence of commands line by line from a text file, using the Send File option on your terminal programme. 

SIMPL is a minimalist shell to help bring up new microcontroller hardware. The code can be written in C or in assembly language if you require a more compact solution.

SIMPL has been influenced by Forth by putting the numerical parameters on the stack prior to executing the function. However it has removed the complexity of creating a dictionary and parsing and looking up multi character commands. Every valid ascii character is treated as a jump to an action routine, before returning back to the interpreter.

The later versions of SIMPL are self-hosting - you can define your own language to match the requirements of your application.

SIMPL normally uses between 1024 and 2048 bytes of flash ROM to implement. It has been ported to a small MSP430 with only 512 bytes of RAM.

Most of the SIMPL development was done using the Arduino IDE and a compatible microcontroller.

The SIMPL interpreter is about 300 lines of C code that allow you to control the peripherals of a microcontroller from a serial terminal connection.

It enables interaction with the hardware and peripherals using a set of serial commands - for example typing 13h will set digital pin 13 high and typing 13l will set it low. 

There are commands to access the ADC channels, perform integer arithmetic and print numerical results and strings to the terminal.

SIMPL uses single ascii characters as commands to control the hardware - and commands can be strung together to make new commands - a litte bit like FORTH.

There are 96 printable ascii characters, but omitting numbers 0-9 and backspace limits SIMPL to 85 unique commands.

A SIMPL interpreter shell can be written in about 300 bytes of assembly language. In addition to this are the function routines - so a typical implementation is about 1k to 2k bytes of ROM. 

The shell can also be written in C, or using the Arduino C++ language - but the ROM size will be significantly larger. Many of the Arduino reference language functions use a significant amount of ROM.

Implementation.

SIMPL can be created from half a dozen basic function routines.

getchar - get the next character from the text input buffer

putchar - send a character to the terminal

number  - parse a string of numerical characters until a non-numeric character (eg space) convert the string to a 16-bit input and put it on the stack

printnumber - output a 16-bit integer to the serial terminal in the form of a decimal numeric string

lookup  - use the value of the ascii character to index into a lookup table or switch-case structure.

Obtain the Code Field Address(CFA) associated with that character.
      
Jump to and execute the code at the CFA. Jump to next at the end of the function code
          
next      Read the next serial character and process it.          

The underlying philosophy of SIMPL is to make the "mechanics" of the interpreter as simple as possible, and to keep the bytecount as low as possible.

The means that it is relatively easy to port from one mcu to another.

SIMPL has been used as a "bring-up" language for a number of mcus - including ATmega, MSP430, ARM Cortex M0, M4, M7.

It has also been used to explore and exercise new microcontroller architectures that have been created as software simulations.


; Serial Interpreted Microcontroller Programming Language

;-------------------------------------------------------------------------------

; SIMPL - a very small Forth Inspired Extensible Language 



; Type ? to get list of commands

; Type H or M to get a test message

; Ken Boak   May to September 2017

; Loops, I/O, Strings and Delays added

Here are some quick instructions - it uses lower case commands usually preceded by an integer number 32767 max

1234p   Print the number 1234

14sp    Take an ADC sample from pin 14 and print it as an integer

_Hello World_   Print the text string contained between the underscores

47d     Define the GPIO pin to be used for input or output

1o      Output logic 1 to the previously defined output pin

0o      Output logic 0 to the previously defined output pin

1000m   A delay of 1000 milliseconds

100u    A delay of 100 microseconds

b       Print the current value of the millisecond counter

c       Print the current valule of the microsecond counter

{ }     Create a loop structure

k       The current value of the loop counter - decrements each time around the loop

Make a LED on pin 47 flash 10 times

47d10{1o100m0o100m}

1o and 0o can be replaced with h and l (HIGH and LOW)

47l Initially set LED pin low

10{h200ml200m}      Flash the LED



1000{1o250u0o250u}  Create a short audio tone on pin 47

100{14sp}    Take 100 ADC samples on pin 14 (AN0) and print them out

10 11+p      Add 10 and 11 and print the result


;		Upper case letters are used to define Users "words"

;		User Routines are defined by capital letters starting  with colon : and end with semicolon ;

;		eg  :F10(100mh200ml);		;	Flash the led 10 times - high for 100mS and low for 200mS

;		You can play sequences of notes through a small speaker  ABC etc

;   :A40{h1106ul1106u);		 musical note A

;   :B5{h986ul986u);			 musical note B

;   :C51{h929ul929u);			 musical note C

;   :D57{h825ul825u);			 musical note D

;   :E64{h733ul733u);			 musical note E

;   :F72{h690ul691u);			 musical note F

;   :G81{h613ul613u);			 musical note G

;   :H_Hello World, and welcome to SIMPL_;   A Banner Message

;   Examples of SIMPL phrases

; 	eg add 123 and 456 and print the result to the terminal

; 	123 456+p

;	  Loop 10 times printing "Spurs are Fab!"

; 	10(_Spurs are Fab!_)

;   Flash a LED 10 times 100mS on 200mS off

;   10(h100ml200m)

;   Toggle a port pin at 1MHz   1000(hlhlhlhlhlhlhlhlhlhl)



;	Lower case letters are used for more complex commands

; a

; b      print the milliseconds counter - for timing durations

; c      print the microseconds counter

; d      Define an I/O pin

; e

; f

; g

; h       set port pin high

; i       input byte from port

; j

; k       access the loop counter variable

; l       set port pin low

; m       milliseconds delay

; n

; o       output bit to port

; p       print the top of stack to terminal

; q       print the ascii character at given RAM location

; r       read input pin

; s       sample the ADC

; t

; u       microseconds delay

; v

; w

; x

; y

; z

; Maths operators +  - * and / are recognised




