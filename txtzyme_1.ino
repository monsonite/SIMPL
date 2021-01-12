unsigned int x = 0;
int d = 13;
const unsigned int bufsize = 80;

void setup() {
  Serial.begin(115200);
  txtEval("_Hello and Welcome to SIMPL_\r\n");   // hint for a startup command
}

void loop() {
  char buf[bufsize];
  txtRead(buf, bufsize);  // wait for commands
  txtEval(buf);
}

void txtRead (char *p, unsigned int n) {
  unsigned int i = 0;
  while (i < (n-1)) {
    while (!Serial.available());  // loop while waiting for input
    char ch = Serial.read();
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
    x = ch - '0';   while (*buf >= '0' && *buf <= '9') {        x = x*10 + (*buf++ - '0');      }      break;
  
    case 'p':       Serial.println(x);       break;
    case 'd':       d = x;                   break;
    case 'i':       pinMode(d,INPUT);
    case 'r':       x = digitalRead(d);      break;
    case 'o':       pinMode(d,OUTPUT);       digitalWrite(d, x%2);      break;
    case 'O':       pinMode(d,OUTPUT);       analogWrite(d,x);          break;  // write analog
    case 'T':       if (x ==0 )              noTone(d);      else       tone(d,x);      break;
    case 'm':       delay(x);                break;
    case '{':       k = x;                   loop = buf;      while ((ch = *buf++) && ch != '}') {      }
    case '}':       if (k) {                 k--;             buf = loop;       }      break;
    case 'k':       x = k;                   break;
    case '_':       while ((ch = *buf++) && ch != '_') {        Serial.print(ch);      }      Serial.println();      break;
    case 's':       x = analogRead(x);      break;
    }
  }
}
