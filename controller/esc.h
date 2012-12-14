/*
    Big thanks to kha from #aeroquad for helping me get this up and running.
*/
    
// PWM servo signal generation using Edge aligned PWM mode (250hz)
unsigned int MotorOut[4] = {1000, 1000, 1000, 1000}; 

void setupFTM0() {
    // Flex timer0 configuration
    FTM0_SC = 0x0c;   // TOF=0 TOIE=0 CPWMS=0 CLKS=01 PS=100 (divide by 16)
    FTM0_MOD = 7500; // 4ms, 12000 = 250hz, 7500 = 400hz
    FTM0_C0SC = 0x28;
    
    // Initial values (3000 = 1ms)
    FTM0_C0V = 3000;
    FTM0_C1V = 3000;
    FTM0_C2V = 3000;
    FTM0_C3V = 3000;

    // PIN configuration
    // PIN name - ALT4 - Teensy 3.0 PIN
    // PTC1 - FTM0_CH0 - 22
    // PTC2 - FTM0_CH1 - 23
    // PTC3 - FTM0_CH2 - 9
    // PTC4 - FTM0_CH3 - 10
    
    // Using PORT_PCR_MUX(4) doesn't work in this case
    PORTC_PCR1 |= 0x400;
    PORTC_PCR2 |= 0x400;
    PORTC_PCR3 |= 0x400;
    PORTC_PCR4 |= 0x400;
}

void updateMotors() {
    FTM0_C0V = MotorOut[0] * 3;
    FTM0_C1V = MotorOut[1] * 3;
    FTM0_C2V = MotorOut[2] * 3;
    FTM0_C3V = MotorOut[3] * 3;
}