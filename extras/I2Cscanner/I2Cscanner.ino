#include <Wire.h>

uint8_t address;
int8_t status;

void setup() {
    Serial.begin(115200);
    Wire.begin();
}

void loop() {
    Serial.println("Scanning...");
    
    for (address = 1; address <= 127; address++) {
        Wire.beginTransmission(address);
        status = Wire.endTransmission();
        
        if (status == 0) {
            Serial.print("I2C device found at 0x");
            Serial.println(address, HEX);
        }
        delay(5);
    }
    
    Serial.println("done");
    
    delay(5000);
}