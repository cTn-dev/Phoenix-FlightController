/*
              FRONT
              CW (0)
                
                O
                |
                |
                |
  CCW (3) O-----#-----O CCW (1)
    LEFT        |       RIGHT
                |
                |
                O
                
              CW (2)
               BACK
*/

void updateMotorsMix() {
    // All of the motor outputs are constrained to standard 1000 - 2000 us PWM
    MotorOut[0] = constrain(throttle + PitchMotorSpeed + YawMotorSpeed, 1000, 2000);
    MotorOut[1] = constrain(throttle - RollMotorSpeed  - YawMotorSpeed, 1000, 2000);
    MotorOut[2] = constrain(throttle - PitchMotorSpeed + YawMotorSpeed, 1000, 2000);
    MotorOut[3] = constrain(throttle + RollMotorSpeed  - YawMotorSpeed, 1000, 2000); 
}