/*  HMC5883L magnetometer integration in C++

    Needs testing
*/

#define HMC5883L_ADDRESS            0x1E

#define HMC5883L_RA_CONFIG_A        0x00
#define HMC5883L_RA_CONFIG_B        0x01
#define HMC5883L_RA_MODE            0x02

#define HMC5883L_MODE_CONTINUOUS    0x00
#define HMC5883L_MODE_SINGLE        0x01
#define HMC5883L_MODE_IDLE          0x02

float magHeadingX, magHeadingY;

class HMC5883L {
    public:
        // Constructor
        HMC5883L() {
        };
        
        void initialize() {
            // Set gain to +- 1.0 Ga
            sensors.i2c_write8(HMC5883L_ADDRESS, HMC5883L_RA_CONFIG_B, 0x20);
            
            // Start in single mode
            sensors.i2c_write8(HMC5883L_ADDRESS, HMC5883L_RA_MODE, HMC5883L_MODE_SINGLE);           
        };
        
        void readMagRaw() {
            Wire.beginTransmission(HMC5883L_ADDRESS);
            Wire.write(0x03);
            Wire.endTransmission(); 

            Wire.requestFrom(HMC5883L_ADDRESS, 6);
            
            magRaw[XAXIS] = (Wire.read() << 8) | Wire.read();
            magRaw[ZAXIS] = (Wire.read() << 8) | Wire.read();
            magRaw[YAXIS] = (Wire.read() << 8) | Wire.read();
            
            // start single conversion
            sensors.i2c_write8(HMC5883L_ADDRESS, HMC5883L_RA_MODE, HMC5883L_MODE_SINGLE);            
        };
        
        void evaluateMag() {            
            const float cosRoll =  cos(kinematicsAngle[XAXIS]);
            const float sinRoll =  sin(kinematicsAngle[XAXIS]);
            const float cosPitch = cos(kinematicsAngle[YAXIS]);
            const float sinPitch = sin(kinematicsAngle[YAXIS]);  

            magX = magRaw[XAXIS] * cosPitch + magRaw[YAXIS] * sinRoll * sinPitch + magRaw[ZAXIS] * cosRoll * sinPitch;
            magY = magRaw[YAXIS] * cosRoll - magRaw[ZAXIS] * sinRoll;

            const float norm = sqrt(magX * magX + magY * magY);
            
            magHeadingX = magX / norm;
            magHeadingY = -magY / norm;           
        };
        
    private:
        int16_t magRaw[3];
        float magX, magY;
};

HMC5883L hmc5883l;

void SensorArray::initializeMag() {
    hmc5883l.initialize();
}

void SensorArray::readMag() {
    hmc5883l.readMagRaw();
}

void SensorArray::evaluateMag() {
    hmc5883l.evaluateMag();
}