#include "FilterProcess.h"
#include "Utilities.h"

FilterProcess::FilterProcess(byte pwm_duty, byte pwm_freq, byte period, 
    byte num_current_reads, byte current_sensor_pin, byte flow_sensor_pin,
    byte motor_pwm_pin, byte motor_brake_pin, byte motor_direction_pin)

  : _u(),
    MotorPWMFrequency    (pwm_freq),
    MotorDutyCycle       (pwm_duty),                 // init pump, set PWM duty cycle
    NumCurrentReads      (num_current_reads),
    IterationPeriodSec   (period),


    _current_sensor_pin  (current_sensor_pin),
    _flow_sensor_pin     (flow_sensor_pin),
    _motor_pwm_pin       (motor_pwm_pin),
    _motor_brake_pin     (motor_brake_pin),
    _motor_direction_pin (motor_direction_pin)

  {}

//FilterProcess::~FilterProcess(){}

void FilterProcess::begin(){
  
  
  // pins and pump operating parmeters
  pinMode(_motor_pwm_pin, OUTPUT); 
  analogWrite(_motor_pwm_pin, MotorDutyCycle);

  TCCR2B = TCCR2B & B11111000 | MotorPWMFrequency;    // PWM frequency via counter2 registers

  pinMode(_motor_direction_pin, OUTPUT);              // motor forward direction 
  digitalWrite(_motor_direction_pin, HIGH);

  pinMode(_motor_brake_pin, OUTPUT);                  // motor brake off
  digitalWrite(_motor_brake_pin, LOW);          

  pinMode(_current_sensor_pin, INPUT);                // init pin to sense current
  analogReference(EXTERNAL);                          // analog read use external reference [wired to 3.3v]

  pinMode(_flow_sensor_pin, INPUT);               // flow sensor init, 5v pulses
  digitalWrite(_flow_sensor_pin, HIGH);           // falling edge via interrupt
  
  
  // stats
  // todo: initialize & set to zero

  // counters & accumulators
  IterationCount          = 0;
  CurrentReadCount        = 0;
  FlowPulseCount          = 0;
  CurrentReadSum          = 0;
  
  // state values
  PriorTimeMilsec         = 0;
  PresentTimeMilsec       = 0;
  LastCurrentReadMilsec   = 0;
  ProcessStartTimeMilsec  = 0;
  ProcessEndTimeMilsec    = 0;
  ProcessState            = 0;

  // parameters
  IterationPeriodMilsec   = (unsigned long) IterationPeriodSec * 1000UL;
  CurrentReadPeriodMilsec = IterationPeriodMilsec / (unsigned long) NumCurrentReads;
}

      
void FilterProcess::CountPulse(){
  ++FlowPulseCount;
}

void FilterProcess::Increment(){
  
  // todo -- think through edge conditions of interrupt and possible disabling
  FlowPulses[IterationCount] = FlowPulseCount; // first empty pulse counter so ready for next interrupt
  FlowPulseCount = 0;

  if((FlowPulses[IterationCount] == 0) && (ProcessState == 1)){        // state 1->2
    ProcessState = 2;
    ProcessEndTimeMilsec = PresentTimeMilsec;
    _u.Log("filter process ended");
  } else if ((FlowPulses[IterationCount] > 0) && (ProcessState == 0)){ // state 0->1
    ProcessState = 1;
    ProcessStartTimeMilsec = PresentTimeMilsec;
    _u.Log("filter process started");
  }

  IterationEndMilsec[IterationCount] = PresentTimeMilsec;
  PriorTimeMilsec                    = PresentTimeMilsec;
  
  CurrentReadSums[IterationCount]    = CurrentReadSum;    // current read sum
  CurrentReadSum                     = 0;

  CurrentReadCounts[IterationCount]  = CurrentReadCount; // current read count
  CurrentReadCount                   = 0;

  if(ProcessState == 1)                                 // wait for process to begin before starting counter
    ++IterationCount;
}


void FilterProcess::ReadCurrent(){
      
    CurrentReadSum += (unsigned int) analogRead(_current_sensor_pin);  // 0.100 ms
    ++CurrentReadCount;
    LastCurrentReadMilsec = PresentTimeMilsec;
}


void FilterProcess::Run(){

  _u.Log("filter process ready to start");
  PriorTimeMilsec = millis();

  while ((ProcessState != 2) && (IterationCount < 20)){
  
    PresentTimeMilsec = millis();  // todo handle overflows

    if((PresentTimeMilsec - LastCurrentReadMilsec) >= CurrentReadPeriodMilsec)
      ReadCurrent();

    if((PresentTimeMilsec - PriorTimeMilsec) >= IterationPeriodMilsec)
      Increment();

  }
}


void FilterProcess::PrintHeader(){  /* data output header */

  // experiment data
  Serial.print("datetime: ");
  Serial.print(_u.Timestamp2(ProcessStartTimeMilsec));
  Serial.print("\n");

  Serial.print("iteration length [ms]: ");
  Serial.print(IterationPeriodMilsec);
  Serial.print("\n");

  Serial.print("motor pwm weight: ");
  Serial.print(MotorDutyCycle);
  Serial.print("\n");

  Serial.print("motor pwm freq: ");
  Serial.print(MotorPWMFrequency);
  Serial.print("\n");

  Serial.print("R1: ");
  Serial.print("10k");
  Serial.print("\n");

  Serial.print("C1: ");
  Serial.print("100uF");
  Serial.print("\n");

  Serial.print("C2: ");
  Serial.print("0.1uF");
  Serial.print("\n");

  Serial.print("D1: ");
  Serial.print("Schottky");
  Serial.print("\n");

//  Serial.print("iteration length [ms]: ");
//  Serial.print(IterationLength);
  Serial.print("\n\n");
  Serial.print("-- stats --");
  Serial.print("\n");
  
  // field labels
  Serial.print("iteration");
  Serial.print(",");
  Serial.print("pulse count");
  Serial.print(",");
  Serial.print("current read sum");
  Serial.print(",");
  Serial.print("current read count");
  Serial.print("\n");
}


void FilterProcess::PrintStats(){
    
  byte i = 0;
  unsigned long ml_0, ml_1, I_0, I_1, process_time;
  float error, amps, flowrate_0, flowrate_1;

  ml_0 = 0UL;
  ml_1 = 0UL;
  I_0  = 0UL;
  I_1  = 0UL;

  Serial.print("\n\n");
  PrintHeader();

      
  for (i=0; i <= IterationCount; i++){
    
    amps = (float) CurrentReadSums[i] / 
           (float) CurrentReadCounts[i] / 
           1023.0 * 2.0;

    Serial.print(i);
    Serial.print(",");
    Serial.print(FlowPulses[i]);
    Serial.print(",");
    Serial.print(amps);
    Serial.print(",");
    Serial.print(CurrentReadCounts[i]);
    Serial.print("\n");
    
    if(FlowPulses[i] > 0){
      ml_1 += (unsigned long) FlowPulses[i];
      I_0  += (unsigned long) CurrentReadSums[i];
      I_1  += (unsigned long) CurrentReadCounts[i];
    }
    
  }

  Serial.print("\n\n");

  ml_0 = _u.GetUserInput("Enter measured volume in ml").toInt();

  Serial.print("\n\n");
  Serial.print("-- calcs --");
  Serial.print("\n");

  //ml_1 = ml_1 / 22UL;
  error        = ( (float) ml_1 / 22.0) / (float) ml_0 - 1.0;
  amps         = (float) I_0 / 1023.0 / (float) I_1 * 2.0;
  process_time = ProcessEndTimeMilsec - ProcessStartTimeMilsec;
  flowrate_0   = (float) ml_0 / (float) process_time * 1000.0 * 60.0;
  flowrate_1   = (float) ml_1 / 22.0 / (float) process_time * 1000.0 * 60.0;
  
  Serial.print("ml_0: ");
  Serial.print(ml_0);
  Serial.print("; ");
  Serial.print("ml_1: ");
  Serial.print(ml_1 / 22);
  Serial.print("; ");
  Serial.print("I_0: ");
  Serial.print(I_0);
  Serial.print("; ");
  Serial.print("I_1: ");
  Serial.print(I_1);
  Serial.print("; ");

  Serial.print("\n\n");

  Serial.print("Error: ");
  Serial.print(error);
  Serial.print("\n");

  Serial.print("Avg amps: ");
  Serial.print(amps);
  Serial.print("\n");
  
  Serial.print("Flow rate [independent]: ");
  Serial.print(flowrate_0);
  Serial.print("\n");

  Serial.print("Flow rate [sensor]: ");
  Serial.print(flowrate_1);
  Serial.print("\n");

  Serial.print("\n\n");

  
}



