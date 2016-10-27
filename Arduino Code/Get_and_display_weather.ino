// Wifi Setup
#define SSID        "[Enter your wifi name here]"                                       // Wifi SSID
#define PASS        "[Enter your wifi password here]"                                 // WiFi Password
#define DEST_HOST   "api.wunderground.com"                           // API site host
#define DEST_IP     "23.222.155.203"                                 // API site IP address
#define API_CURRENT "/api/[Enter your API Key here]/conditions/q/98122.json"  // Wunderground URL
#define API_FORCAST "/api/[Enter your API Key here]/forecast/q/98122.json"
#define CONTINUE    false                                             // Define for readability
#define HALT        true                                              // Define for readibility
#define TIMEOUT     5000                                             // Timeout for echo commands
#define UPDATE      600000                                            // How often to update temperature (10 minutes = 600000)

// SN74141 (1)
#define ledPin_0_a   2
#define ledPin_0_b   3
#define ledPin_0_c   4
#define ledPin_0_d   5

// SN74141 (2)
#define ledPin_1_a   6                
#define ledPin_1_b   7
#define ledPin_1_c   8
#define ledPin_1_d   9

// anod pins
#define ledPin_a_1   10
#define ledPin_a_2   11
#define ledPin_a_3   12

// nixie digits
int d1;
int d2;
int d3;
int d4;
int d5;
int d6;

// wifi command
String cmd;

void setup() 
{
  // initialize Serial port for Wifi:
  Serial.begin(115200);

  // setup nixie output pins
  pinMode(ledPin_0_a, OUTPUT);      
  pinMode(ledPin_0_b, OUTPUT);      
  pinMode(ledPin_0_c, OUTPUT);      
  pinMode(ledPin_0_d, OUTPUT);    
  
  pinMode(ledPin_1_a, OUTPUT);      
  pinMode(ledPin_1_b, OUTPUT);      
  pinMode(ledPin_1_c, OUTPUT);      
  pinMode(ledPin_1_d, OUTPUT);      
  
  pinMode(ledPin_a_1, OUTPUT);      
  pinMode(ledPin_a_2, OUTPUT);      
  pinMode(ledPin_a_3, OUTPUT);

  // Show that the program is starting via nixies 1 and 4.
  digitalWrite(ledPin_0_d, 0);
  digitalWrite(ledPin_0_c, 0);
  digitalWrite(ledPin_0_b, 0);
  digitalWrite(ledPin_0_a, 0);
  digitalWrite(ledPin_1_d, 0);
  digitalWrite(ledPin_1_c, 0);
  digitalWrite(ledPin_1_b, 0);
  digitalWrite(ledPin_1_a, 0);  
  digitalWrite(ledPin_a_1, HIGH);

  // reset WiFi
  while (!Serial) {
    ; // wait for serial port to connect.
  }
  echoCommand("AT+RST", "ready", CONTINUE);
  echoCommand("AT+CWMODE=1", "OK", CONTINUE);  

  //connect to WiFi network
  cmd = "AT+CWJAP=\""; cmd += SSID; cmd += "\",\""; cmd += PASS; cmd += "\"";
  echoCommand(cmd, "OK", CONTINUE);

  // get all three temps
  getTemps();
}

int readTemp(boolean roundIt) {
  delay(1000);
  int val = 0;
  int tens = Serial.read();
  int ones = Serial.read();
  val = (tens - '0') * 10 + (ones - '0');
  if (roundIt) {
    Serial.read();
    int tenths = Serial.read();  
      if ((tenths - '0') >= 5) {
        val += 1;
      }
  }
  return val;
}

boolean echoCommand(String cmd, String ack, boolean halt_on_fail) {
  Serial.println(cmd);
  if (!echoFind(ack))          // timed out waiting for ack string 
    if (halt_on_fail)          // If halt on failure
      errorHalt();             // Critical failure halt.
    else
      return false;            // Let the caller handle it.
  return true;                   // ack blank or ack found
}

boolean echoFind(String keyword) {
  byte current_char   = 0;
  byte keyword_length = keyword.length();
  
  long deadline = millis() + TIMEOUT;              // Calculate timeout deadline
  while(millis() < deadline)                       // Try until deadline
  {
    if (Serial.available())                        // If characters are available
    {
      char ch = Serial.read();
      if (ch == keyword[current_char])
      {
        if (++current_char == keyword_length)
          return true;
      }
    }
  }
  return false;                                    // Timed out
}

void errorHalt() {
  while(true){};
}

// gets the three temperatures
void getTemps() {
  int tempCurrent;
  int tempLow;
  int tempHigh;
  
  // Show we're getting tempCurrent via nixies 2 and 5.
  digitalWrite(ledPin_0_d, 0);
  digitalWrite(ledPin_0_c, 0);
  digitalWrite(ledPin_0_b, 0);
  digitalWrite(ledPin_0_a, 0);
  digitalWrite(ledPin_1_d, 0);
  digitalWrite(ledPin_1_c, 0);
  digitalWrite(ledPin_1_b, 0);
  digitalWrite(ledPin_1_a, 0);  
  digitalWrite(ledPin_a_1, LOW);
  digitalWrite(ledPin_a_2, HIGH);
  
  // connect to weather server
  cmd = "AT+CIPSTART=\"TCP\",\""; cmd += DEST_IP; cmd += "\",80";
  echoCommand(cmd, "OK", CONTINUE);

  // open connection to current temp api
  cmd = "GET "; cmd += API_CURRENT; cmd+= " HTTP/1.1\r\nHost: "; cmd += DEST_HOST; cmd +="\r\n\r\n";
  Serial.print("AT+CIPSEND=");
  Serial.println(cmd.length());

  // get current temp
  if(Serial.find(">")){
    Serial.print(cmd);
    // find the part we are interested in.
    long deadline = millis() + TIMEOUT;
    boolean readSuccess = true;
    while (!Serial.find("\"temp_f\":")){
      if (millis() > deadline){
        readSuccess = false;
        break; // try again if not found
      }
    } 
    if (readSuccess) {
      tempCurrent = readTemp(true);
    } else {
      tempCurrent = 11;
      readSuccess = true;
    }
  }
  
  // close the current temp api connection
  echoCommand("AT+CIPCLOSE", "OK", CONTINUE);  
  delay(2000);

  // Show we're getting the forcast via nixies 3 and 6.
  digitalWrite(ledPin_0_d, 0);
  digitalWrite(ledPin_0_c, 0);
  digitalWrite(ledPin_0_b, 0);
  digitalWrite(ledPin_0_a, 0);
  digitalWrite(ledPin_1_d, 0);
  digitalWrite(ledPin_1_c, 0);
  digitalWrite(ledPin_1_b, 0);
  digitalWrite(ledPin_1_a, 0);  
  digitalWrite(ledPin_a_2, LOW);
  digitalWrite(ledPin_a_3, HIGH);
  
  // connect to weather server
  cmd = "AT+CIPSTART=\"TCP\",\""; cmd += DEST_IP; cmd += "\",80";
  echoCommand(cmd, "OK", CONTINUE);

  
  // open connection to forcast temp api
  cmd = "GET "; cmd += API_FORCAST; cmd+= " HTTP/1.1\r\nHost: "; cmd += DEST_HOST; cmd +="\r\n\r\n";
  Serial.print("AT+CIPSEND=");
  Serial.println(cmd.length());
  
  // get forcast temps
  if(Serial.find(">")){
    Serial.print(cmd);
    
    long deadline = millis() + TIMEOUT;
    boolean readSuccess = true;
    while (!Serial.find("\"fahrenheit\":\"")){
      if (millis() > deadline){
        readSuccess = false;
        break; // try again if not found
      }
    }   
    // Show we're getting the high temp via nixies 2 and 5.
    digitalWrite(ledPin_0_d, 0);
    digitalWrite(ledPin_0_c, 0);
    digitalWrite(ledPin_0_b, 0);
    digitalWrite(ledPin_0_a, 0);
    digitalWrite(ledPin_1_d, 0);
    digitalWrite(ledPin_1_c, 0);
    digitalWrite(ledPin_1_b, 0);
    digitalWrite(ledPin_1_a, 0);  
    digitalWrite(ledPin_a_2, HIGH);
    digitalWrite(ledPin_a_3, LOW);
    
    if (readSuccess) {
      tempHigh = readTemp(false);
    } else {
      tempHigh = 11;
      readSuccess = true;
    }
    
    deadline = millis() + TIMEOUT; 
    while (!Serial.find("\"fahrenheit\":\"")){
      if (millis() > deadline){
        readSuccess = false;
        break; // try again if not found
      }
    }

    // Show we're getting the low temp via nixies 1 and 2.
    digitalWrite(ledPin_0_d, 0);
    digitalWrite(ledPin_0_c, 0);
    digitalWrite(ledPin_0_b, 0);
    digitalWrite(ledPin_0_a, 0);
    digitalWrite(ledPin_1_d, 0);
    digitalWrite(ledPin_1_c, 0);
    digitalWrite(ledPin_1_b, 0);
    digitalWrite(ledPin_1_a, 0);  
    digitalWrite(ledPin_a_2, LOW);
    digitalWrite(ledPin_a_1, HIGH);

    if (readSuccess) {
      tempLow = readTemp(false);
    } else {
      tempLow = 11;
      readSuccess = true;
    }
  }

  // close the forcast api connection
  echoCommand("AT+CIPCLOSE", "OK", CONTINUE);  

  // asign nixie digits the temperatures
  d1 = tempLow / 10;
  d2 = tempLow - (d1 * 10);
  d3 = tempCurrent / 10;
  d4 = tempCurrent - (d3 * 10);
  d5 = tempHigh / 10;
  d6 = tempHigh - (d5 * 10);
}

// Displays two nixie numbers very quickly in a multiplex
void displayNumberSet( int anod, int num1, int num2, int show )
{
  int anodPin;
  int a,b,c,d;

  // Select what anod to fire.
  switch( anod )
  {
    case 0:    anodPin =  ledPin_a_1;    break;
    case 1:    anodPin =  ledPin_a_2;    break;
    case 2:    anodPin =  ledPin_a_3;    break;
  }  
  
  // Load the a,b,c,d.. to send to the SN74141 IC (1)
  switch( num1 )
  {
    case 0: a=1;b=0;c=0;d=1;break;
    case 1: a=0;b=0;c=0;d=0;break;
    case 2: a=1;b=0;c=0;d=0;break;
    case 3: a=0;b=1;c=0;d=0;break;
    case 4: a=1;b=1;c=0;d=0;break;
    case 5: a=0;b=0;c=1;d=0;break;
    case 6: a=1;b=0;c=1;d=0;break;
    case 7: a=0;b=1;c=1;d=0;break;
    case 8: a=1;b=1;c=1;d=0;break;
    case 9: a=0;b=0;c=0;d=1;break;
  }  

  // Write to output pins.
  digitalWrite(ledPin_0_d, d);
  digitalWrite(ledPin_0_c, c);
  digitalWrite(ledPin_0_b, b);
  digitalWrite(ledPin_0_a, a);

  // Load the a,b,c,d.. to send to the SN74141 IC (2)
  switch( num2 )
  {
    case 0: a=1;b=0;c=0;d=1;break;
    case 1: a=0;b=0;c=0;d=0;break;
    case 2: a=1;b=0;c=0;d=0;break;
    case 3: a=0;b=1;c=0;d=0;break;
    case 4: a=1;b=1;c=0;d=0;break;
    case 5: a=0;b=0;c=1;d=0;break;
    case 6: a=1;b=0;c=1;d=0;break;
    case 7: a=0;b=1;c=1;d=0;break;
    case 8: a=1;b=1;c=1;d=0;break;
    case 9: a=0;b=0;c=0;d=1;break;
  }
  
  // Write to output pins
  digitalWrite(ledPin_1_d, d);
  digitalWrite(ledPin_1_c, c);
  digitalWrite(ledPin_1_b, b);
  digitalWrite(ledPin_1_a, a);

  // Turn on this anod.
  digitalWrite(anodPin, HIGH);

  // Delay for each anode to fire
  delay(show);
  
  // Shut off this anod.
  digitalWrite(anodPin, LOW);
}

void showTemps() {  
  long deadline = millis() + UPDATE;
  // run until it's time to update temps
  while(millis() < deadline) {
    displayNumberSet( 0, d4, d1, 1 );
    displayNumberSet( 1, d2, d5, 1 );
    displayNumberSet( 2, d6, d3, 1 );
  }
}

/////////////////////////////////////////////
void loop()     
{
  showTemps();
  getTemps();
}
