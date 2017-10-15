// SIMPL Assembler Framework

// Ken Boak - October 15th 2017

// A means to translate ascii symbols into numbers or text
// To serve as a simple assembler framework

  #define bufRead(addr)      (*(unsigned char *)(addr))
  #define bufWrite(addr, b)   (*(unsigned char *)(addr) = (b))
 
   unsigned char bite;

   int a = 0;          // integer variables a,b,c,d
   int b = 0;
   int c = 0;
   int d = 47;          // d is used to denote the digital port pin for I/O operations
   long x = 0;
   long  y = 0;
   int len = 48;
   int insn = 0;
   long old_millis=0;
   long new_millis=0;
   
   char array[26][48];    
   char name;   
   char* parray;    
   char buf[64];    
   char* addr;
   
//----------------------------------------------------------------------------------------     
void setup()
{
    Serial.begin(115200);
    pinMode(d,OUTPUT);
    pinMode(6,OUTPUT);
   
    Serial.println("SIMPL Assembler");   
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
    while (!Serial.available());
    char ch = Serial.read();
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
  Serial.println(*buf);   // get the name from the first character
  name = *buf ;
  buf++;
  }
 
  bufWrite((parray + (len*(name-65) +i)),*buf);
   
  i++;
 
}

 x = 1;
 
}
 
}

//---------------------------------------------------------------------------------------
// txtEval is a simple switch-case statement that decodes the instruction character
// and performs the required function
//--------------------------------------------------------------------------------------- 
  
  void txtEval (char *buf) {
  unsigned int k = 0;
 
  char *loop;
  char *start;
  char ch;
  while ((ch = *buf++)) {
    
 
    switch (ch) {

   //  Decode number characters and form them into an integer to store in x   
          
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


//---------------------------------------------------------------------------------------------------
// Decode the ascii symbols into SIMPL promitives


      case ' ':
      insn = 0;
      Serial.print(insn);
      Serial.print("    ");
      Serial.println("SPACE"); 
      break;

      case '!':
      insn = 1;
      Serial.print(insn);
      Serial.print("    ");
      Serial.println("STORE"); 
      break;

      case '"':
      insn = 2;
      Serial.print(insn);
      Serial.print("    ");
      Serial.println("DUP"); 
      break;

      case '#':
      insn = 3;
      Serial.print(insn);
      Serial.print("    ");
      Serial.println("LIT"); 
      break;

      case '$':
      insn = 4;
      Serial.print(insn);
      Serial.print("    ");
      Serial.println("SWAP"); 
      break;

      case '%':
      insn = 5;
      Serial.print(insn);
      Serial.print("    ");
      Serial.println("OVER"); 
      break;

      case '&':
      insn = 6;
      Serial.print(insn);
      Serial.print("    ");
      Serial.println("AND"); 
      break;

      case 39 :
      insn = 7;
      Serial.print(insn);
      Serial.print("    ");
      Serial.println("DROP"); 
      break;

      case '(':
      insn = 8;
      Serial.print(insn);
      Serial.print("    ");
      Serial.println("LHS"); 
      break;

      case ')':
      insn = 9;
      Serial.print(insn);
      Serial.print("    ");
      Serial.println("RHS"); 
      break;

      case '*':
      insn = 10;
      Serial.print(insn);
      Serial.print("    ");
      Serial.println("MUL"); 
      break;

      case '+':
      insn = 11;
      Serial.print(insn);
      Serial.print("    ");
      Serial.println("ADD"); 
      break;

      case ',':
      insn = 12;
      Serial.print(insn);
      Serial.print("    ");
      Serial.println("PUSH"); 
      break;

      case '-':
      insn = 13;
      Serial.print(insn);
      Serial.print("    ");
      Serial.println("SUB"); 
      break;

      case '.':
      insn = 14;
      Serial.print(insn);
      Serial.print("    ");
      Serial.println("POP"); 
      break;

      case '/':
      insn = 15;
      Serial.print(insn);
      Serial.print("    ");
      Serial.println("DIV"); 
      break;

      case ':':
      insn = 16;
      Serial.print(insn);
      Serial.print("    ");
      Serial.println("CALL"); 
      break;

      case ';':
      insn = 17;
      Serial.print(insn);
      Serial.print("    ");
      Serial.println("RET"); 
      break;

      case '<':
      insn = 18;
      Serial.print(insn);
      Serial.print("    ");
      Serial.println("LT"); 
      break;

      case '=':
      insn = 19;
      Serial.print(insn);
      Serial.print("    ");
      Serial.println("EQ"); 
      break;

      case '>':
      insn = 20;
      Serial.print(insn);
      Serial.print("    ");
      Serial.println("GT"); 
      break;

      case '?':
      insn = 21;
      Serial.print(insn);
      Serial.print("    ");
      Serial.println("QUERY"); 
      break;

      case '@':
      insn = 22;
      Serial.print(insn);
      Serial.print("    ");
      Serial.println("FETCH"); 
      break;

      case '[':
      insn = 23;
      Serial.print(insn);
      Serial.print("    ");
      Serial.println("SQL"); 
      break;

      case 92 :
      insn = 25;
      Serial.print(insn);
      Serial.print("    ");
      Serial.println("JMP"); 
      break;

      case ']':
      insn = 25;
      Serial.print(insn);
      Serial.print("    ");
      Serial.println("SQR"); 
      break;

      

      case '^':
      insn = 26;
      Serial.print(insn);
      Serial.print("    ");
      Serial.println("XOR"); 
      break;

      case '_':
      insn = 27;
      Serial.print(insn);
      Serial.print("    ");
      Serial.println("OUT"); 
      break;

      case '~':
      insn = 28;
      Serial.print(insn);
      Serial.print("    ");
      Serial.println("COM"); 
      break;

      case '{':
      insn = 29;
      Serial.print(insn);
      Serial.print("    ");
      Serial.println("CURL"); 
      break;

      case '|':
      insn = 31;
      Serial.print(insn);
      Serial.print("    ");
      Serial.println("OR"); 
      break;

      case '}':
      insn = 31;
      Serial.print(insn);
      Serial.print("    ");
      Serial.println("CURR"); 
      break;

     
            
//---------------------------------------------------------------------------------------------------
//  Decode the upper case commands that are the Users commands - stored in RAM

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
      
      name = ch - 65;               //  get the address of where the code is stored
      addr = parray + (len*name);
      
      txtEval(addr);                //  evaluate the commands stored at that address
      break;

  //---------------------------------------------------------------------      
  //  Decode the common lower case alpha commands 
     
      case 'a':
      
      break;
      
      case 'b':
      
      break;

      case 'c':
      
      break;

      case 'd':
      
      break;

      case 'e':
      
      break;

      case 'f':
      
      break;

      case 'g':
      
      break;

      case 'h':
      
      break;

      case 'i':
      
      break;

      case 'j':
      
      break;

      case 'k':
      
      break;

      case 'l':
      
      break;

      case 'm':
      
      break;

      case 'n':
      
      break;

      case 'o':
      
      break;

      case 'p':
      
      break;

      case 'q':
      
      break;

      case 'r':
      
      break;

      case 's':
      
      break;

      case 't':
      
      break;

      case 'u':
      
      break;

      case 'v':
      
      break;

      case 'w':
      
      break;

      case 'x':
      
      break;

      case 'y':
      
      break;

      case 'z':
      
      break;

      //--------------------------------------------------------------------------------------------
      
    }
  }
}
