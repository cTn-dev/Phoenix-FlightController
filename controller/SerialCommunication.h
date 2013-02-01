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
                case 1:
                    for (uint16_t i = 0; i < sizeof(CONFIG_struct); i++) {
                        Serial.write(CONFIG.raw[i]);
                    }                
                
                    /*
                    Serial.print(CONFIG.data.version);
                    
                    Serial.print(CONFIG.data.ACCEL_BIAS[0]);
                    Serial.print(CONFIG.data.ACCEL_BIAS[1]);
                    Serial.print(CONFIG.data.ACCEL_BIAS[2]);   
                    
                    // Altitude
                    Serial.print(CONFIG.data.PID_YAW_c[P], 4);
                    Serial.print(CONFIG.data.PID_YAW_c[I], 4);
                    Serial.print(CONFIG.data.PID_YAW_c[D], 4);
                    Serial.print(CONFIG.data.PID_YAW_c[WG], 4);
                    
                    Serial.print(CONFIG.data.PID_PITCH_c[P], 4);
                    Serial.print(CONFIG.data.PID_PITCH_c[I], 4);
                    Serial.print(CONFIG.data.PID_PITCH_c[D], 4);
                    Serial.print(CONFIG.data.PID_PITCH_c[WG], 4);
                    
                    Serial.print(CONFIG.data.PID_ROLL_c[P], 4);
                    Serial.print(CONFIG.data.PID_ROLL_c[I], 4);
                    Serial.print(CONFIG.data.PID_ROLL_c[D], 4);
                    Serial.print(CONFIG.data.PID_ROLL_c[WG], 4);
                    
                    // Rate
                    Serial.print(CONFIG.data.PID_YAW_m[P], 4);
                    Serial.print(CONFIG.data.PID_YAW_m[I], 4);
                    Serial.print(CONFIG.data.PID_YAW_m[D], 4);
                    Serial.print(CONFIG.data.PID_YAW_m[WG], 4);

                    Serial.print(CONFIG.data.PID_PITCH_m[P], 4);
                    Serial.print(CONFIG.data.PID_PITCH_m[I], 4);
                    Serial.print(CONFIG.data.PID_PITCH_m[D], 4);
                    Serial.print(CONFIG.data.PID_PITCH_m[WG], 4);

                    Serial.print(CONFIG.data.PID_ROLL_m[P], 4);
                    Serial.print(CONFIG.data.PID_ROLL_m[I], 4);
                    Serial.print(CONFIG.data.PID_ROLL_m[D], 4);
                    Serial.print(CONFIG.data.PID_ROLL_m[WG], 4);  

                    // Baro
                    Serial.print(CONFIG.data.PID_BARO[P], 4);
                    Serial.print(CONFIG.data.PID_BARO[I], 4);
                    Serial.print(CONFIG.data.PID_BARO[D], 4);
                    Serial.print(CONFIG.data.PID_BARO[WG], 4);

                    // Sonar
                    Serial.print(CONFIG.data.PID_SONAR[P], 4);
                    Serial.print(CONFIG.data.PID_SONAR[I], 4);
                    Serial.print(CONFIG.data.PID_SONAR[D], 4);
                    Serial.print(CONFIG.data.PID_SONAR[WG], 4);

                    // End of packet
                    Serial.println();
                    */
                break;
                case 2:
                break;
            }
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