/*  PPM (pulse position modulation) sampling done in hardware via timer1.
    Sampling is done via digital pin 8.
    
    PPM buffering via PPM_temp has yet to be integrated
*/

#define PPM_CHANNELS 8
volatile uint16_t startPulse = 0;
volatile uint16_t PPM[PPM_CHANNELS] = {1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000};
//volatile uint16_t PPM_temp[PPM_CHANNELS] = {1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000};
volatile uint8_t ppmCounter = PPM_CHANNELS;
volatile uint16_t PPM_error = 0;

volatile uint8_t RX_signalReceived = 0;

#define TIMER1_FREQUENCY_HZ 50
#define TIMER1_PRESCALER    8
#define TIMER1_PERIOD       (F_CPU/TIMER1_PRESCALER/TIMER1_FREQUENCY_HZ)

// Capture Interrupt Vector
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
 
    // Set signal received flag to 0 every time we accept a valid frame
    RX_signalReceived = 0;
 
    startPulse = stopPulse; // Save time at pulse start
}

// Timer1 setup (shared timer for PPM sampling & PWM servo signal generation)
void setupTimer1RX() {
    // Setup timer1 in normal mode, count at 2MHz
    TCCR1A = 0;
    TCCR1B = (1<<CS11)|(1<<ICES1);
    TIMSK1 |= (1<<ICIE1)|(1<<OCIE1A); // Enable ICP and OCRA interrupts
}

void RX_failSafe() {
}

void initializeReceiver() {
    setupTimer1RX();
}