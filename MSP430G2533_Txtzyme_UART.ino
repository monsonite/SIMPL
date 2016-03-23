// Txtzyme Interpreter for MSP430 Launchpad  - by Ward Cunningham

// A minimal 868 byte Txtzyme for MSP430 Launchpad with MSP430G2533

#include <msp430.h>

#define RXD		BIT1    // Receive Data (RXD) at P1.1
#define TXD		BIT2    // Transmit Data (TXD) at P1.2

unsigned int x = 0;
int d = 6;

/*

void uart_rx_isr(unsigned char c) {
	uart_putc(c);
	P1OUT ^= BIT0;		// toggle P1.0 (red led)
}


void (*uart_rx_isr_ptr)(unsigned char c);


void uart_set_rx_isr_ptr(void (*isr_ptr)(unsigned char c)) 
{
	uart_rx_isr_ptr = isr_ptr;	
}

*/

void uart_init(void)
{
//	uart_set_rx_isr_ptr(0L);

	P1SEL  = RXD + TXD;                       
  	P1SEL2 = RXD + TXD;                       
  	UCA0CTL1 |= UCSSEL_2;                     // SMCLK
  	UCA0BR0 = 104;                             // 1MHz 9600
  	UCA0BR1 = 0;                              // 1MHz 9600
  	UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1
  	UCA0CTL1 &= ~UCSWRST;                     // Initialize USCI state machine
//  	IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt
}

unsigned char uart_getc()
{
    while (!(IFG2&UCA0RXIFG));                // USCI_A0 RX buffer ready?
	return UCA0RXBUF;
}

void uart_putc(unsigned char c)
{
	while (!(IFG2&UCA0TXIFG));              // USCI_A0 TX buffer ready?
  	UCA0TXBUF = c;                          // TX
}

void uart_puts(const char *str)
{
     while(*str) uart_putc(*str++);
}

// Print a 16 bit int number 

  static void printlong(unsigned long num)
  {
  if (num / (unsigned short)10 != 0) printlong(num / (unsigned short)10);
  uart_putc((char)(num % (unsigned short)10) + '0');   
  return;
  }

void delay_mS(int j){
    volatile unsigned long i;
   
           while(j)
{           
           i = 42;             	         // Delay
   	   do (i--);	 
           while (i != 0);	        // busy waiting (bad)
           j--;
}	
         
          
  
}


/*

interrupt(USCIAB0RX_VECTOR) USCI0RX_ISR(void)
{
	if(uart_rx_isr_ptr != 0L) {
		(uart_rx_isr_ptr)(UCA0RXBUF);
	}
}

*/

int main(void)
{
        WDTCTL  = WDTPW + WDTHOLD; 	// Stop WDT
	BCSCTL1 = CALBC1_1MHZ;          // Set DCO
  	DCOCTL  = CALDCO_1MHZ;
 
	P1DIR  = BIT0 + BIT6; 		// P1.0 and P1.6 are the red+green LEDs	
	P1OUT  = BIT0 + BIT6; 		// All LEDs off

        uart_init();                    // Initialise the UART for 96000 baud
//-------------------------------------------------------------------------------    


//  pinMode(d,OUTPUT);
	// register ISR called when data was received
//    uart_set_rx_isr_ptr(uart_rx_isr);

//    __bis_SR_register(GIE);

  uart_puts((char *)"MSP430 Textzyme\n\r");    // send banner message

  while(1)
  {     
  char buf[64];                 // This is the endless while loop which implements the interpreter - just 3 simple functions
  txtRead(buf, 64);
  txtEval(buf);
  } 
}    // End of main

void txtRead (char *p, byte n) {
  byte i = 0;
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

void txtEval (char *buf) {
  unsigned int k = 0; 
  char *loop;
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
      printlong(x);
      break;
    case 'd':
      d = x;
      break;
    case 'i':
//      x = digitalRead(d);
      break;
    case 'o':
 //     digitalWrite(d, x%2);
      break;
    case 'm':
      delay_mS(x);
      break;
    case 'u':
//      delayMicroseconds(x);
      break;
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
    case '_':
      while ((ch = *buf++) && ch != '_') {
        uart_putc(ch);
      }
      uart_putc(10);
      break;
    case 's':
//      x = analogRead(x);
      break;
      
      case 'R':
 {
 P1OUT |= BIT0;
 }
 break;
 case 'r':
 {
 P1OUT &= ~BIT0;
 }
 break;
 case 'G':
 {
 P1OUT |= BIT6;
 }
 break;
 case 'g':
 {
 P1OUT &= ~BIT6;
 }
 break;
      
      
    }
  }
}



