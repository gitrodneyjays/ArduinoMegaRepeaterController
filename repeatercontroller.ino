/////////////////////////////////////////////////////
///             ARDUINO MEGA PINOUT               ///
/////////////////////////////////////////////////////

// PIN 22  - COR DETECT - REPEATER
// PIN 24  - REPEATER PTT
// PIN 25 - CW AUDIO

// PIN 32 - COR DETECT - LINK RADIO 1
// PIN 34  - LINK RADIO 1 PTT
// PIN 35 - CW AUDIO

// PIN 42 - COR DETECT - LINK RADIO 2
// PIN 44 - LINK RADIO 2 PTT 
// PIN 45 - CW AUDIO


/////////////////////////////////////////////////////
///               END PINOUT                      ///
/////////////////////////////////////////////////////


/////////////////////////////////////////////////////
///           USER SPECIFIC STUFF                 ///
/////////////////////////////////////////////////////

// the message string below are stored in flash/program memory. Leave space before and after!
const unsigned char id_message[] PROGMEM = " N4BRC ";

unsigned long pttTimeoutTime = 90000;     // how long before PTT line is timed out (inhibited)
unsigned long pttTimeoutPeriod = 5000;    // how long to stay inactive after timing out
unsigned long hangTimePeriod = 3500;      // how long to hang after COR is lost
unsigned long IDintervalActive = 510000;  // ID interval
unsigned long IDnow = 570000;             // ID now, Hard ID time
int morse_speed = 18;                     // CW Speed in WPM
#define CW_PITCH 795

/////////////////////////////////////////////////////
///         END OF USER SPECIFIC STUFF            ///
/////////////////////////////////////////////////////


unsigned long heartBeatPeriod = 100;      // number of millis of heartbeat flash
unsigned long corDebounceInterval = 250;  // number of millis before cor signal is recognized

#define HIGH_BEEP 1500         // freq high beep Hertz
#define LOW_BEEP 400           // freq low beep Hertz

// repeater states
#define R_IDLE 0
#define R_TRANSMITTING 1
#define R_TIMEOUT 2
#define R_HANGTIME 3


//////////////////////////////////
// Morse Code translation Table //
//////////////////////////////////
const byte morse_table[] PROGMEM = {
  'A', 0B00000101,
  'B', 0B00011000,
  'C', 0B00011010,
  'D', 0B00001100,
  'E', 0B00000010,
  'F', 0B00010010,
  'G', 0B00001110,
  'H', 0B00010000,
  'I', 0B00000100,
  'J', 0B00010111,
  'K', 0B00001101,
  'L', 0B00010100,
  'M', 0B00000111,
  'N', 0B00000110,
  'O', 0B00001111,
  'P', 0B00010110,
  'Q', 0B00011101,
  'R', 0B00001010,
  'S', 0B00001000,
  'T', 0B00000011,
  'U', 0B00001001,
  'V', 0B00010001,
  'W', 0B00001011,
  'X', 0B00011001,
  'Y', 0B00011011,
  'Z', 0B00011100,

  '0', 0B00111111,
  '1', 0B00101111,
  '2', 0B00100111,
  '3', 0B00100011,
  '4', 0B00100001,
  '5', 0B00100000,
  '6', 0B00110000,
  '7', 0B00111000,
  '8', 0B00111100,
  '9', 0B00111110,

  '#', 0B11000101,  // /BK
  '+', 0B00101010,  // /AR
  ',', 0B01110011,
  '-', 0B01100001,
  '.', 0B01010101,
  '/', 0B00110010,
  '=', 0B00110001,
  '?', 0B01001100,
  '^', 0B01000101,  // /SK

  0xff  // end-of-table marker
};


int corSignal = 22;    // Carrier present signal pin (input)
// LOW = Carrier Present   HIGH = No Carrier
int carDetectInd = 23;  // carrier detected indicator
// LOW (off) = No carrier signal detected  HIGH (on) = Carrier Detected
int pttLine = 24;      // PPT Line - This is what keys repeater transmitter
int speakerPin = 25;  // Keyed CW audio (5V signal)

int linkCorSignal = 32;    // Carrier present signal pin (input)
// LOW = Carrier Present   HIGH = No Carrier
int linkCarInd = 33;  // carrier detected indicator
// LOW (off) = No carrier signal detected  HIGH (on) = Carrier Detected
int linkPTTLine = 34; // Link PPT Line - This is what keys the link transmitter
int speaker1Pin = 35;  // Keyed CW audio (5V signal)

int link2CorSignal = 42;    // Carrier present from 2nd link signal pin (input)
// LOW = Carrier Present   HIGH = No Carrier
int link2CarInd = 43;  // carrier detected indicator for 2nd link
// LOW (off) = No carrier signal detected  HIGH (on) = Carrier Detected
int link2PTTLine = 44; // Link PPT Line - This is what keys the 2nd link transmitter
int speaker2Pin = 45;  // Keyed CW audio (5V signal)


int transTimeOutInd = 6;  // Red LED - transmitter time-out indicator
// LOW (off) = normal   HIGH (on) = transmitter timeout period occurring
int hangTimeInd = 5;  // Green LED - hang time indicator

// LOW (off) = not in hang time wait state   HIGH (on) = currently in hang time wait state
int heartBeatLED = 13;  // this is just a heartbeat LED.  It lets us know our loop is running.  this is handy
// when in or near RF environment which can cause microprocessors to lock up.


unsigned long curLoopTime = 0;          // millis at top of current loop
unsigned long heartBeatLast = 0;        // millis of last heartbeat
unsigned long corSigLastDebounce = 0;   // millis of last cor signal debounce;
unsigned long curTransTime = 0;         // number of millis PTT line has been on
unsigned long pttTimeoutStart = 0;      // millis when timeout started
unsigned long hangTimeStart = 0;        // millis hang time period stated
unsigned long lastIDactive = 0;         // millis of last ID
unsigned long timeOfLastTransmit = 0;   // millis of last transmitter unkey
bool heartBeatState = LOW;
bool transInhibit = false;  // inhitbit the transmitter
bool corFirst = true;
bool IDFlag = false;
bool hangLink = false;
bool hangLink2 = false;
int controllerState = R_IDLE;
int ele_len = ((1.2 / morse_speed) * 1000);
byte char2send = 0;


void setup() {
  // set up our pins as outputs and inputs
  pinMode(heartBeatLED, OUTPUT);
  pinMode(hangTimeInd, OUTPUT);
  pinMode(transTimeOutInd, OUTPUT);
  pinMode(linkCarInd, OUTPUT);
  pinMode(link2CarInd, OUTPUT);
  pinMode(carDetectInd, OUTPUT);
  pinMode(pttLine, OUTPUT);
  pinMode(linkPTTLine, OUTPUT);
  pinMode(link2PTTLine, OUTPUT);
  pinMode(speakerPin, OUTPUT);
  pinMode(speaker1Pin, OUTPUT);
  pinMode(speaker2Pin, OUTPUT);
  pinMode(corSignal, INPUT_PULLUP);
  pinMode(linkCorSignal, INPUT_PULLUP);
  pinMode(link2CorSignal, INPUT_PULLUP);

  // default our output states
  digitalWrite(heartBeatLED, LOW);
  digitalWrite(hangTimeInd, LOW);
  digitalWrite(transTimeOutInd, LOW);
  digitalWrite(carDetectInd, LOW);
  digitalWrite(linkCarInd, LOW);
  digitalWrite(link2CarInd, LOW);
  digitalWrite(pttLine, HIGH);
  digitalWrite(linkPTTLine, HIGH);
  digitalWrite(link2PTTLine, HIGH);

  timeOfLastTransmit = 0;
  lastIDactive = 0;
  IDFlag = false;
  corFirst = true;
  controllerState = R_IDLE;
}

void loop()
{
  curLoopTime = millis();
  // flashes the heartbeat LED
  Heartbeat();
  // check if it is time to ID
  checkIDTime();
  // here the different repeater controller states (modes) are handled
  switch (controllerState)
  {
    case R_IDLE:  // idle - waiting for a carrier handler
      checkCOR();
      break;

    case R_TRANSMITTING:  // transmitting handler
      check4Timeout();
      break;

    case R_TIMEOUT:
      waitTimeoutPeriod();  // transmit time-out handler
      break;

    case R_HANGTIME:
      waitHangTimePeriod();  // hang time handler
      break;

    default:
      break;
  }
}


void checkIDTime()
{
  if ((millis() - lastIDactive) < IDintervalActive)
  {
    return;
  }

  if ((millis() - timeOfLastTransmit) < IDnow || IDFlag == true)
  {
    if (controllerState != R_TRANSMITTING)
    {
    send_cw_id();
    delay(100);
    hangTimeStart = millis();
    controllerState = R_HANGTIME; 
    }
  }
}

void send_cw_id()
{
  delay(500);
  digitalWrite(linkPTTLine, LOW);
  digitalWrite(link2PTTLine, LOW);
  digitalWrite(pttLine, LOW);
  delay(500);
  send_morse_msg(id_message);
  delay(300);
  lastIDactive = millis();
  digitalWrite(linkPTTLine, HIGH);
  digitalWrite(link2PTTLine, HIGH);
  digitalWrite(pttLine, HIGH);
  IDFlag = false;
  delay(500);
}

void courtesy_tone()
{
  delay(150);
  tone(speakerPin, HIGH_BEEP);
  delay(100);
  noTone(speakerPin);
}

void beep_boop()
{
  delay(150);
  tone(speakerPin, HIGH_BEEP);
  tone(speaker1Pin, HIGH_BEEP);
  tone(speaker2Pin, HIGH_BEEP);
  delay(100);
  tone(speakerPin, LOW_BEEP);
  tone(speaker1Pin, LOW_BEEP);
  tone(speaker1Pin, LOW_BEEP);
  delay(100);
  noTone(speakerPin);
  noTone(speaker1Pin);
  noTone(speaker2Pin);
}

void boop_beep()
{
  delay(150);
  tone(speakerPin, LOW_BEEP);
  tone(speaker1Pin, LOW_BEEP);
  tone(speaker1Pin, LOW_BEEP);
  delay(100);
  tone(speakerPin, HIGH_BEEP);
  tone(speaker1Pin, HIGH_BEEP);
  tone(speaker2Pin, HIGH_BEEP);
  delay(100);
  noTone(speakerPin);
}

void send_dit()
{
  tone(speakerPin, CW_PITCH);
  tone(speaker1Pin, CW_PITCH);
  tone(speaker2Pin, CW_PITCH);
  delay(ele_len);
  noTone(speakerPin);
  noTone(speaker1Pin);
  noTone(speaker2Pin);
  delay(ele_len);
}

void send_dah()
{
  tone(speakerPin, CW_PITCH);
  tone(speaker1Pin, CW_PITCH);
  tone(speaker2Pin, CW_PITCH);
  delay(ele_len * 3);
  noTone(speakerPin);
  noTone(speaker1Pin);
  noTone(speaker2Pin);
  delay(ele_len);
}

void send_morse_char(byte c)
{
  byte bp = 0;
  byte ditordah = 0;
  bool first = false;
  unsigned int j = 0;

  switch (c)
  {
    case ' ':
      delay(ele_len * 7);
      break;

    case 8:
    case 127:
      break;

    default:
    // while bp is not our end of table flag
      while (bp != 0xff)
      {
        // we should be pointing as an alpha/digit/puncutation character in our table
        bp = pgm_read_byte_near(morse_table + j);

        // have we reached the end of our table ?
        if (bp != 0xff)
        {
          // is the chacater we're pointing to in the Morse table the same as the
          // character we want to send ?
          if (bp == c)
          {
            // yes - so bump our pointer to the Morse code chacter bit pattern for
            // the chacater we want to send
            j++;

            // now get the bit pattern into bp
            bp = pgm_read_byte_near(morse_table + j);

            // start processing the bit pattern
            for (int i = 0; i < 8; i++)
            {
              // get the high bit of the pattern into our ditordah variable
              ditordah = (bp & 128);

              // have we found our start flag yet ?
              if (first == false)
              {
                // if no, is it our start flag
                if (ditordah != 0)
                {
                  // yes - set our flag
                  first = true;
                }

                // now shift the bit pattern one bit to the left and continue
                bp = (bp << 1);
                continue;
              }

              // if we've seen our start flag then send the dash or dot based on the bit
              if (ditordah != 0)
              {
                send_dah();
              }
              else
              {
                send_dit();
              }

              // now shift the bit pattern one bit to the left and continue
              bp = (bp << 1);
            }

            // there is a three element delay between chacaters.  the send_dash() or
            // send_dot() functions already add a one element delay so we delay
            // two more element times.
            delay(ele_len * 2);
            return;
          }
        }

        j++;
        j++;
      }

      break;
  }
}

void send_morse_msg(const unsigned char* msg_adr)
{
  unsigned int p = 0;

  while (true)
  {
    char2send = pgm_read_byte_near(msg_adr + p++);

    if (char2send == 0)
    {
      return;
    }

    send_morse_char(char2send);
  }
}

void waitHangTimePeriod()
{
  if (digitalRead(corSignal) == LOW || digitalRead(linkCorSignal) == LOW || digitalRead(link2CorSignal) == LOW)
  {
    curTransTime = millis();
    if (transInhibit == false)
      {
        if (digitalRead(corSignal) == LOW)
        {
          digitalWrite(linkCarInd, LOW);
          digitalWrite(link2CarInd, LOW);
          digitalWrite(carDetectInd, HIGH);
          digitalWrite(hangTimeInd, LOW);
          digitalWrite(pttLine, HIGH);
          digitalWrite(linkPTTLine, LOW);
          digitalWrite(link2PTTLine, LOW);      
        }
         else if (digitalRead(linkCorSignal) == LOW)
        {
          digitalWrite(carDetectInd, LOW);
          digitalWrite(linkCarInd, HIGH);
          digitalWrite(hangTimeInd, LOW);
          digitalWrite(pttLine, LOW);
          digitalWrite(linkPTTLine, HIGH);
          digitalWrite(link2PTTLine, LOW);      
        }

        else if (digitalRead(link2CorSignal) == LOW)
        {
          digitalWrite(carDetectInd, LOW);
          digitalWrite(linkCarInd, LOW);
          digitalWrite(link2CarInd, HIGH);
          digitalWrite(hangTimeInd, LOW);
          digitalWrite(pttLine, LOW);
          digitalWrite(linkPTTLine, LOW);
          digitalWrite(link2PTTLine, HIGH);      
        }

        timeOfLastTransmit = millis();
        IDFlag = true;
      }
    controllerState = R_TRANSMITTING;
  }
  else
  {
    if ((millis() - hangTimeStart) < hangTimePeriod)
    {
      digitalWrite(linkPTTLine, HIGH);
      digitalWrite(link2PTTLine, HIGH);
      return;
    }
    digitalWrite(pttLine, HIGH);
    digitalWrite(carDetectInd, LOW);
    digitalWrite(linkCarInd, LOW);
    digitalWrite(linkPTTLine, HIGH);
    digitalWrite(link2CarInd, LOW);
    digitalWrite(link2PTTLine, HIGH);
    digitalWrite(hangTimeInd, LOW);
    corSigLastDebounce = millis();
    corFirst = true;
    controllerState = R_IDLE;
  }
}

void waitTimeoutPeriod()
{
  
  if (digitalRead(linkCorSignal) == LOW)
    {
      digitalWrite(linkPTTLine, HIGH);
      digitalWrite(linkCarInd, HIGH);
      digitalWrite(pttLine, LOW);
      digitalWrite(carDetectInd, LOW);
      digitalWrite(link2PTTLine, LOW);
      digitalWrite(link2CarInd, LOW);      
      return;
    }
    else if (digitalRead(corSignal) == LOW)
    {
    digitalWrite(pttLine, HIGH);
    digitalWrite(carDetectInd, HIGH);
    digitalWrite(linkPTTLine, LOW);
    digitalWrite(linkCarInd, LOW);
    digitalWrite(link2PTTLine, LOW);
    digitalWrite(link2CarInd, LOW);
    return;
    }
    else if (digitalRead(link2CorSignal) == LOW)
    {
    digitalWrite(pttLine, LOW);
    digitalWrite(carDetectInd, LOW);
    digitalWrite(linkPTTLine, LOW);
    digitalWrite(linkCarInd, LOW);
    digitalWrite(link2PTTLine, HIGH);
    digitalWrite(link2CarInd, HIGH);
    return;
    }

  if (digitalRead(link2CorSignal) == HIGH)
  {
    digitalWrite(link2CarInd, LOW);
  }
  else
  {
    digitalWrite(link2CarInd, HIGH);
  }
  if (digitalRead(linkCorSignal) == HIGH)
  {
    digitalWrite(linkCarInd, LOW);
  }
  else
  {
    digitalWrite(linkCarInd, HIGH);
  }
  if (digitalRead(corSignal) == HIGH)
  {
    digitalWrite(carDetectInd, LOW);
  }
  else
  {
    digitalWrite(carDetectInd, HIGH);
  }
  
  if ((millis() - pttTimeoutStart) < pttTimeoutPeriod)
  {
    return;
  }
  digitalWrite(transTimeOutInd, LOW);
  transInhibit = false;
  digitalWrite(pttLine, LOW);
  digitalWrite(linkPTTLine, LOW);
  digitalWrite(link2PTTLine, LOW);
  delay(500);
  boop_beep();
  delay(500);
  digitalWrite(pttLine, HIGH);
  digitalWrite(linkPTTLine, HIGH);
  digitalWrite(link2PTTLine, HIGH);
  digitalWrite(carDetectInd, LOW);
  corSigLastDebounce = millis();
  timeOfLastTransmit = millis();
  corFirst = true;
  controllerState = R_IDLE;
}

void check4Timeout()
{
  if (digitalRead(corSignal) == LOW || (digitalRead(linkCorSignal) == LOW) || (digitalRead(link2CorSignal) == LOW))
  {
    if ((millis() - curTransTime) < pttTimeoutTime)
    {
      return;
    }
    delay(300);
    beep_boop();
    delay(300);
    pttTimeoutStart = millis();
    transInhibit = true;
    digitalWrite(transTimeOutInd, HIGH);
    digitalWrite(pttLine, HIGH);
    digitalWrite(linkPTTLine, HIGH);
    digitalWrite(link2PTTLine, HIGH);
    controllerState = R_TIMEOUT;
  }
  else
  {
    digitalWrite(carDetectInd, LOW);
    digitalWrite(linkCarInd, LOW);
    digitalWrite(link2CarInd, LOW);

    if (lastIDactive == 0)
    {
      send_cw_id();
    }
    else
    {
      IDFlag = true;
      hangTimeStart = millis();
      digitalWrite(transTimeOutInd, LOW);
      digitalWrite(hangTimeInd, HIGH);
      controllerState = R_HANGTIME;      
    }
  }
}

void checkCOR()
{
  if (digitalRead(corSignal) == LOW || (digitalRead(linkCorSignal) == LOW) || (digitalRead(link2CorSignal) == LOW))
  {
    if (corFirst == true)
    {
      corSigLastDebounce = millis();
      corFirst = false;
      IDFlag = false;
      return;
    }
    if ((millis() - corSigLastDebounce) < corDebounceInterval)
    {
      return;
    }
    corSigLastDebounce = 0;
    if (transInhibit == false)
    {
      curTransTime = millis();
      
      if (digitalRead(linkCorSignal) == LOW) // Link #1 is receiving
      {
      digitalWrite(linkCarInd, HIGH); // Turn on the Link #1 receive indicator
      digitalWrite(linkPTTLine, HIGH); // Ensuring PTT is off on link #1
      digitalWrite(pttLine, LOW); // Keying the repeater
      digitalWrite(link2PTTLine, LOW); // Keying the 2nd link radio
      }
      
     else if (digitalRead(corSignal) == LOW) // Repeater is receiving
      {
      digitalWrite(carDetectInd, HIGH); // Turn on the Repeater receive indicator
      digitalWrite(pttLine, LOW); // Keying the repeater
      digitalWrite(linkPTTLine, LOW); // Keying the 1st link radio
      digitalWrite(link2PTTLine, LOW); // Keying the 2nd link radio
      }
      
      else if (digitalRead(link2CorSignal) == LOW) // Link #2 is receiving
      {
      digitalWrite(link2CarInd, HIGH); // Turn on the Link #2 receive indicator
      digitalWrite(link2PTTLine, HIGH); // Ensuring PTT is off on link #2
      digitalWrite(pttLine, LOW); // Keying the repeater
      digitalWrite(linkPTTLine, LOW); // Keying the 1st link radio
      }
      controllerState = R_TRANSMITTING;
      }
  }
  else
  {
    corFirst = true;
  }
}

// toggle the heartbeart LED every heartBeatPeriod number of ms.
// except when sending CW or beeps.
void Heartbeat()
{
  if ((millis() - heartBeatLast) < heartBeatPeriod)
  {
    return;
  }

  heartBeatLast += heartBeatPeriod;
  heartBeatState = !heartBeatState;

  digitalWrite(heartBeatLED, heartBeatState);
}

