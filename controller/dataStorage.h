/*  EEPROM based configuration data storage structure

    Whole flight controller configuration is being stored inside an CONFIG structure
    that can be accesed as data union (which is required for easy manipulation with
    the configuration data over serial and easy way of storing this data in EEPROM).
    
    This method allow us to do "smart" memory checking with the new data against
    data stored in EEPROM, which means we doesn't have to re-write whole configuration
    union inside the EEPROM, but we just modify bytes that changed.
    
    This will protect EEPROM from unnecessary writes, extending its lifetime
    (because EEPROM writes are limited, actual number of writes depends on the chip).
*/

#if defined(__MK20DX128__)
    #define EEPROM_SIZE 512
#endif    

struct __attribute__((packed)) CONFIG_struct {
    uint8_t version;
    bool calibrateESC;
    uint16_t minimumArmedThrottle;
    
    // Accelerometer
    int16_t ACCEL_BIAS[3];    
    
    // RX channel assignment
    uint8_t CHANNEL_ASSIGNMENT[8];
    
    // Attitude
    float PID_YAW_c[4];
    float PID_PITCH_c[4];
    float PID_ROLL_c[4];
    
    // Rate
    float PID_YAW_m[4];
    float PID_PITCH_m[4];
    float PID_ROLL_m[4];    
    
    float PID_BARO[4];
    float PID_SONAR[4]; 
    
    // GPS
    float PID_GPS[4];
};

union CONFIG_union {
    struct CONFIG_struct data;
    uint8_t raw[sizeof(CONFIG_struct)];
};

CONFIG_union CONFIG;

void initializeEEPROM() {
    // Default settings should be initialized here
    CONFIG.data.version = 2; // configuration union version (used to check against configurator)
    CONFIG.data.calibrateESC = 0;
    CONFIG.data.minimumArmedThrottle = 1100;
    
    // Accelerometer
    CONFIG.data.ACCEL_BIAS[XAXIS] = 0;
    CONFIG.data.ACCEL_BIAS[YAXIS] = 0;
    CONFIG.data.ACCEL_BIAS[ZAXIS] = 0;
    
    // RX channel assignment
    CONFIG.data.CHANNEL_ASSIGNMENT[0] = 0;
    CONFIG.data.CHANNEL_ASSIGNMENT[1] = 1;
    CONFIG.data.CHANNEL_ASSIGNMENT[2] = 2;
    CONFIG.data.CHANNEL_ASSIGNMENT[3] = 3;
    CONFIG.data.CHANNEL_ASSIGNMENT[4] = 4;
    CONFIG.data.CHANNEL_ASSIGNMENT[5] = 5;
    CONFIG.data.CHANNEL_ASSIGNMENT[6] = 6;
    CONFIG.data.CHANNEL_ASSIGNMENT[7] = 7;
    
    // Altitude
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
    CONFIG.data.PID_YAW_m[WG] = 100.0;
    
    CONFIG.data.PID_PITCH_m[P]  = 80.0;
    CONFIG.data.PID_PITCH_m[I]  = 0.0;
    CONFIG.data.PID_PITCH_m[D]  = -3.0;
    CONFIG.data.PID_PITCH_m[WG] = 1000.0;

    CONFIG.data.PID_ROLL_m[P]  = 80.0;
    CONFIG.data.PID_ROLL_m[I]  = 0.0;
    CONFIG.data.PID_ROLL_m[D]  = -3.0;
    CONFIG.data.PID_ROLL_m[WG] = 1000.0;    
    
    // Baro
    CONFIG.data.PID_BARO[P]  = 25.0;
    CONFIG.data.PID_BARO[I]  = 0.6;
    CONFIG.data.PID_BARO[D]  = -1.0;
    CONFIG.data.PID_BARO[WG] = 25.0;       
    
    // Sonar
    CONFIG.data.PID_SONAR[P]  = 50.0;
    CONFIG.data.PID_SONAR[I]  = 0.6;
    CONFIG.data.PID_SONAR[D]  = -1.0;
    CONFIG.data.PID_SONAR[WG] = 25.0;  

    // GPS
    CONFIG.data.PID_GPS[P] = 0.0;
    CONFIG.data.PID_GPS[I] = 0.0;
    CONFIG.data.PID_GPS[D] = 0.0;
    CONFIG.data.PID_GPS[WG] = 0.0;
    
    // This function will only initialize data variables
    // writeEEPROM() needs to be called manually to store this data in EEPROM
}

void writeEEPROM() {
    for (uint16_t i = 0; i < sizeof(CONFIG_struct); i++) {
        if (CONFIG.raw[i] != EEPROM.read(i)) {
            // Only re-write new data
            // blocks containing the same value will be left alone
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
