/*  PSP - Phoenix Serial Protocol
    
    Inspired by uBlox serial protocol and multiwii serial protocol
    
    Data structure is subject to change without further notice.   
*/

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
                        if (data == 0xB5) { // sync char 1  
                            state++;
                        }
                        break;
                    case 1:
                        if (data == 0x62) { // sync char 2  
                            state++;
                        } else {
                            state = 0; // Restart and try again
                        }
                        break;
                    case 2: // command
                        command = data;
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
            switch (command) {
                case 1: // Requesting configuration union                    
                    ACK();
                    
                    send_UNION();
                    break;
                case 2: // Received configuration union
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
                case 3: { // Requesting Sensor Data (gyro + accel)
                    Serial.write(0xB5); // sync char 1
                    Serial.write(0x62); // sync char 2
                    Serial.write(0x03); // command
                    Serial.write(0x00); // payload length MSB
                    Serial.write(24); // payload length LSB
                    
                    uint8_t crc = 0x03 ^ 0x00 ^ 24;
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
                case 4: { // Requesting TX (RX) Data
                    Serial.write(0xB5); // sync char 1
                    Serial.write(0x62); // sync char 2
                    Serial.write(0x04); // command
                    Serial.write(0x00); // payload length MSB
                    Serial.write(CHANNELS * 2); // payload length LSB  
                    
                    uint8_t crc = 0x04 ^ 0x00 ^ (CHANNELS * 2);
                    for (uint8_t channel = 0; channel < CHANNELS; channel++) {
                        Serial.write(highByte(RX[channel]));
                        Serial.write(lowByte(RX[channel]));
                        
                        crc ^= highByte(RX[channel]);
                        crc ^= lowByte(RX[channel]);
                    }
                    
                    Serial.write(crc); // crc
                    }
                    break;
                case 5: { // Requesting 3D vehicle view
                    Serial.write(0xB5); // sync char 1
                    Serial.write(0x62); // sync char 2
                    Serial.write(0x05); // command
                    Serial.write(0x00); // payload length MSB
                    Serial.write(12); // payload length LSB  

                    uint8_t crc = 0x05 ^ 0x00 ^ 12;
                    for (uint8_t axis = 0; axis <= ZAXIS; axis++) {
                        crc = send_float(kinematicsAngle[axis], crc);
                    }

                    Serial.write(crc); // crc
                    }
                    break;
                case 6: {// Requesting Motor Output
                    Serial.write(0xB5); // sync char 1
                    Serial.write(0x62); // sync char 2
                    Serial.write(0x06); // command
                    Serial.write(0x00); // payload length MSB
                    Serial.write(MOTORS * 2); // payload length LSB (* 2 because of 2 bytes for each motor) 
                    
                    uint8_t crc = 0x06 ^ 0x00 ^ (MOTORS * 2);
                    for (uint8_t motor = 0; motor < MOTORS; motor++) {
                        Serial.write(highByte(MotorOut[motor]));
                        Serial.write(lowByte(MotorOut[motor]));
                        
                        crc ^= highByte(MotorOut[motor]);
                        crc ^= lowByte(MotorOut[motor]);
                    }
                    
                    Serial.write(crc); // crc
                    }
                    break;
                case 8: { // Requesting Accel calibration
                    sensors.calibrateAccel();
                    
                    // Write config to EEPROM
                    writeEEPROM();
                    
                    ACK(); // Ackowledge when calibration is done  

                    // Send over the accel calibration data
                    Serial.write(0xB5); // sync char 1
                    Serial.write(0x62); // sync char 2
                    Serial.write(0x08); // command
                    Serial.write(0x00); // payload length MSB
                    Serial.write(6);    // payload length LSB  
                    
                    uint8_t crc = 0x08 ^ 0x00 ^ 6;
                    for (uint8_t axis = 0; axis <= ZAXIS; axis++) {
                        Serial.write(highByte(CONFIG.data.ACCEL_BIAS[axis]));
                        Serial.write(lowByte(CONFIG.data.ACCEL_BIAS[axis]));
                        
                        crc ^= highByte(CONFIG.data.ACCEL_BIAS[axis]);
                        crc ^= lowByte(CONFIG.data.ACCEL_BIAS[axis]);
                    }
                    
                    Serial.write(crc); // crc
                    }
                    break;
                case 9: // Requesting eeprom re-initialization
                    ACK();
                    
                    initializeEEPROM(); // initializes default values
                    writeEEPROM(); // writes default values to eeprom

                    // Send back configuration union
                    send_UNION();                    
                    break;
                case 10: // Sending motor command
                    // data_buffer should contain 2 bytes (byte 0 = motor number, byte 1 = value)
                    if (data_buffer[0] < MOTORS) { // Check if motor number is within our setup
                        MotorOut[data_buffer[0]] = 1000 + (data_buffer[1] * 10);
                        updateMotors(); // Update ESCs
                    } else { // Motor number is not in our setup
                        REFUSED();
                    }
                    break;
                case 11: // Requesting amount of motors used in current setup
                    Serial.write(0xB5); // sync char 1
                    Serial.write(0x62); // sync char 2
                    Serial.write(0x0B); // command 11
                    Serial.write(0x00); // payload length MSB
                    Serial.write(0x01); // payload length LSB  
                    
                    Serial.write(MOTORS); // payload
                    
                    Serial.write(0x0B ^ 0x00 ^ 0x01 ^ MOTORS); // crc
                    break;
                case 12: // Requesting sensors detected in current setup
                    Serial.write(0xB5); // sync char 1
                    Serial.write(0x62); // sync char 2
                    Serial.write(0x0C); // command 12
                    Serial.write(0x00); // payload length MSB
                    Serial.write(0x02); // payload length LSB  
                    
                    Serial.write(highByte(sensors.sensors_detected)); // payload high byte
                    Serial.write(lowByte(sensors.sensors_detected)); // payload low byte
                    
                    Serial.write(0x0C ^ 0x00 ^ 0x02 ^ highByte(sensors.sensors_detected) ^ lowByte(sensors.sensors_detected)); // crc
                    break;
                default: // Unrecognized command
                    REFUSED();
            }
        };
        
        void ACK() {
            Serial.write(0xB5); // sync char 1
            Serial.write(0x62); // sync char 2
            Serial.write(0x09); // command
            Serial.write(0x00); // payload length MSB
            Serial.write(0x01); // payload length LSB  
            Serial.write(0x01); // payload
            Serial.write(0x09 ^ 0x00 ^ 0x01 ^ 0x01); // crc
        };
        
        void REFUSED() {
            Serial.write(0xB5); // sync char 1
            Serial.write(0x62); // sync char 2
            Serial.write(0x09); // command
            Serial.write(0x00); // payload length MSB
            Serial.write(0x01); // payload length LSB  
            Serial.write(0x00); // payload
            Serial.write(0x09 ^ 0x00 ^ 0x01 ^ 0x00); // crc
        };
        
        void CRC_FAILED(uint8_t crc) {
            Serial.write(0xB5); // sync char 1
            Serial.write(0x62); // sync char 2
            Serial.write(0x15); // command
            Serial.write(0x00); // payload length MSB
            Serial.write(0x01); // payload length LSB  
            Serial.write(crc);  // payload
            Serial.write(0x15 ^ 0x00 ^ 0x01 ^ crc); // crc
        };
        
        void send_UNION() {
            Serial.write(0xB5); // sync char 1
            Serial.write(0x62); // sync char 2
            Serial.write(0x01); // command
            Serial.write(highByte(sizeof(CONFIG))); // payload length MSB
            Serial.write(lowByte(sizeof(CONFIG))); // payload length LSB  
    
            uint8_t crc = 0x01 ^ highByte(sizeof(CONFIG)) ^ lowByte(sizeof(CONFIG));
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
        uint8_t command;
        uint8_t message_crc;
        
        uint16_t payload_length_expected;
        uint16_t payload_length_received;
        
        uint8_t data_buffer[200];
} configurator;

void readSerial() {
    configurator.read_packet();
}