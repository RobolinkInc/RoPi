#include <SmartInventor.h>
#include <Servo.h> 

int speed = 30;               //motor speed 
//this can have a max value of 100

int modifier = 0;

int speedMod = 0;
int buzzMod = 0;
int speedM1 = 0;
int speedM2 = 0;

Servo myservo1;      // create servo object to control a servo1
Servo myservo2;      // create servo object to control a servo2

int servo1angle = 90;
int servo2angle = 90;

int servoSteps = 5;

void setup()
{
  SmartInventor.DCMotorUse();  // DC Motor Use
  SmartInventor.TVRemoconUse();  // TV Remocon Use

  Serial.begin(115200);       // initialize serial communication at 9600 bits per second:
  
//Set up the digital inputs for the bottom IR sensors
  pinMode(11,INPUT);
  pinMode(12,INPUT);
  pinMode(13,INPUT);
  pinMode(14,INPUT);
  pinMode(16,INPUT);
  pinMode(17,INPUT);
  pinMode(18,INPUT);

  myservo1.attach(27);        // attaches the servo 1 on pin 27 to the servo object 
  myservo2.attach(28);        // attaches the servo 2 on pin 28 to the servo object 

  myservo1.write(servo1angle);          // servo1 to go to position in 90 degrees
  myservo2.write(servo2angle);          // servo2 to go to position in 90 degrees
  
  
}

void loop()
{
  
 byte readbyte;

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
  case 75://character "K"
  Serial.println(SmartInventor.TVRemoconData());
  break; 
    //------------------------------------------------------------------------   
  case 76: //character "L"
  
  Serial.print(digitalRead(11));
  Serial.print(" ");
  Serial.print(digitalRead(12));
  Serial.print(" ");
  Serial.print(digitalRead(13));
  Serial.print(" ");
  Serial.print(digitalRead(14));
  Serial.print(" ");
  Serial.print(digitalRead(16));
  Serial.print(" ");
  Serial.print(digitalRead(17));
  Serial.print(" ");
  Serial.println(digitalRead(18));
  break; 

  //------------------------------------------------------------------------  
  case 77:// character "M" 
  SmartInventor.DCMove(forward,speed);                        
  break;  
  //------------------------------------------------------------------------
  case 78:// character "N" 
  SmartInventor.DCMove(backward,speed);         
  break; 
  //------------------------------------------------------------------------
  case 79:// character "O" 
  SmartInventor.DCMove(left,speed); 
  break;
  //------------------------------------------------------------------------
  case 80://character "P"
  SmartInventor.DCMove(right,speed);        
  
  break;
  //------------------------------------------------------------------------
  case 81: //character "Q"
  SmartInventor.DCMove(stop,speed);
  break;
  //------------------------------------------------------------------------
  case 82: //character "R"
  servo1angle = servo1angle - servoSteps;//decrease the position
    if(servo1angle<0)//CHECK NOT BELOW 0
    {
    servo1angle = 0;
    }

        myservo1.write(servo1angle);
 
  break;
   //------------------------------------------------------------------------
  case 83: //character "S"
   servo1angle = servo1angle + servoSteps;//increase the position
     if(servo1angle>180)
    {
    servo1angle = 180;
    }

        myservo1.write(servo1angle);
  
  break;
    //------------------------------------------------------------------------
     //========================================================
  case 84: //character "T"
    servo2angle = servo2angle - servoSteps;//decrease the position
   if(servo2angle<0)
    {
    servo2angle = 0;
    }
   //   Serial.print("servo2:");
   //   Serial.println(servo2angle);
        myservo2.write(servo2angle);
  
  break;
     //========================================================
  case 85: //character "U"
    servo2angle = servo2angle + servoSteps;//increase the position
     if(servo2angle>180)
    {
    servo2angle = 180;
    }

    //Serial.print("servo2:");
      //Serial.println(servo2angle);
        myservo2.write(servo2angle);
  break;
  
  
  //MOTOR SPEED
   //========================================================
     
  case 86: //character "V"
  speed = speed - 10;//decrease the speed
     if(speed<0)
  {
  speed = 0;
  }
    //Serial.print("speed:");
  //Serial.println(speed);
  break;
  
     //========================================================
   case 87: //character "W"
   
   speed = speed + 10;//increase the speed
     if(speed>100)
  {
  speed = 100;
  }

  break;
  
  
  
  //SERVO STEP SIZE
  
  //========================================================
   case 88: //character "X"
     if(servoSteps<100)
  {
  servoSteps = servoSteps + 1;//increase the position
  }
  //Serial.print("servo step size:");
  //Serial.println(servoSteps);
  break;
 
    //========================================================
  case 89: //character "Y"
     if(servoSteps>0)
  {
  servoSteps = servoSteps - 1;//increase the position
  }
    //Serial.print("servo step size:");
  //Serial.println(servoSteps);
  break;
   //========================================================

  case 90: //character "Z"
  
  Serial.print(servo1angle);
  Serial.print(" ");
  Serial.print(servo2angle);
  Serial.print(" ");
  Serial.print(servoSteps);
  Serial.print(" ");
  Serial.print(speed);
  Serial.print(" ");
  Serial.print(analogRead(19));
  Serial.print(" ");
  Serial.print(analogRead(20));
  Serial.print(" ");
  Serial.println(analogRead(21));
  
  //Serial.println(" ");
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
  
  SmartInventor.Buzz(buzzMod, modifier);

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
        SmartInventor.DCMotor(M1, CCW, speedM1);
        SmartInventor.DCMotor(M2, CW, speedM2);
      break;
      case 1:
        SmartInventor.DCMotor(M1, CW, speedM1);
        SmartInventor.DCMotor(M2, CW, speedM2);
      break;
      case 2:
        SmartInventor.DCMotor(M1, CCW, speedM1);
        SmartInventor.DCMotor(M2, CCW, speedM2);
      break;
      case 3:
      default:
        SmartInventor.DCMotor(M1, CW, speedM1);
        SmartInventor.DCMotor(M2, CCW, speedM2);
      break;
    }

  break;
   //========================================================

  default:
  break;
  
  }
  
}  





