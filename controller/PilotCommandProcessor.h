int16_t TX_roll, TX_pitch, TX_throttle, TX_yaw, TX_mode, TX_baro, TX_cam, TX_last;
int16_t throttle = 1000;
bool throttlePanic = false;

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
    
    // Arming-Disarming sequence
    if (TX_throttle < 1100 && TX_yaw > 1850) {
        // controller is now armed
        armed = true;
        
        // Depending
        if (flightMode = ATTITUDE_MODE) {
            commandYawAttitude = kinematicsAngleZ;
            commandYaw = commandYawAttitude;
        } else if (flightMode == RATE_MODE)  {
            commandYaw = 0.0;
        }    
    } else if (TX_throttle < 1100 && TX_yaw < 1250) {
        // controller is now dis-armed
        armed = false;
    }
    
    // Rate-Attitude mode
    if (TX_mode < 1100) {
        if (flightMode == ATTITUDE_MODE) {
            // We just switched from attitude to rate mode
            // That means commandYaw reset
            commandYaw = 0.0;
        }
        
        flightMode = RATE_MODE;
    } else if (TX_mode > 1900) {
        if (flightMode == RATE_MODE) {
            // We just switched from rate to attitude mode
            // That means YAW correction should be applied to avoid YAW angle "jump"
            commandYawAttitude = kinematicsAngleZ;
            commandYaw = commandYawAttitude;
        }
        
        flightMode = ATTITUDE_MODE;
    }
    
    // Altitude hold ON/OFF
    if (TX_baro < 1100) {
        // throttle controlled by stick
        altitudeHold = false;
        
        // reset throttle panic flag
        throttlePanic = false;
    } else if (TX_baro > 1900 && throttlePanic == false) {
        // throttle controlled by baro
        if (altitudeHold == false) { // We just switched on the altitudeHold
            // save the current altitude and throttle
            baroAltitudeToHoldTarget = baroAltitude;
            baroAltitudeHoldThrottle = TX_throttle;
        }
        
        altitudeHold = true;
        
        // Trigger throttle panic if throttle is higher or lower then 100 compared
        // to initial altitude hold throttle.
        if (abs(TX_throttle - baroAltitudeHoldThrottle) > 100) {
            // Pilot will be forced to re-flip the altitude hold switch to reset the throttlePanic flag.
            throttlePanic = true;
            altitudeHold = false;
        }
    }
    
    // Ignore TX_yaw while throttle is below 1100
    if (TX_throttle < 1100) TX_yaw = 1500;
    
    // Channel center = 1500
    TX_roll = TX_roll - 1500;
    TX_pitch = TX_pitch - 1500;
    TX_yaw = TX_yaw - 1500;
    
    // Reverse YAW
    TX_yaw = -TX_yaw;
    
    // YAW deadband (its not 100% necessary, but i am more confiden't having it here)
    // + i don't know anyone that is using YAW trimms on multicopter
    // - it ruins YAW/Rudder trimms on TX
    if (abs(TX_yaw) > 5) { // If yaw signal is bigger then 5 (5us) allow commandYaw to change
        // PWM2RAD = 0.002, ATTITUDE_SCALING = 0.75 * PWM2RAD = 0.0015
        // division by 50 is used to slow down YAW build up 
        if (flightMode == ATTITUDE_MODE) {
            // YAW angle build up over time
            commandYawAttitude += (TX_yaw * 0.0015) / 50;
            
            commandYaw = commandYawAttitude;           
        } else if (flightMode == RATE_MODE) {
            // raw stick input
            commandYaw = (TX_yaw * 0.0015);
        }      
    }
 
    commandRoll = TX_roll * 0.0015;
    commandPitch = TX_pitch * 0.0015;
    
    if (altitudeHold == true) {
        throttle_motor_pid.Compute();
        throttle = baroAltitudeHoldThrottle - constrain(ThrottleMotorSpeed, -50.0, 50.0);
    } else {
        throttle = TX_throttle;
    }
}    