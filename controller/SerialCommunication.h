#if defined(__MK20DX128__)
    #define CONFIGURATOR_DATA_SPEED_DIVIDER 4 // 100hz / divider(4) = 25Hz
#endif

#if defined(__AVR__)
    #define CONFIGURATOR_DATA_SPEED_DIVIDER 8 // 100hz / divider(8) = 12.5Hz
#endif

class Configurator {
    public:
        // Constructor
        Configurator() {
            state = 0;
            
            payload_length_expected = 0;
            payload_length_received = 0;
            
            output_counter = 0;
            output_sensor_data = 0;
            output_RX_data = 0;
            output_kinematics = 0;
            output_motor_out = 0;
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
                        crc = data;
                        
                        state++;
                    break;
                    case 3: // payload length MSB
                        payload_length_expected = 0; // reset
                        payload_length_expected = data << 8;
                        crc ^= data;
                        
                        state++;
                    break; // payload length LSB
                    case 4:
                        payload_length_expected |= data;
                        crc ^= data;
                        
                        state++;
                    break;
                    case 5: // payload
                        data_buffer[payload_length_received] = data;
                        crc ^= data;
                        payload_length_received++;
                        
                        if (payload_length_received >= payload_length_expected) {
                            state++;
                        }
                    break;
                    case 6: // CRC
                        if (crc == data) {
                            // CRC is ok, proecss data
                            process_data();
                        } else {
                            // respond that CRC failed
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
                case 3: // Requesting Sensor Data (gyro + accel)
                    ACK();
                    output_sensor_data = 1;
                break;
                case 4: // Requesting TX (RX) Data
                    ACK();
                    output_RX_data = 1;
                break;
                case 5: // Requesting 3D vehicle view
                    ACK();
                    output_kinematics = 1;
                break;
                case 6: // Requesting Motor Output
                    ACK();
                    output_motor_out = 1;
                break;
                case 7: // Disable all periodic data outputs
                    // there is no ACK in this case (this command is executed silently)
                    output_sensor_data = 0;
                    output_RX_data = 0;
                    output_kinematics = 0;
                    output_motor_out = 0;
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
                    Serial.write(6); // payload length LSB  
                    
                    for (uint8_t axis = 0; axis <= ZAXIS; axis++) {
                        Serial.write(highByte(CONFIG.data.ACCEL_BIAS[axis]));
                        Serial.write(lowByte(CONFIG.data.ACCEL_BIAS[axis]));
                    }
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
                    // Send over the accel calibration data
                    Serial.write(0xB5); // sync char 1
                    Serial.write(0x62); // sync char 2
                    Serial.write(0x0B); // command 11
                    Serial.write(0x00); // payload length MSB
                    Serial.write(0x01); // payload length LSB  
                    
                    Serial.write(MOTORS); // payload
                break;
                case 12: // Requesting sensors detected in current setup
                    Serial.write(0xB5); // sync char 1
                    Serial.write(0x62); // sync char 2
                    Serial.write(0x0C); // command 12
                    Serial.write(0x00); // payload length MSB
                    Serial.write(0x02); // payload length LSB  
                    
                    Serial.write(highByte(sensors.sensors_detected)); // payload high byte
                    Serial.write(lowByte(sensors.sensors_detected)); // payload low byte
                break;
                default: // Unrecognized command
                    REFUSED();
            }
        };
        
        void process_output() {
            output_counter++;
            if (output_counter >= CONFIGURATOR_DATA_SPEED_DIVIDER) {
                if (output_sensor_data) {
                    Serial.write(0xB5); // sync char 1
                    Serial.write(0x62); // sync char 2
                    Serial.write(0x03); // command
                    Serial.write(0x00); // payload length MSB
                    Serial.write(24); // payload length LSB
                    
                    // gyro
                    for (uint8_t axis = 0; axis <= ZAXIS; axis++) {
                        send_float(gyro[axis]);
                    }

                    // accel
                    float norm = sqrt(accel[XAXIS] * accel[XAXIS] + accel[YAXIS] * accel[YAXIS] + accel[ZAXIS] * accel[ZAXIS]);
                    
                    for (uint8_t axis = 0; axis <= ZAXIS; axis++) {
                        send_float((float)(accel[axis] / norm));
                    } 
                }
                
                if (output_RX_data) {
                    Serial.write(0xB5); // sync char 1
                    Serial.write(0x62); // sync char 2
                    Serial.write(0x04); // command
                    Serial.write(0x00); // payload length MSB
                    Serial.write(CHANNELS * 2); // payload length LSB  
                    
                    for (uint8_t channel = 0; channel < CHANNELS; channel++) {
                        Serial.write(highByte(RX[channel]));
                        Serial.write(lowByte(RX[channel]));
                    }                 
                }
                
                if (output_kinematics) {
                    Serial.write(0xB5); // sync char 1
                    Serial.write(0x62); // sync char 2
                    Serial.write(0x05); // command
                    Serial.write(0x00); // payload length MSB
                    Serial.write(12); // payload length LSB  

                    for (uint8_t axis = 0; axis <= ZAXIS; axis++) {
                        send_float(kinematicsAngle[axis]);
                    }                
                }
                
                if (output_motor_out) {
                    Serial.write(0xB5); // sync char 1
                    Serial.write(0x62); // sync char 2
                    Serial.write(0x06); // command
                    Serial.write(0x00); // payload length MSB
                    Serial.write(MOTORS * 2); // payload length LSB (* 2 because of 2 bytes for each motor) 
                    
                    for (uint8_t motor = 0; motor < MOTORS; motor++) {
                        Serial.write(highByte(MotorOut[motor]));
                        Serial.write(lowByte(MotorOut[motor]));
                    }
                }
                
                output_counter = 0; // reset
            }
        };
        
        void ACK() {
            Serial.write(0xB5); // sync char 1
            Serial.write(0x62); // sync char 2
            Serial.write(0x09); // command
            Serial.write(0x00); // payload length MSB
            Serial.write(0x01); // payload length LSB  
            Serial.write(0x01); // payload
        };
        
        void REFUSED() {
            Serial.write(0xB5); // sync char 1
            Serial.write(0x62); // sync char 2
            Serial.write(0x09); // command
            Serial.write(0x00); // payload length MSB
            Serial.write(0x01); // payload length LSB  
            Serial.write(0x00); // payload      
        };
        
        void send_UNION() {
            Serial.write(0xB5); // sync char 1
            Serial.write(0x62); // sync char 2
            Serial.write(0x01); // command
            Serial.write(highByte(sizeof(CONFIG))); // payload length MSB
            Serial.write(lowByte(sizeof(CONFIG))); // payload length LSB  
    
            for (uint16_t i = 0; i < sizeof(CONFIG); i++) {
                Serial.write(CONFIG.raw[i]);
            }  
        };
        
        void send_float(float f) {
            uint8_t *b = (uint8_t*) & f;
            
            for (uint8_t i = 0; i < sizeof(f); i++) {
                Serial.write(b[i]);
            }
        };
    
    private:
        uint8_t data; // variable used to store a single byte from serial
        
        uint8_t state;
        uint8_t command;
        uint8_t crc;
        
        uint16_t payload_length_expected;
        uint16_t payload_length_received;
        
        uint8_t data_buffer[200];

        uint8_t output_counter;
        bool output_sensor_data;
        bool output_RX_data;
        bool output_kinematics;
        bool output_motor_out;
} configurator;

void readSerial() {
    configurator.read_packet();
    configurator.process_output();
}