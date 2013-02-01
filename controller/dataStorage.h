#define P  0 // Proportional
#define I  1 // Integral
#define D  2 // Derivative
#define WG 3 // WindupGuard

#if defined(__MK20DX128__)
    #define EEPROM_SIZE 512
#endif    

struct CONFIG_struct {
    uint8_t version;
    bool calibrateESC;
    
    // Attitude
    double PID_YAW_c[4];
    double PID_PITCH_c[4];
    double PID_ROLL_c[4];
    
    // Rate
    double PID_YAW_m[4];
    double PID_PITCH_m[4];
    double PID_ROLL_m[4];    
    
    #ifdef AltitudeHoldBaro
        double PID_BARO[4];
    #endif
    
    #ifdef AltitudeHoldSonar
        double PID_SONAR[4];
    #endif    

	#ifdef Accelerometer
        int ACCEL_BIAS[3];
	#endif
};

union CONFIG_union {
    struct CONFIG_struct data;
    uint8_t raw[sizeof(data)];
};

CONFIG_union CONFIG;

void initializeEEPROM() {
    // Default settings should be initialized here
    CONFIG.data.version = 1;
    CONFIG.data.calibrateESC = 0;
    
    // Attitude
    CONFIG.data.PID_YAW_c[P]  = 4.0;
    CONFIG.data.PID_YAW_c[I]  = 0.0;
    CONFIG.data.PID_YAW_c[D]  = 0.0;
    CONFIG.data.PID_YAW_c[WG] = 25.0;
    
    CONFIG.data.PID_PITCH_c[P]  = 4.0;
    CONFIG.data.PID_PITCH_c[I]  = 0.0;
    CONFIG.data.PID_PITCH_c[D]  = 0.0;
    CONFIG.data.PID_PITCH_c[WG] = 25.0;

    CONFIG.data.PID_ROLL_c[P]  = 4.0;
    CONFIG.data.PID_ROLL_c[I]  = 0.0;
    CONFIG.data.PID_ROLL_c[D]  = 0.0;
    CONFIG.data.PID_ROLL_c[WG] = 25.0;

    // Rate
    CONFIG.data.PID_YAW_m[P]  = 200.0;
    CONFIG.data.PID_YAW_m[I]  = 5.0;
    CONFIG.data.PID_YAW_m[D]  = 0.0;
    CONFIG.data.PID_YAW_m[WG] = 1000.0;
    
    CONFIG.data.PID_PITCH_m[P]  = 80.0;
    CONFIG.data.PID_PITCH_m[I]  = 0.0;
    CONFIG.data.PID_PITCH_m[D]  = -3.0;
    CONFIG.data.PID_PITCH_m[WG] = 1000.0;

    CONFIG.data.PID_ROLL_m[P]  = 80.0;
    CONFIG.data.PID_ROLL_m[I]  = 0.0;
    CONFIG.data.PID_ROLL_m[D]  = -3.0;
    CONFIG.data.PID_ROLL_m[WG] = 1000.0;    
 
    #ifdef AltitudeHoldBaro
        CONFIG.data.PID_BARO[P]  = 25.0;
        CONFIG.data.PID_BARO[I]  = 0.6;
        CONFIG.data.PID_BARO[D]  = -10.0;
        CONFIG.data.PID_BARO[WG] = 25.0;    
    #endif    
    
    #ifdef AltitudeHoldSonar
        CONFIG.data.PID_SONAR[P]  = 60.0;
        CONFIG.data.PID_SONAR[I]  = 0.6;
        CONFIG.data.PID_SONAR[D]  = -10.0;
        CONFIG.data.PID_SONAR[WG] = 25.0;    
    #endif

	#ifdef Accelerometer
        CONFIG.data.ACCEL_BIAS[0] = -425;
        CONFIG.data.ACCEL_BIAS[1] = 260;
        CONFIG.data.ACCEL_BIAS[2] = 400;
	#endif
    
    // This function will only initialize data "locally"
    // writeEEPROM() have to be called manually to store this data in EEPROM
}

void writeEEPROM() {
    for (uint16_t i = 0; i < sizeof(CONFIG_struct); i++) {
        if (CONFIG.raw[i] != EEPROM.read(i)) {
            // Only re-write new data
            // blocks containing the same value will be left alone (cool huh?)
            EEPROM.write(i, CONFIG.raw[i]);
        }    
    }
}

void readEEPROM() {
    if (EEPROM.read(0) == 255) {
        // No EEPROM values detected, automatic re-initialize
        initializeEEPROM();
    } else {
        // There "is" data in the EEPROM, read it all
        for (uint16_t i = 0; i < sizeof(CONFIG_struct); i++) {
            CONFIG.raw[i] = EEPROM.read(i);
        }
    }
}
