/** attempt #2

 flow sensor - Gems FT-210 
 https://drive.google.com/a/mercerislandwater.com/file/d/0B-nn__bSHZxpNmtJMWJnUkxTVG8

**/

#include <Time.h>
#include <avr/pgmspace.h>
#include "FilterProcess.h"
#include "Utilities.h"

/* pin setup */
byte CurrentSensorPin  = A0;  // analog
byte FlowSensorPin     = 2;  // digital
byte MotorPWMPin       = 3;
byte MotorBrakePin     = 9;
byte MotorDirectionPin = 12;
byte SensorInterrupt   = 0;    // hw interrupt

/* global vars, constants */
const unsigned long SerialBaudRate        = 115200;  // data rate for serial
const unsigned long SerialReadTimeout     = 3000;  // timeout for read ops [micros]

/* bit masks for PWM frequency - arduino uno */
const byte PWM_1 = B00000001;  // 31,373 hz
const byte PWM_2 = B00000010;  // 3,921 hz
const byte PWM_3 = B00000011;  // 980 hz
const byte PWM_4 = B00000100;  // 490 hz [default]
const byte PWM_5 = B00000101;  // 245 hz
const byte PWM_6 = B00000110;  // 123 hz
const byte PWM_7 = B00000111;  // 31 hz

/* menu system */
const char cmd_str_0[] PROGMEM = "-- command menu --";
const char cmd_str_m[] PROGMEM = "m [menu]  -- print command menu";
const char cmd_str_t[] PROGMEM = "t [time]  -- set system time";
const char cmd_str_r[] PROGMEM = "r [run]   -- run filter process";
const char cmd_str_s[] PROGMEM = "s [stats] -- print process stats";
const char cmd_str_n[] PROGMEM = "n [new]   -- create new process";

const char* const cmd_table[] PROGMEM = {
                                        cmd_str_0, cmd_str_m, cmd_str_t, 
                                        cmd_str_r, cmd_str_s, cmd_str_n
                                        };


Utility u;
char command;


FilterProcess fp (100, PWM_4, 5, 10, CurrentSensorPin, FlowSensorPin, MotorPWMPin, MotorBrakePin, MotorDirectionPin);

/* global functions */
void RunFilterProcess()
{
  fp.begin();
  attachInterrupt(SensorInterrupt, ISR_0, FALLING);
  fp.Run();
  detachInterrupt(SensorInterrupt);
}

void PrintFilterProcessStats()
{
  fp.PrintStats();
}



void PrintCommandMenu()
{
  int num_commands, i;
  String s;
  
  num_commands = sizeof(cmd_table)/sizeof(char*);
  
  Serial.print("\n");

  for(i=0; i<num_commands; ++i)
    {
    s = u.FlashString((const char *) pgm_read_word (&cmd_table[i]));
    Serial.println(s);
    }

  Serial.print("\n");
}


char GetCommand()
{
  String s = u.GetUserInput("Enter command ['m' for menu]");
  s.toLowerCase();
  return s[0];
}


void NewFilterProcess()
{
  String s;

  byte pwm_duty, pwm_freq, period, num_current_reads;
  
  s = u.GetUserInput("pwm duty [0-255]");
  pwm_duty = s.toInt();
  
  s = u.GetUserInput("pwm freq [1-7]");
  pwm_freq = s.toInt();

  s = u.GetUserInput("period [seconds]");
  period = s.toInt();
  
  s = u.GetUserInput("num current reads");
  num_current_reads = s.toInt();

  FilterProcess fp_new (pwm_duty, pwm_freq, period, num_current_reads, CurrentSensorPin, FlowSensorPin, MotorPWMPin, MotorBrakePin, MotorDirectionPin);
  fp = fp_new;

}

void RunCommand(char cmd)
{

  switch (cmd) {

    case 'm':
      u.Log(u.FlashString((const char *) pgm_read_word (&cmd_table[1])));
      PrintCommandMenu();
      break;

    case 't':
      u.Log(u.FlashString((const char *) pgm_read_word (&cmd_table[2])));
      u.SetTime();
      break;

    case 'r':
      u.Log(u.FlashString((const char *) pgm_read_word (&cmd_table[3])));
      RunFilterProcess();
      break;

    case 's':
      u.Log(u.FlashString((const char *) pgm_read_word (&cmd_table[4])));
      PrintFilterProcessStats();
      break;

    case 'n':
      u.Log(u.FlashString((const char *) pgm_read_word (&cmd_table[5])));
      NewFilterProcess();
      break;

    default:
      u.Log("bad command");    
  } // switch

}


void setup() {

  Serial.begin(SerialBaudRate);          // open serial
  Serial.setTimeout(SerialReadTimeout);  // serial timeout
}

void loop() {

  u.Log("main loop");

//  fp.begin();
//  attachInterrupt(SensorInterrupt, ISR_0, FALLING);
//  fp.Run();
//  detachInterrupt(SensorInterrupt);
//  fp.PrintStats();

  command = GetCommand();
  RunCommand(command);

  delay(50);
}

void ISR_0(){
  fp.CountPulse();
}

