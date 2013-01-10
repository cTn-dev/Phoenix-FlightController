/*
    Basic structure and definitions taken from aeroquad.
*/

#define BMA180_ADDRESS 0x40
#define BMA180_IDENTITY 0x03
#define BMA180_RESET_REGISTER 0x10
#define BMA180_TRIGER_RESET_VALUE 0xB6
#define BMA180_ENABLE_WRITE_CONTROL_REGISTER 0x0D
#define BMA180_CONTROL_REGISTER 0x10
#define BMA180_BW_TCS 0x20
#define BMA180_LOW_PASS_FILTER_REGISTER 0x20
#define BMA180_10HZ_LOW_PASS_FILTER_VALUE 0x0F
#define BMA180_1200HZ_LOW_PASS_FILTER_VALUE 0X7F
#define BMA180_OFFSET_REGISTER 0x35
#define BMA180_READ_ROLL_ADDRESS 0x02
#define BMA180_READ_PITCH_ADDRESS 0x04
#define BMA180_READ_YAW_ADDRESS 0x06
#define BMA180_BUFFER_SIZE 6

int16_t accelX, accelY, accelZ;
double accelScaleFactor;

double accelXsum, accelYsum, accelZsum;
double accelXsumAvr, accelYsumAvr, accelZsumAvr;
uint8_t accelSamples = 0;

void initializeAccel() {
}

void readAccelSum() {
}

void evaluateAccel() {
}