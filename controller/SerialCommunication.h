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
                case 11: // Received configuration union
                    if (sizeof(data_buffer) == sizeof(CONFIG_struct)) {
                        // process data from buffer (throw it inside union)
                        for (uint16_t i = 0; i < sizeof(data_buffer); i++) {
                            CONFIG.raw[i] = data_buffer[i];
                        }

                        // Write config to EEPROM
                        writeEEPROM();

                        // ACKownledge
                        ACK();
                    } else {
                        // Refuse (buffer size doesn't match struct memory size)
                        REFUSED();
                    }
                break;
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
        char data_buffer[300]; // Current UNION size = 264 bytes = 2112 bits
        
        uint8_t command_i;
        uint8_t data_i;
} configurator;

void readSerial() {
    configurator.read_packet();
}