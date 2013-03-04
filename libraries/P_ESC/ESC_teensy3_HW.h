/*  ESC / Servo signal generation done in hardware by FLEX timer, without ISR (yay!)

    We are using flex timer0 which supports 8 channels.

    Currently generating 400 Hz PWM signal that is fed into electronic speed controllers
    corresponding to each rotor.

    This code will probably be expanded to also generate servo signal for
    gimbal stabilization (hopefully in near future).

    Big thanks to kha from #aeroquad for helping me get this up and running.
   
    == PIN Configuration ==
   
    Channel - PIN name - Teensy 3.0 PIN numbering

    FTM0_CH0 - PTC1 - 22
    FTM0_CH1 - PTC2 - 23
    FTM0_CH2 - PTC3 - 9
    FTM0_CH3 - PTC4 - 10
    FTM0_CH4 - PTD4 - 6
    FTM0_CH5 - PTD5 - 20
    FTM0_CH6 - PTD6 - 21
    FTM0_CH7 - PTD7 - 5   
*/

void setupFTM0() {
    // Flex timer0 configuration
    FTM0_SC = 0x0c;   // TOF=0 TOIE=0 CPWMS=0 CLKS=01 PS=100 (divide by 16)
    
    #ifdef ESC_400HZ
        // 400Hz PWM signal
        FTM0_MOD = 7500;
    #else
        // 250Hz PWM signal
        FTM0_MOD = 12000;
    #endif
    
    FTM0_C0SC = 0x28;
    
    // Initial values (3000 = 1ms)
    #if MOTORS == 3
        FTM0_C0V = 3000;
        FTM0_C1V = 3000;    
        FTM0_C2V = 3000;  

        // PORT Configuration
        PORTC_PCR1 |= 0x400;
        PORTC_PCR2 |= 0x400; 
        PORTC_PCR3 |= 0x400;        
    #elif MOTORS == 4
        FTM0_C0V = 3000;
        FTM0_C1V = 3000;    
        FTM0_C2V = 3000;
        FTM0_C3V = 3000; 

        // PORT Configuration
        PORTC_PCR1 |= 0x400;
        PORTC_PCR2 |= 0x400; 
        PORTC_PCR3 |= 0x400;
        PORTC_PCR4 |= 0x400;        
    #elif MOTORS == 6
        FTM0_C0V = 3000;
        FTM0_C1V = 3000;    
        FTM0_C2V = 3000;
        FTM0_C3V = 3000; 
        FTM0_C4V = 3000;
        FTM0_C5V = 3000;

        // PORT Configuration
        PORTC_PCR1 |= 0x400;
        PORTC_PCR2 |= 0x400; 
        PORTC_PCR3 |= 0x400;
        PORTC_PCR4 |= 0x400;
        PORTD_PCR4 |= 0x400;
        PORTD_PCR5 |= 0x400;        
    #elif MOTORS == 8
        FTM0_C0V = 3000;
        FTM0_C1V = 3000;    
        FTM0_C2V = 3000;
        FTM0_C3V = 3000; 
        FTM0_C4V = 3000;
        FTM0_C5V = 3000;
        FTM0_C6V = 3000;
        FTM0_C7V = 3000;        

        // PORT Configuration
        PORTC_PCR1 |= 0x400;
        PORTC_PCR2 |= 0x400; 
        PORTC_PCR3 |= 0x400;
        PORTC_PCR4 |= 0x400;
        PORTD_PCR4 |= 0x400;
        PORTD_PCR5 |= 0x400;    
        PORTD_PCR6 |= 0x400;
        PORTD_PCR7 |= 0x400;        
    #endif
}

void updateMotors() {
    #if MOTORS == 3
        FTM0_C0V = MotorOut[0] * 3;
        FTM0_C1V = MotorOut[1] * 3;
        FTM0_C2V = MotorOut[2] * 3;    
    #elif MOTORS == 4
        FTM0_C0V = MotorOut[0] * 3;
        FTM0_C1V = MotorOut[1] * 3;
        FTM0_C2V = MotorOut[2] * 3;
        FTM0_C3V = MotorOut[3] * 3;
    #elif MOTORS == 6
        FTM0_C0V = MotorOut[0] * 3;
        FTM0_C1V = MotorOut[1] * 3;
        FTM0_C2V = MotorOut[2] * 3;
        FTM0_C3V = MotorOut[3] * 3; 
        FTM0_C4V = MotorOut[4] * 3; 
        FTM0_C5V = MotorOut[5] * 3;         
    #elif MOTORS == 8
        FTM0_C0V = MotorOut[0] * 3;
        FTM0_C1V = MotorOut[1] * 3;
        FTM0_C2V = MotorOut[2] * 3;
        FTM0_C3V = MotorOut[3] * 3; 
        FTM0_C4V = MotorOut[4] * 3; 
        FTM0_C5V = MotorOut[5] * 3;    
        FTM0_C6V = MotorOut[6] * 3;  
        FTM0_C7V = MotorOut[7] * 3;          
    #endif
}

void initializeESC() {
    setupFTM0();
    
    if (CONFIG.data.calibrateESC) {
        // Calibration sequence requested
        
        // Signal range TOP maximum
        for (uint8_t motor = 0; motor <= MOTORS; motor++) {
            MotorOut[motor] = 2000;
        }
        updateMotors();
        
        // Wait for all ESCs to acknowledge (1 beep)
        delay(5000);
        
        // Signal range BOTTOM minimum
        for (uint8_t motor = 0; motor <= MOTORS; motor++) {
            MotorOut[motor] = 1000;
        }
        updateMotors();     

        // Wait for all ESCs to acknowledge (2 + 1 beep)
        delay(4000);        
        
        // Calibration done
        // disabling the calibration flag and updating EEPROM
        CONFIG.data.calibrateESC = 0;
        writeEEPROM();
    }
}