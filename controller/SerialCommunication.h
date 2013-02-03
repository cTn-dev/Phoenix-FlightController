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
                        if (data == 0x5B) { // Sync char [
                            // reset variables
                            memset(command_buffer, 0, sizeof(command_buffer));
                            memset(data_buffer, 0, sizeof(data_buffer));
                            
                            command_i = 0;
                            data_i = 0;
                            
                            state++;
                        }
                    break;
                    case 1:
                        if (data != 0x3A) { // Divider char :
                            command_buffer[command_i] = data;
                            command_i++;
                        } else {
                            state++;
                        }
                    break;
                    case 2:
                        if (data != 0x5D) { // Ending char ]
                            data_buffer[data_i] = data;
                            data_i++;
                        } else {
                            // Message received
                            // process data and return to beginning
                            process_data();
                            
                            state = 0;
                        }
                    break;
                }
                
                // Used for debugging (for now)
                //Serial.println(data, HEX);
            }        
        };
        
        void process_data() {
            int16_t command = atoi(command_buffer);
            
            switch (command) {
                case 1: // Requesting configuration union
                    Serial.write(0x5B); // [
                    Serial.write(0x31); // 1
                    Serial.write(0x3A); // :
                    
                    for (uint16_t i = 0; i < sizeof(CONFIG_struct); i++) {
                        Serial.write(CONFIG.raw[i]);
                    }  

                    Serial.write(0x5D); // ]
                break;
                case 2: // Activating ESC calibration
                    CONFIG.data.calibrateESC = 1;
                    
                    // Write config to EEPROM
                    writeEEPROM();
                    
                    // ACKownledge
                    ACK();
                break;
                case 11: // Command YAW data
                /*  WOW, this takes so much sketch size.. NO WAY !
                    char* Kp_ = strtok(data_buffer, ",");
                    char* Ki_ = strtok(NULL, ",");
                    char* Kd_ = strtok(NULL, ",");
                    char* WG_ = strtok(NULL, ",");
                    
                    // Apply new pid settings and save to EEPROM
                    CONFIG.data.PID_YAW_c[P] = atof(Kp_);
                    CONFIG.data.PID_YAW_c[I] = atof(Ki_);
                    CONFIG.data.PID_YAW_c[D] = atof(Kd_);
                    CONFIG.data.PID_YAW_c[WG] = atoi(WG_);
                    
                    writeEEPROM();
                */                    
                break;
                /*
                case 12: // Command PITCH data
                break;
                case 13: // Command ROLL data
                break;
                case 14: // Motor YAW data
                break;  
                case 15: // Motor PITCH data
                break;  
                case 16: // Motor ROLL data
                break;  
                case 17: // Baro data
                break;  
                case 18: // Sonar data
                break;
                */
            }
        };
        
        void ACK() {
            Serial.write(0x5B); // [
            Serial.write(0x39); // 9
            Serial.write(0x3A); // :
            Serial.write(0x31); // 1
            Serial.write(0x5D); // ]        
        };
        
        void REFUSED() {
            Serial.write(0x5B); // [
            Serial.write(0x39); // 9
            Serial.write(0x3A); // :
            Serial.write(0x30); // 0
            Serial.write(0x5D); // ]          
        };
    
    private:
        char data; // variable used to store a single byte from serial
        
        uint8_t state;
        
        char command_buffer[4];
        char data_buffer[20];
        
        uint8_t command_i;
        uint8_t data_i;
} configurator;

void readSerial() {
    configurator.read_packet();
}