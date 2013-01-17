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

int16_t magXraw, magYraw, magZraw;
double magX, magY;
double magHeadingX, magHeadingY;

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
            
            magXraw = (Wire.read() << 8) | Wire.read();
            magZraw = (Wire.read() << 8) | Wire.read();
            magYraw = (Wire.read() << 8) | Wire.read();
            
            // start single conversion
            sensors.i2c_write8(HMC5883L_ADDRESS, HMC5883L_RA_MODE, HMC5883L_MODE_SINGLE);            
        };
        
        void evaluateMag() {            
            const double cosRoll =  cos(kinematicsAngleX);
            const double sinRoll =  sin(kinematicsAngleX);
            const double cosPitch = cos(kinematicsAngleY);
            const double sinPitch = sin(kinematicsAngleY);  

            magX = magXraw * cosPitch + magYraw * sinRoll * sinPitch + magZraw * cosRoll * sinPitch;
            magY = magYraw * cosRoll - magZraw * sinRoll;

            const double norm = sqrt(magX * magX + magY * magY);
            
            magHeadingX = magX / norm;
            magHeadingY = -magY / norm;           
        };
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