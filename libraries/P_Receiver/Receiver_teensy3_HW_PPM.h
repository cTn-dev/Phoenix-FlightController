/*  PPM (pulse position modulation) sampling done in hardware via FLEX timer.

    Signal sampling is being done via PORTA_PCR12 (PIN 3).

    We are using flex timer1 which supports only 2 channels.
    This code only utilizes single edge capture which is more then enough in terms of accuracy.

    Code below supports frame buffering, which is used to protect the controller from
    errors introduced (from simple noise to complete RX failure), if any of the channels in a single
    PPM frame triggered the sanity check, whole frame will be droped and PPM signal values will remain
    unchanged until a valid frame can be sampled.

    PPM_error variable will be increased every time a corrupted frame was detected.

    RX_signalReceived flag is set to 0 every time a valid frame is processed, this variables is used to
    "tell" software there was an error in communication (anything from a corrupted frame to a complete 
    receiver failure), this flag is increased by 1 every 10ms, if it reaches 100 (represnting 1 second)
    an subroutine is triggered that should handle this failure condition, from the most simple routines
    that will just disarm the craft (making it fall from the sky like a boiled potato) to more adnvaced 
    auto-descent routines.

    Please remember that this failsafe is here to prevent craft from "flying away on its own" and
    potentionaly harming someone in the proces (damaged craft from a crash is still better then cutting
    someones head off with uncontrollable craft).

    Big thanks to kha from #aeroquad and Ragnorok from #arduino for helping me get this up and running.
    
    TODO: Sync pulse length varies among receivers, should probably provide a "define" style table for defining the
    sync pulse length.
*/
#define RX_PPM_SYNCPULSE 7500 // 2.5ms

#define RX_CHANNELS 16
volatile uint16_t RX[RX_CHANNELS] = {1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000};
volatile uint16_t PPM_temp[RX_CHANNELS] = {1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000}; // Temporary buffer

volatile uint16_t startPulse = 0;
volatile uint8_t  ppmCounter = RX_CHANNELS;
volatile uint16_t PPM_error = 0;

volatile uint8_t RX_signalReceived = 0;

bool failsafeEnabled = false;

extern "C" void ftm1_isr(void) {
    // save current interrupt count/time
    uint16_t stopPulse = FTM1_C0V;
    
    // clear channel interrupt flag (CHF)
    FTM1_C0SC &= ~0x80;
    
    uint16_t pulseWidth = stopPulse - startPulse;

    // Error / Sanity check
    // if pulseWidth < 900us or pulseWidth > 2100us and pulseWidth < RX_PPM_SYNCPULSE
    if (pulseWidth < 2700 || (pulseWidth > 6300 && pulseWidth < RX_PPM_SYNCPULSE)) {
        PPM_error++;
        
        // set ppmCounter out of range so rest and (later on) whole frame is dropped
        ppmCounter = RX_CHANNELS + 1;
    }
    
    if (pulseWidth >= RX_PPM_SYNCPULSE) {  // Verify if this is the sync pulse
        if (ppmCounter <= RX_CHANNELS) {
            // This indicates that we received an correct frame = push to the "main" PPM array
            // if we received an broken frame, it will get ignored here and later get over-written
            // by new data, that will also be checked for sanity.
            
            for (uint8_t i = 0; i < RX_CHANNELS; i++) {
                RX[i] = PPM_temp[i];             
            }
            
            // Bring failsafe flag down every time we accept a valid signal / frame
            RX_signalReceived = 0;
        }
        
        // restart the channel counter
        ppmCounter = 0;
    } else {
        if (ppmCounter < RX_CHANNELS) {            // extra channels will get ignored here
            PPM_temp[ppmCounter] = pulseWidth / 3; // Store measured pulse length in us
            ppmCounter++;                          // Advance to next channel
        }
    }
    
    startPulse = stopPulse; // Save time at pulse start    
}

void setupFTM1() {
    // FLEX Timer1 input filter configuration
    // 4+4×val clock cycles, 48MHz = 4+4*7 = 32 clock cycles = 0.75us
    FTM1_FILTER = 0x07;
    
    // FLEX Timer1 configuration
    FTM1_SC = 0x0C;    // TOF=0 TOIE=0 CPWMS=0 CLKS=01 (system clock) PS=100 (divide by 16)
    FTM1_MOD = 0xFFFF; // modulo to max
    FTM1_C0SC = 0x44;  // CHF=0 CHIE=1 MSB=0 MSA=0 ELSB=0 ELSA=1 DMA=0

    // Enable FTM1 interrupt inside NVIC
    NVIC_ENABLE_IRQ(IRQ_FTM1);
    
    // PIN configuration, alternative function 3
    PORTA_PCR12 |= 0x300;
}

void initializeReceiver() {
    setupFTM1();
}

void RX_failSafe() {
    RX_signalReceived++; // if this flag reaches 10, an auto-descent routine will be triggered.
    
    if (RX_signalReceived > 10) {
        RX_signalReceived = 10; // don't let the variable overflow
        failsafeEnabled = true; // this ensures that failsafe will operate in attitude mode
        
        // Bear in mind that this is here just to "slow" the fall, if you have lets say 500m altitude,
        // this probably won't help you much (sorry).
        // This will slowly (-2 every 100ms) bring the throttle to 1000 (still saved in the PPM array)
        // 1000 = 0 throttle;
        // Descending from FULL throttle 2000 (most unlikely) would take about 1 minute and 40 seconds
        // Descending from HALF throttle 1500 (more likely) would take about 50 seconds
        RX[CONFIG.data.CHANNEL_ASSIGNMENT[THROTTLE]] -= 2;
        
        if (RX[CONFIG.data.CHANNEL_ASSIGNMENT[THROTTLE]] < 1000) {
            RX[CONFIG.data.CHANNEL_ASSIGNMENT[THROTTLE]] = 1000; // don't let the value fall below 1000
        }    
    } else {
        failsafeEnabled = false;
    }
}