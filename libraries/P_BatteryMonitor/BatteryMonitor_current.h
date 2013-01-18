double current = 0;
int16_t currentSum = 0;
uint8_t currentCount;

#define CURRENT_SENSOR_PIN 17
#define CURRENT_SENSOR_ZERO 513 // no current on the load (default 512)
#define CURRENT_SENSOR_FULL_SCALE_V 3.3
#define CURRENT_SENSOR_SENSITIVITY_5V 0.02
#define CURRENT_SENSOR_SENSITIVITY_3V3 0.0132

currentSum += analogRead(CURRENT_SENSOR_PIN);
currentCount++;
delay(1); // sampling at 1khz
    
if (currentCount >= 24) {
    current = currentSum / currentCount;
    // Reset variables
    currentSum = 0;
    currentCount = 0;
    
    current = (current - CURRENT_SENSOR_ZERO) * CURRENT_SENSOR_FULL_SCALE_V / 1024.0 / CURRENT_SENSOR_SENSITIVITY_3V3;
    
    Serial.println(current);
}