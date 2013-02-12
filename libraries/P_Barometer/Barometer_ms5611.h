/*  MS5611 Barometer implementation in C++

    Big shoutout to whoever wrote the aeroquad ms5611 sensor integration
    (because it was used as example/base to create this one)
    
    Needs serious polishing and maybe even some testing
*/

#define MS5611_I2C_ADDRESS      0x77 // address can also be 0x76

#define MS561101BA_PROM_BASE_ADDR  0xA0
#define MS561101BA_PROM_REG_COUNT  8     // number of registers in the PROM
#define MS561101BA_D1_Pressure     0x40
#define MS561101BA_D2_Temperature  0x50
#define MS561101BA_RESET           0x1E

#define MS561101BA_D1D2_SIZE       3

// OSR (Over Sampling Ratio) constants
#define MS561101BA_OSR_256         0x00
#define MS561101BA_OSR_512         0x02
#define MS561101BA_OSR_1024        0x04
#define MS561101BA_OSR_2048        0x06
#define MS561101BA_OSR_4096        0x08


float baroRawAltitude = 0.0;
float baroGroundAltitude = 0.0; 
float baroAltitude = 0.0;
float baroAltitudeRunning = 0.0;

float baroAltitudeToHoldTarget = 0.0;
int16_t baroAltitudeHoldThrottle = 1000;

class MS5611 {
    public:
        // constructor
        MS5611() {
        };
        
        void initialize() {
            baroAltitude = 0.0;
            
            pressureFactor = 1.0 / 5.255;
            baroSmoothFactor = 0.03;
            
            pressure = 0.0;
            
            // Reset the sensor (to populate its internal PROM registers)
            Wire.beginTransmission(MS5611_I2C_ADDRESS);
            Wire.write(MS561101BA_RESET);
            Wire.endTransmission();
            delay(100);
            
            // Read calibration data
            readPROM();
            
            requestRawTemperature(); // setup up next measure() for temperature
            isReadPressure = false;
            pressureCount = 0;
            delay(10);
            measureBaroSum(); // read temperature
            delay(10);
            measureBaro(); // read pressure
            delay(10);
            
            measureGroundBaro();
            delay(500); // Wait for sensor to heat up properly
            
            // check if measured ground altitude is valid
            while (abs(baroRawAltitude - baroGroundAltitude) > 5) {
                delay(26);
                measureGroundBaro();
            }
            
            baroAltitude = baroGroundAltitude;            
        };
        
        void requestRawPressure() {
            Wire.beginTransmission(MS5611_I2C_ADDRESS);
            Wire.write(MS561101BA_D1_Pressure + MS561101BA_OSR_4096);
            Wire.endTransmission();
        }
        
        float readRawPressure() {
            Wire.beginTransmission(MS5611_I2C_ADDRESS);
            Wire.write(0);
            Wire.endTransmission();
            
            Wire.requestFrom(MS5611_I2C_ADDRESS, 3);
            
            lastRawPressure = (Wire.read() << 16) | (Wire.read() << 8) | (Wire.read() << 0);
            
            return (((( lastRawPressure * sens) >> 21) - offset) >> (15 - 5)) / ((float)(1 << 5));
        };
        
        void requestRawTemperature() {
            Wire.beginTransmission(MS5611_I2C_ADDRESS);
            Wire.write(MS561101BA_D2_Temperature + MS561101BA_OSR_4096);
            Wire.endTransmission();
        };
        
        unsigned long readRawTemperature() {
            Wire.beginTransmission(MS5611_I2C_ADDRESS);
            Wire.write(0);
            Wire.endTransmission();
            
            Wire.requestFrom(MS5611_I2C_ADDRESS, 3);  

            lastRawTemperature = (Wire.read() << 16) | (Wire.read() << 8) | (Wire.read() << 0);
            
            int64_t dT = lastRawTemperature - (((long)MS5611Prom[5]) << 8);
            offset  = (((int64_t)MS5611Prom[2]) << 16) + ((MS5611Prom[4] * dT) >> 7);
            sens    = (((int64_t)MS5611Prom[1]) << 15) + ((MS5611Prom[3] * dT) >> 8);

            return lastRawTemperature;
        };
        
        void measureBaro() {
            measureBaroSum();
            evaluateBaroAltitude();
        };

        void measureBaroSum() {
            // switch between pressure and temperature measurements
            // each loop, since it is slow to measure pressure
            if (isReadPressure) {
                rawPressureSum += readRawPressure();
                rawPressureSumCount++;
                
                if (pressureCount == 20) {
                    requestRawTemperature();
                    pressureCount = 0;
                    isReadPressure = false;
                } 
                else {
                    requestRawPressure();
                }
                pressureCount++;
            } else { // select must equal TEMPERATURE
                rawTemperature = (long)readRawTemperature();
                requestRawPressure();
                isReadPressure = true;
            }        
        };
        
        void measureGroundBaro() {
            // measure initial ground pressure (multiple samples)
            float altSum = 0.0;
            for (int i = 0; i < 25; i++) {
                measureBaro();
                altSum += baroRawAltitude;
                delay(12);
            }
            
            baroGroundAltitude = altSum / 25;        
        };
        
        void evaluateBaroAltitude() {
            if (rawPressureSumCount == 0) { // it may occur at init time that no pressure has been read yet!
                return;
            } 

            pressure = rawPressureSum / rawPressureSumCount;
            rawPressureSum = 0.0;
            rawPressureSumCount = 0;            
            
            baroRawAltitude = 44330 * (1 - pow(pressure / 101325.0, pressureFactor)); // returns absolute baroAltitude in meters
            
            baroAltitude = filterSmooth(baroRawAltitude, baroAltitude, baroSmoothFactor);
        };
        
        void getBaroAltitude() {
            baroAltitudeRunning = baroAltitude - baroGroundAltitude;
        };
    
    private:
        // Private variables
        unsigned short MS5611Prom[MS561101BA_PROM_REG_COUNT];
        int64_t offset, sens;
        
        bool isReadPressure;
        float pressureFactor;
        float baroSmoothFactor;
        
        float rawPressureSum;
        uint8_t rawPressureSumCount;
        
        long pressure, rawPressure, rawTemperature;
        uint8_t pressureCount;
        
        long lastRawTemperature;
        long lastRawPressure; 
        
        // Taken from aeroquad
        unsigned char crc4(unsigned short n_prom[]) {
            unsigned short n_rem = 0;           // crc reminder
            unsigned short crc_read;            // original value of the crc

            crc_read  = n_prom[7];              //save read CRC
            n_prom[7] = (0xFF00 & (n_prom[7])); //CRC byte is replaced by 0

            for (int cnt = 0; cnt < 16; cnt++) {   // operation is performed on bytes
                // choose LSB or MSB
                if (cnt%2 == 1) {
                    n_rem ^= (n_prom[cnt >> 1]) & 0x00FF;
                } else {
                    n_rem ^= n_prom[cnt >> 1] >> 8;
                }

                for (int n_bit = 8; n_bit > 0; n_bit--) {
                    if (n_rem & (0x8000)) {
                        n_rem = (n_rem << 1) ^ 0x3000;
                    } else {
                        n_rem = (n_rem << 1);
                    }
                }
            }

            n_rem = (n_rem >> 12) & 0xF; // // final 4-bit reminder is CRC code

            n_prom[7] = crc_read; // restore the crc_read to its original place

            return (n_rem);
        }        
        
        // Taken from aeroquad
        int readPROM() {
            for (int i = 0; i < MS561101BA_PROM_REG_COUNT; i++) {                
                Wire.beginTransmission(MS5611_I2C_ADDRESS);
                Wire.write(MS561101BA_PROM_BASE_ADDR + 2 * i);
                Wire.endTransmission();                
                
                if (Wire.requestFrom(MS5611_I2C_ADDRESS, 2) == 2) {
                    MS5611Prom[i] = (Wire.read() << 8) | Wire.read();
                } else {
                    return 0;
                }
            }

            int crc = crc4(MS5611Prom);
            int crcProm = MS5611Prom[7] & 0xf;
            if (crc == crcProm) {
                return 1;
            }
            return 0;
        }              
};

// Create Baro object
MS5611 baro;

void SensorArray::initializeBaro() {
    baro.initialize();
}

void SensorArray::readBaroSum() {
    baro.measureBaroSum();
}

void SensorArray::evaluateBaroAltitude() {
    baro.evaluateBaroAltitude();
    baro.getBaroAltitude();
}
