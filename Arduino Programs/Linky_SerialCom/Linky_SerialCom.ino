/*------ Board Select ----------------------------
    Arduino Pro or Pro Mini
    Atmega328 (5V, 16Mhz)
  ------------------------------------------------*/
/*-------------------------------------------------
  tunning sensor -linetracer
  Remocon set channel (0~4) - tv remocon
  Low Battery check - 3.7v
  Speed Change => Remocon Mode
  ------------------------------------------------*/
#include "config.h"
#include "ex_Linky.h"
#include "ex_Unplugged.h"
#include "RokitFirmata.h"

int mode = 0;
int basicSpeed = 100;
int speed = 100;
int speedStepSize = 10;


int modifier = 0;
int speedMod = 0;
int buzzMod = 0;
int speedM1 = 0;
int speedM2 = 0;

void setup()
{
  Serial.begin(115200);
  LowBatCheck();        //  Low Battery check - 3.7v
  initialize();
  LedDisplay(1, 100);
}

void loop()
{
  if (ReadVoltage() < 3.7)  LEDColorR(100); //  Low battery, red color
  else if (ReadMic() > 600) LEDColorG(100); //  bright 0~100 ,100: always on
  else  LEDColorG(0); //0: off
  ropi();
}

void ropi(){
  byte readbyte = 0;

  //check if the serial monitor has anything sent
 if(Serial.available() > 0)
 {
  readbyte = Serial.read();
  //if there is something sent via the serial monitor save the byte
 }
 //readbyte will be an ASCII character or letter
  switch(readbyte)
  { 
   //------------------------------------------------------------------------  
   case 76: //character "L"
   //Print all the data
   //top Left IR sensor
  Serial.print(analogRead(SFL));
  Serial.print(",");
  //top Middle IR
  Serial.print(analogRead(SFF));
  Serial.print(",");
  //top Right IR sensor
  Serial.print(analogRead(SFR));
  Serial.print(",");
  //Bottom Left IR sensor
  Serial.print(analogRead(SBL));
  Serial.print(",");
  //Bottom Right IR sensor
  Serial.print(analogRead(SBR));
  Serial.print(",");
  //Read Voltage of battery
  Serial.print(ReadVoltage());
  Serial.print(",");
  //Read the MIC
  Serial.println(ReadMic());
  break; 
  
  //------------------------------------------------------------------------  
  case 77:// character "M" 
  DCMove(forward,speed);                   
  break;  
  
  //------------------------------------------------------------------------
  case 78:// character "N" 
  DCMove(backward,speed);        
  break; 
  //------------------------------------------------------------------------
  case 79:// character "O" 
  
  DCMove(left,speed); 
  break;
  //------------------------------------------------------------------------
  case 80://character "P"
  DCMove(right,speed);       
  break;
  //------------------------------------------------------------------------
  case 81: //character "Q"
  DCMove(stop,speed);
  break;
  //------------------------------------------------------------------------
  //MOTOR SPEED
  //========================================================
     
  case 86: //character "V"
  speed = speed - speedStepSize;//decrease the speed
     if(speed<0)
  {
  speed = 0;
  }
  break;
   //========================================================
   case 87: //character "W"
   
   speed = speed + speedStepSize;//increase the speed
     if(speed>100)
  {
  speed = 100;
  }

  break;
  
  //========================================================
  case 97: //character "a"

  modifier = 0;

  break;
   //========================================================
  case 98: //character "b"

  modifier = 1;

  break;
   //========================================================
  case 99: //character "c"

  modifier = 2;

  break;
   //========================================================
  case 100: //character "d"

  modifier = 3;

  break;
   //========================================================
  case 101: //character "e"

  modifier = 4;

  break;
   //========================================================
  case 102: //character "f"

  modifier = 5;

  break;
   //========================================================
  case 103: //character "g"

  modifier = 6;

  break;
   //========================================================
  case 104: //character "h"

  modifier = 7;

  break;
   //========================================================
  case 105: //character "i"

  modifier = 8;

  break;
   //========================================================
  case 106: //character "j"

  modifier = 9;

  break;
   //========================================================
  case 107: //character "k"

  modifier = 10;

  break;
   //========================================================
  case 108: //character "l"

  speedMod = modifier*10;

  break;
   //========================================================
  case 109: //character "m"

  speed = speedMod + modifier;

  break;
   //========================================================
  case 110: //character "n"

  buzzMod = modifier*1000;

  break;
   //========================================================
  case 111: //character "o"

  buzzMod += modifier*100;

  break;
   //========================================================
  case 112: //character "p"
 
  //Buzz(buzzMod, modifier);
  //SoundProcess(int _melody[], int _tempo[], int _size)
  //buzzMod = 100;
  //modifier = 1;
  //int melody[] = {100,200};
  //int tempo[] = {1,2};
  //SoundProcess(melody, tempo, 1);
  Sound_Coin();
  
  break;
   //========================================================
  case 113: //character "q"

  speedM1 = modifier*10;

  break;
   //========================================================
  case 114: //character "r"

  speedM1 += modifier;

  break;
   //========================================================
  case 115: //character "s"

  speedM2 = modifier*10;

  break;
   //========================================================
  case 116: //character "t"

  speedM2 += modifier;

  break;
   //========================================================
  case 117: //character "u"

    switch(modifier)
    {
      case 0:
        DCMotor(M1, CCW, speedM1);
        DCMotor(M2, CW,  speedM2);
      break;
      case 1:
        DCMotor(M1, CW,  speedM1);
        DCMotor(M2, CW,  speedM2);
      break;
      case 2:
        DCMotor(M1, CCW, speedM1);
        DCMotor(M2, CCW, speedM2);
      break;
      case 3:
      default:
        DCMotor(M1, CW, speedM1);
        DCMotor(M2, CCW, speedM2);
      break;
    }

  break;
   //========================================================

  
  default:
  break;
  
  }
}

