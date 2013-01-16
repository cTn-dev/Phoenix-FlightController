/*  According to MPU6050 datasheet
    this chip should support I2C speeds up to 400kHz (Fast-mode Fm)
    
    However on Teensy 3.0 i am able to reach 2.4MHz (High-speed mode) without any problems.
    (which cuts down the reading time of accel + gyro to about 180us)
*/

// MPU 6050 Registers
#define MPU6050_ADDRESS         0x68   
#define MPUREG_WHOAMI			0x75
#define MPUREG_SMPLRT_DIV		0x19
#define MPUREG_CONFIG			0x1A
#define MPUREG_GYRO_CONFIG		0x1B
#define MPUREG_ACCEL_CONFIG		0x1C
#define MPUREG_FIFO_EN			0x23
#define MPUREG_INT_PIN_CFG		0x37
#define MPUREG_INT_ENABLE		0x38
#define MPUREG_INT_STATUS		0x3A
#define MPUREG_ACCEL_XOUT_H		0x3B
#define MPUREG_ACCEL_XOUT_L		0x3C
#define MPUREG_ACCEL_YOUT_H		0x3D
#define MPUREG_ACCEL_YOUT_L		0x3E
#define MPUREG_ACCEL_ZOUT_H		0x3F
#define MPUREG_ACCEL_ZOUT_L		0x40
#define MPUREG_TEMP_OUT_H		0x41
#define MPUREG_TEMP_OUT_L		0x42
#define MPUREG_GYRO_XOUT_H		0x43
#define MPUREG_GYRO_XOUT_L		0x44
#define MPUREG_GYRO_YOUT_H		0x45
#define MPUREG_GYRO_YOUT_L		0x46
#define MPUREG_GYRO_ZOUT_H		0x47
#define MPUREG_GYRO_ZOUT_L		0x48
#define MPUREG_USER_CTRL		0x6A
#define MPUREG_PWR_MGMT_1		0x6B
#define MPUREG_PWR_MGMT_2		0x6C
#define MPUREG_FIFO_COUNTH		0x72
#define MPUREG_FIFO_COUNTL		0x73
#define MPUREG_FIFO_R_W			0x74


// Configuration bits
#define BIT_SLEEP				0x40
#define BIT_H_RESET				0x80
#define BITS_CLKSEL				0x07
#define MPU_CLK_SEL_PLLGYROX	0x01
#define MPU_CLK_SEL_PLLGYROZ	0x03
#define MPU_EXT_SYNC_GYROX		0x02
#define BITS_FS_250DPS          0x00
#define BITS_FS_500DPS          0x08
#define BITS_FS_1000DPS         0x10
#define BITS_FS_2000DPS         0x18
#define BITS_FS_MASK            0x18
#define BITS_DLPF_CFG_256HZ_NOLPF2  0x00
#define BITS_DLPF_CFG_188HZ         0x01
#define BITS_DLPF_CFG_98HZ          0x02
#define BITS_DLPF_CFG_42HZ          0x03
#define BITS_DLPF_CFG_20HZ          0x04
#define BITS_DLPF_CFG_10HZ          0x05
#define BITS_DLPF_CFG_5HZ           0x06
#define BITS_DLPF_CFG_2100HZ_NOLPF  0x07
#define BITS_DLPF_CFG_MASK          0x07
#define BIT_INT_ANYRD_2CLEAR    0x10
#define BIT_RAW_RDY_EN			0x01
#define BIT_I2C_IF_DIS          0x10
#define BIT_INT_STATUS_DATA		0x01

int16_t gyroX, gyroY, gyroZ;
int16_t accelX, accelY, accelZ;
int16_t gyro_temperature;
double gyroScaleFactor;
double accelScaleFactor;

double gyroXsum, gyroYsum, gyroZsum;
double accelXsum, accelYsum, accelZsum;

double gyroXsumRate, gyroYsumRate, gyroZsumRate;
double accelXsumAvr, accelYsumAvr, accelZsumAvr;

uint8_t gyroSamples = 0;
uint8_t accelSamples = 0;

class MPU6050 {
    public: 
        // Constructor
        MPU6050() {
            // Range = +1000 to -1000 (2000) 1000dps
            // 16 bit scale (2^16) = 65536
            // to get the scale factor in radians = radians(2000.0 / 65536.0)
            gyroScaleFactor = radians(2000.0 / 65536.0);
            
            // Manually defined accel bias
            // To calculate accel bias measure maximum positive and maximum negative value for axis
            // and then calculate average which will be used as bias
            // accelXpositive and accelXnegative should be an average of at least 500 samples
            // biasX = (accelXpositive + accelXnegative) / 2;
            
            // The calibration output isn't really "working" for me, i will enter the X and Y axis
            // values manually.
            accel_bias[0] = -390;
            accel_bias[1] = 150;
            accel_bias[2] = 380;
            
            // Accel scale factor = 9.81 m/s^2 / scale
            // 9.81 / 8192 = 0.00119751
            accelScaleFactor = 9.81 / 8192.0;
        };
        
        void initialize() {
            // Chip reset
            sensors.i2c_write8(MPU6050_ADDRESS, MPUREG_PWR_MGMT_1, BIT_H_RESET);
            
            // Startup delay 
            delay(100);  
            
            // Enable auxiliary I2C bus bypass
            sensors.i2c_write8(MPU6050_ADDRESS, MPUREG_INT_PIN_CFG, 0x02); // I2C _BYPASS _EN 1
            
            // Wake Up device and select GyroZ clock (better performance)
            sensors.i2c_write8(MPU6050_ADDRESS, MPUREG_PWR_MGMT_1, MPU_CLK_SEL_PLLGYROZ);
            sensors.i2c_write8(MPU6050_ADDRESS, MPUREG_PWR_MGMT_2, 0);    
            
            // Sample rate = 1kHz 
            sensors.i2c_write8(MPU6050_ADDRESS, MPUREG_SMPLRT_DIV, 0x00);

            // FS & DLPF   FS = 1000 degrees/s (dps), DLPF = 42Hz (low pass filter)
            sensors.i2c_write8(MPU6050_ADDRESS, MPUREG_CONFIG, BITS_DLPF_CFG_42HZ); 

            // Gyro scale 1000 degrees/s (dps)
            sensors.i2c_write8(MPU6050_ADDRESS, MPUREG_GYRO_CONFIG, BITS_FS_1000DPS);
            
            // Accel scale +-4g (8192LSB/g)
            sensors.i2c_write8(MPU6050_ADDRESS, MPUREG_ACCEL_CONFIG, 0x08);    

            // Initial delay after proper configuration
            // let sensors heat up (especially gyro)
            delay(1500);
        };
        
        // ~1280ms
        void calibrate_gyro() {
            uint8_t i, count = 128;
            int16_t xSum = 0, ySum = 0, zSum = 0;

            for(i = 0; i < count; i++) {
                readGyroRaw();
                xSum += gyroX;
                ySum += gyroY;
                zSum += gyroZ;
                delay(10);
            }
            
            gyro_offset[0] = -xSum / count;
            gyro_offset[1] = -ySum / count;
            gyro_offset[2] = -zSum / count; 
        };
        
        void readGyroRaw() {
            Wire.beginTransmission(MPU6050_ADDRESS);
            Wire.write(MPUREG_GYRO_XOUT_H);
            Wire.endTransmission();
            
            Wire.requestFrom(MPU6050_ADDRESS, 6);
            
            gyroX = (Wire.read() << 8) | Wire.read();
            gyroY = (Wire.read() << 8) | Wire.read();
            gyroZ = (Wire.read() << 8) | Wire.read();
        };
        
        void readAccelRaw() {
            Wire.beginTransmission(MPU6050_ADDRESS);
            Wire.write(MPUREG_ACCEL_XOUT_H);
            Wire.endTransmission();
            
            Wire.requestFrom(MPU6050_ADDRESS, 6);
            
            accelX = (Wire.read() << 8) | Wire.read();
            accelY = (Wire.read() << 8) | Wire.read(); 
            accelZ = (Wire.read() << 8) | Wire.read();
        };        
        
        void readGyroSum() {
            readGyroRaw();
            
            gyroXsum += gyroX;
            gyroYsum += gyroY;
            gyroZsum += gyroZ;
            
            gyroSamples++;
        };        
        
        void readAccelSum() {
            readAccelRaw();
            
            accelXsum += accelX;
            accelYsum += accelY;
            accelZsum += accelZ;  

            accelSamples++;
        };
        
        void evaluateGyro() {
            // Calculate average
            gyroXsumRate = gyroXsum / gyroSamples;
            gyroYsumRate = -(gyroYsum / gyroSamples);
            gyroZsumRate = gyroZsum / gyroSamples;  
            
            // Apply offsets
            gyroXsumRate += gyro_offset[0];
            gyroYsumRate += gyro_offset[1];
            gyroZsumRate += gyro_offset[2];         
            
            // Apply correct scaling (at this point gyroNsumRate is in radians)
            gyroXsumRate *= gyroScaleFactor;
            gyroYsumRate *= gyroScaleFactor;
            gyroZsumRate *= gyroScaleFactor;
            
            // Reset SUM variables
            gyroXsum = 0;
            gyroYsum = 0;
            gyroZsum = 0;
            gyroSamples = 0;
        };
        
        void evaluateAccel() {
            // Calculate average
            accelXsumAvr = accelXsum / accelSamples;
            accelYsumAvr = accelYsum / accelSamples;
            accelZsumAvr = accelZsum / accelSamples;  
            
            // Apply offsets
            accelXsumAvr += accel_bias[0];
            accelYsumAvr += accel_bias[1];
            accelZsumAvr += accel_bias[2];
            
            /* used for debugging calibrated accel
            Serial.print(accelXsumAvr);
            Serial.write('\t');
            Serial.print(accelYsumAvr);
            Serial.write('\t');
            Serial.print(accelZsumAvr);
            Serial.write('\t');
            Serial.println();            
            */
            
            // Apply correct scaling (at this point accelNsumAvr reprensents +- 1g = 9.81 m/s^2)
            accelXsumAvr *= accelScaleFactor;
            accelYsumAvr *= accelScaleFactor;
            accelZsumAvr *= accelScaleFactor;
            
            // Reset SUM variables
            accelXsum = 0;
            accelYsum = 0;
            accelZsum = 0;
            accelSamples = 0;
        };
        
        void readGyroCalibrated() {
            readGyroRaw();
            
            gyroX += gyro_offset[0];
            gyroY += gyro_offset[1];
            gyroZ += gyro_offset[2];
        };
        
        void readAccelCalibrated() {
            readAccelRaw();
            
            accelX += accel_bias[0];
            accelY += accel_bias[1];
            accelZ += accel_bias[2];
        };
        
        void readGyroTemperatutre() {
            Wire.beginTransmission(MPU6050_ADDRESS);
            Wire.write(MPUREG_TEMP_OUT_H);
            Wire.endTransmission();
            
            Wire.requestFrom(MPU6050_ADDRESS, 2);
            
            gyro_temperature = (Wire.read() << 8) | Wire.read();         
        };
    private:
        int16_t gyro_offset[3];
        int16_t accel_bias[3];
};

MPU6050 mpu;

void SensorArray::initializeGyro() {
    mpu.initialize();
    mpu.calibrate_gyro();
}

void SensorArray::initializeAccel() {
}

void SensorArray::readGyroSum() {
    mpu.readGyroSum();
}

void SensorArray::readAccelSum() {
    mpu.readAccelSum();
}

void SensorArray::evaluateGyro() {
    mpu.evaluateGyro();
}

void SensorArray::evaluateAccel() {
    mpu.evaluateAccel();
}