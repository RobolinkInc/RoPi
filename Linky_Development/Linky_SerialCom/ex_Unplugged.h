#include "config.h"
#include <EEPROM.h>

#define SENSOR_INPUT_MODE     DDRC = 0x00
#define SENSOR_DISPLAY_MODE   DDRC = 0xff

#define COMMAND_ADDRESS_START 40
#define LOOP_SET_ADDRESS 35
#define REMOCON_CHANNEL_ADDRESS 26

//------------------------------------------------------------------------------------------------------//
#define DEBUG_SERIAL
//------------------------------------------------------------------------------------------------------//

enum UnplugCode //max : 15 0xf0
{
  typeLed = 0x01,
  typeDelay,
  typeBuzzer,
  typeDCMotor,
  typeSensorFront,
  typeSensorBottom
};

enum UnplugCode_LED //max : 15 0x0f
{
  LED_OFF = 0x01,
  LED_R,
  LED_G,
  LED_B,
  LED_ALL_ON,

  START_INIT = 0xf0,
  START_PRG
};

enum UnplugCode_SENSOR_FRONT //max : 7 0x07
{
  IF_FRONT_ALL_DETECT = 0x01,
  IF_A0_DETECT,
  IF_A1_DETECT,
  IF_A2_DETECT,
};

enum UnplugCode_SENSOR_BOTTOM //max : 7 0x07
{
  IF_UNDER_ALL_DETECT = 0x01,
  IF_UNDER_3_DETECT,
  IF_UNDER_4_DETECT,
};

enum UnplugCode_SOUND
{
  SOUND_START = 0x01,
  SOUND_SAVE,
  SOUND_LOAD,
  SOUND_INPUT ,
  SOUND_DELETE,
  SOUND_DELETE_ALL,
  SOUND_SELECT_LED,
  SOUND_SELECT_MOTOR,
  SOUND_SELECT_BUZZER ,
  SOUND_SELECT_REMOCON,
  SOUND_CHANGE_VALUE,
  SOUND_SELECT_DELAY
};

//------------------------------------------------------------------------------------------------------//
#define DEBUG_SERIAL
//------------------------------------------------------------------------------------------------------//

#define MAX_PROGRAM 50
byte eepBuf[MAX_PROGRAM];

int melodyCode[] = {NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4, NOTE_G4, NOTE_A4, NOTE_B4, NOTE_C5};
int melodyDurations = 4;

byte LED_FORMATION[] = {LED_ALL_ON, LED_OFF, LED_R, LED_G, LED_B};

byte MotorMode[] = {STOP, FORWARD, BACKWARD, LEFT, RIGHT};

byte SensorFrontSelect[] = {IF_FRONT_ALL_DETECT, IF_A0_DETECT, IF_A1_DETECT, IF_A2_DETECT};

byte SensorBottomSelect[] = {IF_UNDER_ALL_DETECT, IF_UNDER_3_DETECT, IF_UNDER_4_DETECT};

int inputDelay = 250;
int basicDelay = 500;

int bright = 50;

byte motorSpeed = 30;
int commandAddress = 0;
boolean ProgramCondition = true;
int _baseLV_L = 950;
int _baseLV_R = 950;
//--------------------------------------------------------------------------------------------------------------------------//

void DeleteCodeProcess(byte _inputKey);
void InputCodeProcess(byte _inputKey);
void ExecuteCodeCheck(byte _inputKey);
void RC_Control();
void ExecuteCodeProcess();
void buzz(int targetPin, long frequency, long length);
void ProgramCountAlarm(byte _count);

//--------------------------------------------------------------------------------------------------------------------------//

void Unplugged(int speed)
{
  motorSpeed = speed;

  //-------------------------------------------------------------------------------//
  commandAddress = EEPROM.read(COMMAND_ADDRESS_START);  // Read commandCount
  //-------------------------------------------------------------------------------//
  boolean ledToggle = false;
  unsigned long checkDebugMillis = millis();

  // EEPROM.write(EEP_LINE_SEN_BASE_L, baseLV_L);
  // EEPROM.write(EEP_LINE_SEN_BASE_R, baseLV_R);
  // _baseLV_L = EEPROM.read(EEP_LINE_SEN_BASE_L);
  // _baseLV_R = EEPROM.read(EEP_LINE_SEN_BASE_R);

  byte i = EEPROM.read(EEP_LINE_SEN_BASE_LH);
  byte j = EEPROM.read(EEP_LINE_SEN_BASE_LL);
  byte k = EEPROM.read(EEP_LINE_SEN_BASE_RH);
  byte l = EEPROM.read(EEP_LINE_SEN_BASE_RL);

  _baseLV_L = ((i << 8) | (j & 0xff));
  _baseLV_R = ((k << 8) | (l & 0xff));

  /*
    Serial.println("---------------");
    Serial.println(_baseLV_L);
    Serial.println(_baseLV_R);
    Serial.println("---------------");
  */

  while (1)
  {
    if (millis() - checkDebugMillis > 450)
    {
      checkDebugMillis = millis();
      ledToggle = !ledToggle;

      if (ledToggle)  LEDColorAll(10, 10, 30);

      else  LEDColorAll(0, 0, 0);
    }

    int keyData = RFreceive();
    if (keyData == 0) keyData =  TVRemoconData();    
    RemoconChannelChange(keyData);

#ifdef DEBUG_SERIAL
    if (keyData != 0)
    {
      Serial.print("RemoconData : ");      Serial.print("\t");      Serial.println(keyData);
    }
#endif

    if (keyData == RemoconCheckCode(KEY_CH4)) RemoconControl(motorSpeed); //Remocon

    ExecuteCodeCheck(keyData);  //ExecuteCodeProcess(); //Start Code
    InputCodeProcess(keyData);  // Input Code
    DeleteCodeProcess(keyData); // Delete Code
  }
}

//--------------------------------------------------------------------------------------------------------------------------//
void ExecuteCodeCheck(byte _inputKey)
{
  if ((_inputKey == RemoconCheckCode(KEY_C)))
  {
    boolean _executeKeyCheck = true;
    delay(800);

    for (int i = 0; i < 4; i++)
    {
      delay(300);
      if (TVRemoconData() != RemoconCheckCode(KEY_C))
      {
        _executeKeyCheck = false;
        break;
      }
    }
    if (_executeKeyCheck) ExecuteCodeProcess();
  }
}
//--------------------------------------------------------------------------------------------------------------------------//
void ExecuteCodeProcess()
{

#ifdef DEBUG_SERIAL
  Serial.println("- ExecuteCodeProcess -");
#endif

  // SmartInventor.TVRemoconClose(); //end remocon
  // Unplug_Led(START_PRG);
  // Unplug_Sound(SOUND_START);


  int melody_Execute[] = {1216, 450, 2016};
  int tempo_Execute[] = {16,  16, 16};
  SoundProcess(melody_Execute, tempo_Execute, 4);

  //-----------------------------------------------------------------------------------//
  int commandCount = EEPROM.read(COMMAND_ADDRESS_START);
  boolean _loop = EEPROM.read(LOOP_SET_ADDRESS);

#ifdef DEBUG_SERIAL
  Serial.print("CommandAddress");  Serial.print("\t");  Serial.println(commandAddress);
  Serial.print("CommandCount");  Serial.print("\t");  Serial.println(commandCount);
#endif

#ifdef DEBUG_SERIAL
  Serial.println("");
  Serial.print("Slot");  Serial.print("\t");  Serial.println("Program");
#endif
  for (int i = 0 ; i < commandCount; i++)
  {
    eepBuf[i] = EEPROM.read(COMMAND_ADDRESS_START + 1 + i);

#ifdef DEBUG_SERIAL
    Serial.print(i);    Serial.print("\t");    Serial.println(eepBuf[i]);
#endif
  }

#ifdef DEBUG_SERIAL
  Serial.println("");  Serial.println("- start -");
#endif

  //-----------------------------------------------------------------------------------//
  do
  {

#ifdef DEBUG_SERIAL
    Serial.println("");
#endif

    for (int i = 0 ; i < commandCount; i++)
    {

#ifdef DEBUG_SERIAL
      Serial.print(i);      Serial.print("\t");      Serial.print(eepBuf[i]);  Serial.print("\t");
#endif

      byte _actType = (eepBuf[i] & 0xf0) >> 4;
      byte _actCommand = eepBuf[i] & 0x0f;

      //------------------------------------------------------------//

      if (_actType == typeSensorFront)
      {
#ifdef DEBUG_SERIAL
        Serial.print("- typeSensorFront -");          Serial.print("\t");
#endif
        if (SensorFrontSelect[(_actCommand & 0x07)] == IF_FRONT_ALL_DETECT)
        {
          if ((_actCommand & 0x08) >> 3)
          {
            if ((analogRead(SFL) < 400) && (analogRead(SFF) < 400) && (analogRead(SFR) < 400))  ProgramCondition = 1;
            else    ProgramCondition = 0;
          }
          else
          {
            if  ((analogRead(SFL) > 400) && (analogRead(SFF) > 400) && (analogRead(SFR) > 400)) ProgramCondition = 1;
            else    ProgramCondition = 0;
          }
        }

        else if (SensorFrontSelect[(_actCommand & 0x07)] == IF_A0_DETECT)
        {
          if (analogRead(SFL) < 400)  ProgramCondition = ((_actCommand & 0x08) >> 3);
          else                        ProgramCondition = !((_actCommand & 0x08) >> 3);
        }
        else if (SensorFrontSelect[(_actCommand & 0x07)] == IF_A1_DETECT)
        {
          if (analogRead(SFF) < 400)  ProgramCondition = ((_actCommand & 0x08) >> 3);
          else                        ProgramCondition = !((_actCommand & 0x08) >> 3);
        }
        else if (SensorFrontSelect[(_actCommand & 0x07)] == IF_A2_DETECT)
        {
          if (analogRead(SFR) < 400)  ProgramCondition = ((_actCommand & 0x08) >> 3);
          else                        ProgramCondition = !((_actCommand & 0x08) >> 3);
        }

#ifdef DEBUG_SERIAL
        Serial.print("\t");        Serial.print(ProgramCondition);
#endif
      }
      else if (_actType == typeSensorBottom)
      {

#ifdef DEBUG_SERIAL
        Serial.print("- typeSensorBottom -");        Serial.print("\t");
#endif
        //---------------------------------------------------------//
        if (SensorBottomSelect[(_actCommand & 0x07)] == IF_UNDER_ALL_DETECT)
        {
          if ((_actCommand & 0x08) >> 3)
          {
            if ((analogRead(SBL) < _baseLV_L) && (analogRead(SBR) < _baseLV_R))  ProgramCondition = 1;
            else    ProgramCondition = 0;
          }
          else
          {
            if ((analogRead(SBL) > _baseLV_L) && (analogRead(SBR) > _baseLV_R))  ProgramCondition = 1;
            else    ProgramCondition = 0;
          }          
        }
        else if (SensorBottomSelect[(_actCommand & 0x07)] == IF_UNDER_3_DETECT)
        {
          if (analogRead(SBL) < _baseLV_L)  ProgramCondition = ((_actCommand & 0x08) >> 3);
          else    ProgramCondition = !((_actCommand & 0x08) >> 3);
        }
        else if (SensorBottomSelect[(_actCommand & 0x07)] == IF_UNDER_4_DETECT)
        {
          if (analogRead(SBR) < _baseLV_R)  ProgramCondition = ((_actCommand & 0x08) >> 3);
          else    ProgramCondition = !((_actCommand & 0x08) >> 3);
        }
        //---------------------------------------------------------//

#ifdef DEBUG_SERIAL
        Serial.print("\t");        Serial.print(ProgramCondition);
#endif
      }

      //------------------------------------------------------------//
      if (ProgramCondition == true)
      {
        if (_actType == typeLed)
        {

#ifdef DEBUG_SERIAL
          Serial.print("- type LED -");
#endif
          if (_actCommand == LED_OFF)           LEDColorAll(0, 0, 0);
          else if (_actCommand == LED_R)        LEDColorAll(bright, 0, 0);
          else if (_actCommand == LED_G)        LEDColorAll(0, bright, 0);
          else if (_actCommand == LED_B)        LEDColorAll(0, 0, 20);
          else if (_actCommand == LED_ALL_ON)   LEDColorAll(20, bright, 20);
        }
        //------------------------------------------------------------//
        else if (_actType == typeBuzzer)
        {
#ifdef DEBUG_SERIAL
          Serial.print("- type Buzzer -");
#endif
          buzz(MELODY_PIN, melodyCode[_actCommand], melodyDurations);
        }
        //------------------------------------------------------------//
        else if (_actType == typeDCMotor)
        {
#ifdef DEBUG_SERIAL
          Serial.print("- type DC Motor -");
#endif
          DCMove(MotorMode[_actCommand], motorSpeed);
        }
        //------------------------------------------------------------//
        else if (_actType == typeDelay)
        {
#ifdef DEBUG_SERIAL
          Serial.print("- type Delay -");
#endif
          delay(basicDelay * _actCommand);
        }
      }
      //------------------------------------------------------------//
#ifdef DEBUG_SERIAL
      Serial.println();
#endif

    }
  } while (_loop);

#ifdef DEBUG_SERIAL
  Serial.println("- program end -");
#endif
  while (1);
}

//--------------------------------------------------------------------------------------------------------------------------//
void InputCodeProcess(byte _inputKey)
{
  int mxSelect, nowSelect = 0;
  boolean inputSuccess = false;
  //--------------------------------------------------------------------------//
  if (_inputKey == RemoconCheckCode(KEY_CH1))  //LED
  {
#ifdef DEBUG_SERIAL
    Serial.println("- LED Input -");    Serial.print("NowSelect : ");    Serial.print("\t");    Serial.println(nowSelect);
#endif

    Sound_Beep1();

    mxSelect = sizeof(LED_FORMATION);

    while ((_inputKey != RemoconCheckCode(KEY_C)))
    {
      _inputKey =  TVRemoconData();

      if ((_inputKey == RemoconCheckCode(KEY_L)))
      {
        if (--nowSelect < 0) nowSelect = mxSelect - 1;
        Sound_Beep1();

#ifdef DEBUG_SERIAL
        Serial.print("NowSelect : ");        Serial.print("\t");        Serial.println(nowSelect);
#endif

      }
      else if ((_inputKey == RemoconCheckCode(KEY_R)))
      {
        if (++nowSelect == mxSelect) nowSelect = 0;
        Sound_Beep1();

#ifdef DEBUG_SERIAL
        Serial.print("NowSelect : ");        Serial.print("\t");        Serial.println(nowSelect);
#endif
      }

      if (LED_FORMATION[nowSelect] == LED_OFF)          LEDColorAll(0, 0, 0);
      else if (LED_FORMATION[nowSelect] == LED_R)       LEDColorAll(bright, 0, 0);
      else if (LED_FORMATION[nowSelect] == LED_G)       LEDColorAll(0, bright, 0);
      else if (LED_FORMATION[nowSelect] == LED_B)       LEDColorAll(0, 0, 20);
      else if (LED_FORMATION[nowSelect] == LED_ALL_ON)  LEDColorAll(20, bright, 20);

      delay(inputDelay);
    }

    EEPROM.write(COMMAND_ADDRESS_START + (++commandAddress), ((typeLed << 4) | LED_FORMATION[nowSelect]));
    EEPROM.write(COMMAND_ADDRESS_START, commandAddress);
    Sound_Beep1();
    inputSuccess = true;


  }


  //--------------------------------------------------------------------------------------------------------------------------//
  else if (_inputKey == RemoconCheckCode(KEY_CH2))  //BUZZER
  {

#ifdef DEBUG_SERIAL
    Serial.println("- Buzzer Input -");    Serial.print("NowSelect : ");    Serial.print("\t");    Serial.println(nowSelect);
#endif

    Sound_Beep1();
    mxSelect = sizeof(melodyCode);
    mxSelect /= 2 ;

    while ((_inputKey != RemoconCheckCode(KEY_C)))
    {
      _inputKey =  TVRemoconData();

      if ((_inputKey == RemoconCheckCode(KEY_L)))
      {
        if (--nowSelect < 0) nowSelect = mxSelect - 1;
        buzz(MELODY_PIN, melodyCode[nowSelect], melodyDurations);

#ifdef DEBUG_SERIAL
        Serial.print("NowSelect : ");        Serial.print("\t");        Serial.println(nowSelect);
#endif

      }

      else if ((_inputKey == RemoconCheckCode(KEY_R)))
      {
        if (++nowSelect == mxSelect) nowSelect = 0;
        buzz(MELODY_PIN, melodyCode[nowSelect], melodyDurations);

#ifdef DEBUG_SERIAL
        Serial.print("NowSelect : ");        Serial.print("\t");        Serial.println(nowSelect);
#endif

      }


      delay(inputDelay);
    }

    EEPROM.write(COMMAND_ADDRESS_START + (++commandAddress), ((typeBuzzer << 4) | nowSelect));
    EEPROM.write(COMMAND_ADDRESS_START, commandAddress);
    Sound_Beep1();
    inputSuccess = true;


  }
  //--------------------------------------------------------------------------------------------------------------------------//
  else if (_inputKey == RemoconCheckCode(KEY_CH3))  //DC MOTOR
  {
#ifdef DEBUG_SERIAL
    Serial.println("- DC Motor Input -");    Serial.print("NowSelect : ");    Serial.print("\t");    Serial.println(nowSelect);
#endif

    Sound_Beep1();
    mxSelect = sizeof(MotorMode);

    while ((_inputKey != RemoconCheckCode(KEY_C)))
    {
      _inputKey =  TVRemoconData();

      if ((_inputKey == RemoconCheckCode(KEY_L)))
      {
        if (--nowSelect < 0) nowSelect = mxSelect - 1;
        Sound_Beep1();

        Serial.print("NowSelect : ");        Serial.print("\t");        Serial.println(nowSelect);
      }

      else if ((_inputKey == RemoconCheckCode(KEY_R)))
      {
        if (++nowSelect == mxSelect) nowSelect = 0;
        Sound_Beep1();

        Serial.print("NowSelect : ");        Serial.print("\t");        Serial.println(nowSelect);
      }

      DCMove(MotorMode[nowSelect], motorSpeed);
      delay(inputDelay);
    }

    DCMove(stop, 0);

    EEPROM.write(COMMAND_ADDRESS_START + (++commandAddress), ((typeDCMotor << 4) | nowSelect));
    EEPROM.write(COMMAND_ADDRESS_START, commandAddress);
    Sound_Beep1();

    inputSuccess = true;

  }

  //--------------------------------------------------------------------------------------------------------------------------//
  else if (_inputKey == RemoconCheckCode(KEY_F1))  //delay
  {

#ifdef DEBUG_SERIAL
    Serial.println("- Delay Input -");
    Serial.print("NowSelect : ");    Serial.print("\t");    Serial.println(nowSelect);
#endif

    Sound_Beep1();
    mxSelect = 9;
    while ((_inputKey != RemoconCheckCode(KEY_C)))
    {
      _inputKey =  TVRemoconData();

      if ((_inputKey == RemoconCheckCode(KEY_L)))
      {
        if (--nowSelect < 0) nowSelect = mxSelect - 1;

        //   Sound_Beep1();

        for (int i = 0; i < nowSelect; i++)
        {
          int noteDuration = 1000 / 10;
          tone(MELODY_PIN, melodyCode[2], noteDuration);
          delay(noteDuration * 1.30);
        }


#ifdef DEBUG_SERIAL
        Serial.print("NowSelect : ");        Serial.print("\t");        Serial.println(nowSelect);
#endif
      }
      else if ((_inputKey == RemoconCheckCode(KEY_R)))
      {
        if (++nowSelect == mxSelect) nowSelect = 0;

        //   Sound_Beep1();

        for (int i = 0; i < nowSelect; i++)
        {
          int noteDuration = 1000 / 10;
          tone(MELODY_PIN, melodyCode[2], noteDuration);
          delay(noteDuration * 1.30);
        }

#ifdef DEBUG_SERIAL
        Serial.print("NowSelect : ");        Serial.print("\t");        Serial.println(nowSelect);
#endif
      }
      delay(inputDelay);
    }

    EEPROM.write(COMMAND_ADDRESS_START + (++commandAddress), ((typeDelay << 4) | nowSelect));
    EEPROM.write(COMMAND_ADDRESS_START, commandAddress);
    Sound_Beep1();

    inputSuccess = true;


  }
  //-----------------------------------------------------------------------------------------------------//
  else if (_inputKey == RemoconCheckCode(KEY_F2))  //LOOP
  {
    Sound_Beep1();
    boolean setLoop = false;
    LEDColorAll(0, bright, 0);

#ifdef DEBUG_SERIAL
    Serial.println("- Loop Input -");    Serial.print("setLoop : ");    Serial.print("\t");    Serial.println(setLoop);
#endif

    while ((_inputKey != RemoconCheckCode(KEY_C)))
    {
      _inputKey =  TVRemoconData();
      if ((_inputKey == RemoconCheckCode(KEY_L)))
      {
        LEDColorAll(bright, 0, 0);
        setLoop = true;
        Sound_Beep1();

#ifdef DEBUG_SERIAL
        Serial.print("NowSelect : ");        Serial.print("\t");        Serial.println(setLoop);
#endif

      }

      else if ((_inputKey == RemoconCheckCode(KEY_R)))
      {
        LEDColorAll(0, bright, 0);
        setLoop = false;
        Sound_Beep1();

#ifdef DEBUG_SERIAL
        Serial.print("NowSelect : ");        Serial.print("\t");        Serial.println(setLoop);
#endif

      }
      delay(inputDelay);
    }

    EEPROM.write(LOOP_SET_ADDRESS, setLoop);
    Sound_Beep1();

    inputSuccess = true;


  }
  //-----------------------------------------------------------------------------------------------------//
  else if (_inputKey == RemoconCheckCode(KEY_F3))  //Sensor input front
  {
#ifdef DEBUG_SERIAL
    Serial.println("- Front Sensor Input -");    Serial.print("NowSelect : ");    Serial.print("\t");    Serial.println(nowSelect);
#endif

    Sound_Beep1();
    mxSelect = sizeof(SensorFrontSelect);
    byte _condition = true;

    while (1)
    {
      _inputKey =  TVRemoconData();

      //---------------------------------------------------------//
      if ((_inputKey == RemoconCheckCode(KEY_U)))
      {
        _condition = true;

#ifdef DEBUG_SERIAL
        Serial.print("Condition : ");        Serial.print("\t");        Serial.println(_condition);
#endif
        break;
      }
      else if ((_inputKey == RemoconCheckCode(KEY_D)))
      {
        _condition = false;

#ifdef DEBUG_SERIAL
        Serial.print("Condition : ");        Serial.print("\t");        Serial.println(_condition);
#endif

        break;
      }
      //---------------------------------------------------------//

      else if ((_inputKey == RemoconCheckCode(KEY_L)))
      {
        if (--nowSelect < 0) nowSelect = mxSelect - 1;
        Sound_Beep1();

#ifdef DEBUG_SERIAL
        Serial.print("NowSelect : ");        Serial.print("\t");        Serial.println(nowSelect);
#endif

      }
      else if ((_inputKey == RemoconCheckCode(KEY_R)))
      {
        if (++nowSelect == mxSelect) nowSelect = 0;
        Sound_Beep1();

#ifdef DEBUG_SERIAL
        Serial.print("NowSelect : ");        Serial.print("\t");        Serial.println(nowSelect);
#endif
      }

      //---------------------------------------------------------//
      if (SensorFrontSelect[nowSelect] == IF_FRONT_ALL_DETECT)
      {
        if ((analogRead(SFL) < 400) && (analogRead(SFF) < 400) && (analogRead(SFR) < 400))  LEDColorAll(bright - 30, bright - 40, bright - 30);
        else   LEDColorAll(0, 0, 0);
      }
      else if (SensorFrontSelect[nowSelect] == IF_A0_DETECT)
      {
        if (analogRead(SFL) < 400)  LEDColorAll(bright, 0, 0);
        else   LEDColorAll(0, 0, 0);
      }
      else if (SensorFrontSelect[nowSelect] == IF_A1_DETECT)
      {
        if (analogRead(SFF) < 400)  LEDColorAll(0, bright - 40, 0);
        else   LEDColorAll(0, 0, 0);
      }
      else if (SensorFrontSelect[nowSelect] == IF_A2_DETECT)
      {
        if (analogRead(SFR) < 400) LEDColorAll(0, 0, bright - 30);
        else   LEDColorAll(0, 0, 0);
      }

      delay(inputDelay);
    }
    EEPROM.write(COMMAND_ADDRESS_START + (++commandAddress), ((typeSensorFront << 4) | (_condition << 3 | nowSelect)));
    EEPROM.write(COMMAND_ADDRESS_START, commandAddress);
    Sound_Beep1();

    inputSuccess = true;



  }
  //-----------------------------------------------------------------------------------------------------//
  else if (_inputKey == RemoconCheckCode(KEY_F4))  //Sensor input Bottom
  {
#ifdef DEBUG_SERIAL
    Serial.println("- Bottom Sensor Input -");    Serial.print("NowSelect : ");    Serial.print("\t");    Serial.println(nowSelect);
#endif

    Sound_Beep1();
    mxSelect = sizeof(SensorBottomSelect);
    byte _condition = true;

    while (1)
    {
      _inputKey =  TVRemoconData();

      //---------------------------------------------------------//
      if ((_inputKey == RemoconCheckCode(KEY_U)))
      {
        _condition = true;

#ifdef DEBUG_SERIAL
        Serial.print("Condition : ");        Serial.print("\t");        Serial.println(_condition);
#endif
        break;
      }
      else if ((_inputKey == RemoconCheckCode(KEY_D)))
      {
        _condition = false;

#ifdef DEBUG_SERIAL
        Serial.print("Condition : ");        Serial.print("\t");        Serial.println(_condition);
#endif

        break;
      }
      //---------------------------------------------------------//

      else if ((_inputKey == RemoconCheckCode(KEY_L)))
      {
        if (--nowSelect < 0) nowSelect = mxSelect - 1;
        Sound_Beep1();

#ifdef DEBUG_SERIAL
        Serial.print("NowSelect : ");        Serial.print("\t");        Serial.println(nowSelect);
#endif

      }
      else if ((_inputKey == RemoconCheckCode(KEY_R)))
      {
        if (++nowSelect == mxSelect) nowSelect = 0;
        Sound_Beep1();

#ifdef DEBUG_SERIAL
        Serial.print("NowSelect : ");        Serial.print("\t");        Serial.println(nowSelect);
#endif
      }

      //---------------------------------------------------------//
      if (SensorBottomSelect[nowSelect] == IF_UNDER_ALL_DETECT)
      {
        if ((analogRead(SBL) < 950) && (analogRead(SBR) < 950))  LEDColorAll(bright - 30, bright - 40, bright - 30);
        else   LEDColorAll(0, 0, 0);
      }
      else if (SensorBottomSelect[nowSelect] == IF_UNDER_3_DETECT)
      {
        if (analogRead(SBL) < 950)  LEDColorAll(bright, 0, 0);
        else   LEDColorAll(0, 0, 0);
      }
      else if (SensorBottomSelect[nowSelect] == IF_UNDER_4_DETECT)
      {
        if (analogRead(SBR) < 950)  LEDColorAll(0, 0, bright - 30);
        else   LEDColorAll(0, 0, 0);
      }
      //---------------------------------------------------------//

      delay(inputDelay);
    }
    EEPROM.write(COMMAND_ADDRESS_START + (++commandAddress), ((typeSensorBottom << 4) | (_condition << 3 | nowSelect)));
    EEPROM.write(COMMAND_ADDRESS_START, commandAddress);
    Sound_Beep1();

    inputSuccess = true;
  }

  if (inputSuccess == true)
  {
#ifdef DEBUG_SERIAL
    Serial.println("- Input Complete -");    Serial.print("ProgramCount : ");    Serial.print("\t");    Serial.println(commandAddress);
#endif
    ProgramCountAlarm(commandAddress);
  }

}
//--------------------------------------------------------------------------------------------------------------------------//
void DeleteCodeProcess(byte _inputKey)
{
  if (_inputKey == RemoconCheckCode(KEY_D))
  {
    boolean _deleteKeyCheck = true;
    delay(1000);
    for (int i = 0; i < 4; i++)
    {
      delay(600);
      if (TVRemoconData() != RemoconCheckCode(KEY_D))
      {
        _deleteKeyCheck = false;
        break;
      }

      LEDColorAll(bright, bright, 0);

      if (i == 1)
      {
        if (commandAddress > 0)   EEPROM.write(COMMAND_ADDRESS_START, --commandAddress);

#ifdef DEBUG_SERIAL
        Serial.println("- Delete one -");
#endif

        int melody_Delete[] = {1237, 1416, 2122};
        int tempo_Delete[] = {12, 16, 4};
        SoundProcess(melody_Delete, tempo_Delete, 3);


#ifdef DEBUG_SERIAL
        Serial.print("ProgramCount : ");        Serial.print("\t");        Serial.println(commandAddress);
#endif
      }
    }
    if (_deleteKeyCheck)
    {
      commandAddress = 0;
      EEPROM.write(COMMAND_ADDRESS_START, commandAddress);
      EEPROM.write(LOOP_SET_ADDRESS, 0);

#ifdef DEBUG_SERIAL
      Serial.println("- Delete All OK -");
#endif

      int melody_Delete_All[] = {2137, 668, 1216, 1700};
      int tempo_Delete_All[] = {12, 16, 12, 4};
      SoundProcess(melody_Delete_All, tempo_Delete_All, 4);


#ifdef DEBUG_SERIAL
      Serial.print("ProgramCount : ");      Serial.print("\t");      Serial.println(commandAddress);
#endif

    }
  }
}
//--------------------------------------------------------------------------------------------------------------------------//

void buzz(int targetPin, long frequency, long length)
{
  int noteDuration = 1000 / length;
  tone(targetPin, frequency, noteDuration);
  int pauseBetweenNotes = noteDuration * 1.30;
  delay(pauseBetweenNotes);
}

void ProgramCountAlarm(byte _count)
{
  for (int i = 0; i < commandAddress; i++)
  {
    int noteDuration = 1000 / 6;
    if (i < 8)        tone(MELODY_PIN, melodyCode[i], noteDuration);
    else if (i < 15)  tone(MELODY_PIN, melodyCode[14 - i], noteDuration);
    else if (i < 22)  tone(MELODY_PIN, melodyCode[i - 14], noteDuration);
    else if (i < 29)  tone(MELODY_PIN, melodyCode[28 - i], noteDuration);
    else if (i < 36)  tone(MELODY_PIN, melodyCode[i - 28], noteDuration);
    else if (i < 43)  tone(MELODY_PIN, melodyCode[42 - i], noteDuration);
    else if (i < 50)  tone(MELODY_PIN, melodyCode[i - 42], noteDuration);
    delay(noteDuration * 1.30);
  }
}






