/*  PSP - Phoenix Serial Protocol
    
    Inspired by uBlox serial protocol and multiwii serial protocol
    
    Data structure is subject to change without further notice.  

    Protocol data structure:
    [SYNC1][SYNC2][CODE][LENGTH_H][LENGTH_L][DATA/DATA ARRAY][CRC]
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
#define PSP_REQ_AUX_TRIGGERED 11


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
                        if (data == PSP_SYNC1) {
                            state++;
                        }
                        break;
                    case 1:
                        if (data == PSP_SYNC2) {
                            state++;
                        } else {
                            state = 0; // Restart and try again
                        }
                        break;
                    case 2:
                        code = data;
                        message_crc = data;
                        
                        state++;
                        break;
                    case 3:
                        payload_length_expected = 0; // reset
                        payload_length_expected = data << 8;
                        message_crc ^= data;
                        
                        state++;
                        break;
                    case 4:
                        payload_length_expected |= data;
                        message_crc ^= data;
                        
                        state++;
                        break;
                    case 5:
                        data_buffer[payload_length_received] = data;
                        message_crc ^= data;
                        payload_length_received++;
                        
                        if (payload_length_received >= payload_length_expected) {
                            state++;
                        }
                        break;
                    case 6:
                        if (message_crc == data) {
                            // CRC is ok, process data
                            process_data();
                        } else {
                            // respond that CRC failed
                            CRC_FAILED(code, message_crc);
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
                case PSP_REQ_CONFIGURATION:
                    send_UNION();
                    break;              
                case PSP_REQ_GYRO_ACC: {
                    protocol_head(PSP_REQ_GYRO_ACC, 24);
                    
                    // gyro
                    for (uint8_t axis = 0; axis <= ZAXIS; axis++) {
                        serialize_float32(gyro[axis]);
                    }

                    // accel
                    float norm = sqrt(accel[XAXIS] * accel[XAXIS] + accel[YAXIS] * accel[YAXIS] + accel[ZAXIS] * accel[ZAXIS]);
                    
                    for (uint8_t axis = 0; axis <= ZAXIS; axis++) {
                        serialize_float32((float)(accel[axis] / norm));
                    } 
                    
                    }
                    break;
#ifdef Magnetometer                    
                case PSP_REQ_MAG:
                    protocol_head(PSP_REQ_MAG, 6);
                    
                    for (uint8_t axis = 0; axis <= ZAXIS; axis++) {
                        serialize_uint8(lowByte(magRaw[axis]));
                        serialize_uint8(highByte(magRaw[axis]));
                    }
                    break;
#endif
#ifdef AltitudeHoldBaro
                case PSP_REQ_BARO:
                    protocol_head(PSP_REQ_BARO, 8);

                    serialize_float32(baroRawAltitude);
                    serialize_float32(baroAltitude);
                    break;
#endif
#ifdef GPS
                // TODO
                /*
                case PSP_REQ_GPS: {
                    }
                    break;
                */
#endif                    
                case PSP_REQ_RC:
                    protocol_head(PSP_REQ_RC, CHANNELS * 2);
                    
                    for (uint8_t channel = 0; channel < CHANNELS; channel++) {
                        serialize_uint8(highByte(RX[channel]));
                        serialize_uint8(lowByte(RX[channel]));
                    }
                    break;
                case PSP_REQ_KINEMATICS:
                    protocol_head(PSP_REQ_KINEMATICS, 12);

                    for (uint8_t axis = 0; axis <= ZAXIS; axis++) {
                        serialize_float32(kinematicsAngle[axis]);
                    }
                    break;
                case PSP_REQ_MOTORS_OUTPUT:
                    protocol_head(PSP_REQ_MOTORS_OUTPUT, MOTORS * 2);

                    for (uint8_t motor = 0; motor < MOTORS; motor++) {
                        serialize_uint8(highByte(MotorOut[motor]));
                        serialize_uint8(lowByte(MotorOut[motor]));
                    }
                    break;
                case PSP_REQ_MOTORS_COUNT:
                    protocol_head(PSP_REQ_MOTORS_COUNT, 1);
                    
                    serialize_uint8(MOTORS);
                    break;
                case PSP_REQ_SENSORS_ALIVE:
                    protocol_head(PSP_REQ_SENSORS_ALIVE, 2);
                    
                    serialize_uint8(highByte(sensors.sensors_detected));
                    serialize_uint8(lowByte(sensors.sensors_detected));
                    break;
                case PSP_REQ_AUX_TRIGGERED:
                    protocol_head(PSP_REQ_AUX_TRIGGERED, 2);
                    
                    serialize_uint8(highByte(AUX_chan_mask));
                    serialize_uint8(lowByte(AUX_chan_mask));
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
                case PSP_SET_EEPROM_REINIT:
                    ACK();
                    
                    initializeEEPROM(); // initializes default values
                    writeEEPROM(); // writes default values to eeprom

                    // Send back configuration union                    
                    send_UNION();                    
                    break;
                case PSP_SET_ACCEL_CALIBRATION:
                    sensors.calibrateAccel();
                    
                    // Write config to EEPROM
                    writeEEPROM();

                    // Send over the accel calibration data
                    protocol_head(PSP_SET_ACCEL_CALIBRATION, 6);
                    
                    for (uint8_t axis = 0; axis <= ZAXIS; axis++) {
                        serialize_uint8(highByte(CONFIG.data.ACCEL_BIAS[axis]));
                        serialize_uint8(lowByte(CONFIG.data.ACCEL_BIAS[axis]));
                    }
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
                default: // Unrecognized code
                    REFUSED();
            }
            
            // send over crc
            protocol_tail();
        };
        
        void protocol_head(uint8_t code, uint16_t length) {
            Serial.write(PSP_SYNC1);
            Serial.write(PSP_SYNC2);
        
            crc = 0; // reset crc
            
            serialize_uint8(code);
            serialize_uint8(highByte(length));
            serialize_uint8(lowByte(length));
        };
        
        void protocol_tail() {
            Serial.write(crc);
        };
        
        void serialize_uint8(uint8_t data) {
            Serial.write(data);
            crc ^= data;
        };
        
        void serialize_float32(float f) {
            uint8_t *b = (uint8_t*) & f;
            
            for (uint8_t i = 0; i < sizeof(f); i++) {
                serialize_uint8(b[i]);
            }
        };
        
        void ACK() {
            protocol_head(PSP_INF_ACK, 1);
            
            serialize_uint8(0x01);
        };
        
        void REFUSED() {
            protocol_head(PSP_INF_REFUSED, 1);
            
            serialize_uint8(0x00);
        };
        
        void CRC_FAILED(uint8_t code, uint8_t failed_crc) {
            protocol_head(PSP_INF_CRC_FAIL, 2);
            
            serialize_uint8(code);
            serialize_uint8(failed_crc);
        };
        
        void send_UNION() {
            protocol_head(PSP_REQ_CONFIGURATION, sizeof(CONFIG));

            for (uint16_t i = 0; i < sizeof(CONFIG); i++) {
                serialize_uint8(CONFIG.raw[i]);
            }
        };
    
    private:
        uint8_t data; // variable used to store a single byte from serial
        
        uint8_t state;
        uint8_t code;
        uint8_t message_crc;
        uint8_t crc;
        
        uint16_t payload_length_expected;
        uint16_t payload_length_received;
        
        uint8_t data_buffer[200];
} configurator;

void readSerial() {
    configurator.read_packet();
}