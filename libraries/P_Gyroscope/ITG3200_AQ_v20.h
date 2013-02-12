/* ITG3200 - on AQ shield v 2.0 
   (ITG3200 is on separate breakout)
   
   This library requires some more documentation
*/

#define ITG3200_ADDRESS                 0x69

#define ITG3200_IDENTITY                0x68
#define ITG3200_IDENTITY_MASK           0x7E
#define ITG3200_MEMORY_ADDRESS          0x1D
#define ITG3200_BUFFER_SIZE             6
#define ITG3200_RESET_ADDRESS           0x3E
#define ITG3200_RESET_VALUE             0x80
#define ITG3200_LOW_PASS_FILTER_ADDR    0x16
#define ITG3200_LOW_PASS_FILTER_VALUE   0x1D       // 10Hz low pass filter
#define ITG3200_OSCILLATOR_ADDR         0x3E
#define ITG3200_OSCILLATOR_VALUE        0x01       // use X gyro oscillator
#define ITG3200_SCALE_TO_RADIANS        823.626831 // 14.375 LSBs per °/sec, / Pi / 180
#define ITG3200_TEMPERATURE_ADDRESS     0x1B

float gyro[3];

class ITG3200 {
    public:
        // Constructor
        ITG3200() {
            gyroScaleFactor = radians(1.0 / 14.375);  //  ITG3200 14.375 LSBs per °/sec
            
            gyroSamples = 0;
        };
        
        void initialize() {
            sensors.i2c_write8(ITG3200_ADDRESS, ITG3200_RESET_ADDRESS, ITG3200_RESET_VALUE); // send a reset to the device
            
            // Startup delay 
            delay(100);
            
            sensors.i2c_write8(ITG3200_ADDRESS, ITG3200_LOW_PASS_FILTER_ADDR, ITG3200_LOW_PASS_FILTER_VALUE); // 10Hz low pass filter
            sensors.i2c_write8(ITG3200_ADDRESS, ITG3200_RESET_ADDRESS, ITG3200_OSCILLATOR_VALUE); // use internal oscillator
            
            // Let the gyro heat up
            delay(1500);
        };
        
        // ~1280ms
        void calibrate_gyro() {
            uint8_t i, count = 128;
            int16_t xSum = 0, ySum = 0, zSum = 0;

            for(i = 0; i < count; i++) {
                readGyroRaw();
                xSum += gyroRaw[XAXIS];
                ySum += gyroRaw[YAXIS];
                zSum += gyroRaw[ZAXIS];
                delay(10);
            }
            
            gyro_offset[XAXIS] = -xSum / count;
            gyro_offset[YAXIS] = -ySum / count;
            gyro_offset[ZAXIS] = -zSum / count; 
            
            // Calibration sanity check
            while (abs(gyro_offset[XAXIS] + gyro_offset[YAXIS] + gyro_offset[ZAXIS]) > 300) {
                // gyro calibration failed, run again
                
                delay(1000); // small delay before next gyro calibration
                calibrate_gyro();
            }            
        };

        void readGyroRaw() {
            Wire.beginTransmission(ITG3200_ADDRESS);
            Wire.write(ITG3200_MEMORY_ADDRESS);
            Wire.endTransmission();
            
            Wire.requestFrom(ITG3200_ADDRESS, ITG3200_BUFFER_SIZE);  

            gyroRaw[XAXIS] = (Wire.read() << 8) | Wire.read();
            gyroRaw[YAXIS] = -((Wire.read() << 8) | Wire.read());
            gyroRaw[ZAXIS] = -((Wire.read() << 8) | Wire.read());      
        };
        
        void readGyroSum() {
            readGyroRaw();
            
            gyroSum[XAXIS] += gyroRaw[XAXIS];
            gyroSum[YAXIS] += gyroRaw[YAXIS];
            gyroSum[ZAXIS] += gyroRaw[ZAXIS];
            
            gyroSamples++;
        }; 

        void evaluateGyro() {
            // Calculate average
            gyro[XAXIS] = gyroSum[XAXIS] / gyroSamples;
            gyro[YAXIS] = -(gyroSum[YAXIS] / gyroSamples);
            gyro[ZAXIS] = gyroSum[ZAXIS] / gyroSamples;    
            
            // Apply offsets
            gyro[XAXIS] += gyro_offset[XAXIS];
            gyro[YAXIS] += gyro_offset[YAXIS];
            gyro[ZAXIS] += gyro_offset[ZAXIS];         
            
            // Apply correct scaling (at this point gyroNsumRate is in radians)
            gyro[XAXIS] *= gyroScaleFactor;
            gyro[YAXIS] *= gyroScaleFactor;
            gyro[ZAXIS] *= gyroScaleFactor;
            
            // Reset SUM variables
            gyroSum[XAXIS] = 0;
            gyroSum[YAXIS] = 0;
            gyroSum[ZAXIS] = 0;
            gyroSamples = 0;            
        };        
    private:  
        int16_t gyro_offset[3];
        int16_t gyroRaw[3];
        float gyroSum[3];
        float gyroScaleFactor;   

        uint8_t gyroSamples;
};

ITG3200 itg;

void SensorArray::initializeGyro() {
    itg.initialize();
    itg.calibrate_gyro();
}

void SensorArray::readGyroSum() {
    itg.readGyroSum();
}

void SensorArray::evaluateGyro() {
    itg.evaluateGyro();
}