// SIMPL - A Serial Interpreted Minimal Programming Language
// Inspired by Txtzyme - by Ward Cunningham
// This version runs on STM32L433 on BlackIce FPGA board - September 2017
// SIMPL allows new words to be defined by preceding them with colon :  (Like Forth)
// New words use CAPITALS - so 26 words are possible in the vocabulary 
// Words A-F have been predefined as musical tones - but you can write over them
// A word can be a maximum of 48 characters long 
// Type ? to get a list off all defined words
 
  #define bufRead(addr)      (*(unsigned char *)(addr))
  #define bufWrite(addr, b)   (*(unsigned char *)(addr) = (b))
 
   unsigned char bite;
   long x = 0;
  long  y = 0;
   int len = 48;
   
   long old_millis=0;
   long new_millis=0;
  
  
   char array[26][48] = {                            // Define a 26 x 48 array for the colon definitions
                         {"6d40{1o1106u0o1106u}"},
                         {"6d45{1o986u0o986u}"},
                         {"6d51{1o929u0o929u}"},
                         {"6d57{1o825u0o825u}"},
                         {"6d64{1o733u0o733u}"},
                         {"6d72{1o690u0o691u}"},
                         {"6d81{1o613u0o613u}"},
                         {"_Hello World, and welcome to SIMPL_"},
                         {"5{ABC}"},
                         {""},
                         {""},
                         {""},
                         {"_This is a test message - about 48 characters_"}
                         };
                         
  
    int a = 0;          // integer variables a,b,c,d
    int b = 0;
    int c = 0;
    int d = 47;          // d is used to denote the digital port pin for I/O operations
    
    char name;
    
    char* parray;
    
    char buf[64];
    
    char* addr;
 
   
  
void setup()
{
    Serial1.begin(115200);
    pinMode(d,OUTPUT);
    delay(2000);
    Serial1.println("Type H for Welcome or ? for Help");
    
    parray = &array[0][0];                // parray is the pointer to the first element 
 
    
}
 
 
 
void loop()
{
 
  txtRead(buf, 64);
  
  txtChk(buf);          // check if it is a colon definition 
 
  txtEval(buf);
 
 
 
}
 
void txtRead (char *p, byte n) {
  byte i = 0;
  while (i < (n-1)) {
    while (!Serial1.available());
    char ch = Serial1.read();
    if (ch == '\r' || ch == '\n') break;
    if (ch >= ' ' && ch <= '~') {
      *p++ = ch;
      i++;
    }
  }
  *p = 0;
}
 
 
void txtChk (char *buf) {       // Check if the text starts with a colon and if so store in temp[]
 
  if (*buf == ':')  {  
  char ch;
  int i =0;
  while ((ch = *buf++)){
    
  if (ch == ':') {
  Serial1.println(*buf);   // get the name from the first character
  name = *buf ;
  buf++;
  }
 
  bufWrite((parray + (len*(name-65) +i)),*buf);
   
  i++;
 
}

 x = 1;
 
}


 
}
 
 
  
  
  
  
  void txtEval (char *buf) {
  unsigned int k = 0;
 
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
      Serial1.println(x);
      break;
      
      /*
      case 'a':
      a = x;
      break;
      
      */
      
      case 'b':
      Serial1.println(millis());
      break;
      
      case 'c':
      Serial1.println(micros());
      break;
      
      case 'd':
      d = x;
      break;
      
      
      
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
      
      
    
      name = ch - 65;
      addr = parray + (len*name);
      
      txtEval(addr);
      break;
      
      case 'a':   
//      analogWrite(d,x);
      break;
 
 
      case '!':     // store
      y = x;
      break;
      
      case '@':
      x = y;
      break;
      
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
      
      case '<':
      if(x<y){x=1;}      // If x<y x= 1 - can be combined with jump j
      else x=0;
      break;
      
      case '>':
      if(x>y){x=1;}      // If x>y x= 1 - can be combined with jump j
      else x=0;
      break;
      
      case 'j':                  // test  if x = 1 and jump next instruction    
      if(x==1){*buf++;}    
      break;
      
      case 'l':
      y = y + 1;
      break;
      
      case 'x':
      x = x + 1;
      break;
      
      case 'y':
      y = y + 1;
      break;
      
      
      case 'n':                  // Output an 8 bit value on I/O Dig 2  - Dig 9
                                 // Can be extended to 12 bits on Dig 2 - Dig 13
      
      if(x>=128){digitalWrite(9,HIGH); x =  x- 128;} else {digitalWrite(9,LOW);}
      if(x>=64){digitalWrite(8,HIGH); x =  x- 64;} else {digitalWrite(8,LOW);}
      if(x>=32){digitalWrite(7,HIGH); x =  x- 32;} else {digitalWrite(7,LOW);}
      if(x>=16){digitalWrite(6,HIGH); x =  x- 16;} else {digitalWrite(6,LOW);}
      if(x>=8){digitalWrite(5,HIGH); x =  x- 8;} else {digitalWrite(5,LOW);}
      if(x>=4){digitalWrite(4,HIGH); x =  x- 4;} else {digitalWrite(4,LOW);}
      if(x>=2){digitalWrite(3,HIGH); x =  x- 2;} else {digitalWrite(3,LOW);}
      if(x>=1){digitalWrite(2,HIGH); x =  x- 1;} else {digitalWrite(2,LOW);}
      
      break;
      
      
      case '?':                             // Print out all the RAM
      parray = &array[0][0];                // reset parray to the pointer to the first element 
      
      for (int j = 0; j<26; j++) {
      
      Serial1.write(j+65);                  // print the caps word name
      Serial1.write(20);                    // space
        
      for (int i=0; i<len; i++) {
      bite = bufRead( parray +  (j *len )+i);          // read the array
      Serial1.write(bite);                           // print the character to the serial port
    }
    
     Serial1.println();
    
   }
   
   
   
   for(int i = 0; i <11; i++) {
   
   Serial1.println();
   
    }
    
   
      
    break;
    
//----------------------------------------------------------------------------------------------------------------------------------------------- 
// Added 15-2-2015 - all appears to be working

    case '(':                                     // The start of a condition test
      k = x;
      start = buf;                               // remember the start position of the test
      while ((ch = *buf++) && ch != ')') {       // get the next character into ch and increment the buffer pointer *buf - evaluate the code
      } 
     
     case ')':
    if (x) {                                    // if x is positive - go around again
     buf = start;
    }    
      break;
      
      case '.':
      while ((ch = *buf++) && ch != '.') {
//        Serial1.print(ch);
         name = ch - 65;
         addr = parray + (len*name);
        while ((ch = *addr++) && ch != '.') {
         Serial1.print(ch); 
          
      }
      
      }
      Serial1.println();
      break;
      
//      txtEval(addr);
//      break;


      case ' ':                              // Transfer x into second variable y
      y= x;
      break;
      
      case '$':                             // Load x with the ASCII value of the next character  i.e. 5 = 35H or 53 decimal  
      x=*(buf-2);
      break; 

//-----------------------------------------------------------------------------------------------------------------------------------------------       
      
    case 'i':
      x = digitalRead(d);
      break;
    case 'o':
      digitalWrite(d, x%2);
      break;
    case 'm':
      delay(x);
      break;
    case 'u':
      delayMicroseconds(x);
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
        Serial1.print(ch);
      }
      Serial1.println();
      break;
    case 's':
//   x = analogRead(x);
      break;
    case 't':
       Serial1.println(micros());
      break;
     
     case 'z':                                                          // z is a non-blocking pause or nap - measured in "zeconds" which allows UART characters,
     
     old_millis = millis();                                             // get the millisecond count when you enter the switch-case  
      
      while (millis()-old_millis<=(x*1000))
      { 

      if (Serial1.available())
      {
      ch = Serial1.read();
      }
      
      // Put the idle loop and escape code here
      if(ch=='£')
      {
        Serial1.println("Escape");
        break;     
      }
    
      }  
                                                                        // interrupts or digital Inputs to break the pause
//       Serial1.println(millis());
//       break;  
      
      
      
    }
  }
}
