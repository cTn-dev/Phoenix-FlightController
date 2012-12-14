// PWM servo signal generation using Edge aligned PWM mode
unsigned int MotorOut[4] = {1000, 1000, 1000, 1000}; 

void setupFTM0() {
    // Flex timer0 configuration
    FTM0_SC = 0x0a;   // TOF=0 TOIE=0 CPWMS=0 CLKS=01 PS=100 (divide by 16)
    FTM0_MOD = 12000; // 4ms
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
    PORTC_PCR1 |= PORT_PCR_MUX(4); // 0x400
    PORTC_PCR2 |= PORT_PCR_MUX(4);
    PORTC_PCR3 |= PORT_PCR_MUX(4);
    PORTC_PCR4 |= PORT_PCR_MUX(4);    
}

void updateMotors() {
    cli(); // disable interrupts
    
    FTM0_C0V = MotorOut[0] * 3;
    FTM0_C1V = MotorOut[1] * 3;
    FTM0_C2V = MotorOut[2] * 3;
    FTM0_C3V = MotorOut[3] * 3;
    
    sei(); // enable interrupts
}