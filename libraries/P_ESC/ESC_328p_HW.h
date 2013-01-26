/*  ESC signal generation via shared timer1.

    MotorOuts array was added to separate variables changed by the FC in real time and PWM motor variables
    that are used inside the interrupt (without this, unpleasant behavior could occrur while system is writing to the 
    MotorOut array and timer triggers interrupt).
    
    Big thanks to kha from #aeroquad for setting up the shared timer.   
    
    This section is unfinished, default motor amount is still hardcoded to 4
*/

uint16_t MotorOuts[MOTORS];
uint8_t motorCounter = 0;
uint16_t motorTotal = 0;
uint8_t motorPins[MOTORS] = {2, 3, 4, 5};

// Comparison Interrupt Vector
ISR(TIMER1_COMPA_vect) { 
    if (motorCounter < MOTORS) {
        digitalWrite(motorPins[motorCounter], 0); // end previous channel
    }

    motorCounter = (motorCounter + 1) % (MOTORS + 1); // advance to next
 
    if (motorCounter < MOTORS) {
        unsigned int now = MotorOuts[motorCounter] * 2;
        
        if (now < 2000) now = 2000; // sanity
        if (now > 4000) now = 4000; // sanity
        
        motorTotal += now; // tally up time
        OCR1A = TCNT1 + now;  // interrupt 
        digitalWrite(motorPins[motorCounter],1); // start pulse
    } else { // motorCounter == MOTORS
        OCR1A = TCNT1 + MOTORS * 4000 + 50 - motorTotal;
        motorTotal = 0;
    } 
}

// Timer1 setup (shared timer for PPM sampling & PWM servo signal generation)
void setupTimer1Esc() {
    // Setup timer1 in normal mode, count at 2MHz
    TCCR1A = 0;
    TCCR1B = (1<<CS11)|(1<<ICES1);
    TIMSK1 |= (1<<ICIE1)|(1<<OCIE1A); // Enable ICP and OCRA interrupts
}

void updateMotors() {
    cli(); // disable interrupts
    
    MotorOuts[0] = MotorOut[0];
    MotorOuts[1] = MotorOut[1];
    MotorOuts[2] = MotorOut[2];
    MotorOuts[3] = MotorOut[3];   
    
    sei(); // enable interrupts
}

void initializeESC() {
    // We will also initialize the separate MotorOuts array values here
    for (uint8_t i = 0; i < MOTORS; i++) {
        MotorOuts[i] = MotorOut[i];
    }    
    
    // Standard timer initialization
    setupTimer1Esc();
}