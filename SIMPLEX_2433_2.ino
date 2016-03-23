#include <msp430.h> 
// SIMPLEX 2_MSP430FR4133_5
// - inspired by Txtzyme Nano Interpreter -  an original idea by Ward Cunningham

// An on going Neo-Retro Computing Mission!
// Ken Boak  March 9th 2016

// SIMPL Interpreter for MSP430FR4133 Launchpad

// Modified 18-3-2016 to run on MSP430FR2433 Module

// This version adds 23K256 32Kx8 external SPI RAM, plus hex-dump utlity
// With addition of timing routines and other debug
// MSP430FR4133 UART, SPI and LCD suport Routines
// - codesize is now 4167 bytes

// This version for FRAM based MSP430 with MSP430FR4133 Launchpad compiled using Energia
// Note - serial uart on P1.1 and P1.2 is running at odd-baud  of 100,000 bits/s  - to be investigated

// This version includes the t and v functions to determine timing tests - note  millis is running at 16 times speed!
// Any time entered for m or u is multiplied by 16
// Any time printed out from b or c id divided by 16!
// There is a debug pin on P1.0 (Green LED) which allows scope probe attachment to confirm times

// RAM connections
/*
              +------U------+
P8.0  /CE     |   23K256    | 3V3
P5.2  MISO    |             | HOLD - pull up to 3V3
      NC      |             | SCK  - P5.1
      0V      |             | MOSI - P5.2
              +-------------+
*/
// Timing

// 1,000,000 empty loops in 2.4uS per iteration
// 100,000,000 empty loops in 2.4uS per iteration

//#include <MSP430FR4133.h>
//#include <MSP430FR2433.h>

#define        RXD	       BIT1                // Receive Data (RXD) at P1.1
#define        TXD	       BIT2                // Transmit Data (TXD) at P1.2
#define        RED             0x20                // Red LED is on Bit 6
#define        GREEN           0x01                // Green LED is on Bit 0
//#define        SS_PIN          BIT4                // CS , active low
#define        SS_PIN          BIT0                // CS , active low  Port 8.0
#define        DEBUG_PIN       BIT0                // toggle on and off marking time to write
#define        DUMMY_BYTE      0xFF                // byte we send when we just want to read slave data
//#define        ssSelect        P1OUT &= ~SS_PIN
//#define        ssDeselect      P1OUT |= SS_PIN
#define        ssSelect        P3OUT &= ~SS_PIN
#define        ssDeselect      P3OUT |= SS_PIN

#define delay_1ms __delay_cycles(16000)
#define bufRead(addr)       (*(unsigned char *)(addr))
#define bufWrite(addr, b)   (*(unsigned char *)(addr) = (b))
#define bit0 0x01   // 1
#define bit1 0x02   // 2
#define bit2 0x04   // 4
#define bit3 0x08   // 8
#define bit4 0x10   // 16
#define bit5 0x20   // 32
#define bit6 0x40   // 64
#define bit7 0x80   // 128
#define SR_WRITE_STATUS 0x01
#define SR_WRITE        0x02
#define SR_READ         0x03
#define SR_READ_STATUS  0x05
#define BYTES_TO_STREAM 1024     // should be less <= 32768
#define PATTERN_BYTE_VALUE 65

//---------------------------------------------------------------------------------
// LCD defines

#define pos1 4                                                 // Digit A1 - L4
#define pos2 6                                                 // Digit A2 - L6
#define pos3 8                                                 // Digit A3 - L8
#define pos4 10                                                // Digit A4 - L10
#define pos5 2                                                 // Digit A5 - L2
#define pos6 18                                                // Digit A6 - L18

const char digit[96] =                                        // Array to hold digits, upper case alpha and punctuation
{
    0xFC,                                                      // "0"
    0x60,                                                      // "1"
    0xDB,                                                      // "2"
    0xF3,                                                      // "3"
    0x67,                                                      // "4"
    0xB7,                                                      // "5"
    0xBF,                                                      // "6"
    0xE4,                                                      // "7"
    0xFF,                                                      // "8"
    0xF7                                                       // "9"
};
//---------------------------------------------------------------------------------

static inline char RWData(char value);
int spi_rx_data = 0 ;


//-----------------------------------------------------------------
// This character array is used to hold the User's words - on the '2553 we only have 512 bytes of RAM

char array[6][32] =      {
                         {"_Hello World, welcome to SIMPL_"},
                         {"_Mary had a Little Lamb_"},
                         {"_This is a test message_"},
                         {"_Hickory, Dickory Dock_"},
                         {"_twas brillig slithy toves_"},
                         {"11{kp_ Green Bottles_}"}

                         };

    char buf[64];                          // Buffer to hold users keyboard entry
    char num_buf[11];                      // long enough to hold a 32 bit long
    char block_array[33];                  // Used to transfer bytes from external RAM and execute x-code

//    int a = 0;                          // integer variables a,b,c,d
//    int b = 0;
//    int c = 0;
//    int d = 6;                          // d is used to denote the digital port pin for I/O operations

    unsigned long x = 0;                // Three gen purpose variables for stack & math operations
    unsigned long y = 0;
    unsigned int  z = 0;
    unsigned int ADC_value=0;

    int hex_value = 0;
    int dec_value = 0;
    int decimal_value = 0;
    char ha = 0;
    char hex_char;

    unsigned char in_byte;

    int len = 32;                        // the max length of a User word
    int length = 0;                      // number of bytes to red/write to SRAM
    int address = 0 ;                    // starting address of RAM R/W streamms
    int RAM_byte =0;                     // contents of RAM at given location
    long old_millis=0;
    long new_millis=0;
    unsigned long time = 0;
    char  name;
    char* parray;
    char* px_array;
    char* addr;
    char mode = 0x41;                  // Sequential mode for SRAM

    unsigned int num = 0;
    unsigned int num_val = 0;
    int i;
    int j;
    int l;

/*
//---------------------------------------------------------------------
// Initialise the USCI B for Master Mode SPI
//---------------------------------------------------------------------
// recommended procedure: set UCSWRST, configure USCI, configure ports, activate
//---------------------------------------------------------------------
void spi_init(void)
{

//---------------------------------------------------------------------
// Configure the Clock for 16 MHz
  BCSCTL1 = CALBC1_16MHZ;
  DCOCTL =  CALDCO_16MHZ;

//---------------------------------------------------------------------
//  Set  UCSWRST
  UCB0CTL1 = UCSWRST;

//---------------------------------------------------------------------
// Configure USCI B0
  UCB0CTL0 |= UCCKPH + UCMSB + UCMST + UCSYNC;     // 3-pin, 8-bit SPI master
  UCB0CTL1 |= UCSSEL_2;                            // SMCLK
  UCB0BR0 |= 2;                                    // 8 MHz SPI-CLK
  UCB0BR1 = 0;
//UCB0MCTL = 0;

//---------------------------------------------------------------------
// Configure  Ports
  P1SEL |= BIT5 + BIT6 + BIT7;
  P1SEL2 |= BIT5 + BIT6 + BIT7;
  P1DIR |= BIT0 + BIT4 + BIT5 | BIT7;

//---------------------------------------------------------------------
// activate
  UCB0CTL1 &= ~UCSWRST;
}

*/
//--------------------------------------------------------------------------------
// SPI Initialisation for MSP430FR4133 LaunchPad
//--------------------------------------------------------------------------------
//  P5.3 MISO
//  P5.2 MOSI
//  P5.1 SCLK
//  P8.0  /RAM_CS

// '2433 pins

    //  P1.3 MISO
    //  P1.2 MOSI
    //  P1.1 SCLK
    //  P3.0 /RAM_CS


void spi_init_4133()

{
// Configure GPIO
 P3DIR |= BIT0;                          // For /RAM_CS
 P1SEL0 |= BIT1 | BIT2 | BIT3;           // SCLK, MISO, MOSI pins

// Configure USCI_B0

    UCB0CTLW0 |= UCSWRST;                     // **Put state machine in reset**
    UCB0CTLW0 |= UCMST|UCSYNC|UCCKPH|UCMSB;   // 3-pin, 8-bit SPI master
                                              // Clock polarity low, MSB
    UCB0CTLW0 |= UCSSEL_2 ;                   // SMCLK
    UCB0BR0 = 0x01;                           // /2,fBitClock = fBRCLK/(UCBRx+1).
    UCB0BR1 = 0;                              //

    UCB0CTLW0 &= ~UCSWRST;                    // **Initialize USCI state machine**
}



//--------------------------------------------------------------------------------
// 23K256 Serial Ram functions
//--------------------------------------------------------------------------------

char SR_getMode(void) {                                    // Read the Mode of the 23K256
    ssSelect;                                                 // select
    RWData(SR_READ_STATUS); // 0x05
    char mode = RWData(DUMMY_BYTE);
    ssDeselect;                                               // de-select
    return mode;
}

void SR_setMode(char mode) {                               // Write Mode to 23K256
    ssSelect;
    RWData(SR_WRITE_STATUS); // 0x01
    RWData(mode);
    ssDeselect;
}

static inline void SR_writestream(unsigned int addr) {          // Write a stream to 23K256
    ssDeselect; // deselect if we are active
    ssSelect;
    RWData(0x02);                                           // Send command
    RWData(addr >> 8);                                      // Send upper address
    RWData(addr);                                           // Send lower address
}

static inline void SR_readstream(unsigned int addr) {             // Read a stream from 23K256
    ssDeselect;
    ssSelect;
    RWData(0x03);                                            // Send command
    RWData(addr >> 8);                                       // Send upper address
    RWData(addr);                                            // Send lower address
}

//-----------------------------------------------------------------
// SPI Send / Receive

static inline char RWData(char value)
{
    UCB0TXBUF = value;
//    while (!(IFG2 & UCB0TXIFG)) {};        // wait for buffer ready
      while (!(UCB0IFG & UCTXIFG)) {};        // wait for buffer ready
//    while  (!(IFG2 &   UCB0RXIFG));        //  USCI_B0 RX  Received?
    while  (!(UCB0IFG &   UCRXIFG));        //  USCI_B0 RX  Received?
    spi_rx_data    =   UCB0RXBUF;          //  Store received data
    return  spi_rx_data;
}

//------------------------------------------------------------------------------------
// UART Initialisation
// UART provides PC comms on 1.2 and P1.2 - uses USCI A

void uart_init_4133(void)

{

 // Configure UART pins
  P1SEL0 |= BIT0 | BIT1;                    // set 2-UART pin as second function

  // Configure UART
  UCA0CTLW0 |= UCSWRST;
  UCA0CTLW0 |= UCSSEL__SMCLK;

  // Baud Rate calculation
  // 8000000/(16*9600) = 52.083
  // Fractional portion = 0.083
  // User's Guide Table 14-4: UCBRSx = 0x49
  // UCBRFx = int ( (52.083-52)*16) = 1
  UCA0BR0 = 4;                             // 8000000/16/9600
  UCA0BR1 = 0x00;
  UCA0MCTLW = 0x5500 | UCOS16 | UCBRF_1;

  UCA0CTLW0 &= ~UCSWRST;                    // Initialize eUSCI


}

/*

void uart_init(void)
{
	P1SEL  = RXD + TXD;
  	P1SEL2 = RXD + TXD;
  	UCA0CTL1 |= UCSSEL_2;                     // SMCLK
  	UCA0BR0 = 156;                            // 1MHz 9600
  	UCA0BR1 = 0;                              // 1MHz 9600
  	UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1
  	UCA0CTL1 &= ~UCSWRST;                     // Initialize USCI state machine
}

*/

//---------------------------------------------------------------------
// UART Primitive Routines uart_putc and uart_get_c

unsigned char uart_getc()
{
//    while (!(IFG2&UCA0RXIFG));                   // USCI_A0 RX buffer ready?

     while (!(UCA0IFG&UCRXIFG));                   // USCI_A0 RX buffer ready?
     return UCA0RXBUF;
}

void uart_putc(unsigned char c)
{
	while (!(UCA0IFG&UCTXIFG));              // USCI_A0 TX buffer ready?
  	UCA0TXBUF = c;                          // TX
}

void uart_puts(const char *str)                 // Output a string
{
     while(*str) uart_putc(*str++);
}

//------------------------------------------------------------------------
// Print a long unsigned int number

  void printlong(unsigned long num)
  {
  if (num / (unsigned long)10 != 0) printlong(num / (unsigned long)10);
  uart_putc((char)(num % (unsigned long)10) + '0');
  return;
  }
//---------------------------------------------------------------------------
// Print a CR-LF

  void crlf(void)                  // send a crlf
  {
    uart_putc(10);
//  uart_putc(13);
  }

  void print_ok()
  {uart_puts((char *)"OK\n\r"); }

 //------------------------------------------------------------------------------------
 // Initialise the ADC on the '4133

void ADC_init_4133(void)
{
}




/*
//-------------------------------------------------------------------------------------
// ADC Configuration

  void ConfigureAdc(void)
{
  ADC10CTL1 = INCH_3 + ADC10DIV_3 ;                     // Channel 3, ADC10CLK/3
  ADC10CTL0 = SREF_0 + ADC10SHT_3 + ADC10ON + ADC10IE;  // Vcc & Vss as reference, Sample and hold for 64 Clock cycles, ADC on, ADC interrupt enable
  ADC10AE0 |= BIT3;                                     // ADC input enable P1.3
}

 int ADC_Read(void)
 {
   ADC10CTL0 |= ENC + ADC10SC;			// Sampling and conversion start
   ADC_value = ADC10MEM;
   return ADC_value;
 }
*/
//-------------------------------------------------------------------------------------
// mS delay routine

void delay_mS(int j)
{
    volatile unsigned long i;
           while(j)
  {
           i = 42;             	         // Delay
   	   do (i--);
           while (i != 0);	        // busy waiting (bad)
           j--;
  }
}
//---------------------------------------------------------------------------------

void setup(void)
{
        WDTCTL  = WDTPW + WDTHOLD; 	             // Stop WDT

        PM5CTL0 &= ~LOCKLPM5;                       // Disable the GPIO power-on default high-impedance mode

//        Serial.begin(100000);                        // initialises timing functions

//	BCSCTL1 = CALBC1_1MHZ;                       // Set DCO
//  	DCOCTL  = CALDCO_1MHZ;
	P1DIR  = BIT0 + BIT6; 		              // P1.0 and P1.6 are the red+green LEDs
	P1OUT  = BIT0 + BIT6; 		              // All LEDs off

        uart_init_4133();                                  // Initialise the '4133 UART for 96000 baud

        spi_init_4133();                                   // Initialise the '4133 SPI on USCIB  8MHz clock, master mode

        uart_puts((char *)"MSP430 SIMPLEX\n\r");      // send opening banner message

        spi_check();                                  // Make sure SPI RAM is connected and in the correct streaming mode

 //       P1SEL |= BIT3;	                              // ADC input pin P1.3
        ADC_init_4133();
        WDTCTL = WDTPW + WDTTMSEL + WDTIS1;            // enable watchdog - interval mode
        parray = &array[0][0];                        // parray is the pointer to the first element of code buffer
        px_array = &block_array[0];                   // px_array is pointer to first element of RAM transfered block array
        delay(1);
        time = micros();
        printlong(time);                 // used for timing functions
        crlf();

//        LCD_print_4133();
}
void  loop(void)
{
//-------------------------------------------------------------------------------
// Interpreter Loop
  while(1)
  {

  textRead(buf, 64);                                  // This is the endless while loop which implements the SIMPL interpreter - just 3 simple functions
  textChk(buf);                                       // check if it is a : character for beginning a colon definition
  textEval(buf);

  }   // end of interpreter loop

}    // End of main

//-------------------------------------------------------------------------------
// Language Functions  - Words
// ------------------------------------------------------------------------------

//  Read the character into the buffer

void textRead (char *p, char n) {
  char i = 0;
  while (i < (n-1)) {
    char ch = uart_getc();
    if (ch == '\r' || ch == '\n') break;
    if (ch >= ' ' && ch <= '~') {
      *p++ = ch;
      i++;
    }
  }
  *p = 0;
}

// ---------------------------------------------------------------------------------------------------------
void textChk (char *buf)       // Check if the text starts with a colon and if so store in user's word RAM array  parray[]
{
  if (*buf == ':')  {
  char ch;
  int i =0;
  while ((ch = *buf++)){

  if (ch == ':') {
  uart_putc(*buf);   // get the name from the first character
  uart_putc(10);
  uart_putc(13);
  name = *buf ;
  buf++;
  }

  bufWrite((parray + (len*(name-65) +i)),*buf);

  i++;

}

 x = 1;

}
}

// ---------------------------------------------------------------------------------------------------------

void textEval (char *buf) {

  char *loop;
  char *start;
  char ch;

  unsigned long k = 0;

  while ((ch = *buf++)) {                         // Is it a number?
    switch (ch) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      x = ch - '0';
      while (*buf >= '0' && *buf <= '9') {
        x = x*10 + (*buf++ - '0');                // If  a number store it in "x"
      }
      break;

//-------------------------------------------------------------------------------
 // User Words
      case 'A':                // Point the interpreter to the array containing the words
      case 'B':
      case 'C':
      case 'D':
      case 'E':
      case 'F':
      case 'G':
      case 'H':
      case 'I':
      case 'J':
      case 'K':
      case 'L':
      case 'M':
      case 'N':
      case 'O':
      case 'P':
      case 'Q':
      case 'R':
      case 'S':
      case 'T':
      case 'U':
      case 'V':
      case 'W':
      case 'X':
      case 'Y':
      case 'Z':

      textEval(parray + (len*(ch-65)));            // Evaluate and execute the User's expression fo RAM

      break;

//---------------------------------------------------------------------------------
//  (a - h  hijacked for SPI RAM extensions - SIMPLEX!)

      case 'a':         // Address  (for writing to RAM)
      address =(x);
      break;

      case 'b':         // Block   - fetch a block from external memory
      get_block(x);
      break;

      case 'c':         // Compile
      hex_print_2(x);
      break;

      case 'd':         // HEX Dump  -  dump x bytes starting at address y
      hex_dump(y,x);
      print_ok();
      break;

/*
      case 'd':         // Decimal Dump
      dump(y,x);        // dump y bytes starting at address x
      break;
*/
      case 'e':         // Execute
      execute_block(x);
      break;

      case 'f':         // File  // fill y bytes starting at address x with test data
      spi_fill(y,x);
      print_ok();
      break;

      case 'g':         // Go
      run_block();
      break;

      case 'n':         // Go
//      LCD_display_n(x);
      break;

//--------------------------------------------------------------------------------
// Primitive and User vocabulary defined in this section
// Timing & Printing Group

      case 'p':
      printlong(x);                        // print long integer
      break;

      case 'q':                            // print integer with crlf
      printlong(x);
      crlf();
      break;

      case 't':
      time = micros();
      printlong(time/16);                 // used for timing functions
      crlf();
      break;

      case 'v':
      time = micros();
      printlong(time/16);                 // used for timing functions
      crlf();
      break;

/*
      case 'd':
      d = x;
      break;
*/

      case '_':                              // Print the string enclosed between underscores  eg.  _Hello_
      while ((ch = *buf++) && ch != '_') {
      uart_putc(ch);
      }
      uart_putc(10);
      break;

      case 92 :                              //  ASCII 92 forward slash \ Copy the text enclosed between \ and \ to RAM
      while ((ch = *buf++) && ch != 92) {
      uart_putc(ch);
      }
      uart_putc(10);
      put_block(address);

      break;

//----------------------------------------------------------
// Arithmetic Group

      case '+':
      x = x+y;
      break;

      case '-':
      x = x-y;
      break;

      case '*':
      x = x*y;
      break;

      case '/':
      x = x/y;
      break;

      case '%':
      x = x%y;
      break;

      case 'x':
      x = x + 1;
      break;

      case 'y':
      y = y + 1;
      break;

//--------------------------------------------------------------------
// Logical Group - provides bitwise logical function between x and y

      case '&':
      x = x&y;                             // Logical AND
      break;

      case '|':
      x = x|y;                             // Logical OR
      break;

      case '^':
      x = x^y;                             // Logical XOR
      break;

      case '~':
      x = !x;                              // Complement x
      break;

      case ' ':                            // Transfer x into second variable y
      k=y;                                 // Transfer loop counter into k
      y= x;
      break;

      case '#':                            // Load x with the ASCII value of the next character  i.e. 5 = 35H or 53 decimal
      x=*(buf-2);
      break;

      case '$':                            // print out a number as either a 2 digit or 4 digit Hexadecimal
      if(x<=255) {hex_print_2(x); break;}
      hex_print_4(x);
      break;

// ----------------------------------------------------------------------
// Memory Group

      case '!':     // store
      y = x;
      break;

      case '@':     // Fetch
      x = y;
      break;

/*
      case 'r':                         // read a byte from RAM
      bite = bufRead(x);                // x = address
      x = bite;
      uart_putc(x);                     // print the character
      break;

      case 'q':                         // read a block of x bytes of RAM at address y
      for (int i=0; i<x; i++) {
      bite = bufRead(y+i);              // read the array
      uart_putc(bite);                  // print the character to the serial port
      }
      break;


      case 'w':                         // write a byte to RAM  address in y, data in x
      bufWrite(y,x);
      break;

*/

//--------------------------------------------------------------------
// Comparison Test and conditional Group

      case '<':
      if(x<y){x=1;}              // If x<y x= 1 - can be combined with jump j
      else x=0;
      break;

      case '>':
      if(x>y){x=1;}              // If x>y x= 1 - can be combined with jump j
      else x=0;
      break;

      case 'j':                  // test  if x = 1 and jump next instruction
      if(x==1){*buf++;}
      break;

//----------------------------------------------------------------------------------
// Print out the current word list

      case '?':                             // Print out all the RAM
      parray = &array[0][0];                // reset parray to the pointer to the first element

      for ( j = 0; j<26; j++) {

      uart_putc(j+65);                  // print the caps word name
      uart_putc(32);                    // space

      for ( l=0; l<len; l++) {
      in_byte = bufRead( parray +  (j *len )+l);          // read the array
      uart_putc(in_byte);                                 // print the character to the serial port
    }

     crlf();

   }
   for( l = 0; l <11; l++)      // add some spaces to make it more legible on the page

   {
   crlf();
   }
    break;
//----------------------------------------------------------------------------------------------------
// Conditional Code branch

    case '[':                                     // The start of a condition test
      k = x;
      start = buf;                               // remember the start position of the test
      while ((ch = *buf++) && ch != ']') {       // get the next character into ch and increment the buffer pointer *buf - evaluate the code
      }

     case ']':
    if (x) {                                    // if x is positive - go around again
     buf = start;
    }
      break;

//--------------------------------------------------------------------------
// Case Statement Selection
// Select some code from a list separated by commas

//5(0p,1p,2p,3p,4p,5p,6p)  should select 5 and print it

case '(':

      k = x;                                                 // copy x to use as the "phrase counter"
                                                             // decrement k to see whether to interpret or not
      while (k)
      {
      ch = *buf++;
      if (ch == ',')
      { k--;}
      }
      break;

      case ',':

      k--;                                              //
      while (k<0)                                       // k < 0 so skip the remaining entries in the list
      {
      ch = *buf++;                                      // skip the remaining characters
      if (ch == ')') {break;}
      }

      break;
//-----------------------------------------------------------------------------------------------------------------------------------------------
// Analogue and Digital Input and Output Group   - these add heavily to total - need to be converted to MSP430

      case 's':
//      x = ADC_Read();         // Adds 38 bytes
      break;

 /*
      case 'a':
      analogWrite(d,x);           // adds 340 bytes
      break;



      case 'i':
      x = digitalRead(d);        // adds 100 bytes
      break;

      case 'o':
      digitalWrite(d, x%2);     // adds 18 bytes
      break;
 */

//-------------------------------------------------------------------
// Delays Group

    case 'm':
    delay(x<<4);
    break;

    case 'u':
    delayMicroseconds(x<<4);
    break;
//---------------------------------------------------------------------

    case '{':
      k = x;
      loop = buf;
      while ((ch = *buf++) && ch != '}') {
      }
    case '}':
      if (k) {
        k--;
        buf = loop;
      }
      break;


    case 'k':
      x = k;
      break;

// -----------------------------------------------------------------------------
// Launchpad LED group  support for red and green LEDs on entry level LaunchPad

      case 'w':
 {
 P1OUT |= BIT0;
 }
 break;

 case 'r':
 {
 P1OUT &= ~BIT0;
 }
 break;

 case 'h':
 {
 P1OUT |= BIT6;
 }
 break;

 case 'l':
 {
 P1OUT &= ~BIT6;
 }
 break;

// ----------------------------------------------------------------------

    }
  }
}

//-----------------------------------------------------------------------------
// SPI RAM Extensions to SIMPL   Test routines etc
//---------------------------------------------------------------------

char RAM_stream_mode()
{

    ssDeselect;
    delay_1ms;

        char chipMode;

        chipMode = SR_getMode();    // check status register for sequential mode
        if (chipMode != 0x41) {
            SR_setMode(0x41);
             return 0;
        }
        return 1;
       }
//---------------------------------------------------------------------------
// Check that the external RAM is present and that it is in Sequential Mode
int spi_check()

{
        ssDeselect;
        delay_1ms;
        char chipMode;

        // make sure there is a 23K256 chip and that
        // is wired properly and in sequential mode

        chipMode = SR_getMode();
        if (chipMode != 0x41)
        {
            SR_setMode(0x41);
            uart_puts((char *)"SPI RAM not found\n\r");
        }
        else
        {
        uart_puts((char *)"32K SPI RAM Connected!\n\r");
        }
}

//--------------------------------------------------------------------------
// RAM mode
// Where mode =
// 0x01  Byte Mode
// 0x81  Page Mode           (Page = 32 bytes)
// 0x41  Sequential Mode
//--------------------------------------------------------------------------

char RAM_byte_mode(char mode)          // Set the RAM into the correct mode
{
        ssDeselect;
        char chipMode;

        chipMode = SR_getMode();       // check status register for byte mode
        if (chipMode != mode) {
        SR_setMode(mode);
        return 0;
        }
        return 1;
}

// -----------------------------------------------------------------------------
//spi_fill()- fill the RAM from address with "initial orders"  characters
// -----------------------------------------------------------------------------

void spi_fill(int address, int length )                 // fill
{
    //unsigned int i;
    char storedValue = 0;
    RAM_stream_mode();
    SR_writestream(address);                                 // start writing at address

    for(j=0; j<6; j++)
    {
    for (i = 0; i < 32; ++i)
    {
    storedValue =  (array[j][i]);
    RWData(storedValue);
    }
    }

}

//-----------------------------------------------------------------------------------------

void put_block(int address)
{

   // unsigned int i;
    char storedValue = 0;
    RAM_stream_mode();
    SR_writestream(address*32);                                 // start writing at address

    for (i = 0; i < 32; ++i)
    {
    storedValue = (buf[i]);                                 // copy key buffer to RAM block
    if(storedValue != 92)                                   // omit opening \ and closing \
    {
    RWData(storedValue);
    uart_putc(storedValue);
    }
    }


// -----------------------------------------------------------------------------
// dump the characters from RAM to screen - putting a newline every 64 bytes
// -----------------------------------------------------------------------------

void dump(int address, int length)

{
  int i =0;

     crlf();                         // Start with a newline!

    RAM_stream_mode();
    SR_readstream(address); // start reading at address 0
    for (i = 0; i < length ; ++i)

       {
       if(i%32 == 0)
       {
         crlf();                       // put a newline every 32 chars

         printlong(i+ address);                          // print the line address followed by four spaces
         uart_putc(0x20);
         uart_putc(0x20);
         uart_putc(0x20);
         uart_putc(0x20);
       }

       RAM_byte = RWData(DUMMY_BYTE);
       if(RAM_byte <= 31) {RAM_byte =  '.';}      // Make it a full stop for unprintable characters
       uart_putc(RAM_byte);
}

}

//--------------------------------------------------------------------------------------------
// Generate a familiar hex dump of the RAM area

void hex_dump(int address, int length)

{
  crlf();                          // Start with a newline!

  int i =0;
  SR_readstream(address); // start reading at address 0

  for (j = address >>5; j <= (address +length)>> 5 ; ++j)
  {
    for (i = 0; i < 32 ; ++i)
       {
       if(i== 0)
       {
         hex_print_4(j<<5);                          // print the line address as HEX  followed by 2 spaces
         uart_putc(0x20);
         uart_putc(0x20);
       }

       RAM_byte = RWData(DUMMY_BYTE);             // Now read and print the data as numbers
      // printlong(RAM_byte);
       hex_print_2(RAM_byte);

       block_array[i] = RAM_byte;                 // block_array is a temporary buffer to hold current RAM block
       uart_putc(0x20);                           // followed by a space
       }

       uart_putc(0x20);                            // Separate columns with 3 spaces
       uart_putc(0x20);
       uart_putc(0x20);

       for (i = 0; i < 32 ; ++i)          // start reading back at address 0 so as to print the characters
       {
       RAM_byte = block_array[i];
       if(RAM_byte <= 31  || RAM_byte >= 127) {RAM_byte =  '.';}      // Print a full stop for all unprintable characters
       uart_putc(RAM_byte);
       }

      crlf();                 // put a newline every 32 chars
  }
}
//-----------------------------------------------------------------
// Convert a char into a 2 character hex pair

void hex_print_2(int dec_value)
{
  ha = dec_value >> 4;
  hex_print_char(ha);
  ha = dec_value - (ha << 4);
  hex_print_char(ha);
}

//-----------------------------------------------------------------
// Convert an int into into a 4 character hex pair
// note all binary mults and divs should use shift ops for efficiency

void hex_print_4(unsigned int decimal_value)
{
  int div_value = decimal_value >> 8;
  hex_print_2(div_value);
  dec_value = decimal_value - (ha << 8);
  hex_print_2(dec_value);
  uart_putc(32);                             // output a space
}

void hex_print_char(char hex_value)        //  decode the A-F numbers
{
  if (ha <=9){hex_char = '0' + ha ;}       // Digits 0-9
  if (ha >= 10) {hex_char = '7' + ha ;}    // Letters A-F
  uart_putc(hex_char);
}
//---------------------------------------------------------------------------
// RAM Block routines

void get_block(int address)

{
  int i =0;

    RAM_stream_mode();
    SR_readstream(address << 5); // start reading at address
    for (i = 0; i <32 ; ++i)
     {
       RAM_byte = RWData(DUMMY_BYTE);
       block_array[i] = RAM_byte;                 // block_array is a temporary buffer to hold current RAM block
//       uart_putc(RAM_byte);

     }
//      crlf();
}


//---------------------------------------------------------------------------------------------
void execute_block(int address)                   // Load a block and execute it
{
  get_block(address);
  run_block();
}

//---------------------------------------------------------------------------------------------
// Point the Interpreter at the code contained in block_array and let it execute it!
void run_block()

{
  textEval(px_array);            // Evaluate and execute the User's expression fo External RAM
}
//---------------------------------That's All Folks---------------------------------------------

 /*

void LCD_print_4133(void)

{

   // Configure LCD pins
    SYSCFG2 |= LCDPCTL;                                        // R13/R23/R33/LCDCAP0/LCDCAP1 pins selected

    LCDPCTL0 = 0xFFFF;
    LCDPCTL1 = 0x07FF;
    LCDPCTL2 = 0x00F0;                                         // L0~L26 & L36~L39 pins selected

    LCDCTL0 = LCDSSEL_0 | LCDDIV_7;                            // flcd ref freq is xtclk

    // LCD Operation - Mode 3, internal 3.08v, charge pump 256Hz
    LCDVCTL = LCDCPEN | LCDREFEN | VLCD_6 | (LCDCPFSEL0 | LCDCPFSEL1 | LCDCPFSEL2 | LCDCPFSEL3);

    LCDMEMCTL |= LCDCLRM;                                      // Clear LCD memory

    LCDCSSEL0 = 0x000F;                                        // Configure COMs and SEGs
    LCDCSSEL1 = 0x0000;                                        // L0, L1, L2, L3: COM pins
    LCDCSSEL2 = 0x0000;

    LCDM0 = 0x21;                                              // L0 = COM0, L1 = COM1
    LCDM1 = 0x84;                                              // L2 = COM2, L3 = COM3

    // Display "123456"
    LCDMEM[pos1] = digit[1];
    LCDMEM[pos2] = digit[2];
    LCDMEM[pos3] = digit[3];
    LCDMEM[pos4] = digit[4];
    LCDMEM[pos5] = digit[5];
    LCDMEM[pos6] = digit[6];

    LCDCTL0 |= LCD4MUX | LCDON;                                // Turn on LCD, 4-mux selected

    PMMCTL0_H = PMMPW_H;                                       // Open PMM Registers for write
    PMMCTL0_L |= PMMREGOFF_L;                                  // and set PMMREGOFF

//    __bis_SR_register(LPM3_bits | GIE);                        // Enter LPM3.5
//    __no_operation();                                          // For debugger
}


void LCD_display_n(int number)

{

  LCDMEMCTL |= LCDCLRM;                                      // Clear LCD memory

    LCDCSSEL0 = 0x000F;                                        // Configure COMs and SEGs
    LCDCSSEL1 = 0x0000;                                        // L0, L1, L2, L3: COM pins
    LCDCSSEL2 = 0x0000;

    LCDM0 = 0x21;                                              // L0 = COM0, L1 = COM1
    LCDM1 = 0x84;                                              // L2 = COM2, L3 = COM3

    // Display "123456"
    int n = number / 100000;
    LCDMEM[pos1] = digit[n];
    n = number - (100000*n);
    n = n/10000;
    LCDMEM[pos2] = digit[n];
    n = number - (10000*n);
    n = n/1000;
    LCDMEM[pos3] = digit[n];
      n = number - (1000*n);
    n = n/100;
    LCDMEM[pos4] = digit[n];
     n = number - (100*n);
    n = n/10;
    LCDMEM[pos5] = digit[n];
     n = number - (10*n);

    LCDMEM[pos6] = digit[n];

    LCDCTL0 |= LCD4MUX | LCDON;                                // Turn on LCD, 4-mux selected

}

*/


