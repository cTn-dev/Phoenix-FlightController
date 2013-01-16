#define MS5611_I2C_ADDRESS      0x77 // address can also be 0x76

double baroRawAltitude = 0.0;
double baroGroundAltitude = 0.0; 
double baroAltitude = 0.0;
double baroAltitudeRunning = 0.0;

double baroAltitudeToHoldTarget = 0.0;
int16_t baroAltitudeHoldThrottle = 1000;

class MS5611 {
    public:
        // constructor
        MS5611() {
        };
        
        void initialize() {
            baroAltitude = 0.0;
            
            pressureFactor = 1.0 / 5.255;
            baroSmoothFactor = 0.02;
            
        };
        
        void requestRawPressure() {
        }
        
        double readRawPressure() {
        };
        
        void requestRawTemperature() {
        };
        
        unsigned long readRawTemperature() {
        };
        
        void measureBaro() {
            measureBaroSum();
            evaluateBaroAltitude();
        };

        void measureBaroSum() {
        };
        
        void measureGroundBaro() {
        };
        
        void evaluateBaroAltitude() {
            if (rawPressureSumCount == 0) { // it may occur at init time that no pressure has been read yet!
                return;
            }        
        };
        
        void getBaroAltitude() {
        };
    
    private:
        bool isReadPressure;
        double pressureFactor;
        double baroSmoothFactor;
        
        double rawPressureSum;
        uint8_t rawPressureSumCount;
        
        long pressure, rawPressure, rawTemperature;
        uint8_t pressureCount;
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
