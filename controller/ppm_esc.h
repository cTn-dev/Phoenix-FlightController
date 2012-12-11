/*
#define TIMER1_FREQUENCY_HZ 50
#define TIMER1_PRESCALER    8
#define TIMER1_PERIOD       (F_CPU/TIMER1_PRESCALER/TIMER1_FREQUENCY_HZ)
*/

// controller definitions (sampling on nano [digital pin 8])
volatile unsigned int startPulse = 0;
#define PPM_CHANNELS 8
volatile int PPM[PPM_CHANNELS] = {1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000}; // ROLL,PITCH,THR,YAW...
volatile byte ppmCounter = PPM_CHANNELS;

#define MOTORS 4
unsigned int MotorOut[MOTORS] = {1000, 1000, 1000, 1000}; 
unsigned int motorCounter = 0;
unsigned int motorTotal = 0;
unsigned char motorPins[MOTORS] = {2, 3, 4, 5};

/*
// Capacitance Interrupt Vector
ISR(TIMER1_CAPT_vect) {
    unsigned int stopPulse = ICR1;
    unsigned int pulseWidth = stopPulse - startPulse;

    if (pulseWidth > 5000) {  // Verify if this is the sync pulse (2.5ms)
        ppmCounter = 0;       // restart the channel counter
    } else {
        if (ppmCounter < PPM_CHANNELS) {      // extra channels will get ignored here
            PPM[ppmCounter] = pulseWidth / 2; // Store measured pulse length in us
            ppmCounter++;                     // Advance to next channel
        }
    }
    
    startPulse = stopPulse; // Save time at pulse start
}

// Comparison Interrupt Vector
ISR(TIMER1_COMPA_vect) { 
    if (motorCounter < MOTORS) {
        digitalWrite(motorPins[motorCounter], 0); // end previous channel
    }

    motorCounter = (motorCounter + 1) % (MOTORS + 1); // advance to next
 
    if (motorCounter < MOTORS) {
        unsigned int now = MotorOut[motorCounter] * 2;
        
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
void setupTimer1() {
    // Setup timer1 in normal mode, count at 2MHz
    TCCR1A = 0;
    TCCR1B = (1<<CS11)|(1<<ICES1);
    TIMSK1 |= (1<<ICIE1)|(1<<OCIE1A); // Enable ICP and OCRA interrupts
}
*/