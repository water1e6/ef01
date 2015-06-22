#ifndef FilterProcess_H
#define FilterProcess_H

#include "Arduino.h"
#include "Utilities.h"

class FilterProcess {

  public:
  FilterProcess(byte pwm_duty, byte pwm_freq, byte period, 
    byte num_current_reads, byte current_sensor_pin, byte flow_sensor_pin,
    byte motor_pwm_pin, byte motor_brake_pin, byte motor_direction_pin);

  void begin();
  void CountPulse();
  void Increment();
  void ReadCurrent(); 
  void Run();
  void PrintHeader();
  void PrintStats();
  
  private:
  // pins
  byte _current_sensor_pin;
  byte _motor_pwm_pin;
  byte _flow_sensor_pin;
  byte _motor_brake_pin;
  byte _motor_direction_pin;

  // pump operating parmeters
  byte                   MotorDutyCycle;               // [0,255] written, PWM pin
  byte                   MotorPWMFrequency;            // see constant definitions


  // stats
  unsigned long          IterationEndMilsec[20];
  unsigned int           CurrentReadSums[20];          // [0,1023] analog read
  byte                   CurrentReadCounts[20];
  unsigned int           FlowPulses[20];

  // counters & accumulators
  byte                   IterationCount;
  byte                   CurrentReadCount;
  volatile unsigned int  FlowPulseCount;
  unsigned int           CurrentReadSum;
  
  // state values
  unsigned long          PriorTimeMilsec;
  unsigned long          PresentTimeMilsec;
  unsigned long          LastCurrentReadMilsec;
  unsigned long          ProcessStartTimeMilsec;
  unsigned long          ProcessEndTimeMilsec;
  byte                   ProcessState;

  // parameters
  byte                   IterationPeriodSec;
  unsigned long          IterationPeriodMilsec;
  byte                   NumCurrentReads;
  unsigned long          CurrentReadPeriodMilsec;


  // for printing etc
  Utility _u;
};


#endif
