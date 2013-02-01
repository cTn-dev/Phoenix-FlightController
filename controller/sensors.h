class SensorArray {
    public:
        SensorArray() {};
        void initializeGyro();
        void readGyroSum();
        void evaluateGyro();
        void initializeAccel();
        void readAccelSum();
        void evaluateAccel();
        void initializeMag();
        void readMag();
        void evaluateMag();
        void initializeBaro();
        void readBaroSum();
        void evaluateBaroAltitude();
        void initializeGPS();
        void readGPS();
        void evaluateGPS();
        
        // I2C stuff
        void i2c_write8 (int16_t deviceAddress, uint8_t registerAddress, int16_t registerValue) {
            // I am using int16_t for dataValue becuase we don't know if we are writing
            // 8 bit signed or unsigned value.
            
            Wire.beginTransmission(deviceAddress);
            Wire.write(registerAddress);
            Wire.write(registerValue);
            Wire.endTransmission();              
        };
        
        int16_t i2c_read16 (int16_t deviceAddress, uint8_t registerAddress) {
            int16_t data;
            
            Wire.beginTransmission(deviceAddress);
            Wire.write(registerAddress);
            Wire.endTransmission();
            
            Wire.requestFrom(deviceAddress, 2);
            
            data = (Wire.read() << 8) | Wire.read();
            
            return data;            
        };
};    

SensorArray sensors;
