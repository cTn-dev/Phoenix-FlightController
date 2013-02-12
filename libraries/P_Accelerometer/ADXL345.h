/*  ADXL345 - (ADXL345 is on separate breakout)

    Beware ! ADXL345 sends the data in reverse (LOW, HIGH)
    
    This library requires some more documentation
*/

#define ADXL345_ADDRESS 0x53

float accel[3];

class ADXL345 {
    public:
        // Constructor
        ADXL345() {
            accelScaleFactor = 9.81 / 256.0;
            accelSamples = 0;
        };

        void initialize(int16_t bias0, int16_t bias1, int16_t bias2) {
            // Setup accel bias from EEPROM
            accel_bias[XAXIS] = bias0;
            accel_bias[YAXIS] = bias1;
            accel_bias[ZAXIS] = bias2;  
            
            sensors.i2c_write8(ADXL345_ADDRESS, 0x2D, 1 << 3);     // set device to *measure*
            sensors.i2c_write8(ADXL345_ADDRESS, 0x31, 0x09);       // set full range and +/- 4G
            sensors.i2c_write8(ADXL345_ADDRESS, 0x2C, 8 + 2 + 1);  // 200hz sampling
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

            accel_bias[XAXIS] = xSum / count;
            accel_bias[YAXIS] = ySum / count;
            accel_bias[ZAXIS] = (zSum / count) - 256; // - 1G
            
            // Reverse calibration forces
            accel_bias[XAXIS] *= -1;
            accel_bias[YAXIS] *= -1;
            accel_bias[ZAXIS] *= -1;            
    
            // Write calibration data to config
            CONFIG.data.ACCEL_BIAS[XAXIS] = accel_bias[XAXIS];
            CONFIG.data.ACCEL_BIAS[YAXIS] = accel_bias[YAXIS];
            CONFIG.data.ACCEL_BIAS[ZAXIS] = accel_bias[ZAXIS];
        };

        void readAccelRaw() {
            Wire.beginTransmission(ADXL345_ADDRESS);
            Wire.write(0x32);
            Wire.endTransmission();

            Wire.requestFrom(ADXL345_ADDRESS, 6);

            accelRaw[XAXIS] = Wire.read() | (Wire.read() << 8);
            accelRaw[YAXIS] = Wire.read() | (Wire.read() << 8);
            accelRaw[ZAXIS] = Wire.read() | (Wire.read() << 8);
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
            accel[XAXIS] += accel_bias[XAXIS];
            accel[YAXIS] += accel_bias[YAXIS];
            accel[ZAXIS] += accel_bias[ZAXIS];

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
        int16_t accel_bias[3];
        float accelScaleFactor; 
        
        int16_t accelRaw[3];
        float accelSum[3]; 
        
        uint8_t accelSamples;
};

ADXL345 adxl;

void SensorArray::initializeAccel() {
    adxl.initialize(CONFIG.data.ACCEL_BIAS[XAXIS], CONFIG.data.ACCEL_BIAS[YAXIS], CONFIG.data.ACCEL_BIAS[ZAXIS]);
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
