/*  BMA180 library

    Beware ! BMA180 sends the data in reverse (LOW, HIGH)

    Big thanks to remspoor on #aeroquad IRC for testing this integration
    
    This library requires some more documentation
*/

#define BMA180_ADDRESS                          0x40

#define BMA180_IDENTITY                         0x03
#define BMA180_RESET_REGISTER                   0x10
#define BMA180_TRIGER_RESET_VALUE               0xB6
#define BMA180_ENABLE_WRITE_CONTROL_REGISTER    0x0D
#define BMA180_CONTROL_REGISTER                 0x10
#define BMA180_BW_TCS                           0x20
#define BMA180_LOW_PASS_FILTER_REGISTER         0x20
#define BMA180_10HZ_LOW_PASS_FILTER_VALUE       0x0F
#define BMA180_1200HZ_LOW_PASS_FILTER_VALUE     0x7F
#define BMA180_OFFSET_REGISTER                  0x35
#define BMA180_READ_ROLL_ADDRESS                0x02
#define BMA180_READ_PITCH_ADDRESS               0x04
#define BMA180_READ_YAW_ADDRESS                 0x06
#define BMA180_BUFFER_SIZE                      6

float accel[3];

class BMA180 {
    public:
        // Constructor
        BMA180() {
            accelScaleFactor = 9.81 / 2048.0;
            
            accel_bias[XAXIS] = 0;
            accel_bias[YAXIS] = 0;
            accel_bias[ZAXIS] = 0;
        
            accelSamples = 0;
        };

        void initialize(int bias0, int bias1, int bias2) {
            accel_bias[XAXIS] = bias0;
            accel_bias[YAXIS] = bias1;
            accel_bias[ZAXIS] = bias2;

            // Check if sensor is alive
            Wire.beginTransmission(BMA180_ADDRESS);
            Wire.write(0x00);
            Wire.endTransmission();
            
            Wire.requestFrom(BMA180_ADDRESS, 1);
            
            uint8_t register_value = Wire.read();
            
            if (register_value == BMA180_IDENTITY) {
                sensors.sensors_detected |= ACCELEROMETER_DETECTED;
            } else {
                return;
            }            
            
            initialize();
        };
    
        void initialize() {
            sensors.i2c_write8(BMA180_ADDRESS, BMA180_RESET_REGISTER, BMA180_TRIGER_RESET_VALUE); // reset
            delay(10);
            
            sensors.i2c_write8(BMA180_ADDRESS, BMA180_ENABLE_WRITE_CONTROL_REGISTER, BMA180_CONTROL_REGISTER); // enable writing to control registers

            Wire.beginTransmission(BMA180_ADDRESS);
            Wire.write(BMA180_BW_TCS);
            Wire.endTransmission();

            Wire.requestFrom(BMA180_ADDRESS, 1);
            uint8_t data = Wire.read();

            sensors.i2c_write8(BMA180_ADDRESS, BMA180_LOW_PASS_FILTER_REGISTER, data & BMA180_1200HZ_LOW_PASS_FILTER_VALUE); // set low pass filter to 1.2kHz (value = 0000xxxx)

            Wire.beginTransmission(BMA180_ADDRESS);
            Wire.write(BMA180_OFFSET_REGISTER);
            Wire.endTransmission();
    
            Wire.requestFrom(BMA180_ADDRESS, 1);
            data = Wire.read();
            data &= 0xF1;
            data |= 0x08; // set range select bits for +/-4g
            sensors.i2c_write8(BMA180_ADDRESS, BMA180_OFFSET_REGISTER, data);
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
            accel_bias[ZAXIS] = (zSum / count) - 2048; // - 1G

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
            Wire.beginTransmission(BMA180_ADDRESS);
            Wire.write(BMA180_READ_ROLL_ADDRESS);
            Wire.endTransmission();

            Wire.requestFrom(BMA180_ADDRESS, BMA180_BUFFER_SIZE);

            accelRaw[XAXIS] = -(Wire.read() | (Wire.read() << 8)) >> 2;
            accelRaw[YAXIS] = (Wire.read() | (Wire.read() << 8)) >> 2;
            accelRaw[ZAXIS] = (Wire.read() | (Wire.read() << 8)) >> 2;
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

            // Apply correct scaling (at this point accelNsumAvr reprensents +- 1g = 9.81 m/s^2)
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

BMA180 bma;

void SensorArray::initializeAccel() {
    bma.initialize(CONFIG.data.ACCEL_BIAS[XAXIS], CONFIG.data.ACCEL_BIAS[YAXIS], CONFIG.data.ACCEL_BIAS[ZAXIS]);
}

void SensorArray::calibrateAccel() {
    bma.calibrate_accel();
}

void SensorArray::readAccelSum() {
    bma.readAccelSum();
}

void SensorArray::evaluateAccel() {
    bma.evaluateAccel();
}
