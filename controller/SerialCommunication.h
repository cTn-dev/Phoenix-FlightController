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
                    Serial.write(highByte(sizeof(CONFIG_struct))); // payload LSB  
            
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
                default: // Unrecognized command
                    REFUSED();
            }
        };
        
        void process_output() {
            if (output_sensor_data) {
                dataType = 3;
                byte vBuffer[12];    
                
                // Gyro
                vBuffer[0] = highByte((int16_t) (gyro[XAXIS] * gyro_scale));
                vBuffer[1] = lowByte((int16_t) (gyro[XAXIS] * gyro_scale));
                vBuffer[2] = highByte((int16_t) (gyro[YAXIS] * gyro_scale));
                vBuffer[3] = lowByte((int16_t) (gyro[YAXIS] * gyro_scale));
                vBuffer[4] = highByte((int16_t) (gyro[ZAXIS] * gyro_scale));
                vBuffer[5] = lowByte((int16_t) (gyro[ZAXIS] * gyro_scale)); 
                
                // Accel
                vBuffer[6] = highByte((int16_t) (accel[XAXIS] * accel_scale));
                vBuffer[7] = lowByte((int16_t) (accel[XAXIS] * accel_scale));
                vBuffer[8] = highByte((int16_t) (accel[YAXIS] * accel_scale));
                vBuffer[9] = lowByte((int16_t) (accel[YAXIS] * accel_scale));
                vBuffer[10] = highByte((int16_t) (accel[ZAXIS] * accel_scale));
                vBuffer[11] = lowByte((int16_t) (accel[ZAXIS] * accel_scale));   
            }
            
            if (output_RX_data) {
                dataType = 4;
                vBuffer[16];

                vBuffer[0] = highByte((int16_t) (RX[0] * rx_scale));
                vBuffer[1] = lowByte((int16_t) (RX[0] * rx_scale));
                vBuffer[2] = highByte((int16_t) (RX[1] * rx_scale));
                vBuffer[3] = lowByte((int16_t) (RX[1] * rx_scale));
                vBuffer[4] = highByte((int16_t) (RX[2] * rx_scale));
                vBuffer[5] = lowByte((int16_t) (RX[2] * rx_scale));
                vBuffer[6] = highByte((int16_t) (RX[3] * rx_scale));
                vBuffer[7] = lowByte((int16_t) (RX[3] * rx_scale));
                vBuffer[8] = highByte((int16_t) (RX[4] * rx_scale));
                vBuffer[9] = lowByte((int16_t) (RX[4] * rx_scale));
                vBuffer[10] = highByte((int16_t) (RX[5] * rx_scale));
                vBuffer[11] = lowByte((int16_t) (RX[5] * rx_scale));
                vBuffer[12] = highByte((int16_t) (RX[6] * rx_scale));
                vBuffer[13] = lowByte((int16_t) (RX[6] * rx_scale));
                vBuffer[14] = highByte((int16_t) (RX[7] * rx_scale));
                vBuffer[15] = lowByte((int16_t) (RX[7] * rx_scale));                
            }
            
            if (output_kinematics) {
                dataType = 5;
                vBuffer[6];   
                
                vBuffer[0] = highByte((int16_t) (kinematicsAngleX * kinematics_scale));
                vBuffer[1] = lowByte((int16_t) (kinematicsAngleX * kinematics_scale));
                vBuffer[2] = highByte((int16_t) (kinematicsAngleY * kinematics_scale));
                vBuffer[3] = lowByte((int16_t) (kinematicsAngleY * kinematics_scale));
                vBuffer[4] = highByte((int16_t) (kinematicsAngleZ * kinematics_scale));
                vBuffer[5] = lowByte((int16_t) (kinematicsAngleZ * kinematics_scale));              
            }
            
            if (output_motor_out) {
                dataType = 6;
                #if MOTORS == 3
                    vBuffer[6];
                    
                    vBuffer[0] = highByte((int16_t) (MotorOut[0] * motor_scale));
                    vBuffer[1] = lowByte((int16_t) (MotorOut[0] * motor_scale));
                    vBuffer[2] = highByte((int16_t) (MotorOut[1] * motor_scale));
                    vBuffer[3] = lowByte((int16_t) (MotorOut[1] * motor_scale));
                    vBuffer[4] = highByte((int16_t) (MotorOut[2] * motor_scale));
                    vBuffer[5] = lowByte((int16_t) (MotorOut[2] * motor_scale));
                #elif MOTORS == 4
                    vBuffer[8];
                    
                    vBuffer[0] = highByte((int16_t) (MotorOut[0] * motor_scale));
                    vBuffer[1] = lowByte((int16_t) (MotorOut[0] * motor_scale));
                    vBuffer[2] = highByte((int16_t) (MotorOut[1] * motor_scale));
                    vBuffer[3] = lowByte((int16_t) (MotorOut[1] * motor_scale));
                    vBuffer[4] = highByte((int16_t) (MotorOut[2] * motor_scale));
                    vBuffer[5] = lowByte((int16_t) (MotorOut[2] * motor_scale));
                    vBuffer[6] = highByte((int16_t) (MotorOut[3] * motor_scale));
                    vBuffer[7] = lowByte((int16_t) (MotorOut[3] * motor_scale));
                #elif MOTORS == 6
                    vBuffer[12];
                    
                    vBuffer[0] = highByte((int16_t) (MotorOut[0] * motor_scale));
                    vBuffer[1] = lowByte((int16_t) (MotorOut[0] * motor_scale));
                    vBuffer[2] = highByte((int16_t) (MotorOut[1] * motor_scale));
                    vBuffer[3] = lowByte((int16_t) (MotorOut[1] * motor_scale));
                    vBuffer[4] = highByte((int16_t) (MotorOut[2] * motor_scale));
                    vBuffer[5] = lowByte((int16_t) (MotorOut[2] * motor_scale));
                    vBuffer[6] = highByte((int16_t) (MotorOut[3] * motor_scale));
                    vBuffer[7] = lowByte((int16_t) (MotorOut[3] * motor_scale));
                    vBuffer[8] = highByte((int16_t) (MotorOut[4] * motor_scale));
                    vBuffer[9] = lowByte((int16_t) (MotorOut[4] * motor_scale));
                    vBuffer[10] = highByte((int16_t) (MotorOut[5] * motor_scale));
                    vBuffer[11] = lowByte((int16_t) (MotorOut[5] * motor_scale));                    
                #elif MOTORS == 8
                    vBuffer[16];
                    
                    vBuffer[0] = highByte((int16_t) (MotorOut[0] * motor_scale));
                    vBuffer[1] = lowByte((int16_t) (MotorOut[0] * motor_scale));
                    vBuffer[2] = highByte((int16_t) (MotorOut[1] * motor_scale));
                    vBuffer[3] = lowByte((int16_t) (MotorOut[1] * motor_scale));
                    vBuffer[4] = highByte((int16_t) (MotorOut[2] * motor_scale));
                    vBuffer[5] = lowByte((int16_t) (MotorOut[2] * motor_scale));
                    vBuffer[6] = highByte((int16_t) (MotorOut[3] * motor_scale));
                    vBuffer[7] = lowByte((int16_t) (MotorOut[3] * motor_scale));
                    vBuffer[8] = highByte((int16_t) (MotorOut[4] * motor_scale));
                    vBuffer[9] = lowByte((int16_t) (MotorOut[4] * motor_scale));
                    vBuffer[10] = highByte((int16_t) (MotorOut[5] * motor_scale));
                    vBuffer[11] = lowByte((int16_t) (MotorOut[5] * motor_scale));  
                    vBuffer[12] = highByte((int16_t) (MotorOut[6] * motor_scale));
                    vBuffer[13] = lowByte((int16_t) (MotorOut[6] * motor_scale));  
                    vBuffer[14] = highByte((int16_t) (MotorOut[7] * motor_scale));
                    vBuffer[15] = lowByte((int16_t) (MotorOut[7] * motor_scale));  
                #endif
            }
            
            if (output_sensor_data || output_RX_data || output_kinematics || output_motor_out) {
                Serial.write(0x5B); // [
                Serial.write(dataType); // DataType
                Serial.write(0x3A); // :
                Serial.write(vBuffer, sizeof(vBuffer));
                Serial.write(0x5D); // ] 
            }
        };
        
        void ACK() {
            Serial.write(0xB5); // sync char 1
            Serial.write(0x62); // sync char 2
            Serial.write(0x09); // command
            Serial.write((uint8_t) 0x00); // payload length MSB
            Serial.write(0x01); // payload LSB  
            Serial.write(0x01); // payload
        };
        
        void REFUSED() {
            Serial.write(0xB5); // sync char 1
            Serial.write(0x62); // sync char 2
            Serial.write(0x09); // command
            Serial.write((uint8_t) 0x00); // payload length MSB
            Serial.write(0x01); // payload LSB  
            Serial.write((uint8_t) 0x00); // payload      
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
        
        // Variables used in the data output section
        uint8_t dataType;
        uint8_t vBuffer[];
        
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