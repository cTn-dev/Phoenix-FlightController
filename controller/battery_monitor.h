/*
    Simple Battery voltage monitor 
    by cTn
    
    AnalogRead scale on Teensy 3.0 is 3.3 / 1024 = 0.0032
    My "default" voltage divider is using 10K R1 and 1K5 R2 with 100nF capacitor paralel to R2
    Voltage divider scale is (10 + 1.5) / 1.5 = 13%
    Real voltage = ADC scale / (13% * 100)
    
    Measured values will be always averaged over 5 samples
*/

// Battery voltage monitor PIN
#define BAT_V_MONITOR_PIN 15
#define BAT_V_MONITOR_WARNING 10.5
#define BAT_V_MONITOR_ALARM 9.9

#define BAT_V_MONITOR_SCALE 0.0032
#define BAT_V_MONITOR_SCALE_FACTOR (BAT_V_MONITOR_SCALE / 0.1304) 

float BatteryVoltage = 0;
bool BatteryWarning = false;
bool BatteryAlarm = false;

float BatteryVoltageSum = 0;
uint8_t BatteryVoltageSamples = 0;

// Variables used for warning/alarm indication via orientation lights
uint8_t BatteryBlinkCounter = 0;
bool BatteryBlinkState = false;

void measureBatteryVoltage() {
    // Read analog PIN value into variable
    BatteryVoltageSum += analogRead(BAT_V_MONITOR_PIN);

    BatteryVoltageSamples++;
    
    if (BatteryVoltageSamples >= 5) {
        // Calculate Average
        BatteryVoltageSum = BatteryVoltageSum / BatteryVoltageSamples;
        
        // Properly scale it
        BatteryVoltage = BatteryVoltageSum * BAT_V_MONITOR_SCALE_FACTOR;
        
        // Reset SUM variables
        BatteryVoltageSum = 0;
        BatteryVoltageSamples = 0;
        
        // Warning & critical battery voltage flag handling
        if (BatteryVoltage < BAT_V_MONITOR_ALARM) {
            // Battery critical
            BatteryAlarm = true;
        } else if (BatteryVoltage < BAT_V_MONITOR_WARNING) {
            // Battery low
            BatteryWarning = true;
        } else {
            // Reset flags to "OFF" state
            BatteryAlarm = false;
            BatteryWarning = false;
        }
        
        #ifdef DISABLE_BATTERY_ALARM
            BatteryAlarm = false;
            BatteryWarning = false;            
        #endif
    }
}

/* This code should be in the main setup()
    pinMode(BAT_V_MONITOR_PIN, INPUT); // Battery voltage input pin
*/

/* This code should be entered inside the 50Hz loop

    // Orientation lights are also used to indicate battery voltage during flight
    // Warning = slow blinking
    // Alarm = fast blinking
    if (BatteryAlarm || BatteryWarning) {
        BatteryBlinkCounter++;
        
        uint8_t BlinkSpeed = 6; // Default blink speed (for battery warning)
        if (BatteryAlarm) BlinkSpeed = 1; // Fast blink speed (for battery critical)
        
        if (BatteryBlinkCounter >= BlinkSpeed) {
            BatteryBlinkState = !BatteryBlinkState;
            BatteryBlinkCounter = 0;
            
            digitalWrite(LED_ORIENTATION, BatteryBlinkState);
        }   
    } else {
        digitalWrite(LED_ORIENTATION, HIGH); // set to HIGH by default
    }
    
*/