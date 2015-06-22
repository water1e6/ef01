#ifndef Utilities_H
#define Utilities_H

#include "Arduino.h"
#include <Time.h>
#include <avr/pgmspace.h>


class Utility{

  private:
  String printDigits(int digits);
  time_t TimeFromMilsec(unsigned long time_milsec);
  
  public:
  Utility();
  void   SetTime();
  void   Log(String s);
  String Timestamp();
  String Timestamp2(unsigned long milsec=0);
  String GetUserInput(String prompt_msg);
  String FlashString(const char* str);

};




#endif


