int16_t TX_roll, TX_pitch, TX_throttle, TX_yaw, TX_mode, TX_altitude, TX_pos_hold, TX_last;
bool throttlePanic = false;

void processPilotCommands() {
    // read data into variables
    cli(); // disable interrupts
    
    TX_roll = RX[0];     // CH-1 AIL
    TX_pitch = RX[1];    // CH-2 ELE
    TX_throttle = RX[2]; // CH-3 THR
    TX_yaw = RX[3];      // CH-4 RUD
    TX_mode = RX[4];     // CH-5 FULL ELE switch (off = rate, on = attitude)
    TX_altitude = RX[5]; // CH-6
    TX_pos_hold = RX[6]; // CH-7
    TX_last = RX[7];     // CH-8
    
    sei(); // enable interrupts
    
    // Arming-Disarming sequence
    if (TX_throttle < 1100 && TX_yaw > 1850) {
        if (armed == false) {
            // We just armed the controller
            
            reset_PID_integrals(); // Reset all integrals inside PID controllers to 0
        }
        
        if (flightMode = ATTITUDE_MODE) {
            commandYawAttitude = kinematicsAngle[ZAXIS];
            commandYaw = commandYawAttitude;
        } else if (flightMode == RATE_MODE)  {
            commandYaw = 0.0;
        } 
        
        armed = true;
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
            commandYawAttitude = kinematicsAngle[ZAXIS];
            commandYaw = commandYawAttitude;
        }
        
        flightMode = ATTITUDE_MODE;
    }
    
    // Altitude hold ON/OFF
    #if defined(AltitudeHoldBaro) || defined(AltitudeHoldSonar)
        if (TX_altitude < 1100) {
            // throttle controlled by stick
            altitudeHoldBaro = false;
            altitudeHoldSonar = false;
            
            // reset throttle panic flag
            throttlePanic = false;
        }
    #endif

    #ifdef AltitudeHoldBaro
        else if (TX_altitude > 1400 && TX_altitude < 1600 && throttlePanic == false) {
            // throttle controlled by baro
            if (altitudeHoldBaro == false) { // We just switched on the altitudeHoldBaro
                // save the current altitude and throttle
                baroAltitudeToHoldTarget = baroAltitudeRunning;
                baroAltitudeHoldThrottle = TX_throttle;
            }
            
            altitudeHoldSonar = false;
            altitudeHoldBaro = true;
            
            // Trigger throttle panic if throttle is higher or lower then 100 compared
            // to initial altitude hold throttle.
            if (abs(TX_throttle - baroAltitudeHoldThrottle) > 100) {
                // Pilot will be forced to re-flip the altitude hold switch to reset the throttlePanic flag.
                throttlePanic = true;
                altitudeHoldBaro = false;
            }
        }
    #endif
    
    #ifdef AltitudeHoldSonar
        else if (TX_altitude > 1900 && throttlePanic == false) {
            // throttle controlled by sonar
            if (altitudeHoldSonar == false) { // We just switched on the altitudeHoldSonar
                sonarAltitudeToHoldTarget = sonarAltitude;
                sonarAltitudeHoldThrottle = TX_throttle;
            }
            
            altitudeHoldBaro = false;
            altitudeHoldSonar = true;
            
            // Trigger throttle panic if throttle is higher or lower then 100 compared
            // to initial altitude hold throttle.
            if (abs(TX_throttle - sonarAltitudeHoldThrottle) > 100) {
                // Pilot will be forced to re-flip the altitude hold switch to reset the throttlePanic flag.
                throttlePanic = true;
                altitudeHoldSonar = false;
            }        
        }
    #endif
    
    // GPS Position Hold
    #ifdef GPS
        if (TX_pos_hold < 1100) {
            // Position hold disabled
            if (positionHoldGPS == true) { // We just switched off the position hold
            }
            
            positionHoldGPS = false;
        } else if (TX_pos_hold > 1900) {
            // Position hold enabled
            if (positionHoldGPS == false) { // We just switched on the position hold
                // current heading (from magnetometer should be saved here)
                // current GPS pos should be saved here
            }
            
            positionHoldGPS = true;
        }
    #endif
    
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
    } else {
        // Pilot sticks didn't changed but commandYawAttitude is also accesed directly by kinematics
        // so in theory it could change, and we need to handle this.
        if (flightMode == ATTITUDE_MODE) {
            commandYaw = commandYawAttitude;
        }
    }
 
    commandRoll = TX_roll * 0.0015;
    commandPitch = TX_pitch * 0.0015;
    
    // Compute throttle according to altitude switch (pilot input/baro/sonar)
    if (altitudeHoldBaro == false && altitudeHoldSonar == false) {
        throttle = TX_throttle;
    }
    
    #ifdef AltitudeHoldBaro
        else if (altitudeHoldBaro == true) {
            altitude_hold_baro_pid.Compute();
            throttle = baroAltitudeHoldThrottle - constrain(AltitudeHoldMotorSpeed, -200.0, 200.0);
        }
    #endif
    
    #ifdef AltitudeHoldSonar
        else if (altitudeHoldSonar == true) {
            altitude_hold_sonar_pid.Compute();
            throttle = sonarAltitudeHoldThrottle - constrain(AltitudeHoldMotorSpeed, -200.0, 200.0);
        }
    #endif
    
    #ifdef GPS
        if (positionHoldGPS == true) {
            // compute gps pids
        }
    #endif
}    