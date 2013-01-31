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
    double PID_YAW_command[4];
    double PID_PITCH_command[4];
    double PID_ROLL_command[4];
    
    // Rate
    double PID_YAW_motor[4];
    double PID_PITCH_motor[4];
    double PID_ROLL_motor[4];    
    
    #ifdef AltitudeHoldBaro   
        double PID_BARO[4];
    #endif
    
    #ifdef AltitudeHoldSonar
        double PID_SONAR[4];
    #endif
};

union CONFIG_union {
    struct CONFIG_struct data;
    uint8_t raw[sizeof(data)];
} CONFIG;


void writeEEPROM() {
    for (uint16_t i = 0; i < sizeof(CONFIG_struct); i++) {
        if (CONFIG.raw[i] != EEPROM.read(i)) {
            // Only re-write new data
            // blocks containing the same value will be left alone (cool huh?)
            EEPROM.write(i, CONFIG.raw[i]);
        }    
    }
}

void initializeEEPROM() {
    // Default settings should be initialized here
    CONFIG.data.version = 1;
    CONFIG.data.calibrateESC = 0;
    
    // Attitude
    CONFIG.data.PID_YAW_command[P]  = 4.0;
    CONFIG.data.PID_YAW_command[I]  = 0.0;
    CONFIG.data.PID_YAW_command[D]  = 0.0;
    CONFIG.data.PID_YAW_command[WG] = 25.0;
    
    CONFIG.data.PID_PITCH_command[P]  = 4.0;
    CONFIG.data.PID_PITCH_command[I]  = 0.0;
    CONFIG.data.PID_PITCH_command[D]  = 0.0;
    CONFIG.data.PID_PITCH_command[WG] = 25.0;

    CONFIG.data.PID_ROLL_command[P]  = 4.0;
    CONFIG.data.PID_ROLL_command[I]  = 0.0;
    CONFIG.data.PID_ROLL_command[D]  = 0.0;
    CONFIG.data.PID_ROLL_command[WG] = 25.0;

    // Rate
    CONFIG.data.PID_YAW_motor[P]  = 200.0;
    CONFIG.data.PID_YAW_motor[I]  = 5.0;
    CONFIG.data.PID_YAW_motor[D]  = 0.0;
    CONFIG.data.PID_YAW_motor[WG] = 1000.0;
    
    CONFIG.data.PID_PITCH_motor[P]  = 80.0;
    CONFIG.data.PID_PITCH_motor[I]  = 0.0;
    CONFIG.data.PID_PITCH_motor[D]  = -3.0;
    CONFIG.data.PID_PITCH_motor[WG] = 1000.0;

    CONFIG.data.PID_ROLL_motor[P]  = 80.0;
    CONFIG.data.PID_ROLL_motor[I]  = 0.0;
    CONFIG.data.PID_ROLL_motor[D]  = -3.0;
    CONFIG.data.PID_ROLL_motor[WG] = 1000.0;    

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
    
    writeEEPROM();
}

void readEEPROM() {
    if (EEPROM.read(0) == 255) {
        // No EEPROM values detected, re-initialize
        initializeEEPROM();
    }

    for (uint16_t i = 0; i < sizeof(CONFIG_struct); i++) {
        CONFIG.raw[i] = EEPROM.read(i);
    }
}