/*  ESC signal generation via shared timer1.

    This port for atmega328p is UNTESTED.
*/

#define MOTORS 4
unsigned int MotorOut[4] = {1000, 1000, 1000, 1000};
unsigned int MotorOuts[4] = {1000, 1000, 1000, 1000};  
unsigned int motorCounter = 0;
unsigned int motorTotal = 0;
unsigned char motorPins[MOTORS] = {2, 3, 4, 5};

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
    } else { // motorCounter==MOTORS
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
    setupTimer1Esc();
}