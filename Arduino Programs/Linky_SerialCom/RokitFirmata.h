#include "config.h"

#define VERSION             0b01000001
#define SERIAL_BAUDRATE     57600

#define DCMOTOR               0x0D
#define LED_COLOR             0x0E
#define BUZZER                0x0F
#define STREAM_ON             0x0B
#define STREAM_OFF            0x0C

#define READY                 0xE0
#define OFF                   0xD0

long arduinoPin = 0;
byte recvBytes[5] = {0};
byte bufferBytes[20][6];
int cnt = 0;
int col = 0;
int follow = 0;
long exMillis = 0;
boolean isReceiving = false;
boolean isReady = true;
boolean isStreaming = true;
byte exInByte = 0;
volatile unsigned int pitchData = 0;
volatile unsigned int dur = 0;

void serialRead();
//-----------------------------------------//

int soundCount = 0;
int soundCheck = 0;
int soundMn = 1024;
int soundMx = 0;

//-----------------------------------------//

void RokitFiramata()
{
  //  Serial.begin(SERIAL_BAUDRATE);
  //  initialize();

  LEDColorR( 0x20);
  LEDColorG( 0x20);
  LEDColorB( 0x20);
  Sound_FireBall();
  LEDColorR( 0x00);
  LEDColorG( 0x00);
  LEDColorB( 0x00);

  for (int j = 0; j < 20; j++)
  {
    bufferBytes[j][5] = OFF;
  }

  while (1)
  {
    //---------------------------------------------//
    if (soundCount++ < 3500)
    {
      int val = analogRead(MIC_PIN);
      soundMn = min(soundMn, val);
      soundMx = max(soundMx, val);
    }
    else
    {
      soundCheck = soundMx - soundMn;
      soundCount = 0;
      soundMn = 1024;
      soundMx = 0;
    }
    //---------------------------------------------//

    serialRead();

    if (isReady == true)
    {
      switch (bufferBytes[follow][1])
      {
        case DCMOTOR:
          {
            DCMotor(bufferBytes[follow][2], bufferBytes[follow][3], bufferBytes[follow][4]);
            break;
          }
        case LED_COLOR:
          {
            LEDColorR( bufferBytes[follow][2]);
            LEDColorG( bufferBytes[follow][3]);
            LEDColorB( bufferBytes[follow][4]);
            break;
          }
        case BUZZER:
          {
            pitchData = ((unsigned int)bufferBytes[follow][3] << 8) | bufferBytes[follow][2];
            dur =  bufferBytes[follow][4] * 2;
            //Serial.print("pitch: "); Serial.print(pitchData); Serial.print(" duration: "); Serial.println(dur);
            tone(4, pitchData, dur);
            break;
          }
        case STREAM_ON:
          {
            isStreaming = true;
            break;
          }

        case STREAM_OFF:
          {
            isStreaming = false;
            break;
          }
      }
      bufferBytes[follow][5] = OFF;

      follow++;
      if (follow >= 20) follow = 0;
      if (bufferBytes[follow][5] == OFF) isReady = false;  else isReady = true;
    }
    else
    {
      for (int j = 0; j < 20; j++)  {
        if (bufferBytes[j][5] == READY) {
          isReady = true;
          follow = j;
          break;
        }
      }
    }

    if ((millis() - exMillis) >= 30 )
    {
      if (arduinoPin <= 4)                //ADC
      {
        int val = analogRead(arduinoPin);
        long sendVal = (arduinoPin << 16) | val;
        if (isStreaming == true) Serial.println(sendVal);
        if (arduinoPin == 4) arduinoPin = 5;
      }
      else if (arduinoPin == 6)
      {
        float volt = ReadVoltage() * 100;
        long sendVal = (arduinoPin << 16) | (unsigned int)volt;
        if (isStreaming == true) Serial.println(sendVal);
      }
      else if (arduinoPin == 7)
      {
        int mic = soundCheck;
        long sendVal = (arduinoPin << 16) | (unsigned int)mic;
        if (isStreaming == true) Serial.println(sendVal);
        arduinoPin = 8;
      }
      else if (arduinoPin == 9)          //IR_REMOCON - Pin number matched to number of Rokitbrick vacant receiving buffer
      {
        unsigned long sendVal = (arduinoPin << 16) | ((unsigned int)(VERSION << 8)) | (unsigned char)TVRemoconData();
        if (isStreaming == true) Serial.println(sendVal);
      }
      arduinoPin++;
      if (arduinoPin >= 10) arduinoPin = 0;
      exMillis = millis();
    }
  }
}

//FF FF dataNumber command parameter..
void serialRead() {
  while (Serial.available()) {

    byte inByte = (byte)Serial.read();
    //Serial.print(inByte, HEX);
    if (isReceiving == true)
    {
      recvBytes[cnt] =  inByte;
      //Serial.print(recvBytes[cnt]);
      cnt++;

      if (recvBytes[0] < cnt)
      {
        isReady = true; isReceiving = false; cnt = 0;
        for (int j = 0; j < 5; j++)
        {
          bufferBytes[col][j] = recvBytes[j];
        }
        bufferBytes[col][5] = READY; col++;
        if (col >= 20) col = 0;
      }
    }
    if ((inByte == 0xFF) && (exInByte == 0xFF)) {
      isReceiving = true;
      exInByte = 0;
    }
    else {
      exInByte = inByte;
    }
  }
}


