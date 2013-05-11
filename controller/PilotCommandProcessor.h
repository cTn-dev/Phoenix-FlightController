/*  Pilot / Receiver command handling routine

    Featuring dynamic channel assignment and dynamic auxiliary funtion assignment
    with "side" support for automatic receiver failsafe routines.
    
    Dynamic channel assignment was initially requested by Politt @aeroquad hangout.
    
    Dynamic auxiliary funtion assignment was inspired by very similar feature originally found
    in multiwii flight control software.
*/

int16_t TX_roll, TX_pitch, TX_throttle, TX_yaw, TX_AUX1, TX_AUX2, TX_AUX3, TX_AUX4;
int16_t TX_AUX5, TX_AUX6, TX_AUX7, TX_AUX8, TX_AUX9, TX_AUX10, TX_AUX11, TX_AUX12;
uint64_t AUX_chan_mask;
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
    TX_AUX1     = RX[CONFIG.data.CHANNEL_ASSIGNMENT[4]];
    TX_AUX2     = RX[CONFIG.data.CHANNEL_ASSIGNMENT[5]];
    TX_AUX3     = RX[CONFIG.data.CHANNEL_ASSIGNMENT[6]];
    TX_AUX4     = RX[CONFIG.data.CHANNEL_ASSIGNMENT[7]];
    TX_AUX5     = RX[CONFIG.data.CHANNEL_ASSIGNMENT[8]];
    TX_AUX6     = RX[CONFIG.data.CHANNEL_ASSIGNMENT[9]];
    TX_AUX7     = RX[CONFIG.data.CHANNEL_ASSIGNMENT[10]];
    TX_AUX8     = RX[CONFIG.data.CHANNEL_ASSIGNMENT[11]];
    TX_AUX9     = RX[CONFIG.data.CHANNEL_ASSIGNMENT[12]];
    TX_AUX10    = RX[CONFIG.data.CHANNEL_ASSIGNMENT[13]];
    TX_AUX11    = RX[CONFIG.data.CHANNEL_ASSIGNMENT[14]];
    TX_AUX12    = RX[CONFIG.data.CHANNEL_ASSIGNMENT[15]];
    
    sei(); // enable interrupts
    
    // Set the mask
    AUX_chan_mask = 0x00; // reset the mask
    
    if (TX_AUX1 < 1250) { // LOW
        AUX_chan_mask |= 1 << 0;
    } else if (TX_AUX1 < 1750) { // MID
        AUX_chan_mask |= 1 << 1;
    } else { //HIGH
        AUX_chan_mask |= 1 << 2;
    }

    if (TX_AUX2 < 1250) {
        AUX_chan_mask |= 1 << 3;
    } else if (TX_AUX2 < 1750) {
        AUX_chan_mask |= 1 << 4;
    } else {
        AUX_chan_mask |= 1 << 5;
    }

    if (TX_AUX3 < 1250) {
        AUX_chan_mask |= 1 << 6;
    } else if (TX_AUX3 < 1750) {
        AUX_chan_mask |= 1 << 7;
    } else {
        AUX_chan_mask |= 1 << 8;
    }
    
    if (TX_AUX4 < 1250) {
        AUX_chan_mask |= 1 << 9;
    } else if (TX_AUX4 < 1750) {
        AUX_chan_mask |= 1 << 10;
    } else {
        AUX_chan_mask |= 1 << 11;
    }
    
    if (TX_AUX5 < 1250) {
        AUX_chan_mask |= 1 << 12;
    } else if (TX_AUX5 < 1750) {
        AUX_chan_mask |= 1 << 13;
    } else {
        AUX_chan_mask |= 1 << 14;
    }
    
    if (TX_AUX6 < 1250) {
        AUX_chan_mask |= 1 << 15;
    } else if (TX_AUX6 < 1750) {
        AUX_chan_mask |= 1 << 16;
    } else {
        AUX_chan_mask |= 1 << 17;
    }
 
    if (TX_AUX7 < 1250) {
        AUX_chan_mask |= 1 << 18;
    } else if (TX_AUX7 < 1750) {
        AUX_chan_mask |= 1 << 19;
    } else {
        AUX_chan_mask |= 1 << 20;
    }
    
    if (TX_AUX8 < 1250) {
        AUX_chan_mask |= 1 << 21;
    } else if (TX_AUX8 < 1750) {
        AUX_chan_mask |= 1 << 22;
    } else {
        AUX_chan_mask |= 1 << 23;
    }
    
    if (TX_AUX9 < 1250) {
        AUX_chan_mask |= 1 << 24;
    } else if (TX_AUX9 < 1750) {
        AUX_chan_mask |= 1 << 25;
    } else {
        AUX_chan_mask |= 1 << 26;
    }
    
    if (TX_AUX10 < 1250) {
        AUX_chan_mask |= 1 << 27;
    } else if (TX_AUX10 < 1750) {
        AUX_chan_mask |= 1 << 28;
    } else {
        AUX_chan_mask |= 1 << 29;
    }
    
    if (TX_AUX11 < 1250) {
        AUX_chan_mask |= 1ULL << 30; // Starting to use ULL because we will go over the 32bit barier soon
    } else if (TX_AUX11 < 1750) {
        AUX_chan_mask |= 1ULL << 31;
    } else {
        AUX_chan_mask |= 1ULL << 32;
    }
    
    if (TX_AUX12 < 1250) {
        AUX_chan_mask |= 1ULL << 33;
    } else if (TX_AUX12 < 1750) {
        AUX_chan_mask |= 1ULL << 34;
    } else {
        AUX_chan_mask |= 1ULL << 35;
    }
   
    // Arming-Disarming sequence
    if (TX_throttle < 1100 && TX_yaw > 1850) {
        if (armed == false) {
            // We just armed the controller
            
            reset_PID_integrals(); // Reset all integrals inside PID controllers to 0
        }
        
        if (flightMode == ATTITUDE_MODE) {
            commandYawAttitude = kinematicsAngle[ZAXIS];
        } else if (flightMode == RATE_MODE)  {
            commandYaw = 0.0;
        } 
        
        armed = true;
    } else if ((TX_throttle < 1100 && TX_yaw < 1250) || (TX_throttle <= CONFIG.data.minimumArmedThrottle && failsafeEnabled)) {
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
    
    // TX accelerometer trimming
    if (armed == false && TX_throttle > 1750) {
        // We jest enter trimming mode
        static bool changed = false;
        
        // We will "lock" the user in here untill trimming is done
        while (TX_throttle > 1750) {
            // Fetch latest values
            TX_roll     = RX[CONFIG.data.CHANNEL_ASSIGNMENT[0]];
            TX_pitch    = RX[CONFIG.data.CHANNEL_ASSIGNMENT[1]];
            TX_throttle = RX[CONFIG.data.CHANNEL_ASSIGNMENT[2]];
            TX_yaw      = RX[CONFIG.data.CHANNEL_ASSIGNMENT[3]];
            
            // Flip the flag if anything will be changed
            if (TX_roll > 1750 || TX_roll < 1250 || TX_pitch > 1750 || TX_pitch < 1250) {
                changed = true;
            }
            
            // Check the sticks
            if (TX_roll > 1750) {
                CONFIG.data.ACCEL_BIAS[YAXIS] += 5;
            } else if (TX_roll < 1250) {
                CONFIG.data.ACCEL_BIAS[YAXIS] -= 5;
            }
            
            if (TX_pitch > 1750) {
                CONFIG.data.ACCEL_BIAS[XAXIS] -= 5;
            } else if (TX_pitch < 1250) {
                CONFIG.data.ACCEL_BIAS[XAXIS] += 5;
            }

            if (TX_yaw > 1750 && changed) {
                // save data
                writeEEPROM();
                
                // reset flag
                changed = false;
            }
            
            // Blink LED to indicate activity
            Arduino_LED_state = !Arduino_LED_state;
            digitalWrite(LED_ARDUINO, Arduino_LED_state); 
            
            delay(500); // 0.5s loop delay to allow precise trimming
        }
    }
    
    // Rate-Attitude mode
    if (CONFIG.data.CHANNEL_FUNCTIONS[0] & AUX_chan_mask || failsafeEnabled) {
        if (flightMode == RATE_MODE) {
            // We just switched from rate to attitude mode
            // That means YAW correction should be applied to avoid YAW angle "jump"
            commandYawAttitude = kinematicsAngle[ZAXIS];
        }
        
        flightMode = ATTITUDE_MODE;
    } else {
        if (flightMode == ATTITUDE_MODE) {
            // We just switched from attitude to rate mode
            // That means commandYaw reset
            commandYaw = 0.0;
        }
        
        flightMode = RATE_MODE;
    }

#if defined(AltitudeHoldBaro) || defined(AltitudeHoldSonar)
    // Altitude hold ON/OFF
    if ((CONFIG.data.CHANNEL_FUNCTIONS[1] & AUX_chan_mask) == false) {
        // throttle controlled by stick
        altitudeHoldBaro = false;
        altitudeHoldSonar = false;
        
        // reset throttle panic flag
        throttlePanic = false;
    }
#endif

#ifdef AltitudeHoldBaro
    else if ((CONFIG.data.CHANNEL_FUNCTIONS[1] & AUX_chan_mask) && throttlePanic == false) {
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
    else if ((CONFIG.data.CHANNEL_FUNCTIONS[2] & AUX_chan_mask) && throttlePanic == false) {
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
    if (CONFIG.data.CHANNEL_FUNCTIONS[3] & AUX_chan_mask) {
        // Position hold enabled
        if (positionHoldGPS == false) { // We just switched on the position hold
            // current heading (from magnetometer should be saved here)
            // current GPS pos should be saved here
        }
        
        positionHoldGPS = true;
    } else {
        // Position hold disabled
        if (positionHoldGPS == true) { // We just switched off the position hold
        }
        
        positionHoldGPS = false;
    }
#endif
    
    // Ignore TX_yaw while throttle is below minimum armed throttle
    if (TX_throttle < CONFIG.data.minimumArmedThrottle) {
        TX_yaw = 1500;
        
        // Keep command yaw and attitude yaw error around ~0
        commandYawAttitude = kinematicsAngle[ZAXIS];
        commandYaw = 0.0;
    }    
    
    // Channel center = 1500
    TX_roll = TX_roll - 1500;
    TX_pitch = -(TX_pitch - 1500);
    TX_yaw = TX_yaw - 1500;
    
    // Reverse YAW
    TX_yaw = -TX_yaw;
    
    // YAW deadband (tiny 5us deadband to cancel any noise/offset from the TX)
    // PWM2RAD = 0.002
    // ATTITUDE_SCALING = 0.75 * PWM2RAD = 0.0015 // 0.75 is just an arbitrary scale factor to limit the range to about ~43 degrees (0.75 radians)
    if (abs(TX_yaw) > 5) { // If yaw signal is bigger then 5 (5us) allow commandYaw to change
        if (flightMode == ATTITUDE_MODE) {
            // YAW angle build up over time
            commandYawAttitude += (TX_yaw * 0.0015) / 8; // division by 8 is used to slow down YAW build up 
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