#include "arduino.h"
#include <EEPROM.h>
//motor pin (AIN1 ~ BIN2)

#define  EEP_MOTOR_SPEED  25

#define  EEP_REMOCON_CHANNEL  26
#define  EEP_MOTOR_OFFSET_L   27
#define  EEP_MOTOR_OFFSET_R   28

#define  EEP_LINE_SEN_BASE_LH  21
#define  EEP_LINE_SEN_BASE_LL  22

#define  EEP_LINE_SEN_BASE_RH  23
#define  EEP_LINE_SEN_BASE_RL  24



#define  EEP_FIRST_START_CHECK1   30
#define  EEP_FIRST_START_CHECK2   31
#define  EEP_FIRST_START_CHECK3   32
#define  EEP_FIRST_START_CHECK4   33

#define AVOID        1
#define LINE_TRACER  2
#define CLIFF_BOT    3
#define UNPLUGGED      4
#define FIRMATA      5

#define AIN1  3
#define AIN2  7

#define BIN1  6
#define BIN2  8

#define IR_PIN  2 //ir receiver

#define MELODY_PIN   4    //buzzer
#define MIC_PIN      A7   //MIC
#define BATCHECK_PIN A6   //BAT CHECK

//sensor pin (SFL ~ SBR)
#define SFL   A0   // left   ir sensor
#define SFF   A1   // front  ir sensor
#define SFR   A2   // right  ir sensor

#define SBL   A3   // bottom left ir sensor
#define SBR   A4   // bottom right ir sensor

// Switch
#define SW1   19
#define SW2   5

// power check resistor
#define R1  2200.0
#define R2  1000.0

#define LED_RED   10
#define LED_BLUE  9
#define LED_GREEN 11

#define  MX_SP  100
#define  MX_BR  100

//--------------------------------------------------//
#define LED_RED_ON    digitalWrite(LED_RED, HIGH)
#define LED_GREEN_ON  digitalWrite(LED_GREEN, HIGH)
#define LED_BLUE_ON   digitalWrite(LED_BLUE, HIGH)

#define LED_RED_OFF   digitalWrite(LED_RED, LOW)
#define LED_GREEN_OFF digitalWrite(LED_GREEN, LOW)
#define LED_BLUE_OFF  digitalWrite(LED_BLUE, LOW)
//--------------------------------------------------//

//dc
#define CW    1
#define CCW   2
#define STOP    3
#define LOOSE   4

#define FORWARD   1
#define BACKWARD  2
#define LEFT      5
#define RIGHT     6

#define forward   FORWARD
#define backward  BACKWARD

#define stop    STOP
#define loose   LOOSE

#define left    LEFT
#define right   RIGHT

#define M1  1
#define M2  2

void initialize();
int ModeSelect();

void LED_Init();
void LEDControl();
void LEDColorAll(int red, int green, int blue);
void LEDColorR(int value);
void LEDColorG(int value);
void LEDColorB(int value);

void Timer1Set();

void Sound_FireBall();
void Sound_Coin();
void Sound_1up();
void SoundProcess(int _melody[], int _tempo[], int _size);
void Sound_Beep1();
void Sound_Beep2();

void DCControl();
void DCInit();
void DCMotor(char SelectMotor, char direction, char speed);
void DCMove(byte direction, byte speed);
void DCOffsetInput(char SelectMotor, int value);
int DCSpeedLoad();

void PrintSensor();

int ReadMic();
float ReadVoltage();

int RFreceive();

void TVRemocon();
int TVRemoconData();
void CheckLeadCode();
void CheckCustomCode();
void CheckDataCode();

boolean TimeCheckCode(word interval); //micros seconds
boolean TimeCheckLeadcode(word interval);  //milliseconds
boolean TimeCheckEnd(word interval);  //milliseconds
boolean TimeOutCheckEnd(word interval);  //milliseconds

void RemoconChannelChange(int _code);
int RemoconCheckCode(int _code);

void TestMode();
void LowBatCheck();
void LedDisplay(int count, int time);

/***********************************************************************/

/***********************************************************************/
//TVRemocon

#define  KEY_U       1
#define  KEY_D       2
#define  KEY_L       3
#define  KEY_R       4
#define  KEY_C       5

#define  KEY_F1      6
#define  KEY_F2      7
#define  KEY_F3      8
#define  KEY_F4      9

#define  KEY_CH1     10
#define  KEY_CH2     11
#define  KEY_CH3     12
#define  KEY_CH4     13

#define  KEY_FL     14
#define  KEY_FR     15


#define  LeadCodetime   8
#define  TimeChecktime  3000
#define  EndChecktime   7
#define  TimeCheckdatatime  1500

#define  findLeadCode 1
#define  startReadCode 2

#define customCodeTVremocon 0b1110000011100000

#define CH1 1
#define CH2 2
#define CH3 3
#define CH4 4

#define  CH1_KEY_U   0x80
#define  CH1_KEY_D   0x40
#define  CH1_KEY_L   0x20
#define  CH1_KEY_R   0x10
#define  CH1_KEY_C   0xF0
#define  CH1_KEY_F1  0x08
#define  CH1_KEY_F2  0x88
#define  CH1_KEY_F3  0x48
#define  CH1_KEY_F4  0xC8

#define  CH1_KEY_CH1  0x14
#define  CH1_KEY_CH2  0x15
#define  CH1_KEY_CH3  0x16
#define  CH1_KEY_CH4  0x17

#define  CH2_KEY_U   0x8D
#define  CH2_KEY_D   0x4D
#define  CH2_KEY_L   0x2D
#define  CH2_KEY_R   0x1D
#define  CH2_KEY_C   0xFD
#define  CH2_KEY_F1  0x04
#define  CH2_KEY_F2  0x84
#define  CH2_KEY_F3  0x44
#define  CH2_KEY_F4  0xC4

#define  CH2_KEY_CH1  0xE4
#define  CH2_KEY_CH2  0xE5
#define  CH2_KEY_CH3  0xE6
#define  CH2_KEY_CH4  0xE7

#define  CH3_KEY_U   0x83
#define  CH3_KEY_D   0x43
#define  CH3_KEY_L   0x23
#define  CH3_KEY_R   0x13
#define  CH3_KEY_C   0xF3
#define  CH3_KEY_F1  0x0C
#define  CH3_KEY_F2  0x8C
#define  CH3_KEY_F3  0x4C
#define  CH3_KEY_F4  0xCC

#define  CH3_KEY_CH1  0x64
#define  CH3_KEY_CH2  0x65
#define  CH3_KEY_CH3  0x66
#define  CH3_KEY_CH4  0x67

#define  CH4_KEY_U   0x8B
#define  CH4_KEY_D   0x4B
#define  CH4_KEY_L   0x2B
#define  CH4_KEY_R   0x1B
#define  CH4_KEY_C   0xFB
#define  CH4_KEY_F1  0x02
#define  CH4_KEY_F2  0x82
#define  CH4_KEY_F3  0x42
#define  CH4_KEY_F4  0xC2

#define  CH4_KEY_CH1  0xA4
#define  CH4_KEY_CH2  0xA5
#define  CH4_KEY_CH3  0xA6
#define  CH4_KEY_CH4  0xA7

#define  Rf_KEY_UP  0x00
#define  Rf_KEY_U 0x01
#define  Rf_KEY_D 0x02
#define  Rf_KEY_L 0x04
#define  Rf_KEY_R 0x08

#define  Rf_KEY_1 0x10
#define  Rf_KEY_2 0x20
#define  Rf_KEY_3 0x40
#define  Rf_KEY_4 0x80
#define  Rf_KEY_5 0x100
#define  Rf_KEY_6 0x200

/*************************************************
   Public Constants
 *************************************************/

#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978



