;-------------------------------------------------------------------------------
; SIMPL - a very small Forth Inspired Extensible Language
; Implementing the Initialisation, TextTead, TextEval and UART routines in MSP430 assembly language
;
; A Forth-Like Language in under 1024 bytes

; Ken Boak   May June 2017

; Loops, I/O, Strings and Delays added
; Jump table reduced by 36 entries (72 bytes)
; times_32 subroutine further reduces codesize


; This version 860 bytes    9600 baud communications with 1MHz DCO

; Input and output to port P2 of Launchpad added with "i" and "o" commands

; SIMPL_430ASM_16

; Primitive Instructions

; These allow basic maths an logical instructions on 16-bit integers     +  -  /  *  &  |   ^   ~
; Stack Manipulation   DUP DROP PUSH POP SWAP OVER
; Memory transfers with FETCH and STORE
; Compilation mode with : and ;
; Simple decrementing loops   (..........)
; Input and Output
; Print a string _Hello World_

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
;	NOP       Space
;   LIT       #


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

;		:A40{h1106ul1106u);			 musical note A
;		:B5{h986ul986u);			 musical note B
;       :C51{h929ul929u);			 musical note C
;       :D57{h825ul825u);			 musical note D
;       :E64{h733ul733u);			 musical note E
;       :F72{h690ul691u);			 musical note F
;       :G81{h613ul613u);			 musical note G
;       :H_Hello World, and welcome to SIMPL_;   A Banner Message

;A
;B
;C
;D
;E
;F
;G
;H
;I
;J
;L
;M
;N
;O
;P
;Q
;R
;S
;T
;U
;V
;W
;X
;Y
;Z



;   Examples of SIMPL phrases

; 	eg add 123 and 456 and print the result to the terminal

; 	123 456+p

;	Loop 10 times printing "Spurs are Fab!"

; 	10(_Spurs are Fab!_)

;   Flash a LED 10 times 100mS on 200mS off

;   10(h100ml200m)


;   Toggle a port pin at 1MHz   1000(hlhlhlhlhlhlhlhlhlhl)

;	_____________________________________________________________________

;-------------------------------------------------------------------------------
            .cdecls C,LIST,"msp430.h"     ; Include device header file

;-------------------------------------------------------------------------------
            .def    RESET                   ; Export program entry-point to
                                            ; make it known to linker.
;-------------------------------------------------------------------------------
; Variables
;-------------------------------------------------------------------------------
			.sect "vars"

			.bss parray, 256

 			.bss x, 2
			.bss name, 2

;-------------------------------------------------------------------------------
 ; Using the register model of CH Ting's Direct Thread Model of MSP430 eForth
 ; CPU registers

 ; Register Usage

; R0    MSP430 PC Program Counter
; R1    MSP430 SP Stack Pointer
; R2    MSP430 SR Status Register

tos 			.equ R4
stack 			.equ R5
ip 				.equ R6			; instruction pointer
temp0 			.equ R7			; loop start
temp1 			.equ R8			; loop counter k
temp2 			.equ R9			; millisecond delay
temp3 			.equ R10        ; microsecond delay
temp4 			.equ R11		; scratch
instr 			.equ R12		; instruction fetched into R12 for decoding
temp5 			.equ R13		; arithmetic / scratch
temp6 			.equ R14		; used for buffer pointer
temp7 			.equ R15	    ; Return from alpha  next IP

;-------------------------------------------------------------------------------
; Macros

pops 			.macro     	;DROP
				mov.w @stack +, tos
				.endm

pushs 			.macro 		;DUP
				decd.w stack
				mov.w tos, 0(stack)
				.endm;

; Constants

$NEXT 			.macro
				jmp next
;				mov @ip+, pc ; fetch code address into PC
				.endm


; $NEST 			.macro
;				.align 2
;				call #DOLST ; fetch code address into PC, W = PFA
;				.endm

;$CONST 			.macro
;				.align 2
; 				call #DOCON ; fetch code address into PC, W = PFA
;				.endm

;------------------------------------------------------------------------------
;; Assembler constants - not used in this versionb

COMPO 		.equ 040H 		;lexicon compile only bit
IMEDD 		.equ 080H 		;lexicon immediate bit
MASKK 		.equ 07F1FH 	;lexicon bit mask
CELLL 		.equ 2 			;size of a cell
BASEE 		.equ 10 		;default radix
VOCSS 		.equ 8 			;depth of vocabulary stack
BKSPP 		.equ 8 			;backspace
LF 			.equ 10 		;line feed
CRR 		.equ 13 		;carriage return
ERR 		.equ 27 		;error escape
TIC 		.equ 39 		;tick
CALLL 		.equ 012B0H 	;NOP CALL opcodes
UPP 		.equ 200H
DPP 		.equ 220H
SPP 		.equ 378H 		;data stack
TIBB		.equ 380H 		;terminal input buffer
RPP 		.equ 3F8H 		;return stacl
CODEE  		.equ 0C000H 	;code dictionary
COLDD  		.equ 0FFFEH 	;cold start vector
EM  		.equ 0FFFFH 	;top of memory




;-------------------------------------------------------------------------------
            .text                           ; Assemble into program memory.
            .retain                         ; Override ELF conditional linking
                                            ; and retain current section.
            .retainrefs                     ; And retain any sections that have
                                            ; references to current section.

;-------------------------------------------------------------------------------------------------------------------
; We now come to decoding the characters - where based on the value of the character we perform some action routine
; But first we need to determine whether the characers form part of a number
; these must be decoded separately and put on the stack - using the "number" routine
;-------------------------------------------------------------------------------------------------------------------

; Register Usage

; ip  - instruction pointer to the current character in the input buffer
; R12   is the accumulator for the number - then stored in location #0x380
; R13   Temporary - use in x10 multipication
; R14


; 17 Instructions



number:
				CMP.B   #0x19,R13				; Is it greater than '9'
				JHS		mod_pc


				SUB.W   #0x0030,R12				; subtract 0x30 to get a decimal number

number1: 		CMP.B   #0x0030,0x0000(ip)		; >= '0'   Is the next digit a number
				JLO     endNumber				; break
				CMP.B   #0x003a,0x0000(ip)		; <= '9'
				JHS     endNumber				; break

times_10:										; This multipies R12 by 10

				ADDC.W	R12,R12                 ; R12 = 2 * R12
				MOV.W	R12,R13				    ; R13 = 2 * R12
				ADDC.W	R12,R12				    ; R12 = 4 * R12
				ADDC.W	R12,R12				    ; R12 = 8 x R12
				ADDC.W  R13,R12                 ; R12 = 10 x R12


				MOV.B   @ip+,R14			    ; Increment the instruction pointer fetching the next digit
				SUB.W   #0x0030,R14				; Subtract 0x30 to make it a decimal between 0 and 9
				ADD.W   R14, R12			    ; Add in the next digit
				JMP     number1					; process the next digit

endNumber:		MOV.W   R12, tos				; Put the number in tos - the top of stack
;				JMP     next				    ; process the next character







; -------------------------------------------------------------------------------------------------------------------

; Character is either a primitive or an alpha - so form CALL address
; Restore R14 to start of RAM buffer
; Get the current character location
; If it's a primitive between 0x20 and 0x3F - point to a look-up table and fetch it's code segment address
; If its an Alpha, or character >0x40 calculate it's code address from (char - 65)x32
; Character is in R13 so  calculate the destination address


; next fetches the next ascii character instruction from memory, decodes it into a jump address and executes the code
; found at that code address
; Each executed word jumps back to next
; Numbers are treated differenty - they are enummerated and put onto the stack by the number routine

; Now we need to decode the instructions using a jump table
; Jump table uses 2 bytes per instruction - 120 bytes
; Upper case letters and numbers have their own separate decode routines "alpha" and "number"


next:			MOV.B   @ip+,R12				; Get the next character from the instruction memory
				MOV.W   R12,R13				    ; Copy into R13 - as needed to decode Jump Address
				SUB.W   #0x0020,R13				; subtract 32 to remove offset of space

				CMP.B   #0x0021,R13				; Is it greater than ascii 64? Therefor Upper Case alpha
				JHS		alpha

				CMP.B   #0x0010,R13				; Is it greater than 0x10 therefore a number
				JHS		number

mod_pc:			ADD.W	R13,R13					; double it for word address
				ADD.W	R13,pc					; jump to table entry

jump_table:		jmp space			; SP
				jmp store			; !
				jmp dup				; "
				jmp lit				; #
				jmp swap			; $
				jmp over			; %
				jmp and				; &
				jmp drop			; '
				jmp left_par		; (
				jmp right_par		; )
				jmp mult			; *
				jmp add				; +
				jmp push			; ,
				jmp sub				; -
				jmp pop				; .
				jmp div				; /

; -------------------------------------------------------------------------------------------
; This section must be 20 bytes long in order to maintain the integrity of the jump table


OK:				MOV.W #0x4F, R12
				CALL 	#uart_putc			; output "O"
				MOV.W #0x4B, R12
				CALL 	#uart_putc			; output "K"						; Print OK
				ret

				nop



; ------------------------------------------------------------------------------------------


				jmp colon			; :
				jmp semi			; ;
				jmp less			; <
				jmp equal			; =
				jmp greater			; >
				jmp	query			; ?
				jmp fetch			; @


; -----------------------------------------------------------------------------------------------------------------------------
; This section must be 52 bytes long in order to maintain the integrity of the jump table


; UART Routines

uart_getc:		BIT.B   #1,&IFG2			; while (!(IFG2&UCA0RXIFG)) // USCI_A0 RX buffer ready?
				JEQ     (uart_getc)
				MOV.B   &UCA0RXBUF,R12		; return UCA0RXBUF;
				ret


uart_putc:		BIT.B   #2,&IFG2			; while (!(IFG2&UCA0TXIFG))  // USCI_A0 TX buffer ready?
				JEQ     (uart_putc)
                MOV.B   R12,&UCA0TXBUF		; UCA0TXBUF = c;    // TX
				ret


prompt:			MOV.W #0x3E, R12
				CALL 	#uart_putc			; output ">"
				ret


crlf:			MOV.W #0x0A, R12
				CALL 	#uart_putc			; output CR
				MOV.W #0x0D, R12
				CALL 	#uart_putc			; output LF
				ret


; -----------------------------------------------------------------------------------------------------------------------------


				jmp	square_left		; [
				jmp f_slash			; \ ;
				jmp square_right	; ]
				jmp xor				; ^
				jmp underscore		; _
				jmp tick			; `
				jmp lower_a			; a
				jmp lower_b			; b
				jmp lower_c			; c
				jmp lower_d			; d
				jmp lower_e			; e
				jmp lower_f			; f
				jmp lower_g			; g
				jmp lower_h			; h
				jmp lower_i			; i
				jmp lower_j			; j
				jmp lower_k			; k
				jmp lower_l			; l
				jmp lower_m			; m
				jmp lower_n			; n
				jmp lower_o			; o
				jmp lower_p			; p
				jmp lower_q			; q
				jmp lower_r			; r
				jmp lower_s			; s
				jmp lower_t			; t
				jmp lower_u			; u
				jmp lower_v			; v
				jmp lower_w			; w
				jmp lower_x			; x
				jmp lower_y			; y
				jmp lower_z			; z
				jmp	curly_left		; {
				jmp or				; |
				jmp curly_right		; }
				jmp inv				; ~
				jmp delete			; del

				call #crlf
				call #prompt
				jmp textRead		; Loop back to Interpreter  0x80 is used as null terminator

;-----------------------------------------------------------------------------------------

; Handle the alpha and lower case chars

alpha:			CMP.B   #0x003A,R13				; Is it greater than Z ascii 90?
				JHS		mod_pc					; Then it should be handled by jump table mechanism

				call    #times_32			    ; subtract ascii "A" from R13 and multiply by 32 then add 0x220 offset

				MOV.W   ip,R15				    ; Save the current ip on the return stack R15
												; R13 now contains the jump address for the alpha code
				MOV.W	R13,ip					; instruction pointer

				JMP     next				    ; process the next character



;-----------------------------------------------------------------------------------------
; Handle the primitive instructions


push:
dup:
space:			pushs			; Move a 2nd number onto the stack
				$NEXT



store:

				mov.w @stack+, 0(tos)
				pops
				$NEXT


lit:

				$NEXT

swap:           mov.w tos, temp0
                mov.w @stack, tos
                mov.w temp0,0( stack)
                $NEXT

over:			mov.w @stack, temp0
				pushs
				mov.w temp0, tos
				$NEXT





and:			and @stack +, tos
				$NEXT


pop:
drop:			pops
				$NEXT



left_par:									; code enters here on getting a left parenthesis

				MOV.W   tos,R8				; save tos to R8  (R8 is the loop counter k)
				MOV.W   ip,R7				; loop-start = ip the current instruction pointer at start of loop
				JMP     next				; get the next character and execute it

right_par:									; code enters here if instruction it's a right parenthesis

				; TST.W   R8				; is loop counter zero
				; JEQ     next			    ; terminate loop

				DEC.W   R8					; decrement loop counter R8
				JEQ     next			    ; terminate loop
				MOV.W   R7,ip				; set instruction pointer to the start of the loop
				JMP     next				; go around loop again until loop counter = 0


mult:			add.w tos,tos				; left shift tos
				$NEXT


add:			add @stack +, tos
				$NEXT





sub:			sub @stack +, tos
NEGAT:			inv tos
				inc tos
				$NEXT



div:
				rra.w tos
				$NEXT

semi:			; On encountering a semicolon return program control to the next character in the input buffer
				call #OK
				call #crlf

				MOV.W	R15,ip			; restore the ip
				$NEXT


query:			$NEXT

fetch:			mov.b   @tos, R12
				mov.b	R12,tos
				$NEXT

square_right:

f_slash:

square_left:

curly_right:

curly_left:

underscore:								; Print the enclosed text


print_start:
				MOV.B   @ip+,R12	    ; Get the next character from the instruction memory
				CMP.B   #0x005f,R12     ; is it an underscore
				jeq     print_end
				CALL 	#uart_putc	    ; send it to uart
				jmp  	print_start
print_end		call    #crlf			; line feed at end of text string
				$NEXT


tick:			; tick allows access to the loop counter

;				MOV.W   R8,tos
;				$NEXT




delete:			$NEXT

or:				bis @stack +, tos
				$NEXT

xor:			xor 	@stack +, tos
				$NEXT

inv:			inv tos
				$NEXT

less:			cmp @stack +, tos
				jz FALSE
				jge TRUE
				jmp FALSE

equal:			xor @stack +, tos
				jnz FALSE
				jmp TRUE

greater:		cmp @stack +, tos
				jge FALSE
				jmp TRUE

FALSE: 			clr tos
				$NEXT

TRUE: 			mov #0x01, tos
 				$NEXT


;------------------------------------------------------------------------------------------------
;lower case routines

lower_a:

				$NEXT

lower_b:

				$NEXT

lower_c:

				$NEXT

lower_d:

				$NEXT

lower_e:

				$NEXT

lower_f:

				$NEXT

lower_g:

				$NEXT

lower_h:
				MOV.B   #0x0001,&P1OUT							; P1OUT  = BIT0   LED1 on
				$NEXT

lower_i:					; Read the switch on P1.3

		   mov.b   	&P2IN,tos        ; read switch at P1.3
		   $NEXT

           jc      Off                     ; if P1.3 open branch to Off label

On:          bic.b   #00000001b,&P1OUT       ; clear P1.0 (red off)
            bis.b   #01000000b,&P1OUT       ; set P1.6 (green on)
 ;           jmp     Wait                    ; branch to a delay routine

Off:
			bis.b   #00000001b,&P1OUT       ; set P1.0 (red on)
            bic.b   #01000000b,&P1OUT       ; clear P1.6 (green off)

				$NEXT

lower_j:

				$NEXT

lower_k:
				; k allows access to the loop counter variable stored in R8
				MOV.W   R8,tos
				$NEXT

lower_l:
				MOV.B   #0x0000,&P1OUT					; P1OUT  = BIT0   LED1 off
				$NEXT

lower_m:												; millisecond delay
				MOV.W   tos,R10
mS_loop:
				mov.w   #5232,R9						; 5232 gives 1mS at 16MHz
uS3_loop:		DEC.W	R9
				JNE		uS3_loop

				DEC.W	R10
				JNE		mS_loop
				$NEXT

lower_n:

				$NEXT

lower_o:												; output to port 2
				mov.b	tos,&P2OUT
				$NEXT

lower_p:
;-------------------------------------------------------------------------------------------------
; Take the 16 bit value in top of stack register and print to terminal as an integer
; do by repeated subtraction of powers of 10
; Uses R10,11,12,13
;-------------------------------------------------------------------------------------------------

printNum:			MOV.W	#10000,R10		; R10 used as the decimation register
					CLR.W   R11				; Use R11 as scratch
					CLR.W	R13
					MOV.W   tos,R12			; copy the top of stack into R12
					CLRC					; clear the carry

sub10K:				SUB.W   R10,R12
				    JLO      end10K

add10K:				ADD.B #1,R11			; increments the digit count
add_zero:			ADD.W R10,R13			; R13 increases by the decimal value each time
					JMP   sub10K

end10K:			;TST.W   R11				;  If R11 is zero - don't print leading zeros
				;JEQ	    skip_print
do_print:			ADD.B   #0x30,R11		; make it an ascii number character
					MOV.W   R11,R12
					CALL 	#uart_putc		; output character

				;JMP 	dec_stack
skip_print:		;TST.W	tos
				;JGE		do_print
dec_stack:			SUB.W   R13,tos			; Decrement the stack count by n x 10
					CLR.W   R11				; Use R11 as scratch
					CLR.W	R13
					MOV.W   tos,R12

decimate:			CMP.W   #10000,R10
					JEQ     use1K
					CMP.W   #1000,R10
					JEQ     use100
					CMP.W   #100,R10
					JEQ     use10
					CMP.W   #10,R10
					JEQ     use1

newline:
					call   #crlf

					JMP     next

use1K:				MOV.W	#1000,R10
					JMP		sub10K
use100:				MOV.W	#100,R10
					JMP		sub10K
use10:				MOV.W	#10,R10
					JMP		sub10K
use1:				MOV.W	#1,R10
					JMP		sub10K




lower_q:		mov.b @tos, R12
			;	MOV.B   tos,R12
				CALL 	#uart_putc

				$NEXT

lower_r:

				$NEXT

lower_s:

				$NEXT

lower_t:

				$NEXT

lower_u:									; 3 microsecond deelay

				MOV.W   tos,R10
uS_loop:
				DEC.W	R10
				JNE		uS_loop
				$NEXT

lower_v:

				$NEXT

lower_w:

				$NEXT

lower_x:

				$NEXT


lower_y:

				$NEXT

lower_z:

				$NEXT
;--------------------------------------------------------------------------------
; User Routines









;-------------------------------------------------------------------------------
; Main loop here
;-------------------------------------------------------------------------------

main:

;-------------------------------------------------------------------------------
RESET:       ;	mov.w   #03E0h,SP               ; Initialize stackpointer

				mov #RPP, SP 					; set up stack
				mov #SPP, stack
				clr tos

StopWDT:     	mov.w   #WDTPW|WDTHOLD,&WDTCTL  ; Stop watchdog timer WDTCTL  = WDTPW + WDTHOLD; Stop WDT


OSC_GPIO_init:	; Run the CPU at full 16MHz with 921600 baud UART

				MOV.B   &CALBC1_1MHZ,&BCSCTL1					;BCSCTL1 = CALBC1_16MHZ;  Set DCO
				MOV.B   &CALDCO_1MHZ,&DCOCTL 					;DCOCTL  = CALDCO_16MHZ;
;				MOV.B   #0x0000,&P1OUT							;P1OUT  = BIT0 + BIT6; 				// All LEDs off
SetupP1:     	bis.b   #041h,&P1DIR            				;P1.0 P1.6  output  as P1.0 and P1.6 are the red+green LEDs
SetupP2:		bis.b   #0FFh,&P2DIR							;Switches on P2.4 to P2.7, LEDs on P2.0 - P2.3
				MOV.B   #0x0006,&P1SEL							;Initialise the UART for 921600 baud
				MOV.B   #0x0006,&P1SEL2							;P1SEL2 = RXD + TXD;
				BIS.B   #0x0080,&UCA0CTL1						;UCA0CTL1 |= UCSSEL_2// SMCLK
				MOV.B   #0x0068,&UCA0BR0						;UCA0BR0 = 138 // 1MHz 9600
				CLR.B   &UCA0BR1								;UCA0BR1 = 0 // 1MHz 9600
				MOV.B   #2,&UCA0MCTL							;UCA0MCTL = UCBRS0  // Modulation UCBRSx = 1
				BIC.B   #1,&UCA0CTL1							;UCA0CTL1 &= ~UCSWRST Initialize USCI state machine

				call	#crlf
				call	#OK
				call	#crlf
				call 	#prompt
				call	#crlf




;-------------------------------------------------------------------------------
; This implements the SIMPL interpreter is MSP430 assembly Language


;-------------------------------------------------------------------------------
; textRead
; ------------------------------------------------------------------------------

; Get a character from the UART and store it in the input buffer starting at 0x0200

; Supports immediate mode - where characters are stored at 0x200 and executed directly
; Supports compiled mode - where string starts with a colon and the next character "NAME" is an upper case alpha
; String is stored at 0x200 plus a multiple of 32 byte offset according to the value of "NAME"

; Register Usage

; The input buffer - start is at 0x0200, which is pointed to by R14
; R11 is a counter to ensure that we don't exceed 64 characters in input buffer
; R12 receives the character from the uart_get_c routine and puts in the buffer, pointed to by R14
; R14 is the current character position in the input buffer


; 33 instructions


textRead:		MOV.W   #0x0200,R14			; R14 = start of input buffer in RAM
;				CLR.B   R11					; i = 0

getChar:	   	CALL    #uart_getc    		; char ch = uart_getc()
				CMP.B   #0x000d,R12			; is it carriage return?  0d
                JEQ     textEnd
				CMP.B   #0x000a,R12     	; Is it  newline?  0a
                JEQ     textEnd
				CMP.B   #0x0020,R12     	; if (ch >= ' ' && ch <= '~')
                JLO     nonValid
				CMP.B   #0x007f,R12			; this looks for 0x80 as null terminator
				JHS     nonValid
				CMP.B   #0x003A,R12			; is it colon?  0x3A
				JNE     notColon

colon:			; If the input character is a colon

				CALL    #uart_getc    	    ; get the next character - which is the NAME
				call    #times_32			; subtract ascii "A" from R13 and multiply by 32 then add 0x220
;				ADD.W   R13,R14				; Add (32*R13) to the index pointer R14
;				ADD.W   #0x020,R14			; Add to array pointer  0x0220
				MOV.W	R13,R14
				MOV.B   R12,0x0000(R14)     ; Store character at RAM buffer indexed by R14
											; R14 now contains the destination address
				JMP     incPointer

notColon:		INC.W   R14			    	; Increment buffer pointer
				MOV.B   R12,0xffff(R14) 	; Store character at RAM buffer indexed by R14

incPointer:  ;		INC.B   R11			    	; Increment the input buffer counter ;

nonValid:		;CMP.B   #0x003f,R11			; If input pointer <64 loop back to start
               ; JLO     getChar		    	; loop back and get next character
                jmp 	getChar

textEnd:		mov.b	#0x80,0x0000(R14)	; Put a null terminating (0x80) zero on the end of the buffer

  				MOV.W   #0x0200,ip			; set ip (instruction pointer) - to start of input buffer in RAM at address 0x0200
				jmp     next			    ; get the next instruction






times_32:		MOV.B   R12,R13			    ; move the 1st character after the colon to "name" variable in R13
				SUB.B   #0x0041,R13			; Calculate the destination address - subtract 65 to remove offset of letter A
				ADD.W   R13,R13				; Double R13	; multiply by 2
				ADD.W   R13,R13				; Double R13    ; multiply by 4
				ADD.W   R13,R13				; Double R13	; multiply by 8
times_4:		ADD.W   R13,R13				; Double R13	; multiply by 16
				ADD.W   R13,R13				; Double R13	; multiply by 32
				ADD.W   #0x220,R13			; Add start address of command RAM (0x0220) to  (32*R13)

				ret


; Stack Pointer definition
;-------------------------------------------------------------------------------
            .global __STACK_END
            .sect   .stack

;-------------------------------------------------------------------------------
; Interrupt Vectors
;-------------------------------------------------------------------------------
            .sect   ".reset"                ; MSP430 RESET Vector
            .short  RESET

			.end
