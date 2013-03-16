int16_t TX_roll, TX_pitch, TX_throttle, TX_yaw, TX_mode, TX_altitude, TX_pos_hold, TX_last;
bool throttlePanic = false;

void processPilotCommands() {
    // read data into variables
    cli(); // disable interrupts
    
    // Channel assignment variables are loaded from eeprom
    // allowing user to "dynamically" (via configurator) change the rx channel assignment
    // potentionally allowing to "wire" channels from RX to FC completely wrong 
    // (and then fixing them manually in Channel Assigner)
    TX_roll     = RX[CONFIG.data.CHANNEL_ASSIGNMENT[0]];
    TX_pitch    = RX[CONFIG.data.CHANNEL_ASSIGNMENT[1]];
    TX_throttle = RX[CONFIG.data.CHANNEL_ASSIGNMENT[2]];
    TX_yaw      = RX[CONFIG.data.CHANNEL_ASSIGNMENT[3]];
    TX_mode     = RX[CONFIG.data.CHANNEL_ASSIGNMENT[4]];
    TX_altitude = RX[CONFIG.data.CHANNEL_ASSIGNMENT[5]];
    TX_pos_hold = RX[CONFIG.data.CHANNEL_ASSIGNMENT[6]];
    TX_last     = RX[CONFIG.data.CHANNEL_ASSIGNMENT[7]];
    
    sei(); // enable interrupts
    
    // Arming-Disarming sequence
    if (TX_throttle < 1100 && TX_yaw > 1850) {
        if (armed == false) {
            // We just armed the controller
            
            reset_PID_integrals(); // Reset all integrals inside PID controllers to 0
        }
        
        if (flightMode = ATTITUDE_MODE) {
            commandYawAttitude = kinematicsAngle[ZAXIS];
        } else if (flightMode == RATE_MODE)  {
            commandYaw = 0.0;
        } 
        
        armed = true;
    } else if (TX_throttle < 1100 && TX_yaw < 1250) {
        if (armed == true) {
            // We just dis-armed the controller
            
            // Reset all motors to 0 throttle / power
            for (uint8_t i = 0; i < MOTORS; i++) {
                MotorOut[i] = 1000;
            }

            updateMotors(); // Update ESCs
        }
        
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
        }
        
        flightMode = ATTITUDE_MODE;
    }
    
#if defined(AltitudeHoldBaro) || defined(AltitudeHoldSonar)
    // Altitude hold ON/OFF
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
    
#ifdef GPS
    // GPS Position Hold
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
    
    // YAW deadband (tiny 5us deadband to cancel any noise/offset from the TX)
    // PWM2RAD = 0.002
    // ATTITUDE_SCALING = 0.75 * PWM2RAD = 0.0015 // 0.75 is just an arbitrary scale factor to limit the range to about ~43 degrees (0.75 radians)
    if (abs(TX_yaw) > 5) { // If yaw signal is bigger then 5 (5us) allow commandYaw to change
        if (flightMode == ATTITUDE_MODE) {
            // YAW angle build up over time
            commandYawAttitude += (TX_yaw * 0.0015) / 48; // division by 48 is used to slow down YAW build up 
            NORMALIZE(commandYawAttitude); // +- PI
        } else if (flightMode == RATE_MODE) {
            // raw stick input
            commandYaw = (TX_yaw * 0.0015);
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