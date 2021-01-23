// SIMPL

// A Serial Interpreted Minimal Programming Language

// Inspired by Txtzyme - by Ward Cunningham

// This is the 32-bit SIMPL minimal framework Filename simpl_uno_32bit_1

// This is the slim version of simpl that removes most of the Arduino specific routines - saving codespace

// In this version: Added printlong() to print out a 32 bit integer plus some 32bit arithmetic and timing functions

// 32bit support pushes codesize up by about 1Kbytes - as all math routines now are 32 bit


// This 32 bit SIMPL kernel compiles in 1688 bytes - leaving lots of room for other stuff
// The core kernel is less than 2k bytes
// SIMPL allows new words to be defined by preceding them with colon :  (Like Forth)
// New words use CAPITALS - so 26 words are possible in the user's vocabulary

// A word can be a maximum of 48 characters long 
// Type ? to get a list off all defined words

#define F_CPU 16000000UL        // define the clock frequency as 16MHz
#define BAUD 115200

#include <util/setbaud.h>      // Set up the Uart baud rate generator

#define bufRead(addr)      (*(unsigned char *)(addr))
#define bufWrite(addr, b)   (*(unsigned char *)(addr) = (b))
   
// This character array is used to hold the User's words

char array[26][48] = {     };   // Define a 26 x 48 array for the colon definitions
                                                    
    int a = 0;          // integer variables a,b,c,d
    int b = 0;
    int c = 0;
    int d =13;          // d is used to denote the digital port pin for I/O operations Pin 13 on Arduino

    unsigned long x = 0;    // Three gen purpose variables
    unsigned long y = 0;
    unsigned int z = 0;

    unsigned char bite;
 
    int len = 48;        // the max length of a User word
    
    long D_num = 0;
    long D_val = 0;
    long D_decade = 0;
    
    
    char name0;
    
    char* parray;
    
    char buf[64];
    
    char* addr;
 
    unsigned int num = 0;
    unsigned int num_val = 0;
    int j;
    char num_buf[11];            // long enough to hold a 32 bit long
    int decade = 0;
    char digit = 0;
    

//----------------------------------------------------------------------------------------------------- 
 void u_putchar(char c) {
    loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait until data register empty. */
    UDR0 = c;
}

char u_getchar(void) {
    loop_until_bit_is_set(UCSR0A, RXC0); /* Wait until data exists. */
    return UDR0;
DDRD = DDRD | B11111100;  // Sets pins 2 to 7 as outputs without changing the value of pins 0 & 1, which are RX & TX 
}

// Print a CR-LF

  void crlf(void)                  // send a crlf 
  {  
  u_putchar(10);
  u_putchar(13); 
  }

// -----------------------------------------------------------------------------------
// Setup the various array and initialisation routines
 
void setup() 
 { 

  pinMode(13,OUTPUT);
  
// Enable UART
   UBRR0H = UBRRH_VALUE;
   UBRR0L = UBRRL_VALUE;

#if USE_2X
    UCSR0A |= _BV(U2X0);
#else
    UCSR0A &= ~(_BV(U2X0));
#endif

    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); // 8-bit data 
    UCSR0B = _BV(RXEN0)  | _BV(TXEN0) ; // Enable RX and TX 
}



// -----------------------------------------------------------------------------------

void loop()                // This is the endless while loop which implements the interpreter

{ 
  txtRead(buf, 64);        // Get the next "instruction" character from the buffer 
  txtChk(buf);             // check if it is a : character for beginning a colon definition 
  txtEval(buf);            // evaluate and execute the instruction 
}
 
// ---------------------------------------------------------------------------------------------------------
// Functions
 
void txtRead (char *p, byte n)
{
  byte i = 0;
  while (i < (n-1)) {
    char ch = u_getchar();                    // get the character from the UART
    if (ch == '\r' || ch == '\n') break;
    if (ch >= ' ' && ch <= '~') {
      *p++ = ch;
      i++;
    }
  }
  *p = 0;
}
 
// --------------------------------------------------------------------------------------------------------- 
void txtChk (char *buf)       // Check if the text starts with a colon and if so store in temp[]
{       
  if (*buf == ':')  {  
  char ch;
  int i =0;
  while ((ch = *buf++)){
    
  if (ch == ':') {
  u_putchar(*buf);   // get the name from the first character
  crlf();
  name0 = *buf ;
  buf++;
  }
 
  bufWrite((parray + (len*(name0-65) +i)),*buf); 
  i++;
 
}

 x = 1;
 
} 
}

// ---------------------------------------------------------------------------------------------------------  
   void txtEval (char *buf)    // Evaluate the instruction and jump to the action toutine 
   {
   unsigned long k = 0;
 
   char *loop;
   char *start;
   char ch;
   while ((ch = *buf++)) {
  
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
        x = x*10 + (*buf++ - '0');
      }
      break;
      
    case 'p':
    printlong(x);               //  Print out x as a 32-bit decimal integer
    crlf();
    break;

     case ' ':                 // Transfer x into second variable y
     k = y;                    // Transfer loop counter into k 
     y = x;
     break;

// -----------------------------------------------------------------------------------------
// Put the rest of the command interpreter words here as case statements     
// -----------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------     
// User Words     
      
      case 'A':            // Point the interpreter to the array containing the words
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
  
      name0 = ch - 65;
      addr = parray + (len*name0);
      
      txtEval(addr);
      break;

      //----------------------------------------------------------------------------------
      
// Print out the current word list
      
      case '?':                             // Print out all the RAM
      parray = &array[0][0];                // reset parray to the pointer to the first element 
      
      for (int j = 0; j<26; j++) {
      
      u_putchar(j+65);                  // print the caps word name
      u_putchar(32);                    // space
        
      for (int i=0; i<len; i++) {
      bite = bufRead( parray +  (j *len )+i);          // read the array
      u_putchar(bite);                                 // print the character to the serial port
    }
    
     crlf();
   
   }  
   
    break; 
}

   }

   }

//--------------------------------------------------------------------------------------
// UART Routines
//--------------------------------------------------------------------------------------

// Print a 16 bit int number

void printnum(int num)
{

// num is likely going to be a 16 bit unsigned int - so we are handling up to 5 digits
// We need to test which decade it is in - and convert the leading digit to ascii - remembering to suppress leading zeroes

num_val = num;                // make a copy of num for later

// Extract the digits into the num_buff 

decade = 10000;

for (j = 5; j>0; j--)

{
z = num/decade;
num_buf[j]=z+48;
num = num - (decade*z);
decade = decade/10;
}

// Now print out the array - correcting to allow for leading zero suppression

if (num_val == 0)
{
{num_buf[5] = 48;}
}
  decade = 10000;            // we need to know what decade we are in for leading zero suppression
  j=5;  
  while(num_buf[j]!=0)
  {
  if(num_buf[j]  == 48  &&  (num_val <= decade)) {j--;}       // suppress leading zeroes    
  else 
  { 
  u_putchar(num_buf[j]);    // send the number
  num_buf[j]=0;             // erase the array for next time
  j--;
  } 
   decade = decade/10;     // update the decade  
  } 
  if(!num_val){u_putchar(48); }  // separately handle the case when num  == 0  
  }
 

 //---------------------------------------------------------------------------------------------------------
 // Print a 32 bit integer
 
 void printlong(long D_num)
{
// num is likely going to be a 16 bit iunsigned int - so we are handling up to 5 digits
// We need to test which decade it is in - and convert the leading digit to ascii - remembering to suppress leading zeroes

D_val = D_num;                // make a copy of num for later

// Extract the digits into the num_buff 

D_decade = 1000000000;

for (j = 10; j>0; j--)

{
z = D_num/D_decade;
num_buf[j]=z+48;
D_num = D_num - (D_decade*z);
D_decade = D_decade/10;
}

// Now print out the array - correcting to allow for leading zero suppression

if (D_val == 0)
{
{num_buf[10] = 48;}
}
  D_decade = 1000000000;            // we need to know what decade we are in for leading zero suppression
  j=10;  
  while(num_buf[j]!=0)
  {
  if(num_buf[j]  == 48  &&  (D_val <= D_decade)) {j--;}       // suppress leading zeroes    
  else 
  { 
  u_putchar(num_buf[j]);    // send the number
  num_buf[j]=0;             // erase the array for next time
  j--;
  } 
   D_decade = D_decade/10;     // update the decade  
  } 
  if(!D_val){u_putchar(48); }  // separately handle the case when num  == 0 

  u_putchar(10);
  u_putchar(13); 
  
}

// That's All Folks........... 
 
 
 
 
 
 
