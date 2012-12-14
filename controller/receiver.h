/*
    Big thanks to kha from #aeroquad and Ragnorok from #arduino for helping me get this up and running.
*/

// Arduino standard library imports
#include <Arduino.h>

volatile uint16_t startPulse = 0;
#define PPM_CHANNELS 8
volatile int PPM[PPM_CHANNELS] = {1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000};
volatile byte ppmCounter = PPM_CHANNELS;
volatile uint16_t PPM_error = 0;

extern "C" void ftm1_isr(void) {
    // save current interrupt count/time
    uint16_t stopPulse = FTM1_C0V;
    
    // clear channel interrupt flag (CHF)
    FTM1_C0SC &= ~0x80;
    
    uint16_t pulseWidth = stopPulse - startPulse;

    if (pulseWidth < 2700 || (pulseWidth > 6100 && pulseWidth < 12000)) {
        PPM_error++;
    }
    
    if (pulseWidth > 7500) {  // Verify if this is the sync pulse (2.5ms)
        ppmCounter = 0;       // restart the channel counter
    } else {
        if (ppmCounter < PPM_CHANNELS) {      // extra channels will get ignored here
            PPM[ppmCounter] = pulseWidth / 3; // Store measured pulse length in us
            ppmCounter++;                     // Advance to next channel
        }
    }
    
    startPulse = stopPulse; // Save time at pulse start    
}

void setupFTM1() {
    // FLEX Timer1 input filter configuration
    // 4+4×val clock cycles, 48MHz = 4+4*7 = 32 clock cycles = 0.75us
    FTM1_FILTER = 0x7;
    
    // FLEX Timer1 configuration
    FTM1_SC = 0x0c;    // TOF=0 TOIE=0 CPWMS=0 CLKS=01 (system clock) PS=100 (divide by 16)
    FTM1_MOD = 0xffff; // modulo to max
    FTM1_C0SC = 0x44;  // CHF=0 CHIE=1 MSB=0 MSA=0 ELSB=0 ELSA=1 DMA=0

    // enable interrupt in NVIC
    NVIC_ENABLE_IRQ(IRQ_FTM1);
    
    // PIN configuration (teensy 3.0 pin3 = PTA12)
    // we are using pin alternative function 3 
    // PORT_PCR_MUX(3) works in this case, but we will set it manually anyway
    PORTA_PCR12 |= 0x300; // 0x300
}
