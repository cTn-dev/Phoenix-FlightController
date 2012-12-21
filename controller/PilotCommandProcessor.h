int16_t TX_roll, TX_pitch, TX_throttle, TX_yaw, TX_mode, TX_baro, TX_cam, TX_last;
double commandYaw, commandPitch, commandRoll;

void processPilotCommands() {
    // read data into variables
    cli(); // disable interrupts
    TX_roll = PPM[0];     // CH-1 AIL
    TX_pitch = PPM[1];    // CH-2 ELE
    TX_throttle = PPM[2]; // CH-3 THR
    TX_yaw = PPM[3];      // CH-4 RUD
    TX_mode = PPM[4];     // CH-5 FULL ELE switch (off = rate, on = attitude)
    TX_baro = PPM[5];     // CH-6 FULL AIL switch (off = standard altitude control by stick, on = altitude controled via barometer)
    TX_cam = PPM[6];      // CH-7
    TX_last = PPM[7];     // CH-8
    sei(); // enable interrupts
    
    #ifdef RX_GRAPH
        Serial.print(TX_roll);
        Serial.write('\t');
        Serial.print(TX_pitch);
        Serial.write('\t');   
        Serial.print(TX_throttle);
        Serial.write('\t');
        Serial.print(TX_yaw);
        Serial.write('\t');  
        Serial.print(TX_mode);
        Serial.write('\t'); 
        Serial.print(TX_baro);
        Serial.write('\t');
        Serial.print(TX_cam);
        Serial.write('\t');       
        Serial.print(TX_last);
        Serial.write('\t');
        Serial.print(PPM_error);
        Serial.write('\t');        
        Serial.println();
    #endif
    
    if (TX_throttle < 1100 && TX_yaw > 1850) {
        // controller is now armed
        armed = true;
        
        // reset command YAW and kinematics YAW to 0
        commandYaw = 0.0;
        kinematicsAngleZ = 0.0;
    } else if (TX_throttle < 1100 && TX_yaw < 1250) {
        // controller is now dis-armed
        armed = false;
    }
    
    if (TX_mode < 1100) {
        // RATE mode
        flightMode = false;
    } else if (TX_mode > 1900) {
        // ATTITUDE mode
        flightMode = true;
    }
    
    if (TX_baro < 1100) {
        // throttle controlled by stick
    } else if (TX_baro > 1900) {
        // throttle controlled by baro
    }
    
    // Ignore TX_yaw while throttle is below 1100
    if (TX_throttle < 1100) TX_yaw = 1500;
    
    // Channel center = 1500
    TX_roll = TX_roll - 1500;
    TX_pitch = TX_pitch - 1500;
    TX_yaw = TX_yaw - 1500;
    
    // Reverse YAW
    TX_yaw = -TX_yaw;
    
    // PWM2RAD = 0.002, ATTITUDE_SCALING = 0.75 * PWM2RAD = 0.0015
    // division by 50 is used to slow down YAW build up 
    if (flightMode) {
        // ATTITUDE mode = YAW angle build up over time
        commandYaw += (TX_yaw * 0.0015) / 50;
    }    
    else {
        // RATE mode = raw stick input
        commandYaw = (TX_yaw * 0.0015);
    }    
    commandRoll = TX_roll * 0.0015;
    commandPitch = TX_pitch * 0.0015;
}    