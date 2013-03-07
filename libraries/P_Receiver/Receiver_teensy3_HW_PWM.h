/*  PWM (Pulse Width Modulation) sampling done in hardware via pin interrupts and PIT timer.
    
    I am using one, out of 4 PIT timers build into the Teensy 3.0.
    
    Pins used for PWM signal capture are fully definable by the user.
    However, i do recommend avoiding FLEX timer enabled pins, (Teensy 3.0 pin numbering) 5, 6, 9, 10, 20, 21, 22, 23,
    as those are used for PWM signal generation (for Electronic Speed Controllers).
    
    Using the hardware timer for timing / counting the pulses gives us superior accuracy of ~21 nano seconds.
    
    This receiver code also utilizes a more advanced failsafe sequence.
    In case of receiver malfunction / signal cable damage, RX Failasefe will kicks in
    which will start auto-descent sequence.
    
*/

#define CHANNELS 8
uint8_t PWM_PINS[CHANNELS] = {2, 3, 4, 5, 6, 7, 8, 9};

volatile uint16_t RX[CHANNELS] = {1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000};
volatile uint32_t PWM_time[CHANNELS] = {0, 0, 0, 0, 0, 0, 0, 0};

volatile uint16_t RX_failsafeStatus;
volatile uint8_t RX_signalReceived = 0;

void readPWM(uint8_t channel) {
    uint32_t now = PIT_CVAL0; // Current counter value
    uint32_t delta = PWM_time[channel] - now; // Delta (calculated in reverse, because PIT is downcounting timer)
    
    // All of the number below are scaled to use with Teensy 3.0 running at 48Mhz
    if (delta < 120000 && delta > 43200) { // This is a valid pulse
        RX[channel] = delta / 48;
        
        // Bring failsafe status flag for current channel down every time we accept a valid signal
        RX_failsafeStatus &= ~(1 << channel);
    } else { // Beginning of the pulse
        PWM_time[channel] = now;
    }
}

// ISRs
void PWM_ISR_0() {
    readPWM(0);
}
void PWM_ISR_1() {
    readPWM(1);
}
void PWM_ISR_2() {
    readPWM(2);
}
void PWM_ISR_3() {
    readPWM(3);
}
void PWM_ISR_4() {
    readPWM(4);
}
void PWM_ISR_5() {
    readPWM(5);
}
void PWM_ISR_6() {
    readPWM(6);
}
void PWM_ISR_7() {
    readPWM(7);
}

void (*PWM_Handlers [])(void) = {
    PWM_ISR_0, 
    PWM_ISR_1, 
    PWM_ISR_2, 
    PWM_ISR_3, 
    PWM_ISR_4, 
    PWM_ISR_5, 
    PWM_ISR_6, 
    PWM_ISR_7
};

void initializeReceiver() {
    // initialize PIT timer (teensy running at 48000000)
    PIT_MCR = 0x00;          // Turn on PIT
    PIT_LDVAL0 = 0xFFFFFFFF; // Load initial value of 4294967295
    PIT_TCTRL0 = 0x01;       // Start the counter
    
    for (uint8_t i = 0; i < CHANNELS; i++) {
        pinMode(PWM_PINS[i], INPUT);
        attachInterrupt(PWM_PINS[i], PWM_Handlers[i], CHANGE);
    }
}

void RX_failSafe() {
    if (RX_failsafeStatus > 0) {
        RX_signalReceived++; // if this flag reaches 10, an auto-descent routine will be triggered.
    } else {
        // Raise the FLAGS
        RX_failsafeStatus |= (1 << CHANNELS) - 1;
        
        // Reset the counter
        RX_signalReceived = 0;
    }
    
    if (RX_signalReceived > 10) {
        RX_signalReceived = 10; // don't let the variable overflow
        
        // Bear in mind that this is here just to "slow" the fall, if you have lets say 500m altitude,
        // this probably won't help you much (sorry).
        // This will slowly (-2 every 100ms) bring the throttle to 1000 (still saved in the PPM array)
        // 1000 = 0 throttle;
        // Descending from FULL throttle 2000 (most unlikely) would take about 1 minute and 40 seconds
        // Descending from HALF throttle 1500 (more likely) would take about 50 seconds
        RX[2] -= 2;
        RX[4] = 2000; // force attitude mode
        
        if (RX[2] < 1000) {
            RX[2] = 1000; // don't let the value fall below 1000
            
            // at this point, we will also disarm
            armed = false;
        }    
    }
}