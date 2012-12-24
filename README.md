Flight Controller based on MK20DX128 (Teensy 3.0)
=================================================

I2C Setup
---------
This is required for getting higher data reading speeds (mostly for gyro and accel)
  - In Arduino/libraries/Wire/Wire.cpp change I2C0_F = 0x27; to I2C0_F = 0x0D;
This will set the I2C Bus Speed to FM (Fast-mode), reaching speeds up to 1Mbit/s, 
which is exactly how much we need (6 registers for gyro + 6 registers for) = 12 bytes = 96 bits
at 1Khz update rate that is 1000 * 96 bits = 96 000 bits/s

PIN setup (Teensy 3.0 pin numbering)
------------------------------------
  - I2C SCL 19
  - I2C SDA 18
  - PPM in 3
  - Rotor 1 22
  - Rotor 2 23
  - Rotor 3 9
  - Rotor 4 10
  - Orientation lights 14
  - Battery monitor 15