/*
    Phoenix flight controller was initially build to support only one array of sensors but
    with minor sensor changes it seemed like a waste to just "drop" previous sensor support
    with this idea in mind i will try to make the flight controller feature set flexible as
    possible to accommodate as much hw as possible.

    Defines below only enable/disable "pre-defined" hw setups, you can always define your own
    setup in the == Hardware setup == section.
*/

// Arduino standard library imports
#include <Arduino.h>
#include <Wire.h>

// Custom imports
#include "controller.h"
#include "sensors.h"
#include "math.h"
#include "PID.h"

// == Hardware setup/s == 
#define Maggie

#ifdef Maggie
    // Features requested
    #define Accelerometer
    #define Magnetometer
    //#define AltitudeHoldBaro
    #define AltitudeHoldSonar
    //#define GPS
    
    // Critical sensors on board (gyro/accel)
    #include <mpu6050.h>
    
    // Magnetometer
    #include <Magnetometer_HMC5883L.h>
    
    // Barometer
    //#include <Barometer_bmp085.h>
    
    // Sonar
    #include <Sonar_srf04.h>
    
    // GPS (ublox neo 6m)
    //#include <GPS_ublox.h>
    
    // Kinematics used
    #include <kinematics_CMP.h>
    
    // Receiver
    #include <Receiver_teensy3_HW_PPM.h>
    
    // Motor / ESC setup
    #include <ESC_teensy3_HW.h>    
#endif
// == END of Hardware setup ==

// PID definitions
double YawCommandPIDSpeed, PitchCommandPIDSpeed, RollCommandPIDSpeed;
double YawMotorSpeed, PitchMotorSpeed, RollMotorSpeed, AltitudeHoldMotorSpeed;

PID yaw_command_pid(&kinematicsAngleZ, &YawCommandPIDSpeed, &commandYaw, 4.0, 0.0, 0.0, 25.0);
PID pitch_command_pid(&kinematicsAngleY, &PitchCommandPIDSpeed, &commandPitch, 4.0, 0.0, 0.0, 25.0);
PID roll_command_pid(&kinematicsAngleX, &RollCommandPIDSpeed, &commandRoll, 4.0, 0.0, 0.0, 25.0);

PID yaw_motor_pid(&gyroZsumRate, &YawMotorSpeed, &YawCommandPIDSpeed, 200.0, 5.0, 0.0, 1000.0);
PID pitch_motor_pid(&gyroYsumRate, &PitchMotorSpeed, &PitchCommandPIDSpeed, 80.0, 0.0, -3.0, 1000.0);
PID roll_motor_pid(&gyroXsumRate, &RollMotorSpeed, &RollCommandPIDSpeed, 80.0, 0.0, -3.0, 1000.0);

#ifdef AltitudeHoldBaro
    PID altitude_hold_baro_pid(&baroAltitudeToHoldTarget, &AltitudeHoldMotorSpeed, &baroAltitudeRunning, 25.0, 0.6, -10.0, 25.0);
#endif

#ifdef AltitudeHoldSonar    
    PID altitude_hold_sonar_pid(&sonarAltitudeToHoldTarget, &AltitudeHoldMotorSpeed, &sonarAltitude, 60.0, 0.6, -10.0, 25.0);
#endif  
  
// Include this last as it contains objects from previous declarations
#include "PilotCommandProcessor.h"  
  
void setup() {
    // PIN settings
    pinMode(LED_PIN, OUTPUT); // build in status LED
    pinMode(LED_ORIENTATION, OUTPUT); // orientation lights
    
    // Initialize serial communication
    Serial.begin(115200);   
 
    // Join i2c bus as master
    Wire.begin();

    initializeESC();    
    initializeReceiver();
    
    sensors.initializeGyro();
    
    #ifdef Accelerometer
        sensors.initializeAccel();
    #endif
    
    #ifdef Magnetometer
        sensors.initializeMag();
    #endif
    
    #ifdef AltitudeHoldBaro
        sensors.initializeBaro();    
    #endif
    
    #ifdef AltitudeHoldSonar
        initializeSonar();
    #endif    
    
    // All is ready, start the loop
    all_ready = true;
}

void loop() {   
    // Dont start the loop until everything is ready
    if (!all_ready) return; 
 
    // Used to measure loop performance
    itterations++;
    
    // Timer
    currentTime = micros();
    
    // Read data (not faster then every 1 ms)
    if (currentTime - sensorPreviousTime >= 1000) {
        sensors.readGyroSum();
        
        #ifdef Accelerometer
            sensors.readAccelSum();        
        #endif
        
        #ifdef AltitudeHoldSonar
            // Bring sonar pin down (complete TLL trigger pulse)
            readSonarFinish();
        #endif    
        
        sensorPreviousTime = currentTime;
    }    
    
    // 100 Hz task loop (10 ms)
    if (currentTime - previousTime > 10000) {
        frameCounter++;
        
        process100HzTask();
        
        // 50 Hz tak (20 ms)
        if (frameCounter % TASK_50HZ == 0) {
            process50HzTask();
        }
        
        // 10 Hz task (100 ms)
        if (frameCounter % TASK_10HZ == 0) {
            process10HzTask();
        }  
        
        // 1 Hz task (1000 ms)
        if (frameCounter % TASK_1HZ == 0) {
            process1HzTask();
        }
        
        // Reset frameCounter back to 0 after reaching 100 (1s)
        if (frameCounter >= 100) {
            frameCounter = 0;
        }
        
        previousTime = currentTime;
    }
}

void process100HzTask() {    
    sensors.evaluateGyro();
    sensors.evaluateAccel();
    
    #ifdef AltitudeHoldBaro
        // Baro is being sampled every 10ms (because measuring pressure is slow) 
        sensors.readBaroSum();
    #endif    
    
    #ifdef GPS
        sensors.readGPS();
    #endif
    
    // Update kinematics with latest data
    kinematics_update(&accelXsumAvr, &accelYsumAvr, &accelZsumAvr, &gyroXsumRate, &gyroYsumRate, &gyroZsumRate);
    
    if (flightMode == ATTITUDE_MODE) {
        // Compute command PIDs (with kinematics correction)
        yaw_command_pid.Compute();
        pitch_command_pid.Compute();
        roll_command_pid.Compute();
    } else if (flightMode == RATE_MODE) {
        // Stick input, * 4.0 is the rotation speed factor
        YawCommandPIDSpeed = commandYaw * 4.0;
        PitchCommandPIDSpeed = commandPitch * 4.0;
        RollCommandPIDSpeed = commandRoll * 4.0;        
    }   
    
    // Compute motor PIDs (rate-based)    
    yaw_motor_pid.Compute();
    pitch_motor_pid.Compute();
    roll_motor_pid.Compute();     
    
    if (armed) {
        // All of the motor outputs are constrained to standard 1000-2000us)
        MotorOut[0] = constrain(throttle + PitchMotorSpeed + RollMotorSpeed + YawMotorSpeed, 1000, 2000);
        MotorOut[1] = constrain(throttle + PitchMotorSpeed - RollMotorSpeed - YawMotorSpeed, 1000, 2000);
        MotorOut[2] = constrain(throttle - PitchMotorSpeed - RollMotorSpeed + YawMotorSpeed, 1000, 2000);
        MotorOut[3] = constrain(throttle - PitchMotorSpeed + RollMotorSpeed - YawMotorSpeed, 1000, 2000);

        updateMotors();
    } else {
        MotorOut[0] = 1000;
        MotorOut[1] = 1000;
        MotorOut[2] = 1000;
        MotorOut[3] = 1000;
        
        updateMotors();
    }

    #ifdef DATA_VISUALIZATION
        // Gyro data
        Serial.print(gyroXsumRate);
        Serial.write(',');
        Serial.print(gyroYsumRate);
        Serial.write(',');
        Serial.print(gyroZsumRate);
        Serial.write(',');        
        
        // Accel data
        Serial.print(accelXsumAvr);
        Serial.write(',');
        Serial.print(accelYsumAvr);
        Serial.write(','); 
        Serial.print(accelZsumAvr);
        Serial.write(',');         
        
        // Kinematics data
        Serial.print(kinematicsAngleX * RAD_TO_DEG);
        Serial.write(',');      
        Serial.print(kinematicsAngleY * RAD_TO_DEG);
        Serial.write(',');      
        Serial.print(kinematicsAngleZ * RAD_TO_DEG);  
        Serial.write(','); 
        
        // TX/RX data
        Serial.print(TX_roll);
        Serial.write(',');   
        Serial.print(TX_pitch);
        Serial.write(','); 
        Serial.print(TX_throttle);
        Serial.write(','); 
        Serial.print(TX_yaw);    
        Serial.write(','); 
        Serial.print(TX_mode);    
        Serial.write(',');         
        Serial.print(TX_altitude);
        Serial.write(','); 
        Serial.print(TX_cam);
        Serial.write(','); 
        Serial.print(TX_last);
        Serial.write(',');
        
        // PPM error
        Serial.print(PPM_error);
        Serial.write(',');
        
        // Motor out
        Serial.print(MotorOut[0]);
        Serial.write(',');
        Serial.print(MotorOut[1]);
        Serial.write(',');
        Serial.print(MotorOut[2]);
        Serial.write(',');
        Serial.print(MotorOut[3]);
        
        Serial.println();     
    #endif    
}

void process50HzTask() {
    processPilotCommands();
    
    #ifdef AltitudeHoldBaro
        sensors.evaluateBaroAltitude();
    #endif    
}

void process10HzTask() {
    // Trigger RX failsafe function every 100ms
    RX_failSafe();
    
    #ifdef AltitudeHoldSonar
        // Request sonar reading
        readSonar();
    #endif    
    
    #ifdef Magnetometer
        sensors.readMag();
    #endif
    
    // Print itterations per 100ms
    #ifdef DISPLAY_ITTERATIONS
        Serial.println(itterations);
    #endif
    
    // Reset Itterations
    itterations = 0;    
}

void process1HzTask() {   
    // Blink LED to indicated activity
    Alive_LED_state = !Alive_LED_state;
    digitalWrite(LED_PIN, Alive_LED_state);

    // Orientation ligts
    // also displaying armed / dis-armed status
    if (armed) {
        digitalWrite(LED_ORIENTATION, HIGH);
    } else {
        digitalWrite(LED_ORIENTATION, LOW);
    }
}