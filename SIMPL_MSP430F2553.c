
//  SIMPL for MSP430F2553 by Zhe Hu  31-8-2019

#include <msp430.h> 

void Init_GPIO();
void Init_UART();

//global variables
char buf[64];                          // Buffer to hold users keyboard entry

#define COBS_BUF_SIZE 40
unsigned char cobs_buf[COBS_BUF_SIZE];
unsigned char send_buffer[COBS_BUF_SIZE + 2];
unsigned char cmd_byte;

enum _OPMode
{
    TXTMODE, COBSMODE
} opmode = TXTMODE;

unsigned long x = 0;
unsigned long y = 0;
unsigned long z = 0;
unsigned long t = 0;
unsigned long data_buf[8];

char array[26][32] = { { "_Hello World, welcome to SIMPL_" }, {
        "_Mary had a Little Lamb_" },
                       { "_This is a test message_" }, {
                               "_Hickory, Dickory Dock_" },
                       { "_twas brillig slithy toves_" }, {
                               "11{kp_ Green Bottles_}" }

};
static const int len = 32;                      // the max length of a User word
char * parray;

#define bufRead(addr)       (*(unsigned char *)(addr))
#define bufWrite(addr, b)   (*(unsigned char *)(addr) = (b))

//end global variables

#define delay_1ms __delay_cycles(8000)
#define delay_1us __delay_cycles(4)

void delay_uS(int i)
{
//   volatile unsigned long i;

    while (i)
    {
        delay_1us;
        i--;
    }

}

void delay_mS(long i)
{
    // Delay
    while (i)
    {
        delay_1ms;
        i--;
    }
    // busy waiting (bad)
}

//print helper
unsigned char uart_getc()
{
    while (!(UCA0IFG & UCRXIFG))
        ;                   // USCI_A0 RX buffer ready?
    return UCA0RXBUF;
}

void uart_putc(unsigned char c)
{
    while (!(UCA0IFG & UCTXIFG))
        ;              // USCI_A0 TX buffer ready?
    UCA0TXBUF = c;                          // TX
}

void uart_puts(const char *str)                 // Output a string
{
    while (*str)
        uart_putc(*str++);
}

void printnum(unsigned long num)
{
    if (num / (unsigned long) 10 != 0)
        printnum(num / (unsigned long) 10);
    uart_putc((char) (num % (unsigned long) 10) + '0');
    return;
}

void crlf(void)                  // send a crlf
{
    uart_puts("\n\r");
}

void print_ok()
{
    uart_puts((char *) "OK");
    crlf();
}

//txtzyme
void store_pgm(char *buf) // Check if the text starts with a colon and if so store in user's word RAM array  parray[]
{
    char name = *buf++;
    int i = 0;
    while (*buf)
    {
        bufWrite((parray + (len * (name - 65) + i)), *buf++);
        i++;
    }
    bufWrite((parray + len*(name-65)+i), 0);
}

void textEval(char *buf)
{
    unsigned long temp;
    char ch, in_byte, mode;
    int j, l, k;
    char * loop;
    //  char * start;

    while ((ch = *buf++))
    {
        switch (ch)
        {
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
            while (*buf >= '0' && *buf <= '9')
            {
                x = x * 10 + (*buf++ - '0');     // If  a number store it in "x"
            }
            break;
        case ' ': //shift-in
            t = z;
            z = y;
            y = x;
            break;
        case '~': //swap
            temp = x;
            x = y;
            y = temp;
            break;
        case '`': //drop
            x = y;
            y = z;
            z = t;
            break;
        case '+':
            x = x + y;
            y = z;
            z = t;
            break;
        case '*':
            x = x * y;
            y = z;
            z = t;
            break;

        case 'p':
            printnum(x);                        // print long integer
            break;

        case 'v':
            uart_puts((char *) "PR1 v0.1\n\r");
            crlf();

            break;

        case '_': // Print the string enclosed between underscores  eg.  _Hello_
            while ((ch = *buf++) && ch != '_')
            {
                uart_putc(ch);
            }
            crlf();
            break;

        case ':':
            store_pgm(buf);
            return; //jump out already
            //break;
            //-------------------------------------------------------------------------------
            // User Words
        case 'A':     // Point the interpreter to the array containing the words
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
            textEval(parray + (len * (ch - 65))); // Evaluate and execute the User's expression fo RAM
            break;
        case '?':
            // Print out all the RAM

            parray = &array[0][0]; // reset parray to the pointer to the first element

            uart_puts("PRG:");

            for (j = 0; j < 26; j++)
            {
                uart_putc(j + 65);                  // print the caps word name
                uart_putc(' ');

                for (l = 0; l < len; l++)
                {
                    in_byte = bufRead(parray + (j * len) + l); // read the array
                    uart_putc(in_byte); // print the character to the serial port
                }
                crlf();
            }
            crlf();
            uart_puts("STK: ");
            printnum(t);
            uart_putc(' ');
            printnum(z);
            uart_putc(' ');
            printnum(y);
            uart_putc(' ');
            printnum(x);
            crlf();
            uart_puts("ARRY: ");
            for (j = 0; j < 8; j++)
            {
                printnum(data_buf[j]);
                uart_putc(' ');
            }
            crlf();
            uart_puts("COBS: ");
            for (j = 0; j < COBS_BUF_SIZE; j++)
            {
                printnum(cobs_buf[j]);
                uart_putc(' ');
            }

            break;
        case '[':                               //start of array of data
            mode = 0;
            j = 0;
            break;

        case '(':  // start of case
            mode = 1;
            k = x;
            while (k)
            {
                ch = *buf++;
                if (ch == ',')
                {
                    k--;  //skip to the comma before the code
                }
            }
            break;
        case ',':
            if (mode)  //case mode; done execution, skip to the end
            {
                while (1)
                {
                    ch = *buf++;
                    if (ch == ')')
                    {
                        break;
                    }
                }

            }
            else
            {
                data_buf[(j & 0x07)] = x;
                j++;
            }
            break;

        case ']':
            data_buf[(j & 0x07)] = x;
            x = j + 1;
            break;
        case '@':
            x = data_buf[(x & 0x07)];
            break;
        case 'm':
            delay_mS(x);
            break;

        case 'u':
            delay_uS(x);
            break;
        case 'h':
        {
            P1OUT |= BIT1;
        }
            break;

        case 'l':
        {
            P1OUT &= ~BIT1;
        }
            break;

        case '{':
            k = x;
            loop = buf;
            while ((ch = *buf++) && ch != '}')
            {
            }
        case '}':
            if (k)
            {
                k--;
                buf = loop;
            }
            break;

        case 'k':
            x = k;
            break;
        }
    }
}

void exec_cmd()
{
    int j, i;
    switch (cmd_byte)
    {
    case 0x84:
        j = 0;
        for (i = 0; i < COBS_BUF_SIZE + 2; i++)
        {
            send_buffer[i] = 0;
        }
        for (i = 1; i < COBS_BUF_SIZE; i++)
        {
            send_buffer[i] = cobs_buf[i];
        }
        for (i = 1; i < COBS_BUF_SIZE + 2; i++)
        {
            if (send_buffer[i] == 0)
            {
                send_buffer[i] = i - j;
                j = i;
            }
        }
        for (i = 0; i < COBS_BUF_SIZE + 2; i++)
        {
            uart_putc(cobs_buf[i]);
        }
        break;
    case 0x94:
        opmode = TXTMODE;
        break;
    case 0x95:
        opmode = COBSMODE;
        break;
    }
}

/**
 * main.c
 */
void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

    PM5CTL0 &= ~LOCKLPM5; // Disable the GPIO power-on default high-impedance mode
                          // to activate 1previously configured port settings
    __bis_SR_register(SCG0);                 // disable FLL
    CSCTL3 |= SELREF__REFOCLK;               // Set REFO as FLL reference source
    CSCTL0 = 0;                              // clear DCO and MOD registers
    CSCTL1 &= ~(DCORSEL_7);             // Clear DCO frequency select bits first
    CSCTL1 |= DCORSEL_3;                     // Set DCO = 8MHz
    CSCTL2 = FLLD_0 + 243;                   // DCODIV = 8MHz
    __delay_cycles(3);
    __bic_SR_register(SCG0);                 // enable FLL
    while (CSCTL7 & (FLLUNLOCK0 | FLLUNLOCK1))
        ; // Poll until FLL is locked

    CSCTL4 = SELMS__DCOCLKDIV | SELA__REFOCLK; // set default REFO(~32768Hz) as ACLK source, ACLK = 32768Hz
                                               // default DCODIV as MCLK and SMCLK source

    // Configure GPIO
    Init_GPIO();
    Init_UART(); //baudrate = 115200

    uart_puts((char *) "PR1 on MSP430F2433\n\r"); // send opening banner message

    unsigned char ch;
    char * p;
    int i, zero_cnt;
    enum _State
    {
        IDLE = 0, TXT, COBS
    } state = IDLE;

    parray = &array[0][0]; // parray is the pointer to the first element of code buffer
    //cobs + txtzyme
    while (1)
    {
        ch = uart_getc();

        switch (state)
        {
        case IDLE:
            i = 0;
            if ((ch & 0x80) == 0x80)
            { //MSB==1
                cmd_byte = ch;
                exec_cmd();
            }
            else
            {
                if (opmode == COBSMODE && ch <= COBS_BUF_SIZE + 1)
                {
                    zero_cnt = ch;
                    state = COBS;

                }
                else if (opmode == TXTMODE && ch >= ' ' && ch <= '~')
                {
                    p = buf;
                    *p++ = ch;
                    state = TXT;
                }
            }
            break;
        case COBS:
            zero_cnt--;
            if (ch == 0)
            {
                state = IDLE;
            }
            else
            {
                if (zero_cnt == 0)
                {
                    cobs_buf[i] = 0;
                    zero_cnt = ch;
                }
                else
                {
                    cobs_buf[i] = ch;
                }
                if (++i == COBS_BUF_SIZE)
                {
                    state = IDLE;
                }
            }

            break;
        case TXT:
            if (ch == '\n' || ch == '\r')
            {
                *p = 0; //terminate input string for textEval
                //exe
                textEval(buf);
                state = IDLE;
            }
            else
            {
                *p++ = ch;
                ++i;
            }
            if (i >= 64)
            {
                //input overflow
                state = IDLE;
            }

            break;

        }

    }

}

void Init_GPIO()
{
    P1DIR = 0xFF;
    P2DIR = 0xFF;
    P3DIR = 0xFF;
    P1REN = 0xFF;
    P2REN = 0xFF;
    P3REN = 0xFF;
    P1OUT = 0x01;
    P2OUT = 0xFF;
    P3OUT = 0x17;
}

void Init_UART() //115200
{

    // Configure UART pins
    P1SEL0 |= BIT4 | BIT5;                  // set 2-UART pin as second function

    // Configure UART
    UCA0CTLW0 |= UCSWRST;
    UCA0CTLW0 |= UCSSEL__SMCLK;

    UCA0BR0 = 4;                             // 8000000/16/115200
    UCA0BR1 = 0x00;
    UCA0MCTLW = 0x55 | UCOS16 | UCBRF_1;

    UCA0CTLW0 &= ~UCSWRST;                    // Initialize eUSCI

}
