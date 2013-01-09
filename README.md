Flight Controller based on K20DX128 (Teensy 3.0)
=================================================

I2C Setup
---------
This is required for getting higher data reading speeds (mostly for gyro and accel)
  - In Arduino/libraries/Wire/Wire.cpp change I2C0_F = 0x27; to I2C0_F = 0x00;
  - I also added the modified Wire library to the repository
  
This will set the I2C Bus Speed to FM (Fast-mode), reaching speeds up to 2.4Mbit/s
By having faster I2C Bus to work with, we get a "spare" time between sensor reads
which will give us quite a bit of spare computing time that we can utilize elsewhere
(6 registers for gyro + 6 registers for) = 12 bytes = 96 bits 
at 1Khz update rate that is 1000 * 96 bits = 96 000 bits/s + [protocol overhead]

PIN setup (Teensy 3.0 pin numbering)
------------------------------------
  - I2C SCL 19
  - I2C SDA 18
  
  - PPM in (receiver) 3
  
  - Rotor 1 20
  - Rotor 2 21
  - Rotor 3 22
  - Rotor 4 23
  
  - Orientation lights / Armed-Disarmed indicator 14
  
Filters, kinematics, data handling
----------------------------------
  - Initial raw data from sensors (read every 1ms = 1000Hz) is being averaged by a simple averaging filter
  - Averaged data from sensors are being processed every 10ms (100Hz)
  - By default i am using my own complementary kinematics algorythm
  - ARG kinematics (from aeroquad) is also supported (can be enabled by simple include change)
  - Flight controller supports 2 modes
    - Rate | (ACRO) gyro only
    - Attitude | gyro with accel corrections
  - Pilot commands are being read by PPM sampling code via single PIN (with HW timer), this is the only pilot input supported for now
  - Stabilization and pilot commands are mixed together by 2 separate PID controllers
    - First (only used in attitude mode) mixes pilot commands with kinematics output
    - Second (used in both attitude and rate mode) mixes output from first PID or raw stick input with gyroRate output
  - For ESC signal output i am using an build in 8 channel FLEX timer (yes you can controll octocopter with this)
    - ESC PWM signal supports both 250Hz and 400Hz update rate (running at 400Hz by default)    