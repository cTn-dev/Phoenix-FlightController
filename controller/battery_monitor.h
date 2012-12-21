/*
    Simple Battery voltage monitor 
    by cTn
    
    AnalogRead scale on Teensy 3.0 is 3.3 / 1024 = 0.0032
    My "default" voltage divider is using 10K R1 and 1K5 R2 with 100nF capacitor paralel to R2
    Voltage divider scale is (10 + 1.5) / 1.5 = 13%
    Real voltage = ADC scale / (13% * 100)
*/

// Battery voltage monitor PIN
#define BAT_V_MONITOR_PIN 15
#define BAT_V_MONITOR_R1 10000.0
#define BAT_V_MONITOR_R2 1500.0
#define BAT_V_MONITOR_WARNING 10.5
#define BAT_V_MONITOR_ALARM 9.9

#define BAT_V_MONITOR_SCALE 0.0032
#define BAT_V_MONITOR_SCALE_FACTOR (BAT_V_MONITOR_SCALE / 0.1304) 

float BatteryVoltage = 0;
bool BatteryWarning = false;
bool BatteryAlarm = false;

// Variables used for warning/alarm indication via orientation lights
uint8_t BatteryBlinkCounter = 0;
bool BatteryBlinkState = false;

void measureBatteryVoltage() {
    BatteryVoltage = analogRead(BAT_V_MONITOR_PIN);
    BatteryVoltage *= BAT_V_MONITOR_SCALE_FACTOR;

    if (BatteryVoltage < BAT_V_MONITOR_ALARM) {
        BatteryAlarm = true;
    } else if (BatteryVoltage < BAT_V_MONITOR_WARNING) {
        BatteryWarning = true;
    }
}