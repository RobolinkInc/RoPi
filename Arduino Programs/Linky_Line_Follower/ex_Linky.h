#include "config.h"

void RemoconControl(int speed);
void LineTracer(int speed);
void Avoid(int speed);
void CliffBot(int speed);

//-----------------------------------------------------------------------------------------------------------------------//
//---------------------------------------------------- RemoconControl ---------------------------------------------------//
//-----------------------------------------------------------------------------------------------------------------------//
void RemoconControl(int speed)
{
  byte _speedOffsetL = EEPROM.read(EEP_MOTOR_OFFSET_L);
  byte _speedOffsetR = EEPROM.read(EEP_MOTOR_OFFSET_R);

  boolean _OffsetResetFlag = false;
  unsigned long OffsetResetMillis;

  Serial.println(speed);

  int melody[] = {NOTE_F5, NOTE_A5, NOTE_C6};
  int tempo[] = {8, 8, 8};
  SoundProcess(melody, tempo, 3);
  delay(200);

  while (1)
  {
    int keyData = RFreceive();
    if (keyData == 0) keyData =  TVRemoconData();

    //------------------------------------------------- DC CONTROL -----------------------------------------------------//

    if (keyData == RemoconCheckCode(KEY_U))       DCMove(forward, speed);
    else  if (keyData == RemoconCheckCode(KEY_L)) DCMove(left, speed);
    else  if (keyData == RemoconCheckCode(KEY_R)) DCMove(right, speed);
    else  if (keyData == RemoconCheckCode(KEY_D)) DCMove(backward, speed);
    else  DCMove(stop, 0);

    //----------------------------------------------- DC SPEED CHANGE ---------------------------------------------------//
    if ((keyData == RemoconCheckCode(KEY_F1)) || (keyData == RemoconCheckCode(KEY_F2)))
    {
      if ((keyData == RemoconCheckCode(KEY_F1)) &&  (speed < MX_SP))
      {
        speed += 10;
        if (speed > MX_SP)  speed = MX_SP;  //MX_SP = 100;
        EEPROM.write(EEP_MOTOR_SPEED, speed);
        tone(4, 750 + (speed * 5), 80);
      }

      else  if ((keyData == RemoconCheckCode(KEY_F2)) && (speed > 20))
      {
        speed -= 10;
        if (speed < 20)  speed = 20;
        EEPROM.write(EEP_MOTOR_SPEED, speed);
        tone(4, 750 + (speed * 5), 80);
      }

      Serial.print("Speed");  Serial.print("\t"); Serial.println(speed);
      delay(350);
    }

    //------------------------------------ DC OFFSET CALIBRATION ------------------------------------//
    //--------------------------------------------- M1 ----------------------------------------------//
    if ((keyData == RemoconCheckCode(KEY_CH1)) || (keyData == RemoconCheckCode(KEY_CH2))) // M1
    {
      if (((keyData == RemoconCheckCode(KEY_CH1)) && (_speedOffsetL < 100)) || ((keyData == RemoconCheckCode(KEY_CH2)) && (_speedOffsetL > 0)))
      {
        if ((keyData == RemoconCheckCode(KEY_CH1)) && (_speedOffsetL < 100))    DCOffsetInput(M1, _speedOffsetL++); // M1 Offset Up
        else if ((keyData == RemoconCheckCode(KEY_CH2)) && (_speedOffsetL > 0)) DCOffsetInput(M1, _speedOffsetL--); // M1 Offset Down
        EEPROM.write(EEP_MOTOR_OFFSET_L, _speedOffsetL);
      }
      tone(4, 1100, 60);
      delay(500);
      Serial.print("OffsetL");  Serial.print("\t"); Serial.println(_speedOffsetL);
    }
    //--------------------------------------------- M2 ----------------------------------------------//
    else if ((keyData == RemoconCheckCode(KEY_CH3)) || (keyData == RemoconCheckCode(KEY_CH4))) // M2
    {
      if (((keyData == RemoconCheckCode(KEY_CH3)) &&  (_speedOffsetR < 100)) || ((keyData == RemoconCheckCode(KEY_CH4)) && (_speedOffsetR > 0)))
      {
        if ((keyData == RemoconCheckCode(KEY_CH3)) &&  (_speedOffsetR < 100))   DCOffsetInput(M2, _speedOffsetR++); // M2 Offset up
        else if ((keyData == RemoconCheckCode(KEY_CH4)) && (_speedOffsetR > 0)) DCOffsetInput(M2, _speedOffsetR--); // M2 Offset Down
        EEPROM.write(EEP_MOTOR_OFFSET_R, _speedOffsetR);
      }
      tone(4, 1000, 60);
      delay(500);
      Serial.print("OffsetR");  Serial.print("\t"); Serial.println(_speedOffsetR);
    }

    //---------------------------------------- Offset Reset -----------------------------------------//

    if (keyData == RemoconCheckCode(KEY_F4))    //Offset Reset
    {
      if (_OffsetResetFlag == false)
      {
        _OffsetResetFlag = true;
        OffsetResetMillis = millis();
      }
      else if ((_OffsetResetFlag == true) && ((millis() > OffsetResetMillis + 1500)))
      {
        _OffsetResetFlag = false;
        _speedOffsetL = 50;
        _speedOffsetR = 50;
        DCOffsetInput(M1, _speedOffsetL);
        DCOffsetInput(M2, _speedOffsetR);
        EEPROM.write(EEP_MOTOR_OFFSET_L, _speedOffsetL);
        EEPROM.write(EEP_MOTOR_OFFSET_R, _speedOffsetR);
        int melody[] = {988, 1019};
        int tempo[] = {16, 4};
        SoundProcess(melody, tempo, 2);
        delay(500);
        Serial.println("Offset Reset");
      }
    }

    //----------------------------------------------------------------------------------------------//
  }
}


//-----------------------------------------------------------------------------------------------------------------------//
//---------------------------------------------------- LineTracer MODIFIED-----------------------------------------------//
//-----------------------------------------------------------------------------------------------------------------------//
int baseLV = 950;

void LineTracer(int speed)
{
  delay(300);
  
  int sensorFF = analogRead(SFF);
  int  baseLV_L = 1023;
  int  baseLV_R = 1023;

  for (int i = 0; i < 100; i++)
  {
    //Serial.println(analogRead(SBL));
    baseLV_L  = min(analogRead(SBL), baseLV_L);
    baseLV_R  = min(analogRead(SBR), baseLV_R);
  }

  baseLV_L += 50;
  baseLV_R += 50;

  EEPROM.write(EEP_LINE_SEN_BASE_LH, (baseLV_L >> 8) & 0xff);
  EEPROM.write(EEP_LINE_SEN_BASE_LL, baseLV_L & 0xff);

  EEPROM.write(EEP_LINE_SEN_BASE_RH, (baseLV_R >> 8) & 0xff);
  EEPROM.write(EEP_LINE_SEN_BASE_RL, baseLV_R & 0xff);


  // Sound_Beep2();

  int melody[] = {1702, 593, 1243};
  int tempo[] = {12, 12, 8};
  int obstacle[] = {1680, 1210};
  int tempo2[] = {5, 5};
  SoundProcess(melody, tempo, 3);
  
  
  while (1)
  {
    int sensorFF = analogRead(SFF);
    int sensorFL
    int sensorBL = analogRead(SBL);
    int sensorBR = analogRead(SBR);
    if (sensorFF < 400)
  {
    DCMove(backward, speed);
    delay(150);
    SoundProcess(obstacle, tempo2, 2);
    DCMove(right, speed);
    delay(900);
  }
    if (sensorBL < baseLV_L && sensorBR < baseLV_R)   DCMove(forward, speed);
    else if (sensorBL > baseLV_L && sensorBR < baseLV_R)    DCMove(right, speed);
    else if (sensorBL > baseLV_L && sensorBR > baseLV_R)   DCMove(left, speed);
    else if (sensorBL < baseLV_L)   DCMove(left, speed);
    else if (sensorBR < baseLV_R)   DCMove(right, speed);
  }
}


//-----------------------------------------------------------------------------------------------------------------------//
//------------------------------------------------------- Avoid ---------------------------------------------------------//
//-----------------------------------------------------------------------------------------------------------------------//

void Avoid(int speed)
{
  int sensorFL = analogRead(SFL);
  int sensorFF = analogRead(SFF);
  int sensorFR = analogRead(SFR);


  if (sensorFF < 400)
  {
    DCMove(backward, speed);
    delay(150);
    DCMove(left, speed);
    delay(250);
  }
  else if (sensorFL < 400)  DCMove(right, speed);
  else if (sensorFR < 400)  DCMove(left, speed);
  else   DCMove(forward, speed);
}



//-----------------------------------------------------------------------------------------------------------------------//
//----------------------------------------------------- CliffBot --------------------------------------------------------//
//-----------------------------------------------------------------------------------------------------------------------//

void CliffBot(int speed)
{
  int sensorBL = analogRead(SBL);
  int sensorBR = analogRead(SBR);

  if (sensorBL > 900)
  {
    DCMove(backward, speed);
    delay(700);
    DCMove(right, speed);
    delay(600);
  }
  else if (sensorBR > 900)
  {
    DCMove(backward, speed);
    delay(700);
    DCMove(left, speed);
    delay(600);
  }
  else
  {
    DCMove(forward, speed);
  }
}

