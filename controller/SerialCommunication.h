/*  PSP - Phoenix Serial Protocol
    
    Inspired by uBlox serial protocol and multiwii serial protocol
    
    Data structure is subject to change without further notice.   
*/

#define PSP_SYNC1 0xB5
#define PSP_SYNC2 0x62

#define PSP_REQ_CONFIGURATION 1
#define PSP_REQ_GYRO_ACC      2
#define PSP_REQ_MAG           3
#define PSP_REQ_BARO          4
#define PSP_REQ_GPS           5
#define PSP_REQ_RC            6
#define PSP_REQ_KINEMATICS    7
#define PSP_REQ_MOTORS_OUTPUT 8
#define PSP_REQ_MOTORS_COUNT  9
#define PSP_REQ_SENSORS_ALIVE 10


#define PSP_SET_CONFIGURATION     101
#define PSP_SET_EEPROM_REINIT     102
#define PSP_SET_ACCEL_CALIBRATION 103
#define PSP_SET_MAG_CALIBRATION   104
#define PSP_SET_MOTOR_TEST_VALUE  105

#define PSP_INF_ACK      201
#define PSP_INF_REFUSED  202
#define PSP_INF_CRC_FAIL 203


class Configurator {
    public:
        // Constructor
        Configurator() {
            state = 0;
            
            payload_length_expected = 0;
            payload_length_received = 0;
        };
    
        void read_packet() {
            while (Serial.available()) {
                data = Serial.read();
                
                switch (state) {
                    case 0:
                        if (data == PSP_SYNC1) { // sync char 1  
                            state++;
                        }
                        break;
                    case 1:
                        if (data == PSP_SYNC2) { // sync char 2  
                            state++;
                        } else {
                            state = 0; // Restart and try again
                        }
                        break;
                    case 2: // code
                        code = data;
                        message_crc = data;
                        
                        state++;
                        break;
                    case 3: // payload length MSB
                        payload_length_expected = 0; // reset
                        payload_length_expected = data << 8;
                        message_crc ^= data;
                        
                        state++;
                        break; // payload length LSB
                    case 4:
                        payload_length_expected |= data;
                        message_crc ^= data;
                        
                        state++;
                        break;
                    case 5: // payload
                        data_buffer[payload_length_received] = data;
                        message_crc ^= data;
                        payload_length_received++;
                        
                        if (payload_length_received >= payload_length_expected) {
                            state++;
                        }
                        break;
                    case 6: // CRC
                        if (message_crc == data) {
                            // CRC is ok, proecss data
                            process_data();
                        } else {
                            // respond that CRC failed
                            CRC_FAILED(message_crc);
                        }
                        
                        // reset variables
                        memset(data_buffer, 0, sizeof(data_buffer));
                        
                        payload_length_received = 0;
                        state = 0;
                        break;
                }
            }        
        };
        
        void process_data() {
            switch (code) {
                case PSP_REQ_CONFIGURATION: // Requesting configuration union
                    send_UNION();
                    break;              
                case PSP_REQ_GYRO_ACC: { // Requesting Sensor Data (gyro + accel)
                    Serial.write(PSP_SYNC1); // sync char 1
                    Serial.write(PSP_SYNC2); // sync char 2
                    Serial.write(PSP_REQ_GYRO_ACC); // code
                    Serial.write(0x00); // payload length MSB
                    Serial.write(24); // payload length LSB
                    
                    uint8_t crc = PSP_REQ_GYRO_ACC ^ 0x00 ^ 24;
                    // gyro
                    for (uint8_t axis = 0; axis <= ZAXIS; axis++) {
                        crc = send_float(gyro[axis], crc);
                    }

                    // accel
                    float norm = sqrt(accel[XAXIS] * accel[XAXIS] + accel[YAXIS] * accel[YAXIS] + accel[ZAXIS] * accel[ZAXIS]);
                    
                    for (uint8_t axis = 0; axis <= ZAXIS; axis++) {
                        crc = send_float((float)(accel[axis] / norm), crc);
                    } 
                    
                    Serial.write(crc); // crc
                    }
                    break;
                case PSP_REQ_RC: {
                    Serial.write(PSP_SYNC1); // sync char 1
                    Serial.write(PSP_SYNC2); // sync char 2
                    Serial.write(PSP_REQ_RC); // code
                    Serial.write(0x00); // payload length MSB
                    Serial.write(CHANNELS * 2); // payload length LSB  
                    
                    uint8_t crc = PSP_REQ_RC ^ 0x00 ^ (CHANNELS * 2);
                    for (uint8_t channel = 0; channel < CHANNELS; channel++) {
                        Serial.write(highByte(RX[channel]));
                        Serial.write(lowByte(RX[channel]));
                        
                        crc ^= highByte(RX[channel]);
                        crc ^= lowByte(RX[channel]);
                    }
                    
                    Serial.write(crc); // crc
                    }
                    break;
                case PSP_REQ_KINEMATICS: {
                    Serial.write(PSP_SYNC1); // sync char 1
                    Serial.write(PSP_SYNC2); // sync char 2
                    Serial.write(PSP_REQ_KINEMATICS); // code
                    Serial.write(0x00); // payload length MSB
                    Serial.write(12); // payload length LSB  

                    uint8_t crc = PSP_REQ_KINEMATICS ^ 0x00 ^ 12;
                    for (uint8_t axis = 0; axis <= ZAXIS; axis++) {
                        crc = send_float(kinematicsAngle[axis], crc);
                    }

                    Serial.write(crc); // crc
                    }
                    break;
                case PSP_REQ_MOTORS_OUTPUT: {
                    Serial.write(PSP_SYNC1); // sync char 1
                    Serial.write(PSP_SYNC2); // sync char 2
                    Serial.write(PSP_REQ_MOTORS_OUTPUT); // code
                    Serial.write(0x00); // payload length MSB
                    Serial.write(MOTORS * 2); // payload length LSB (* 2 because of 2 bytes for each motor) 
                    
                    uint8_t crc = PSP_REQ_MOTORS_OUTPUT ^ 0x00 ^ (MOTORS * 2);
                    for (uint8_t motor = 0; motor < MOTORS; motor++) {
                        Serial.write(highByte(MotorOut[motor]));
                        Serial.write(lowByte(MotorOut[motor]));
                        
                        crc ^= highByte(MotorOut[motor]);
                        crc ^= lowByte(MotorOut[motor]);
                    }
                    
                    Serial.write(crc); // crc
                    }
                    break;
                case PSP_SET_ACCEL_CALIBRATION: { // Requesting Accel calibration
                    sensors.calibrateAccel();
                    
                    // Write config to EEPROM
                    writeEEPROM();

                    // Send over the accel calibration data
                    Serial.write(PSP_SYNC1); // sync char 1
                    Serial.write(PSP_SYNC2); // sync char 2
                    Serial.write(PSP_SET_ACCEL_CALIBRATION); // code
                    Serial.write(0x00); // payload length MSB
                    Serial.write(6);    // payload length LSB  
                    
                    uint8_t crc = PSP_SET_ACCEL_CALIBRATION ^ 0x00 ^ 6;
                    for (uint8_t axis = 0; axis <= ZAXIS; axis++) {
                        Serial.write(highByte(CONFIG.data.ACCEL_BIAS[axis]));
                        Serial.write(lowByte(CONFIG.data.ACCEL_BIAS[axis]));
                        
                        crc ^= highByte(CONFIG.data.ACCEL_BIAS[axis]);
                        crc ^= lowByte(CONFIG.data.ACCEL_BIAS[axis]);
                    }
                    
                    Serial.write(crc); // crc
                    }
                    break;
                case PSP_SET_EEPROM_REINIT:
                    ACK();
                    
                    initializeEEPROM(); // initializes default values
                    writeEEPROM(); // writes default values to eeprom

                    // Send back configuration union
                    send_UNION();                    
                    break;
                case PSP_SET_MOTOR_TEST_VALUE:
                    // data_buffer should contain 2 bytes (byte 0 = motor number, byte 1 = value)
                    if (data_buffer[0] < MOTORS) { // Check if motor number is within our setup
                        MotorOut[data_buffer[0]] = 1000 + (data_buffer[1] * 10);
                        updateMotors(); // Update ESCs
                    } else { // Motor number is not in our setup
                        REFUSED();
                    }
                    break;
                case PSP_REQ_MOTORS_COUNT:
                    Serial.write(PSP_SYNC1); // sync char 1
                    Serial.write(PSP_SYNC2); // sync char 2
                    Serial.write(PSP_REQ_MOTORS_COUNT); // code 11
                    Serial.write(0x00); // payload length MSB
                    Serial.write(0x01); // payload length LSB  
                    
                    Serial.write(MOTORS); // payload
                    
                    Serial.write(PSP_REQ_MOTORS_COUNT ^ 0x00 ^ 0x01 ^ MOTORS); // crc
                    break;
                case PSP_REQ_SENSORS_ALIVE:
                    Serial.write(PSP_SYNC1); // sync char 1
                    Serial.write(PSP_SYNC2); // sync char 2
                    Serial.write(PSP_REQ_SENSORS_ALIVE); // code
                    Serial.write(0x00); // payload length MSB
                    Serial.write(0x02); // payload length LSB  
                    
                    Serial.write(highByte(sensors.sensors_detected)); // payload high byte
                    Serial.write(lowByte(sensors.sensors_detected)); // payload low byte
                    
                    Serial.write(PSP_REQ_SENSORS_ALIVE ^ 0x00 ^ 0x02 ^ highByte(sensors.sensors_detected) ^ lowByte(sensors.sensors_detected)); // crc
                    break;
                    
                // SET
                case PSP_SET_CONFIGURATION:
                    if (payload_length_received == sizeof(CONFIG)) {
                        // process data from buffer (throw it inside union)
                        for (uint16_t i = 0; i < sizeof(CONFIG); i++) {
                            CONFIG.raw[i] = data_buffer[i];
                        }

                        // Write config to EEPROM
                        writeEEPROM();

                        ACK();
                    } else {
                        // Refuse (buffer size doesn't match struct memory size)
                        REFUSED();
                    }
                    break;                
                default: // Unrecognized code
                    REFUSED();
            }
        };
        
        void ACK() {
            Serial.write(PSP_SYNC1); // sync char 1
            Serial.write(PSP_SYNC2); // sync char 2
            Serial.write(PSP_INF_ACK); // code
            Serial.write(0x00); // payload length MSB
            Serial.write(0x01); // payload length LSB  
            Serial.write(0x01); // payload
            Serial.write(PSP_INF_ACK ^ 0x00 ^ 0x01 ^ 0x01); // crc
        };
        
        void REFUSED() {
            Serial.write(PSP_SYNC1); // sync char 1
            Serial.write(PSP_SYNC2); // sync char 2
            Serial.write(PSP_INF_REFUSED); // code
            Serial.write(0x00); // payload length MSB
            Serial.write(0x01); // payload length LSB  
            Serial.write(0x00); // payload
            Serial.write(PSP_INF_REFUSED ^ 0x00 ^ 0x01 ^ 0x00); // crc
        };
        
        void CRC_FAILED(uint8_t crc) {
            Serial.write(PSP_SYNC1); // sync char 1
            Serial.write(PSP_SYNC2); // sync char 2
            Serial.write(PSP_INF_CRC_FAIL); // code
            Serial.write(0x00); // payload length MSB
            Serial.write(0x02); // payload length LSB  
            Serial.write(code); // payload 1
            Serial.write(crc);  // payload 2
            Serial.write(PSP_INF_CRC_FAIL ^ 0x00 ^ 0x02 ^ code ^ crc); // crc
        };
        
        void send_UNION() {
            Serial.write(PSP_SYNC1); // sync char 1
            Serial.write(PSP_SYNC2); // sync char 2
            Serial.write(PSP_REQ_CONFIGURATION); // code
            Serial.write(highByte(sizeof(CONFIG))); // payload length MSB
            Serial.write(lowByte(sizeof(CONFIG))); // payload length LSB  
    
            uint8_t crc = PSP_REQ_CONFIGURATION ^ highByte(sizeof(CONFIG)) ^ lowByte(sizeof(CONFIG));
            for (uint16_t i = 0; i < sizeof(CONFIG); i++) {
                Serial.write(CONFIG.raw[i]);
                crc ^= CONFIG.raw[i];
            }
            
            Serial.write(crc); // crc
        };
        
        uint8_t send_float(float f, uint8_t crc) {
            uint8_t *b = (uint8_t*) & f;
            
            for (uint8_t i = 0; i < sizeof(f); i++) {
                Serial.write(b[i]);
                crc ^= b[i];
            }
            
            return crc;
        };
    
    private:
        uint8_t data; // variable used to store a single byte from serial
        
        uint8_t state;
        uint8_t code;
        uint8_t message_crc;
        
        uint16_t payload_length_expected;
        uint16_t payload_length_received;
        
        uint8_t data_buffer[200];
} configurator;

void readSerial() {
    configurator.read_packet();
}