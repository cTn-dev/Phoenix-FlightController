int16_t current;
double currentAmps;
double batteryMonitorCapacityUsed = 0.0;

#define CURRENT_SENSOR_PIN 17
#define CURRENT_SENSOR_ZERO 513 // no current on the load (default 512)
#define CURRENT_SENSOR_FULL_SCALE_V 3.3
#define CURRENT_SENSOR_SENSITIVITY_5V 0.02
#define CURRENT_SENSOR_SENSITIVITY_3V3 0.0132
  
void readBatteryMonitorCurrent() {
    current = analogRead(CURRENT_SENSOR_PIN);
    currentAmps = (current - CURRENT_SENSOR_ZERO) * CURRENT_SENSOR_FULL_SCALE_V / 1024.0 / CURRENT_SENSOR_SENSITIVITY_3V3;
    
    // currentAmps / 10 because of the 10hz sampling rate and * 1000 to move the output value from Amps per second
    // to mAmps per second
    batteryMonitorCapacityUsed += (currentAmps / 10.0) * 1000.0;
}

double getBatteryMonitorCapacityUsed() {
    // batteryMonitorCapacityUsed is saved in mAps / ssecond, in here we will divide that by 60 = mAps / minute
    // and another division by 60 will get the output to mAps / hour.
    return batteryMonitorCapacityUsed / 60.0 / 60.0;
}

double getBatteryMonitorLiveCurrent() {
    return currentAmps;
}
