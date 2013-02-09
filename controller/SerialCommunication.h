class Configurator {
    public:
        // Constructor
        Configurator() {
            state = 0;
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
                        
                        state++;
                    break;
                    case 3: // payload length MSB
                        payload_length_expected = 0; // reset
                        payload_length_expected = data << 8;
                        
                        state++;
                    break; // payload length LSB
                    case 4:
                        payload_length_expected |= data;
                        
                        state++;
                    break;
                    case 5: // payload
                        data_buffer[payload_length_received] = data;
                        payload_length_received++;
                        
                        if (payload_length_received >= payload_length_expected) {
                            process_data();
                            
                            // reset variables
                            memset(command_buffer, 0, sizeof(command_buffer));
                            memset(data_buffer, 0, sizeof(data_buffer));
                            
                            payload_length_received = 0;
                            
                            state = 0;
                        }
                    break;
                }
            }        
        };
        
        void process_data() {            
            switch (command) {
                case 1: // Requesting configuration union                    
                    ACK();
                    
                    Serial.write(0xB5); // sync char 1
                    Serial.write(0x62); // sync char 2
                    Serial.write(0x01); // command
                    Serial.write(highByte(sizeof(CONFIG_struct))); // payload length MSB
                    Serial.write(highByte(sizeof(CONFIG_struct))); // payload length LSB  
            
                    for (uint16_t i = 0; i < sizeof(CONFIG_struct); i++) {
                        Serial.write(CONFIG.raw[i]);
                    }              
                break;
                case 2: // Received configuration union
                    if (payload_length_received == sizeof(CONFIG_struct)) {
                        // process data from buffer (throw it inside union)
                        for (uint16_t i = 0; i < sizeof(data_buffer); i++) {
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
                default: // Unrecognized command
                    REFUSED();
            }
        };
        
        void process_output() {
            if (output_sensor_data) {
                Serial.write(0xB5); // sync char 1
                Serial.write(0x62); // sync char 2
                Serial.write(0x03); // command
                Serial.write(0x00); // payload length MSB
                Serial.write(12); // payload length LSB
                
                // gyro
                for (uint8_t axis = 0; axis <= ZAXIS; axis++) {
                    Serial.write(highByte((int16_t) (gyro[axis] * gyro_scale)));
                    Serial.write(lowByte((int16_t) (gyro[axis] * gyro_scale)));
                }

                // accel
                for (uint8_t axis = 0; axis <= ZAXIS; axis++) {
                    Serial.write(highByte((int16_t) (accel[axis] * accel_scale)));
                    Serial.write(lowByte((int16_t) (accel[axis] * accel_scale)));
                } 
            }
            
            if (output_RX_data) {
                Serial.write(0xB5); // sync char 1
                Serial.write(0x62); // sync char 2
                Serial.write(0x04); // command
                Serial.write(0x00); // payload length MSB
                Serial.write(16); // payload length LSB  
                
                for (uint8_t channel = 0; channel <= PPM_CHANNELS; channel++) {
                    Serial.write(highByte((int16_t) (RX[channel] * rx_scale)));
                    Serial.write(lowByte((int16_t) (RX[channel] * rx_scale)));
                }                 
            }
            
            if (output_kinematics) {
                Serial.write(0xB5); // sync char 1
                Serial.write(0x62); // sync char 2
                Serial.write(0x05); // command
                Serial.write(0x00); // payload length MSB
                Serial.write(6); // payload length LSB  

                for (uint8_t axis = 0; axis <= ZAXIS; axis++) {
                    Serial.write(highByte((int16_t) (kinematicsAngle[axis] * kinematics_scale)));
                    Serial.write(lowByte((int16_t) (kinematicsAngle[axis] * kinematics_scale)));
                }                
            }
            
            if (output_motor_out) {
                #if MOTORS == 3
                    Serial.write(0xB5); // sync char 1
                    Serial.write(0x62); // sync char 2
                    Serial.write(0x06); // command
                    Serial.write(0x00); // payload length MSB
                    Serial.write(6); // payload length LSB  
                    
                    for (uint8_t motor = 0; motor <= MOTORS; motor++) {
                        Serial.write(highByte((int16_t) (MotorOut[motor] * motor_scale)));
                        Serial.write(lowByte((int16_t) (MotorOut[motor] * motor_scale)));
                    }                     
                #elif MOTORS == 4
                    Serial.write(0xB5); // sync char 1
                    Serial.write(0x62); // sync char 2
                    Serial.write(0x06); // command
                    Serial.write(0x00); // payload length MSB
                    Serial.write(8); // payload length LSB  
                    
                    for (uint8_t motor = 0; motor <= MOTORS; motor++) {
                        Serial.write(highByte((int16_t) (MotorOut[motor] * motor_scale)));
                        Serial.write(lowByte((int16_t) (MotorOut[motor] * motor_scale)));
                    }                    
                #elif MOTORS == 6
                    Serial.write(0xB5); // sync char 1
                    Serial.write(0x62); // sync char 2
                    Serial.write(0x06); // command
                    Serial.write(0x00); // payload length MSB
                    Serial.write(12); // payload length LSB  
                    
                    for (uint8_t motor = 0; motor <= MOTORS; motor++) {
                        Serial.write(highByte((int16_t) (MotorOut[motor] * motor_scale)));
                        Serial.write(lowByte((int16_t) (MotorOut[motor] * motor_scale)));
                    }                      
                #elif MOTORS == 8
                    Serial.write(0xB5); // sync char 1
                    Serial.write(0x62); // sync char 2
                    Serial.write(0x06); // command
                    Serial.write(0x00); // payload length MSB
                    Serial.write(16); // payload length LSB  
                    
                    for (uint8_t motor = 0; motor <= MOTORS; motor++) {
                        Serial.write(highByte((int16_t) (MotorOut[motor] * motor_scale)));
                        Serial.write(lowByte((int16_t) (MotorOut[motor] * motor_scale)));
                    }                    
                #endif
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
    
    private:
        char data; // variable used to store a single byte from serial
        
        uint8_t state;
        
        char command_buffer[4];
        uint8_t command;
        
        uint16_t payload_length_expected = 0;
        uint16_t payload_length_received = 0;
        
        char data_buffer[300]; // Current UNION size = 264 bytes = 2112 bits

        bool output_sensor_data = 0;
        bool output_RX_data = 0;
        bool output_kinematics = 0;
        bool output_motor_out = 0;
        
        // Scale factors used to transmit double/float data over serial with just 2 bytes
        int16_t gyro_scale = 65535.0 / 20.0;
        int16_t accel_scale = 65535.0 / 3.0;   
        int16_t kinematics_scale = 65535.0 / TWO_PI;
        int16_t rx_scale = 65535.0 / 4000.0;
        int16_t motor_scale = 65535.0 / 4000.0;
} configurator;

void readSerial() {
    configurator.read_packet();
    configurator.process_output();
}