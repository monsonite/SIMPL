# SIMPL

# Introduction.

A serial, interpreted, minimal programming language - adapted for various microcontrollers including Arduino, ARM, MSP430 and experimental FPGA/simulated soft core processors.

SIMPL is intended to be a minimal, human readable scripting language based on the printable ascii character set. 

It provides a serial interface to a microcontroller using easy to remember, single character commands, and a simple interface to sent text strings and numerical output to the screen.

It can be applied to almost any cpu architecture.

# History and Inspiration

It started in 2009 when I required an interactive control method for microcontrollers from a serial terminal.

As a hardware engineer and only an occasional programmer it was driven by the underlying requirements of keeping the onchip resources to an absolute minimum and the interpreter code as simple as possible.

It began with a serial command interpreter for exercising Arduino hardware.

A single uppercase alpha character followed by a numerical parameter, allowed me to step between various routines coded into ROM. 

This was inspired the hex-monitor/debug programs of the mid-1970s to 1980s where uppercase characters followed by a 16-bit address or 8-bit data word allowed you to Examine, Modify and Run programs stored in RAM. 

WozMon on the Apple I was representative of this class of programs. 

https://steckschwein.de/2018/07/22/wozmon-a-memory-monitor-in-256-bytes/ 

A very small interpreter, few commands and tight assembley language programming allowed a useful hex editor in fewer than 256 bytes of code - fitting into a 256byte PROM on the Apple I.  

What started as a variant on a hex monitor has evolved into an interactive language toolkit for exercising a virtual machine VM.

Historical documentation can be found in my blog "Sustainable Suburbia" starting in May 2013: 

http://sustburbia.blogspot.com/2013/05/txtzyme-minimal-interpreter-and.html

At all times I apply the KISS principle - Keep it SIMPL, stupid.

# txtzyme

The breakthrough I needed for what became SIMPL was inspired by Ward Cunningham's Txtzyme nano-interpreter https://github.com/WardCunningham/Txtzyme which I first encountered in May of 2013. I combined Ward's elegant and compact nano-interpreter with my uppercase serial command interpreter.

A compact version of Ward's txtzyme that will run on an ATmega328 Arduino is located here:

https://github.com/monsonite/SIMPL/blob/master/txtzyme_1.ino

It uses 4418 bytes of flash ROM, much of which is attributed to the Arduino I/O functions. 

Many of these can be rewritten for brevity. The baudrate is 115200.

Ward had written a compact interpreter in Arduino C++ which allowed the Arduino's peripheral I/O functions to be accessed using only 13 primitive commands all operating with a 16-bit "register" x :

p print the number contained in x as a 16-bit integer

d define a digital pin

i test the digital pin for input

o set the digital pin either logic 1 or 0

m a millisecond delay

u a microsecond delay

{ start a loop

} end a loop

k read the loop counter

s sample the ADC

_ Print out the text enclosed between the underscored e.g. _Hello World_

h Print out a list of commands and basic help

v Print out the version of the mcu


An early version of Ward's Txtzyme interpreter can be seen here - only about 100 lines of C code:

https://github.com/WardCunningham/Txtzyme/blob/master/Arduinozyme/Arduinozyme.ino

It immediately became apparent that Ward had created the REPL in 2 simple routines contained within the main loop:

txtRead - If serial character present, put it into next location of a serial input buffer - until CRLF pressed

txtEval - Take character from input buffer, identify using switch-case structure and perform action. Loop back to txtRead

It was obvious that by extending the switch-case statement within txtEval, that many more commands could be added. So I added maths, logic and comparison operations and a second variable "y". For example:

123 456 + p       // Put 123 in x, 456 in y, ADD them together in x and print out x followed by a CRLF

Maths operators + - * and / and logical operations & | ^ and ~ were added at this time.

I then started thinking about conditional execution.  Ward had already provided an example of how a snippet of code would be conditionally executed depending on whether a port pin was read as 0 or 1. 

This made ingenious use of his loop structure - code enclosed between braces {........}

9{........}   Repeat the enclosed code 9 times

1{........}   Execute the enclosed code once

0{........}   Skip the enclosed code

An unexpended bonus of this was that comments could be added using a comment in braces preceded by a zero:

0{ This is a comment that will be ignored by interpreter }

Any operation that results in x being 1 or 0 can be used to determine whether the enclosed code is executed or not.

This allowed a simple mechanism for the comparison operators:

Less than <

Equal to =

Greater than >


# Extensibility

Having come from a Forth background, I was familiar how Forth can be extended with new user functions using the "Colon Definition"

Forth uses a dictionary structure and a dictionary look-up to find the location of the code to execute for a particular function. I decided that this could be massively simplified, by restricting the user to only 26 user functions, allocated to the uppercase characters A-Z.

The second big breakthrough was realising that I could point the interpreter to any address in RAM, and not just the input buffer, and have it execute the ascii-based code located there. This gave a simple mechanism for users to write their own SIMPL code into RAM.

You can effectively point the txtEval function to any buffer in memory and it will execute the virtual commands found there until it finds a CRLF.

# Other Influences      

SIMPL builds upon Ward's philosopy of keeping things simple, but adds significantly more functionality and provides the means to make a user extensible language.

SIMPL was also heavily influenced by Charles H. Moore's Forth. It uses the same reverse polish notation RPN,  placing the operands on the stack before executing the operator function.

SIMPL has it's roots in the first electronic computers of the late 1940s where the instruction set was based on uppercase alphabetic characters read from a teletype paper tape into memory. Teletype character sets were limited to uppercase alpha characters, numerals and a few punctuation symbols.

Each character was chosen to have a strong mnemonic value so that it was easily remembered, and made code listings easier to read and write - with fewer mistakes. 

See the Cambridge EDSAC from 1948 for examples of an alpha character instruction set:  https://www.cl.cam.ac.uk/~mr10/Edsac/edsacposter.pdf


SIMPL has evolved as a human readable pseudo-language to control a 16-bit virtual machine based on a Minimal Instruction Set Computer (MISC). If the interpreter is written in C it is easily extended to accommodate 32-bit words.

SIMPL performs the Read-Eval-Print Loop (REPL) required by a serial interpreter shell.  

A MISC is a computer architecture typically defined by a very small instruction set - often with only 8 to 32 instructions. 

One example is the PDP-8, a 12-bit machine from 1965 which had 8 primitive instructions plus the means to control operations on it's accumulator by directly decoding the bitfield from the instruction word. Despite the lack of instructions, the PDP-8 was a versatile machine with it's hardware deficiencies being made up for in software macros.

Additionally, new User functions can be built up by concatenating exisisting functions - this makes SIMPL extensible.

Unlike Forth, there is no dictionary and this avoids the overheads of additional parsing and dictionary searches. SIMPL can be written in about 2kbytes or less, whereas Forth may require 6k bytes. This makes it quicker and easier to port to the assembly language of an unfamiliar microcontroller.

The use of single ascii character commands means that a function can be executed with nothing more than a table look-up to obtain an execution address.

SIMPL is effectively a human readable language for implementing a 16-bit Von Neumann virtual machine on top of the native assembly language of the cpu.

It can be applied to any resource limited processor, it is easier to read and write than assembler but not as complex as a full Forth implementation. 


# Functionality.


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

# Further Thoughts

SIMPL code is very compact. A lot can be performed using short snippets, taking up very small memory resources. It is the equivalent of a programming "Shorthand" - designed to be quick and concise but still remain human readable.

Most User Definitions are going to be fewer than 32 characters long. A source code editor could be implemented easily using a 40 character x 25 row screen of text.

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


# Implementation.


SIMPL can be created from half a dozen basic function routines. These implement the Read-Eval-Print Loop REPL

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



Here are some quick instructions - it uses lower case commands usually preceded by an integer number 65536 max

p      print out the number on the top of the stack followed by a carriage return

1234p   Print the number 1234

14sp    Take an ADC sample from pin 14 and print it as an integer

_Hello World_   Print the text string contained between the underscores

47d     Define the GPIO pin to be used for input or output

1o      Output logic 1 to the previously defined output pin

0o      Output logic 0 to the previously defined output pin

1000m   A delay of 1000 milliseconds

100u    A delay of 100 microseconds

b       Print the current value of the millisecond counter - useful for timing how long functions take

c       Print the current valule of the microsecond counter - useful for timing how long functions take

{ }     Create a loop structure

k       The current value of the loop counter - decrements each time around the loop

10{kp}   Print out the value of the loop counter as it decrements

Make a LED on pin 47 flash 10 times

47d10{1o100m0o100m}

1o and 0o can be replaced with h and l (HIGH and LOW)

47d10{h100ml1000m}

47dl Initially set LED pin low

10{h200ml200m}      Flash the LED 10 times  200mS on, 1200mS off

Audio Tones, connect a small speaker to the output pin - use u the microsecond delay function

1000{1o250u0o250u}  Create a short audio tone on pin 47

100{14sp}    Take 100 ADC samples on pin 14 (AN0) and print them out

10 11+p      Add 10 and 11 and print the result

Maths operators +  - * and / are recognised

Logic operators & | ^ and ~  (AND OR XOR NOT) are recognised



;		Upper case letters are used to define Users "words"

;		User Routines are defined by capital letters starting  with colon : and end with semicolon ;

;		eg  :F10(100mh200ml);		;	Flash the led 10 times - high for 100mS and low for 200mS

;		You can play sequences of notes through a small speaker  ABC etc

;   :A40{h1106ul1106u);		       musical note A

;   :B5{h986ul986u);			 musical note B

;   :C51{h929ul929u);			 musical note C

;   :D57{h825ul825u);			 musical note D

;   :E64{h733ul733u);			 musical note E

;   :F72{h690ul691u);			 musical note F

;   :G81{h613ul613u);			 musical note G

;   :H_Hello World, and welcome to SIMPL_;   A Banner Message

;   Examples of SIMPL phrases

;   eg add 123 and 456 and print the result to the terminal

;   123 456+p

;   Loop 10 times printing "Spurs are Fab!" Curley brackets { and } are used to enclose the code to be repeated within the loop

;   The number outside the opening brace is the loopcounter. The contents of the loop will be executed until the loopcounter is decremented to zero  

;   10{_Spurs are Fab!_}

;   Flash a LED 10 times 100mS on 200mS off

;   10{h100ml200m}

;   Toggle a port pin at 1MHz   1000{hlhlhlhlhlhlhlhlhlhl}

;--------------------------------__LIST OF CHARACTERS, FUNCTIONS AND OPERATORS__-----------------------------------------------

;	Lower case letters are used for more complex inbuilt commands executed from ROM. Here are some suggestions - but not extensive



; a

; b      print the milliseconds counter - for timing durations

; c      print the microseconds counter

; d      Define a digital I/O pin

; e

; f

; g

; h       set port pin high

; i       input byte from port

; j

; k       access the loop counter variable

; l       set port pin low

; m       milliseconds delay

; n       Ouput an 8-bit binary number on 8 consecutive port pins

; o       output bit to port

; p       print the top of stack to terminal

; q       print the ascii character at given RAM location

; r       read input pin

; s       sample the ADC

; t       Toggle a port pin - equivalent to {hl}

; u       microseconds delay

; v

; w

; x       Three registers that form a stack

; y       Three registers that form a stack

; z       Three registers that form a stack

;         Sybolic Operators

; +       ADD

; -       SUB

; *       MUL

; /       DIV

; &       AND

; |       OR

; ^       XOR

; ~       NOT INVert

; @       Fetch

; !       Store

; "       Stack operation DUP

; '       Stack operation DROP

; $       Stack operation SWAP

; %       Stack operation OVER

; :       Start colon definition

; ;       End colon definition

; #       16-bit literal

; <       LESS than

; =       EQUAL

; >       GREATER than

; \       Comment   \This is a comment

; ,       Array separator

; .       Print top of stack (same as p)

;         Conditional code

; (       Start an array separated by commas

; )       End of array

; {       Start a Loop

; }       End of Loop

; Start a switch-case [

; End a switch-case   ]

; _Hello World_       Print a text string enclosed inside the underscores








