/*
Table V. Communications Register

0      1   2   3   4   5   6   7
0/DRDY RS2 RS1 RS0 R/W CH2 CH1 CH0

0/DRDY   For a write operation, a 0 must be written to this bit; status is the same as the DRDY output pin

RS2–RS0 Register Selection Bits. select registers for next read or write operation

RS2 RS1 RS0   Register                          Size
______________________________________________________________________
0   0   0     Communications Register           8 Bits
0   0   1     Mode Register                     8 Bits
0   1   0     Filter High Register              8 Bits
0   1   1     Filter Low Register               8 Bits
1   0   0     Test Register                     8 Bits
1   0   1     Data Register                     16 Bits or 24 Bits
1   1   0     Zero-Scale Calibration Register   24 Bits
1   1   1     Full-Scale Calibration Register   24 Bits

CH2–CH0 Channel Select. These three bits select a channel either for conversion or for access to calibration coefficients.
Power-On or RESET status of these bits is 1,0,0 selecting the differential pair AIN1 and AIN2.

CH2 CH1 CH0   AIN(+) AIN(–)   Type       Calibration Register Pair
______________________________________________________________________
0 0 0         AIN1   AIN6     Pseudo     Pair 0
0 0 1         AIN2   AIN6     Pseudo     Pair 1
0 1 0         AIN3   AIN6     Pseudo     Pair 2
0 1 1         AIN4   AIN6     Pseudo     Pair 2
1 0 0         AIN1   AIN2     Fully      Pair 0
1 0 1         AIN3   AIN4     Fully      Pair 1
1 1 0         AIN5   AIN6     Fully      Pair 2
1 1 1         AIN6   AIN6     Test       Pair 2


Mode Register (RS2-RS0 = 0, 0, 1); Power On/Reset Status: 00␣ Hex
The Mode Register is an eight bit register from which data can either be read or to which data can be written. Table VIII outlines the
bit designations for the Mode Register.

Table VIII. Mode Register

0   1   2   3   4   5   6   7
MD2 MD1 MD0 G2  G1  G0  BO  FSYNC

MD2 MD1 MD0   Operating Mode
______________________________________________________________________
0   0   0     Normal
0   0   1     Self-Calibration;
0   1   0     Zero-Scale System Calibration
0   1   1     Full-Scale System Calibration
1   0   0     System-Offset Calibration
1   0   1     Background Calibration
1   1   0     Zero-Scale Self-Calibration    
1   1   1     Full-Scale Self-Calibration

G2 G1 G0      Gain Setting
______________________________________________________________________
0  0  0       1
0  0  1       2
0  1  0       4
0  1  1       8
1  0  0       16
1  0  1       32
1  1  0       64
1  1  1       128

BO  Burnout Current. A 0 in this bit turns off the on-chip burnout currents. This is the default (Power-On
or RESET) status of this bit. A 1 in this bit activates the burnout currents. When active, the burnout
currents connect to the selected analog input pair, one to the AIN(+) input and one to the AIN(–) input

FSYNC  Filter Synchronization. When this bit is high, the nodes of the digital filter, the filter control logic and
the calibration control logic are held in a reset state and the analog modulator is also held in its reset
state. When this bit goes low, the modulator and filter start to process data and a valid word is available
in 3 × 1/(output update rate), i.e., the settling time of the filter. This FSYNC bit does not affect the
digital interface and does not reset the DRDY output if it is low.


Filter Registers. Power On/Reset Status: Filter High Register: 01␣ Hex. Filter Low Register: 40␣ Hex.

There are two 8-bit Filter Registers on the AD7714 from which data can either be read or to which data can be written. Tables IX
and X outline the bit designations for the Filter Registers.

Table IX. Filter High Register (RS2–RS0 = 0, 1, 0)

0   1  2   3    4    5    6   7
B/U WL BST ZERO FS11 FS10 FS9 FS8

Table X. Filter Low Register (RS2–RS0 = 0, 1, 1)
0   1   2   3   4   5   6   7
FS7 FS6 FS5 FS4 FS3 FS2 FS1 FS0

B/U  Bipolar/Unipolar Operation. A 0 in this bit selects Bipolar Operation. This is the default (Power-On or RESET)
status of this bit. A 1 in this bit selects unipolar operation.

WL  Word Length
BST  Current Boost
ZERO   To ensure correct operation of the A Versions of the part, a 0 must be written to this bit.
FS11–FS0  Filter Selection. The on-chip digital filter provides a Sinc3 (or (Sinx/x)3 ) filter response. The 12 bits of data
programmed into these bits determine the filter cut-off frequency, the position of the first notch of the filter and
the data rate for the part. In association with the gain selection, it also determines the output noise (and hence
the effective resolution) of the device.

The first notch of the filter occurs at a frequency determined by the relationship:
filter first notch frequency =␣ (fCLK␣ IN/128)/code
where code is the decimal equivalent of the code in bits FS0 to FS11 and is in the range 19 to 4,000. With the
nominal fCLK IN of 2.4576␣ MHz, this results in a first notch frequency range from 4.8␣ Hz to 1.01␣ kHz. To
ensure correct operation of the AD7714, the value of the code loaded to these bits must be within this range.
Failure to do this will result in unspecified operation of the device
*/




                               // b'/u  wl  bst  zero  fs11-8 // fs11-0
#define ADC_FILTER_HIGH 0x00    // 0     0    0   0     0000   // 0x014 [20]
                                // fs7-0
#define ADC_FILTER_LOW  0x14    // 0001 0100


