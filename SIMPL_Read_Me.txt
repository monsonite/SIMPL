; Serial Interpreted Microcontroller Programming Language
;-------------------------------------------------------------------------------
; SIMPL - a very small Forth Inspired Extensible Language in under 1024 bytes
; Implementing the Initialisation, TextTead, TextEval and UART routines in MSP430 assembly language
;
; Ken Boak   May to September 2017

; Loops, I/O, Strings and Delays added
; Jump table reduced by 36 entries (72 bytes)
; times_32 subroutine further reduces codesize
; This version 860 bytes

; Input and output to port P2 of Launchpad added with "i" and "o" commands

; SIMPL_430ASM_15

; Primitive Instructions

; These allow basic maths an logical instructions on 16-bit integers     +  -  /  *  &  |   ^   ~
; Stack Manipulation   DUP DROP PUSH POP SWAP OVER
; Memory transfers with FETCH and STORE
; Compilation mode with : and ;
; Simple decrementing loops   (..........)
; Input and Output
; Print a string  _Hello World_

; Note as of 13/06-2017 - not all of these are fully implemented


;	ADD       +
;	SUB       -
;	SHR       /
;	SHL       *
;	AND       &
;	OR        |
;	XOR       ^
;	INV       ~
;	DUP       “
;	DROP      `
;	PUSH      ,
;	POP       ‘
;	SWAP      $
;	OVER      %
;	FETCH     @
;	STORE     !
;	CALL      :
;	RETURN    ;
;	JMP       \
;	JE        =
;	JGT       >
;	JLT       <
;	TO-R      {
;	FROM-R    }
;	LOOP-Strt (
;	LOOP-End  )
;	IN        [
;	OUT       ]
;	KEY       ?
;	PRINT     _
;       NOP       Space
;       LIT       #


;	Lower case letters are used for more complex commands

;a
;b
;c
;d
;e
;f
;g
;h       set port pin high
;i       input byte from port
;j
;k       access the loop counter variable
;l       set port pin low
;m       milliseconds delay
;n
;o       output byte to port
;p       print the to of stack to terminal
;q       print the ascii character at given RAM location
;r       read input pin
;s       sample the ADC
;t
;u       microseconds delay
;v
;w
;x
;y
;z


;		Upper case letters are used to define Users "words"

;		User Routines are defined by capital letters starting  with colon : and end with semicolon ;

;		eg  :F10(100mh200ml);		;	Flash the led 10 times - high for 100mS and low for 200mS

;		You can play sequences of notes through a small speaker  ABC etc

;   :A40{h1106ul1106u);		        musical note A
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


;------------------------------------------------------------
