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

void setup()
{
  Serial.begin(115200);
  LowBatCheck();        //  Low Battery check - 3.7v
  initialize();

  //-------------------------------------------------------------------------------------------------------------------------------------------//
  // if (!digitalRead(SW1) && !digitalRead(SW2))     TestMode(); // sw1, sw2 Push hold and Power On
  //-------------------------------------------------------------------------------------------------------------------------------------------//

  mode = ModeSelect();
  //if (mode == FIRMATA)  RokitFiramata();
  //basicSpeed = DCSpeedLoad();

  Sound_1up();
  LedDisplay(1, 100);
}

void loop()
{
  //if (mode == AVOID)              Avoid(basicSpeed);      //  sensor Left
  //else if (mode == LINE_TRACER)   LineTracer(basicSpeed); //  sensor Right
  //else if (mode == UNPLUGGED)     Unplugged(basicSpeed);   //  sensor Front  (Unplugged & Remocon)
  
  //this is a forever loop
  LineTracer(basicSpeed);

  int sound = ReadMic();      // Sensing clap sound
  float vin = ReadVoltage();  // Read Volatage

  if (vin < 3.7)  LEDColorR(100);       //  Low battery
  else if (sound > 600) LEDColorG(100); //  bright 0~100 ,100: always on
  else  LEDColorG(0); //0: off

  //PrintSensor();
}


