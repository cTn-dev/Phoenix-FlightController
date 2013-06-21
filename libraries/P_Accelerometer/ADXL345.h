/*  ADXL345 - (ADXL345 is on separate breakout)

    Beware ! ADXL345 sends the data in reverse (LOW, HIGH)
    
    This library requires some more documentation
*/

#define ADXL345_ADDRESS 0x53
#define ADXL345_DEVID   0xE5

float accel[3];

class ADXL345 {
    public:
        // Constructor
        ADXL345() {
            accelScaleFactor = 9.81 / 256.0;
            accelSamples = 0;
        };

        void initialize() {
            // Check if sensor is alive
            Wire.beginTransmission(ADXL345_ADDRESS);
            Wire.write(0x00);
            Wire.endTransmission();
            
            Wire.requestFrom(ADXL345_ADDRESS, 1);
            
            uint8_t register_value = Wire.read();
            
            if (register_value == ADXL345_DEVID) {
                sensors.sensors_detected |= ACCELEROMETER_DETECTED;
            } else {
                return;
            }
            
            sensors.i2c_write8(ADXL345_ADDRESS, 0x2D, 0x08);  // set device to *measure*
            sensors.i2c_write8(ADXL345_ADDRESS, 0x31, 0x09);  // set full range and +/- 4G
            sensors.i2c_write8(ADXL345_ADDRESS, 0x2C, 0x0B);  // 200hz sampling
            
            // setup axis mapping
            if (CONFIG.data.ACCEL_AXIS_MAP.initialized == 0) { // check if map was defined before, if not "save default" order set  
                CONFIG.data.ACCEL_AXIS_MAP.axis1 = 0; // x
                CONFIG.data.ACCEL_AXIS_MAP.axis2 = 1; // y
                CONFIG.data.ACCEL_AXIS_MAP.axis3 = 2; // z
                
                CONFIG.data.ACCEL_AXIS_MAP.axis1_sign = 0;
                CONFIG.data.ACCEL_AXIS_MAP.axis2_sign = 0;
                CONFIG.data.ACCEL_AXIS_MAP.axis3_sign = 0;
                
                CONFIG.data.ACCEL_AXIS_MAP.initialized = 1;
                
                // save default in eeprom
                writeEEPROM();
            }
        };

        // ~1280ms (only runs when requested)
        void calibrate_accel() {
            uint8_t i, count = 128;
            int32_t xSum = 0, ySum = 0, zSum = 0;

            for(i = 0; i < count; i++) {
                readAccelRaw();
                xSum += accelRaw[XAXIS];
                ySum += accelRaw[YAXIS];
                zSum += accelRaw[ZAXIS];
                delay(10);
            }

            CONFIG.data.ACCEL_BIAS[XAXIS] = xSum / count;
            CONFIG.data.ACCEL_BIAS[YAXIS] = ySum / count;
            CONFIG.data.ACCEL_BIAS[ZAXIS] = (zSum / count) - 256; // - 1G
            
            // Reverse calibration forces
            CONFIG.data.ACCEL_BIAS[XAXIS] *= -1;
            CONFIG.data.ACCEL_BIAS[YAXIS] *= -1;
            CONFIG.data.ACCEL_BIAS[ZAXIS] *= -1;            
        };

        void readAccelRaw() {
            Wire.beginTransmission(ADXL345_ADDRESS);
            Wire.write(0x32);
            Wire.endTransmission();

            Wire.requestFrom(ADXL345_ADDRESS, 6);

            accelRaw[CONFIG.data.ACCEL_AXIS_MAP.axis1] = (Wire.read() | (Wire.read() << 8)) * (CONFIG.data.ACCEL_AXIS_MAP.axis1_sign?-1:1);
            accelRaw[CONFIG.data.ACCEL_AXIS_MAP.axis2] = (Wire.read() | (Wire.read() << 8)) * (CONFIG.data.ACCEL_AXIS_MAP.axis2_sign?-1:1);
            accelRaw[CONFIG.data.ACCEL_AXIS_MAP.axis3] = (Wire.read() | (Wire.read() << 8)) * (CONFIG.data.ACCEL_AXIS_MAP.axis3_sign?-1:1);
        };

        void readAccelSum() {
            readAccelRaw();
            
            accelSum[XAXIS] += accelRaw[XAXIS];
            accelSum[YAXIS] += accelRaw[YAXIS];
            accelSum[ZAXIS] += accelRaw[ZAXIS];
    
            accelSamples++;
        };
    
        void evaluateAccel() {
            // Calculate average
            accel[XAXIS] = accelSum[XAXIS] / accelSamples;
            accel[YAXIS] = accelSum[YAXIS] / accelSamples;
            accel[ZAXIS] = accelSum[ZAXIS] / accelSamples;

            // Apply offsets
            accel[XAXIS] += CONFIG.data.ACCEL_BIAS[XAXIS];
            accel[YAXIS] += CONFIG.data.ACCEL_BIAS[YAXIS];
            accel[ZAXIS] += CONFIG.data.ACCEL_BIAS[ZAXIS];

            // Apply correct scaling (at this point accel reprensents +- 1g = 9.81 m/s^2)
            accel[XAXIS] *= accelScaleFactor;
            accel[YAXIS] *= accelScaleFactor;
            accel[ZAXIS] *= accelScaleFactor;

            // Reset SUM variables
            accelSum[XAXIS] = 0;
            accelSum[YAXIS] = 0;
            accelSum[ZAXIS] = 0;
            accelSamples = 0;
        };
    
    private:
        float accelScaleFactor; 
        
        int16_t accelRaw[3];
        float accelSum[3]; 
        
        uint8_t accelSamples;
};

ADXL345 adxl;

void SensorArray::initializeAccel() {
    adxl.initialize();
}

void SensorArray::calibrateAccel() {
    adxl.calibrate_accel();
}

void SensorArray::readAccelSum() {
    adxl.readAccelSum();
}

void SensorArray::evaluateAccel() {
    adxl.evaluateAccel();
}
