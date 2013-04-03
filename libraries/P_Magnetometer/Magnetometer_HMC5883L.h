/*  HMC5883L magnetometer implementation

    Featuring accelerometer tilt compensation, for more information visit
    @ https://www.loveelectronics.co.uk/Tutorials/13/tilt-compensated-compass-arduino-tutorial
    
    Supporting 360 deg heading (+- 180deg), to use with 0-360 deg kinematics add + PI
    
    Output inside absolute heading is in radians.
*/

#define HMC5883L_ADDRESS            0x1E

#define HMC5883L_RA_CONFIG_A        0x00
#define HMC5883L_RA_CONFIG_B        0x01
#define HMC5883L_RA_MODE            0x02

#define HMC5883L_MODE_CONTINUOUS    0x00
#define HMC5883L_MODE_SINGLE        0x01
#define HMC5883L_MODE_IDLE          0x02

#define HMC5883L_GAIN_07            0x00 // +- 0.7 Ga
#define HMC5883L_GAIN_10            0x20 // +- 1.0 Ga (default)
#define HMC5883L_GAIN_15            0x40 // +- 1.5 Ga
#define HMC5883L_GAIN_20            0x60 // +- 2.0 Ga
#define HMC5883L_GAIN_32            0x80 // +- 3.2 Ga
#define HMC5883L_GAIN_38            0xA0 // +- 3.8 Ga
#define HMC5883L_GAIN_45            0xC0 // +- 4.5 Ga
#define HMC5883L_GAIN_65            0xE0 // +- 6.5 Ga

float magHeadingX, magHeadingY;
float magHeadingAbsolute = 0.0;

class HMC5883L {
    public:
        // Constructor
        HMC5883L() {
        };
        
        void initialize() {
            // Check if sensor is alive
            Wire.beginTransmission(HMC5883L_ADDRESS);
            Wire.write(0x00);
            Wire.endTransmission();
            
            Wire.requestFrom(HMC5883L_ADDRESS, 1);
            
            uint8_t register_value = Wire.read();
            
            if (register_value == 0x10) {
                sensors.sensors_detected |= MAGNETOMETER_DETECTED;
            } else {
                return;
            }          
        
            // Set gain to +- 1.0 Ga
            sensors.i2c_write8(HMC5883L_ADDRESS, HMC5883L_RA_CONFIG_B, HMC5883L_GAIN_10);
            
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
            float cosRoll =  cos(kinematicsAngle[XAXIS]);
            float sinRoll =  sin(kinematicsAngle[XAXIS]);
            float cosPitch = cos(kinematicsAngle[YAXIS]);
            float sinPitch = sin(kinematicsAngle[YAXIS]);  

            // Apply accelerometer tilt corrections
            magX = magRaw[XAXIS] * cosPitch + magRaw[ZAXIS] * sinPitch;
            magY = magRaw[XAXIS] * sinRoll * sinPitch + magRaw[YAXIS] * cosRoll - magRaw[ZAXIS] * sinRoll * cosPitch;

            // Normalize measurements
            float norm = sqrt(magX * magX + magY * magY);
            
            magHeadingX = magX / norm;
            magHeadingY = -magY / norm;

            // Calculate absolute heading
            magHeadingAbsolute = atan2(magHeadingY, magHeadingX);
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