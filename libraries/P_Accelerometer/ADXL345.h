/*  ADXL345 - (ADXL345 is on separate breakout)

    Beware ! ADXL345 sends the data in reverse (LOW, HIGH)
    
    This library requires some more documentation
*/

#define ADXL345_ADDRESS 0x53

double accel[3];

class ADXL345 {
    public:
        // Constructor
        ADXL345() {
            accelScaleFactor = 9.81 / 256.0;
            
            accel_bias[XAXIS] = 0;
            accel_bias[YAXIS] = 0;
            accel_bias[ZAXIS] = 0;
        
            accelSamples = 0;
        };
        void initialize(int bias0, int bias1, int bias2) {
            accel_bias[XAXIS] = bias0;
            accel_bias[YAXIS] = bias1;
            accel_bias[ZAXIS] = bias2;
            initialize();
        };
        void initialize() {
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
            accel_bias[ZAXIS] = zSum / count;
    
            // Code will stop here, printing out the calibration data in serial console.
            // Re-Calibrating each loop.
            // Offsets to the values returned on a flat surface are meant to be either hardcoded during initialization
            // or stored inside eeprom.
            while (1) {
                Serial.print(accel_bias[XAXIS]);
                Serial.write('\t');
                Serial.print(accel_bias[YAXIS]);
                Serial.write('\t');
                Serial.print(accel_bias[ZAXIS]);
                Serial.write('\t');
                Serial.println();

                delay(5000);
                calibrate_accel();
            }
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
        double accelScaleFactor; 
        
        int16_t accelRaw[3];
        double accelSum[3]; 
        
        uint8_t accelSamples;
};

ADXL345 adxl;

void SensorArray::initializeAccel() {
    adxl.initialize(CONFIG.data.ACCEL_BIAS[0], CONFIG.data.ACCEL_BIAS[1], CONFIG.data.ACCEL_BIAS[2]);
    
    // Only runs when requested
    // adxl.calibrate_accel();
}

void SensorArray::readAccelSum() {
    adxl.readAccelSum();
}

void SensorArray::evaluateAccel() {
    adxl.evaluateAccel();
}
