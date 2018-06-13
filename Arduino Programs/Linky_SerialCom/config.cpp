#include "config.h"
//LED
volatile int brightR, brightG, brightB = 0;
volatile byte brightCount = 0;

//DC
volatile unsigned char dcDirection1, dcDirection2, dcSpeed1, dcSpeed2, StatePinA, StatePinB, StatePinC, StatePinD, DCPWMCount;
unsigned char DCOffsetL, DCOffsetR;

//TV Remocon
unsigned char Button_up, Button_down, Button_right, Button_center, Button_func1, Button_func2, Button_func3, Button_func4, Button_ch1, Button_ch2, Button_ch3, Button_ch4, Button_funcl, Button_funcr;
volatile unsigned char IR_in, remoconUseTV, dataCode, remoconChannel, inputStatus, stateTVReceive, dataCount, codeBuffer;
volatile long CodePreviousMicros, EndPreviousMillis, LeadPreviousMillis, TimeOutPreviousMillis;

byte channelSetFlag = 0;
int channelSetTimer, tvRemoconChannel = 0;

int RFdataCode;
byte _count, _header, _packet[6];

volatile int irSensor[7];
int speedPublic;

//-----------------------------------------------------------------------------------------------------------------------------------------------------//

//-----------------------------------------------------------------------------------------------------------------------------------------------------//

void initialize()
{
  pinMode(MIC_PIN, INPUT);
  pinMode(MELODY_PIN, OUTPUT);
  pinMode(IR_PIN, INPUT);     //ir receiver
  pinMode(SW1, INPUT);
  pinMode(SW2, INPUT);
  //-------------------------------------------------------------------//
  if (!((EEPROM.read(EEP_FIRST_START_CHECK1) == 'R') && (EEPROM.read(EEP_FIRST_START_CHECK2) == 'O') && (EEPROM.read(EEP_FIRST_START_CHECK3) == 'B') && (EEPROM.read(EEP_FIRST_START_CHECK4) == 'O')))
  {
    Serial.println("FirstStart");
    EEPROM.write(EEP_FIRST_START_CHECK1, 'R');
    EEPROM.write(EEP_FIRST_START_CHECK2, 'O');
    EEPROM.write(EEP_FIRST_START_CHECK3, 'B');
    EEPROM.write(EEP_FIRST_START_CHECK4, 'O');

    EEPROM.write(EEP_REMOCON_CHANNEL, 1);
    EEPROM.write(EEP_MOTOR_OFFSET_L, 50);
    EEPROM.write(EEP_MOTOR_OFFSET_R, 50);
  }

  //-------------------------------------------------------------------//
  tvRemoconChannel = EEPROM.read(EEP_REMOCON_CHANNEL);    //channel
  if (!((tvRemoconChannel == 1) || (tvRemoconChannel == 2) || (tvRemoconChannel == 3) || (tvRemoconChannel == 4)))   tvRemoconChannel = 1;

  DCOffsetL = EEPROM.read(EEP_MOTOR_OFFSET_L);
  DCOffsetR = EEPROM.read(EEP_MOTOR_OFFSET_R);

  if ((DCOffsetL < 0) || (DCOffsetL > 100))
  {
    EEPROM.write(EEP_MOTOR_OFFSET_L, 50);
    DCOffsetL = EEPROM.read(EEP_MOTOR_OFFSET_L);
  }
  if ((DCOffsetR < 0) || (DCOffsetR > 100))
  {
    EEPROM.write(EEP_MOTOR_OFFSET_R, 50);
    DCOffsetR = EEPROM.read(EEP_MOTOR_OFFSET_R);
  }
  //-------------------------------------------------------------------//
  speedPublic = EEPROM.read(EEP_MOTOR_SPEED); //speed

  if ((speedPublic < 0) || (speedPublic > 100))
  {
    EEPROM.write(EEP_MOTOR_SPEED, 30);
    speedPublic = EEPROM.read(EEP_MOTOR_SPEED);
  }
  //-------------------------------------------------------------------//
  DCInit();
  LED_Init();
  LEDColorAll(0, 0, 0);
  Timer1Set();
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------//

// -------------------------------------- TIMER 1 -------------------------------------- //
void Timer1Set()
{
  TIMSK1 &= ~(1 << TOIE1);
  TCCR1A &= ~((1 << WGM11) | (1 << WGM10));
  TCCR1B &= ~(1 << CS12);
  TCCR1B = ((1 << CS11) | (1 << CS10));
  TIMSK1 |= (1 << TOIE1);
}

//-----------------------------------------------------------------------------------//

ISR(TIMER1_OVF_vect)
{
  TCNT1 = 0xFFCA ; // 217 us //TVRemocon

  TVRemocon();
  LEDControl();
  DCControl();

  if (channelSetTimer > 0) channelSetTimer--;
  if (++DCPWMCount > MX_SP) DCPWMCount = 0;
  if (++brightCount > MX_BR) brightCount = 0;
}

// ---------------------------------- Read Sensor ----------------------------------- //
//-----------------------------------------------------------------------------------//
int ReadMic()
{
  int mn = 1024;
  int mx = 0;
  for (int i = 0; i < 500; ++i)
  {
    int val = analogRead(MIC_PIN);
    mn = min(mn, val);
    mx = max(mx, val);
  }
  return mx - mn;
}
//-----------------------------------------------------------------------------------//
float ReadVoltage()
{
  int value = analogRead(BATCHECK_PIN);
  float vout = (value * 5.0) / 1023;
  float vin = vout / (R2 / (R1 + R2));
  return vin;
}
//-----------------------------------------------------------------------------------//

// -------------------------------------- DC MOTOR -------------------------------------- //
//----------------------------------------------------------------------------------------//
void DCControl()
{
  //M1
  if ((dcDirection1 == STOP) || (dcDirection1 == LOOSE))
  {
    if (dcDirection1 == STOP) //STop
    {
      PORTD |= 0b01000000;
      PORTB |= 0b00000001;
    }
    else        //loose
    {
      PORTD  &= ~ 0b01000000;
      PORTB  &= ~ 0b00000001;
    }
  }

  else
  {
    if (DCPWMCount < dcSpeed1)
    {
      if (StatePinA == HIGH)     PORTB |= 0b00000001;
      else      PORTD |= 0b01000000;
    }
    else
    {
      PORTD  &= ~ 0b01000000;
      PORTB  &= ~ 0b00000001;
    }
  }

  //M2
  if ((dcDirection2 == STOP) || (dcDirection2 == LOOSE))
  {
    if (dcDirection2 == STOP) //STop
    {
      PORTD |= 0b00001000;
      PORTD |= 0b10000000;
    }
    else        //loose
    {
      PORTD &= ~0b00001000;
      PORTD &= ~0b10000000;
    }
  }

  else
  {
    if (DCPWMCount < dcSpeed2 )
    {
      if (StatePinB == HIGH)        PORTD |= 0b10000000;
      else    PORTD |= 0b00001000;
    }
    else
    {
      PORTD &= ~0b00001000;
      PORTD &= ~0b10000000;
    }
  }
}


//----------------------------------------------------------------------------------------//

void DCInit()
{
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, LOW);
}
//-----------------------------------------------------------------------------------//
int DCSpeedLoad()
{
  int _speed = speedPublic;
  return _speed;
}

void DCOffsetInput(char SelectMotor, int value)
{
  if (SelectMotor == M1)        DCOffsetL = value;
  else if (SelectMotor == M2)   DCOffsetR = value;
}

//-----------------------------------------------------------------------------------//
void DCMotor(char SelectMotor, char direction, char speed)
{
  if (SelectMotor == M1)
  {
    int _value = DCOffsetL;
    if (_value == 50) _value = 0;
    else  _value = _value - 50;

    dcSpeed1 = speed + _value;
    if (dcSpeed1 > 100) dcSpeed1 = 100;

    dcDirection1 = direction;
    if (direction == CW)     StatePinA = HIGH;
    else if (direction == CCW)    StatePinA = LOW;
  }

  else if (SelectMotor == M2)
  {
    int _value = DCOffsetR;
    if (_value == 50) _value = 0;
    else  _value = _value - 50;

    dcSpeed2 = speed + _value;
    if (dcSpeed2 > 100) dcSpeed2 = 100;

    dcDirection2 = direction;
    if (direction == CW)       StatePinB = HIGH;
    else if (direction == CCW)   StatePinB = LOW;

  }
}
//-----------------------------------------------------------------------------------//
void DCMove(byte direction, byte speed)
{
  if (direction == forward)
  {
    DCMotor(M1, CCW, speed);
    DCMotor(M2, CW, speed);
  }
  else if (direction == backward)
  {
    DCMotor(M1, CW, speed);
    DCMotor(M2, CCW, speed);
  }
  else if (direction == left)
  {
    DCMotor(M1, CW, speed);
    DCMotor(M2, CW, speed);
  }
  else if (direction == right)
  {
    DCMotor(M1, CCW, speed);
    DCMotor(M2, CCW, speed);
  }
  else if (direction == stop)
  {
    DCMotor(M1, STOP, 0);
    DCMotor(M2, STOP, 0);
  }
  else if (direction == loose)
  {
    DCMotor(M1, LOOSE, 0);
    DCMotor(M2, LOOSE, 0);
  }
}


// -------------------------------------- LED -------------------------------------- //
//-----------------------------------------------------------------------------------//
void LEDControl()
{
  // LED Control
  if (brightCount < brightB)   PORTB |=  0b00000010;
  else    PORTB &= ~ 0b00000010;

  if (brightCount < brightR)   PORTB |=  0b00000100;
  else    PORTB &= ~ 0b00000100;

  if (brightCount < brightG)   PORTB |=  0b00001000;
  else    PORTB &= ~ 0b00001000;
}
//-----------------------------------------------------------------------------------//
void LEDColorAll(int red, int green, int blue)
{
  brightR = red;
  brightG = green;
  brightB = blue;
}
//-----------------------------------------------------------------------------------//
void LEDColorR(int value)
{
  brightR = value;
}
//-----------------------------------------------------------------------------------//
void LEDColorG(int value)
{
  brightG = value;
}
//-----------------------------------------------------------------------------------//
void LEDColorB(int value)
{
  brightB = value;
}
//-----------------------------------------------------------------------------------//
void LED_Init()
{
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_BLUE, LOW);
}
//----------------------------------------------------------------------------------//



//-------------------------------- Buzzer ------------------------------------------//
//----------------------------------------------------------------------------------//
//1up
//int melody[] = {1319, 1568, 2637, 2093, 2349, 3136};
//int tempo[] = {25, 25, 25, 25, 25, 25};

void Sound_Beep1()
{
  int melody[] = {2637, 1568, 1216};
  int tempo[] = {16, 16, 12};
  SoundProcess(melody, tempo, 3);
}
//----------------------------------------------------------------------------------//
void Sound_Beep2()
{
  int melody[] = {167, 593};
  int tempo[] = {64, 8};
  SoundProcess(melody, tempo, 2);
}
//----------------------------------------------------------------------------------//
void Sound_1up()
{
  int melody[] = {1319, 1568, 2637, 2093, 2349, 3136};
  int tempo[] = {25, 25, 25, 25, 25, 25};
  SoundProcess(melody, tempo, 6);
}
//----------------------------------------------------------------------------------//
void Sound_Coin()
{
  int melody[] = {988, 1319};
  int tempo[] = {16, 2};
  SoundProcess(melody, tempo, 2);
}
//----------------------------------------------------------------------------------//
void Sound_FireBall()
{
  int melody[] = {392, 784, 1568};
  int tempo[] = {36, 36, 36};
  SoundProcess(melody, tempo, 3);
}
//----------------------------------------------------------------------------------//
void SoundProcess(int _melody[], int _tempo[], int _size)
{
  for (int thisNote = 0; thisNote < _size; thisNote++)
  {
    int noteDuration = 1000 / _tempo[thisNote];
    tone(MELODY_PIN, _melody[thisNote], noteDuration);
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
  }
}

//---------------------------------------------------------------------------------//

//-------------------------------- REMOCON ------------------------------------------//

int  RFreceive()
{
  word data = 0xFFFF;
  int num = Serial.available();

  for (int i = 0; i < num; i++)
  {
    int input = Serial.read();
    if (input >= 0)
    {
      _packet[_count++] = (byte)input;
      if (_count > 6)    _header, _count = 0;
      else
      {
        if ((_count == 2) && (_packet[0] == 0xFF && _packet[1] == 0x55))    _header = 1;
      }
      if ((_header == 1) && (_count == 6))
      {
        _header, _count = 0;

        if (_packet[2] == (byte)(~_packet[3]) && _packet[4] == (byte)(~_packet[5]))
        {
          data = _packet[2];
          data |= (_packet[4] << 8) & 0xFF00;
        }
      }
    }
  }
  if (data == 0xFFFF)  RFdataCode - 1;
  else                 RFdataCode = data;
  return RFdataCode;
}


//----------------------------------------------------------------------------------//
void RemoconChannelChange(int _code)
{
  byte _ch;

  if ((_code == CH1_KEY_U) && (tvRemoconChannel != 1))
  {
    if (channelSetFlag == 0)
    {
      channelSetFlag = CH1;
      channelSetTimer = 8000;
    }

    if (channelSetTimer == 0)
    {
      if (channelSetFlag == CH1)
      {
        _ch = CH1;
        EEPROM.write(EEP_REMOCON_CHANNEL, _ch);
        Serial.println(_ch);
        tvRemoconChannel = _ch;
        for (byte i = 0; i < _ch; i++)
        {
          LEDColorAll(100, 0, 0);
          delay(250);
          LEDColorAll(0, 0, 0);
          delay(250);
        }
      }
    }
  }

  else if ((_code == CH2_KEY_U) && (tvRemoconChannel != 2))
  {
    if (channelSetFlag == 0)
    {
      channelSetFlag = CH2;
      channelSetTimer = 8000;
    }

    if (channelSetTimer == 0)
    {
      if (channelSetFlag == CH2)
      {
        _ch = CH2;

        EEPROM.write(EEP_REMOCON_CHANNEL, _ch);
        //      Serial.println(_ch);
        tvRemoconChannel = _ch;
        for (byte i = 0; i < _ch; i++)
        {
          LEDColorAll(100, 0, 0);
          delay(250);
          LEDColorAll(0, 0, 0);
          delay(250);
        }
      }
    }
  }

  else if ((_code == CH3_KEY_U) && (tvRemoconChannel != 3))
  {
    if (channelSetFlag == 0)
    {
      channelSetFlag = CH3;
      channelSetTimer = 8000;
    }

    if (channelSetTimer == 0)
    {
      if (channelSetFlag == CH3)
      {
        _ch = CH3;

        EEPROM.write(EEP_REMOCON_CHANNEL, _ch);
        //    Serial.println(_ch);
        tvRemoconChannel = _ch;
        for (byte i = 0; i < _ch; i++)
        {
          LEDColorAll(100, 0, 0);
          delay(250);
          LEDColorAll(0, 0, 0);
          delay(250);
        }
      }
    }
  }

  else if ((_code == CH4_KEY_U) && (tvRemoconChannel != 4))
  {
    if (channelSetFlag == 0)
    {
      channelSetFlag = CH4;
      channelSetTimer = 8000;
    }

    if (channelSetTimer == 0)
    {
      if (channelSetFlag == CH4)
      {
        _ch = CH4;
        EEPROM.write(EEP_REMOCON_CHANNEL, _ch);
        //    Serial.println(_ch);
        tvRemoconChannel = _ch;
        for (byte i = 0; i < _ch; i++)
        {
          LEDColorAll(100, 0, 0);
          delay(250);
          LEDColorAll(0, 0, 0);
          delay(250);
        }
      }
    }
  }
  else if (channelSetTimer == 0)
  {
    channelSetFlag = 0;
    channelSetTimer = -1;
  }
}
//----------------------------------------------------------------------------------//
int RemoconCheckCode(int _code)
{
  if (RFdataCode != 0)
  {
    if (_code == KEY_U) _code = Rf_KEY_U;
    else if (_code == KEY_D) _code = Rf_KEY_D;
    else if (_code == KEY_L) _code = Rf_KEY_L;
    else if (_code == KEY_R) _code = Rf_KEY_R;

    //  else if (_code == KEY_C) _code = Rf_KEY_C;

    else if (_code == KEY_F1) _code = Rf_KEY_1;
    else if (_code == KEY_F2) _code = Rf_KEY_2;
    else if (_code == KEY_CH4) _code = Rf_KEY_3;
    else if (_code == KEY_F4) _code = Rf_KEY_4;

    //  else if (_code == KEY_F3) _code = Rf_KEY_3;
    //  else if (_code == KEY_F4) _code = Rf_KEY_4;
    //   else if (_code == KEY_FR) _code = Rf_KEY_5;
    //   else if (_code == KEY_FL) _code = Rf_KEY_6;
    //   else if (_code == KEY_CH4)  _code = 256;
  }

  else if (tvRemoconChannel == 1)
  {
    if (_code == KEY_U) _code = CH1_KEY_U;
    else if (_code == KEY_D) _code = CH1_KEY_D;
    else if (_code == KEY_L) _code = CH1_KEY_L;
    else if (_code == KEY_R) _code = CH1_KEY_R;
    else if (_code == KEY_C) _code = CH1_KEY_C;
    else if (_code == KEY_F1) _code = CH1_KEY_F1;
    else if (_code == KEY_F2) _code = CH1_KEY_F2;
    else if (_code == KEY_F3) _code = CH1_KEY_F3;
    else if (_code == KEY_F4) _code = CH1_KEY_F4;
    else if (_code == KEY_CH1) _code = CH1_KEY_CH1;
    else if (_code == KEY_CH2) _code = CH1_KEY_CH2;
    else if (_code == KEY_CH3) _code = CH1_KEY_CH3;
    else if (_code == KEY_CH4) _code = CH1_KEY_CH4;
  }

  else if (tvRemoconChannel == 2)
  {
    if (_code == KEY_U) _code = CH2_KEY_U;
    else if (_code == KEY_D) _code = CH2_KEY_D;
    else if (_code == KEY_L) _code = CH2_KEY_L;
    else if (_code == KEY_R) _code = CH2_KEY_R;
    else if (_code == KEY_C) _code = CH2_KEY_C;
    else if (_code == KEY_F1) _code = CH2_KEY_F1;
    else if (_code == KEY_F2) _code = CH2_KEY_F2;
    else if (_code == KEY_F3) _code = CH2_KEY_F3;
    else if (_code == KEY_F4) _code = CH2_KEY_F4;
    else if (_code == KEY_CH1) _code = CH2_KEY_CH1;
    else if (_code == KEY_CH2) _code = CH2_KEY_CH2;
    else if (_code == KEY_CH3) _code = CH2_KEY_CH3;
    else if (_code == KEY_CH4) _code = CH2_KEY_CH4;
  }

  else if (tvRemoconChannel == 3)
  {
    if (_code == KEY_U) _code = CH3_KEY_U;
    else if (_code == KEY_D) _code = CH3_KEY_D;
    else if (_code == KEY_L) _code = CH3_KEY_L;
    else if (_code == KEY_R) _code = CH3_KEY_R;
    else if (_code == KEY_C) _code = CH3_KEY_C;
    else if (_code == KEY_F1) _code = CH3_KEY_F1;
    else if (_code == KEY_F2) _code = CH3_KEY_F2;
    else if (_code == KEY_F3) _code = CH3_KEY_F3;
    else if (_code == KEY_F4) _code = CH3_KEY_F4;
    else if (_code == KEY_CH1) _code = CH3_KEY_CH1;
    else if (_code == KEY_CH2) _code = CH3_KEY_CH2;
    else if (_code == KEY_CH3) _code = CH3_KEY_CH3;
    else if (_code == KEY_CH4) _code = CH3_KEY_CH4;
  }

  else if (tvRemoconChannel == 4)
  {
    if (_code == KEY_U) _code = CH4_KEY_U;
    else if (_code == KEY_D) _code = CH4_KEY_D;
    else if (_code == KEY_L) _code = CH4_KEY_L;
    else if (_code == KEY_R) _code = CH4_KEY_R;
    else if (_code == KEY_C) _code = CH4_KEY_C;
    else if (_code == KEY_F1) _code = CH4_KEY_F1;
    else if (_code == KEY_F2) _code = CH4_KEY_F2;
    else if (_code == KEY_F3) _code = CH4_KEY_F3;
    else if (_code == KEY_F4) _code = CH4_KEY_F4;
    else if (_code == KEY_CH1) _code = CH4_KEY_CH1;
    else if (_code == KEY_CH2) _code = CH4_KEY_CH2;
    else if (_code == KEY_CH3) _code = CH4_KEY_CH3;
    else if (_code == KEY_CH4) _code = CH4_KEY_CH4;
  }
  return _code;
}

//----------------------------------------------------------------------------------//

int TVRemoconData()
{
 // RemoconChannelChange(dataCode);
  return dataCode;
}
//----------------------------------------------------------------------------------//
void TVRemocon()
{
  if (TimeOutCheckEnd(220))
  {
    dataCode = 0;
    codeBuffer = 0;
    dataCount = 0;
    stateTVReceive = 0;
  }

  IR_in = !(((~PIND) >> 2) & 0b1);
  if (stateTVReceive == startReadCode)      CheckDataCode();      //3step - DataCode check
  else if (stateTVReceive == findLeadCode)  CheckCustomCode();    //2step - CustomCodeStart
  else if (stateTVReceive == 0)             CheckLeadCode();      //1step - Lead code check
}

void CheckLeadCode()
{
  if (!inputStatus)
  {
    if (IR_in == LOW)
    {
      inputStatus = 1;
      LeadPreviousMillis = millis();
    }
  }
  else
  {
    if (IR_in == HIGH)
    {
      inputStatus = 0;
      if (TimeCheckLeadcode(LeadCodetime))
      {
        stateTVReceive = findLeadCode;
        CodePreviousMicros = micros();
      }
    }
  }
}

//----------------------------------------------------------------------------------//
void CheckCustomCode()
{
  if (IR_in == LOW)
  {
    if (TimeCheckCode(TimeChecktime))
    {
      stateTVReceive = startReadCode;
      CodePreviousMicros = micros();
      EndPreviousMillis = millis();
      inputStatus = 1;
    }
    else    stateTVReceive = 0;
  }
}
//----------------------------------------------------------------------------------//
void CheckDataCode()
{
  if (TimeCheckEnd(EndChecktime))
  {
    TimeOutPreviousMillis = millis();
    codeBuffer = 0;
    dataCount = 0;
    stateTVReceive = 0;
  }

  else if (inputStatus)
  {
    if (IR_in == HIGH)
    {
      CodePreviousMicros = micros();
      EndPreviousMillis = millis();
      inputStatus = 0;
    }
  }

  else  //!inputStatus
  {
    if (IR_in == LOW)
    {
      if (dataCount > 15 && dataCount < 24)
      {
        codeBuffer = codeBuffer << 1;
        if (TimeCheckCode(TimeCheckdatatime))    codeBuffer |= 0x01; //data 1
        else    codeBuffer &= ~0x01;  //data 0
      }
      dataCount++;
      inputStatus = 1;
      if (dataCount == 24)     dataCode = codeBuffer;
    }
  }
}
//----------------------------------------------------------------------------------//
boolean TimeCheckCode(word interval) //micros seconds
{
  boolean time = false;
  unsigned long currentTimes = micros();
  if (currentTimes - CodePreviousMicros > interval)
  {
    CodePreviousMicros = currentTimes;
    time = true;
  }
  return time;
}
//----------------------------------------------------------------------------------//
boolean TimeCheckLeadcode(word interval)  //milliseconds
{
  boolean time = false;
  unsigned long currentTimes = millis();
  if (currentTimes - LeadPreviousMillis > interval)
  {
    LeadPreviousMillis = currentTimes;
    time = true;
  }
  return time;
}
//----------------------------------------------------------------------------------//
boolean TimeCheckEnd(word interval)  //milliseconds
{
  boolean time = false;
  unsigned long currentTimes = millis();
  if (currentTimes - EndPreviousMillis > interval)
  {
    EndPreviousMillis = currentTimes;
    time = true;
  }
  return time;
}
//----------------------------------------------------------------------------------//
boolean TimeOutCheckEnd(word interval)  //milliseconds
{
  boolean time = false;
  unsigned long currentTimes = millis();
  if (currentTimes - TimeOutPreviousMillis > interval)
  {
    TimeOutPreviousMillis = currentTimes;
    time = true;
  }
  return time;
}

//---------------------------------------------------------------------------------//

void LowBatCheck()
{
  if ((analogRead(A6) * 5.0) / 1023 / (R2 / (R1 + R2)) < 3.7)
  {
    pinMode(LED_RED, OUTPUT);
    while (1)
    {
      LED_RED_ON; delay(1000); LED_RED_OFF; delay(1000);
    }
  }
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------//

void TestMode()
{
  Sound_Beep1();

  do {
    delay(10);
  }    while (!digitalRead(SW1) || !digitalRead(SW2));

  int melodyCode[] = {NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4, NOTE_G4, NOTE_A4, NOTE_B4, NOTE_C5};
  int melodyDurations[] =  {8, 8, 8, 8, 8, 8, 8, 8};
  SoundProcess(melodyCode, melodyDurations, 8);

  unsigned long _loopTime = millis();

  byte ledSequence, motorSequence = 0;
  int speed = 70;
  int bright = 20;

  while (1)
  {
    if ((!digitalRead(SW1)) && (digitalRead(SW2)))
    {
      motorSequence = 1;
      do {
        delay(10);
      }    while (!digitalRead(SW1));
    }

    if ((digitalRead(SW1)) && (!digitalRead(SW2)))
    {
      if (ledSequence == 0)
      {
        LEDColorAll(bright, bright, bright);
        ledSequence = 1;
      }
      else
      {
        LEDColorAll(0, 0, 0);
        ledSequence = 0;
      }
      do {
        delay(10);
      }    while (!digitalRead(SW2));
    }

    if ( millis() - _loopTime  > 1000)
    {
      if (ledSequence >= 1) ledSequence++;
      if (ledSequence > 3)  ledSequence = 1;

      if (motorSequence > 0)  motorSequence++;
      if (motorSequence > 6)  motorSequence = 0;

      if (ledSequence == 1)           LEDColorAll(bright, 0, 0);
      else if (ledSequence == 2)      LEDColorAll(0, bright, 0);
      else if (ledSequence == 3)      LEDColorAll(0, 0, bright);

      if (motorSequence == 2)         DCMove(forward, 40);
      else if (motorSequence == 3)    DCMove(backward, 40);
      else if (motorSequence == 4)    DCMove(left, 40);
      else if (motorSequence == 5)    DCMove(right, 40);
      else if (motorSequence == 6)    DCMove(stop, 40);

      _loopTime = millis();
    }
    if ((millis() % 100) == 0)   PrintSensor();
  }
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------//



int ModeSelect()
{
  int _mode = 0;
  if (analogRead(SFF) < 400)           _mode = UNPLUGGED; //_mode = CLIFF_BOT;  
  else if (analogRead(SFR) < 400)      _mode = LINE_TRACER;
  else if (analogRead(SFL) < 400)      _mode = AVOID;
  else                                 _mode = FIRMATA;
  return _mode;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------//

void LedDisplay(int count, int time)
{
  for (int i = 0; i < count ; i++)
  {
    LEDColorAll(100, 0, 0);
    delay(time);
    LEDColorAll(0, 100, 0);
    delay(time);
    LEDColorAll(0, 0, 100);
    delay(time);
  }
  LEDColorAll(0, 0, 0);
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------//


void PrintSensor()
{

  Serial.print(analogRead(SFL));
  Serial.print("\t");
  Serial.print(analogRead(SFF));
  Serial.print("\t");
  Serial.print(analogRead(SFR));
  Serial.print("\t");
  Serial.print(analogRead(SBL));
  Serial.print("\t");
  Serial.print(analogRead(SBR));
  Serial.print("\t");
  Serial.print(ReadVoltage());
  Serial.print("\t");
  Serial.println(ReadMic());

}



