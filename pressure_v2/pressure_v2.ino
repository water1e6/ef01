#include <Time.h>
#include <SPI.h>
#include "Utilities.h"


    /* pins */
byte         ADCSelectPin           = 7;
byte         ADCResetPin            = 8;
byte         DRDYPin                = 9;
byte         ArduinoSSPin           = 10;
byte         MOSIPin                = 11;
byte         MISOPin                = 12;
byte         SCLKPin                = 13;


    /* registers */
byte         reg_comm               = B000;
byte         reg_mode               = B001;
byte         reg_filter_high        = B010;
byte         reg_filter_low         = B011;
byte         reg_test               = B100;
byte         reg_data               = B101;
byte         reg_cal_zero           = B110;
byte         reg_cal_full           = B111;


    /* channels */
byte         ch_pseudo_1            = B000;
byte         ch_pseudo_2            = B001;
byte         ch_pseudo_3            = B010;
byte         ch_pseudo_4            = B011;
byte         ch_diff_1              = B100;
byte         ch_diff_2              = B101;
byte         ch_diff_3              = B110;
byte         ch_test                = B111;
byte         ch_pseudo_5            = B110;


    /* modes */
byte         mode_normal            = B000;
byte         mode_cal_self          = B001;
byte         mode_cal_zero          = B010;
byte         mode_cal_full          = B011;
byte         mode_cal_offset        = B100;
byte         mode_cal_bg            = B101;
byte         mode_cal_zs_self       = B110;
byte         mode_cal_fs_self       = B111;


    /* gains */
byte         gain_1                 = B000;
byte         gain_2                 = B001;
byte         gain_4                 = B010;
byte         gain_8                 = B011;
byte         gain_16                = B100;
byte         gain_32                = B101;
byte         gain_64                = B110;
byte         gain_128               = B111;


    /* settings */
boolean      setting_unipolar       = 0;
boolean      setting_24bit          = 1;
boolean      setting_current_boost  = 0;
boolean      setting_zero           = 0;
unsigned int setting_filter_code    = 4000;  // [19, 4000]
boolean      setting_burnout        = 0;
boolean      setting_reset_filters  = 0;


boolean W = 0;
boolean R = 1;

unsigned int ClockSpeed = 1000000;
const byte SPIBufferSize = 3;
byte bytearr[SPIBufferSize];

Utility u;

void ResetADC()
{
  u.Log("ADC reset starting [5s]");
  digitalWrite(ADCResetPin, LOW);
  delay(5000);
  digitalWrite(ADCResetPin,  HIGH);
};


void DumpBytes(byte _num_bytes=1, byte _bytearr[]={})
{
 
  if (_num_bytes > SPIBufferSize)
  {
    u.Log("Warning: buffer smaller than num_bytes.");
    _num_bytes = SPIBufferSize;
  }
  for (int i=0; i<_num_bytes; ++i)
    u.Log(String(_bytearr[i],BIN));

  return;
}


void WaitDRDY()
{
  u.Log("wait drdy");
  while(digitalRead(DRDYPin) == HIGH) 
    {}
  return;
};


void SPIBytes(byte _b_out, byte _num_bytes, byte _bytearr[], boolean _drdy)
{
  if (_num_bytes > SPIBufferSize)
    {
    u.Log("Error: buffer cannot fit num_bytes.  Read aborted.");
    return;
    }

  SPI.beginTransaction(SPISettings(ClockSpeed, MSBFIRST, SPI_MODE1));  // 2 MHz clock / sigbit / clock data mode [idle low, use high->low edge]
  digitalWrite(ADCSelectPin, LOW);

  // todo clear buffer ... bytearr[i] = 0;
  // todo interrupts to catch drdy transition

  for (int i=0; i<_num_bytes; ++i)
    {
    if (_drdy) 
      WaitDRDY();
    
    _bytearr[i] = SPI.transfer(_b_out);
    }

  digitalWrite(MOSIPin, HIGH);
  digitalWrite(ADCSelectPin, HIGH);
  SPI.endTransaction();

  return;
}


byte CommCmd(byte reg, byte channel, boolean readonly)
{
  byte b = 0;

  b += reg << 4;
  b += (byte) readonly << 3;
  b += channel;
  
  return b;
}


byte FilterHighCmd(boolean _unipolar, boolean _24bit, boolean _current_boost,
                   boolean _zero, int _filter_code)
{
  byte _cmd = 0;

  _cmd += (byte) _unipolar       <<  7;
  _cmd += (byte) _24bit          <<  6;
  _cmd += (byte) _current_boost  <<  5;
  _cmd += (byte) _zero           <<  4;
  _cmd += _filter_code           >>  8;
  
  return _cmd;
}


byte FilterLowCmd(int _filter_code)
{
  unsigned int code = 0;
  byte _cmd = 0;

  code = _filter_code << 8;
  _cmd = code >> 8;
  return _cmd;
}


byte ModeCmd(byte _mode, byte _gain, boolean _burnout, boolean _reset_filters)
{
  byte b = 0;

  b += _mode << 5;
  b += _gain << 3;
  b += _burnout << 1;
  b += _reset_filters;
  
  return b;
}


void InitADC(byte _c, byte _m, byte _g)
{
  u.Log(String(_c));
  u.Log("init channel");

  SPIBytes(CommCmd(reg_filter_high, _c, W),1,bytearr,0);
  SPIBytes(FilterHighCmd(setting_unipolar, 
                         setting_24bit, 
                         setting_current_boost, 
                         setting_zero, 
                         setting_filter_code),1,bytearr,0);
  u.Log("command:");
  u.Log(String(FilterHighCmd(setting_unipolar, 
                         setting_24bit, 
                         setting_current_boost, 
                         setting_zero, 
                         setting_filter_code)));
  SPIBytes(CommCmd(reg_filter_high, _c, R),1,bytearr,0);
  SPIBytes(0,1,bytearr,1);
  u.Log("check filter high reg");
  DumpBytes(1,bytearr);

  SPIBytes(CommCmd(reg_filter_low, _c, W),1,bytearr,0);
  SPIBytes(FilterLowCmd(setting_filter_code),1,bytearr,0);

  SPIBytes(CommCmd(reg_mode, _c, W),1,bytearr,0);
  SPIBytes(ModeCmd(_m, 
                   _g, 
                   setting_burnout, 
                   setting_reset_filters),1,bytearr,0);

  
  SPIBytes(CommCmd(reg_data, _c, R),1,bytearr,0);
  SPIBytes(0,3,bytearr,1);
  u.Log("data reg after init:");
  DumpBytes(3,bytearr);
  u.Log("channel init complete");
}


void RunCommand(char cmd)
{

  switch (cmd) {

    case 'm':
      break;

    case 't':
      break;

    case 'r':
      break;

    case 's':
      break;

    case 'n':
      break;

    default:
      u.Log("bad command");    
  } // switch

}


char GetCommand()
{
  String s;

  s = u.GetUserInput("Enter command ['m' for menu]");
  s.toLowerCase();

  return s[0];
}




void setup(void) {
  Serial.begin(115200);

  pinMode(ADCSelectPin, OUTPUT);
  digitalWrite(ADCSelectPin, HIGH);

  pinMode(ADCResetPin, OUTPUT);
  digitalWrite(ADCResetPin, HIGH);

  pinMode(ArduinoSSPin, OUTPUT);
  digitalWrite(ArduinoSSPin, HIGH);

  pinMode(MOSIPin, OUTPUT);
  digitalWrite(MOSIPin, HIGH);

  pinMode(MISOPin, INPUT);

  pinMode(SCLKPin, OUTPUT);
  digitalWrite(SCLKPin, LOW);

  // debug
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);

  SPI.begin();
  ResetADC();
  delay(3000);
  InitADC(ch_test, mode_cal_self, gain_1);
  




    /* settings */
  setting_unipolar       = 1;
  setting_24bit          = 1;
  setting_current_boost  = 1;
  setting_zero           = 0;
  setting_filter_code    = 20;  // [19, 4000]
  setting_burnout        = 0;
  setting_reset_filters  = 0;

  InitADC(ch_diff_1, mode_cal_self, gain_128);
  InitADC(ch_diff_1, mode_normal, gain_128);

  SPIBytes(CommCmd(reg_cal_full, ch_diff_1, R),1,bytearr,0);
  SPIBytes(0,3,bytearr,1);
  u.Log("full cal reg for ch1 diff");
  DumpBytes(3,bytearr);

  SPIBytes(CommCmd(reg_cal_zero, ch_diff_1, R),1,bytearr,0);
  SPIBytes(0,3,bytearr,1);
  u.Log("zero cal reg for ch1 diff");
  DumpBytes(3,bytearr);

  SPIBytes(CommCmd(reg_cal_full, ch_diff_2, R),1,bytearr,0);
  SPIBytes(0,3,bytearr,1);
  u.Log("full cal reg for ch2 diff");
  DumpBytes(3,bytearr);  //

  SPIBytes(CommCmd(reg_cal_zero, ch_diff_2, R),1,bytearr,0);
  SPIBytes(0,3,bytearr,1);
  u.Log("zero cal reg for ch2 diff");
  DumpBytes(3,bytearr);  // 31, 64, 0
  
  
  int i=0;  
  while (i<100)
  {
//    u.Log(String(analogRead(A0)));
//    u.Log(String(analogRead(A1)));

    delay(2200);

    SPIBytes(CommCmd(reg_data, ch_diff_1, R),1,bytearr,0);
    SPIBytes(0,3,bytearr,1);
    DumpBytes(3,bytearr);

    ++i;
  }

}



void loop(void) {
  GetCommand();
  delay(50);

}

