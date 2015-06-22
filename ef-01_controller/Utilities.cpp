#include "Utilities.h"
#include <Time.h>
#include <avr/pgmspace.h>


Utility::Utility()
{
}

String Utility::Timestamp()
{
  String str;
  str = "";
  str += year();
  str += "-";
  str += month();
  str += "-";
  str += day();
  str += " ";
  str += hour();
  str += printDigits(minute());
  str += printDigits(second());
  return str;
}


String Utility::Timestamp2(unsigned long milsec){  // print timestamp to serial
  time_t t;
  if (milsec != 0)
    t = TimeFromMilsec(milsec);
  else
    t = now();
  
  String str;
  str = "";
  str += year(t);
  str += "-";
  str += month(t);
  str += "-";
  str += day(t);
  str += " ";
  str += hour(t);
  str += printDigits(minute(t));
  str += printDigits(second(t));
  return str;
}


String Utility::printDigits(int digits){  // timestamp helper
  String str;
  str = ":";
  if(digits < 10)
    str += "0";
  str += digits;
  return str;
}

  
String Utility::GetUserInput(String prompt_msg){
  String input = "";
  
  Serial.print("\n");
  Serial.print(prompt_msg);
  
  while (Serial.available() == 0);
  delay(50);
  input = Serial.readStringUntil('\n');

  Serial.print("  ...  ");
  Serial.print(input);
  Serial.print("\n\n");
  return input;
}


void Utility::SetTime(){  // datetime sync via user on serial

  time_t t = GetUserInput("Enter time [\"date -v -7H +%s\"]").toInt();
  setTime(t);

  Log("System time was updated");
}


void Utility::Log(String s)
{
  Serial.print(Timestamp());
  Serial.print(": ");
  Serial.print(s);
  Serial.print("\n");
}

String Utility::FlashString(const char * str) 
{
  char c;
  String s = "";

  if (!str)
    return s;

  while ((c = pgm_read_byte(str++)))
    s += c;
//    Serial.print (c);
  
//  Serial.print ("\n");
  return s;
}

time_t Utility::TimeFromMilsec(unsigned long time_milsec)
{
  unsigned long offset = 0;
  unsigned long current_sec = millis()    / 1000;
  unsigned long base_sec    = time_milsec / 1000;
  
  if (current_sec > base_sec)
  {
    offset = current_sec - base_sec;
    return now() - offset;
  } 
  else
  {
    offset = base_sec - current_sec;
    return now() + offset;
  }  
}
