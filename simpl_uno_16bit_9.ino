// SIMPL -  A Serial Interpreted Minimal Programming Language 1312 bytes, 

#define F_CPU 16000000UL        // define the clock frequency as 16MHz
#define BAUD 115200
#include <util/setbaud.h>       // Set up the Uart baud rate generator
#define bufRead(addr)       (*(unsigned char *)(addr))
#define bufWrite(addr, b)   (*(unsigned char *)(addr) = (b))
   
char array[26][48] = {     };   // Define a 26 x 48 array for the colon definitions                                                  
    int a = 0;                  // integer variables a,b,c,d
    int b = 0;
    int c = 0;
    int d = 13;                 // d is used to denote the digital port pin for I/O operations Pin 13 on Arduino    
    int t = 0;
    int u = 0;
    int v = 0;
    int w = 0;
    int x = 0;
    int y = 0;
    int z = 0;
    char sp = 0;       // stack pointer
    char rp = 0;       // return stack pointer
    int M[256];       // 256 words of memory
    int S[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} ; // 16 level data stack S
    int R[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} ; // 16 level return stack R         
    char bite;
    char len = 48;        // the max length of a User word
    char name0;
    char* parray;
    char buf[64];
    char* addr;
    unsigned int num = 0;
    unsigned int num_val = 0;
    char j;
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

  void crlf(void)                  // send a crlf 
  {  
  u_putchar(10);
  u_putchar(13); 
  }

  void push()
  { S[sp] = x ; sp++; }

  void pop()
  { sp-- ; x = S[sp]; y = S[sp-1]; }

// Setup the various array and initialisation routines
 
int main()
 { 
   UBRR0H = UBRRH_VALUE;   // Enable UART
   UBRR0L = UBRRL_VALUE;
   UCSR0A |= _BV(U2X0);
   UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); // 8-bit data 
   UCSR0B = _BV(RXEN0)  | _BV(TXEN0) ; // Enable RX and TX 

while(1)                   // This is the endless while loop which implements the interpreter
{ 
  txtRead(buf, 64);        // Get the next "instruction" character from the buffer 
  txtChk(buf);             // check if it is a : character for beginning a colon definition 
  txtEval(buf);            // evaluate and execute the instruction 
}
return 0;
 }
 
// -----------------------------------------------Functions-------------------------------------------------
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

     case '+':      pop() ;   x = x + y;      break;     // ADD
     case '-':      pop() ;   x = x - y;      break;     // SUB
     case '&':      pop() ;   x = x & y;      break;     // AND
     case '|':      pop() ;   x = x | y;      break;     // OR
     case '^':      pop() ;   x = x ^ y;      break;     // XOR

     case '@':      pop() ;   x = M[x] ;      break;     // fetch
     case '!':      pop() ;   M[y] = x ;      break;     // Store     
//     case '*':      x = x*y;      break;     // MUL
//     case '/':      x = x/y;      break;     // DIV
     
    case 'p':
    printnum(x);               //  Print out x as a 16-bit decimal integer
    crlf();
    break;

    case 'm':
    printnum(M[x]);               //  Print out M[x] as a 16-bit decimal integer
    crlf();
    break;

     case '.':                 // Pop x off the stack
     pop() ;                    // Pre-decrement stack pointer                           
     printnum(x);              // Print out x as a 16-bit decimal integer
     crlf();
     break;

     case ',':                 // Push x onto the stack
     push() ;                   // at current stack pointer and post-increment stack pointer     
     break;

     case ' ':                 // Push x onto the stack
     push() ;                  // at current stack pointer and post-increment stack pointer     
     break;  

     // Looping and program control group

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
     
    case 'k':      x = k;      break;

    case '_': // Print the string enclosed between underscores  eg.  _Hello_
            while ((ch = *buf++) && ch != '_')
            {
                u_putchar(ch);
            }
            crlf();
            break;

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
//---------------------------------------------------------------------------------------------
          
      case '?':                             // Print out the current word list
      parray = &array[0][0];                // reset parray to point to the first element 
      
      for (int j = 0; j<26; j++) {
      
      u_putchar(j+65);                      // print the caps word name
      u_putchar(32);                        // space
        
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

//--------------------------- Print a 16 bit integer-----------------------------------
static void printnum(unsigned int num) {
  if (num / (unsigned int)10 != 0) printnum(num / (unsigned int)10);
  u_putchar((char)(num % (unsigned int)10) + '0');   
  return;  
  crlf();
}

 
 
 
