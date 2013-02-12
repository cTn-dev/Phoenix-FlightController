/*  Current monitor based on ACS758X 100B Hall Effect Linear Current Sensor

    Could use some optimizations and better documentation.
*/
#define CURRENT_SENSOR_PIN 17
#define CURRENT_SENSOR_ZERO 512 // no current on the load (default 512)
#define CURRENT_SENSOR_FULL_SCALE_V 3.3
#define CURRENT_SENSOR_SENSITIVITY_5V 0.02
#define CURRENT_SENSOR_SENSITIVITY_3V3 0.0132

int16_t current;
float currentAmps;
float batteryMonitorCapacityUsed = 0.0;
unsigned long batteryMonitorCurrent_timer;
  
void readBatteryMonitorCurrent() {
    // Current time
    unsigned long now = millis();
    float dT = 1000.0 / (now - batteryMonitorCurrent_timer); // Delta Time = 1 second / time passed = Hz
    
    current = analogRead(CURRENT_SENSOR_PIN);
    
    // Dead band
    if (abs(current) - CURRENT_SENSOR_ZERO > 1) {
        currentAmps = (current - CURRENT_SENSOR_ZERO) * CURRENT_SENSOR_FULL_SCALE_V / 1024.0 / CURRENT_SENSOR_SENSITIVITY_3V3;
        
        // Update capacity used with new data
        batteryMonitorCapacityUsed += (currentAmps / dT);

        // Save time for next comparison
        batteryMonitorCurrent_timer = now;
    } else {
        currentAmps = 0.0;
    }
}

float getBatteryMonitorCapacityUsed() {
    // batteryMonitorCapacityUsed is saved in A / hour, in here we will multiply the saved value by 1000 to convert from A to mA
    // divide that by 60 = mAps / minute
    // and another division by 60 will get the output to mAps / hour.
    return batteryMonitorCapacityUsed * 1000.0 / 60.0 / 60.0;
}

float getBatteryMonitorLiveCurrent() {
    return currentAmps;
}
