/*
    Big thanks to kha from #aeroquad and Ragnorok from #arduino for helping me get this up and running.
*/

#define PPM_CHANNELS 8
volatile uint16_t startPulse = 0;
volatile uint16_t PPM[PPM_CHANNELS] = {1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000};
volatile uint16_t PPM_temp[PPM_CHANNELS] = {1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000};
volatile uint8_t ppmCounter = PPM_CHANNELS;
volatile uint16_t PPM_error = 0;

extern "C" void ftm1_isr(void) {
    // save current interrupt count/time
    uint16_t stopPulse = FTM1_C0V;
    
    // clear channel interrupt flag (CHF)
    FTM1_C0SC &= ~0x80;
    
    uint16_t pulseWidth = stopPulse - startPulse;

    if (pulseWidth < 2700 || (pulseWidth > 6100 && pulseWidth < 12000)) {
        PPM_error++;
        
        // set ppmCounter out of range so rest and (later on) whole frame is dropped
        ppmCounter = PPM_CHANNELS + 1;
    }
    
    if (pulseWidth > 12000) {  // Verify if this is the sync pulse (4ms >)
        if (ppmCounter == PPM_CHANNELS) {
            // This indicates that we received an correct frame = push to the "main" PPM array
            // if we received an broken frame, it will get ignored here and later get over-written
            // by new data, that will also be checked for sanity.
            
            for (uint8_t i = 0; i < PPM_CHANNELS; i++) {
                PPM[i] = PPM_temp[i];
            }
        }
        ppmCounter = 0; // restart the channel counter
    } else {
        if (ppmCounter < PPM_CHANNELS) {           // extra channels will get ignored here
            PPM_temp[ppmCounter] = pulseWidth / 3; // Store measured pulse length in us
            ppmCounter++;                          // Advance to next channel
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
