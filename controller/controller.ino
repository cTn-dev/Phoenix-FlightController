// Arduino standard library imports
#include <Arduino.h>
#include <Wire.h>

// Custom imports
#include "controller.h"
#include "receiver.h"
#include "PID.h"
#include "esc.h"
#include "math.h"

#include "kinematics_CMP.h"
//#include "kinematics_ARG.h"
//#include "kinematics_DCM.h"

#include "mpu6050.h"
#include "bmp085.h"

// Sensor definitions
MPU6050 mpu;
BMP085 baro;

// PID definitions
double YawCommandPIDSpeed, PitchCommandPIDSpeed, RollCommandPIDSpeed;
double YawMotorSpeed, PitchMotorSpeed, RollMotorSpeed, ThrottleMotorSpeed;

PID yaw_command_pid(&kinematicsAngleZ, &YawCommandPIDSpeed, &commandYaw, 4.0, 0.0, 0.0, 25.0);
PID pitch_command_pid(&kinematicsAngleY, &PitchCommandPIDSpeed, &commandPitch, 4.0, 0.0, 0.0, 25.0);
PID roll_command_pid(&kinematicsAngleX, &RollCommandPIDSpeed, &commandRoll, 4.0, 0.0, 0.0, 25.0);

PID yaw_motor_pid(&gyroZsumRate, &YawMotorSpeed, &YawCommandPIDSpeed, 200.0, 5.0, 0.0, 1000.0);
PID pitch_motor_pid(&gyroYsumRate, &PitchMotorSpeed, &PitchCommandPIDSpeed, 80.0, 0.0, -300.0, 1000.0);
PID roll_motor_pid(&gyroXsumRate, &RollMotorSpeed, &RollCommandPIDSpeed, 80.0, 0.0, -300.0, 1000.0);
PID throttle_motor_pid(&baroAltitudeToHoldTarget, &ThrottleMotorSpeed, &baroAltitude, 25.0, 0.6, 0.0, 25.0);
  
// Include this last as it contains objects from previous declarations
#include "PilotCommandProcessor.h"  
  
void setup() {    
    // Initialize serial communication
    Serial.begin(115200);   
 
    // Join i2c bus as master
    Wire.begin();

    // ESC timer and PIN setup
    setupFTM0();
    
    // RX timer and PIN setup
    setupFTM1();    
 
    // PIN settings
    pinMode(LED_PIN, OUTPUT); // build in status LED
    pinMode(LED_ORIENTATION, OUTPUT); // orientation lights
    
    // Initialize sensors
    mpu.initialize();
    mpu.calibrate_gyro();
    baro.initialize();
    
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
        mpu.readGyroSum();
        mpu.readAccelSum();        
        
        #ifdef SENSOR_DATA_RAW
            Serial.print(accelX);
            Serial.write('\t');    
            Serial.print(accelY);
            Serial.write('\t');   
            Serial.print(accelZ);
            Serial.write('\t');   
            Serial.print(gyroX);
            Serial.write('\t');   
            Serial.print(gyroY);
            Serial.write('\t');   
            Serial.print(gyroZ);
            Serial.write('\t');  
            Serial.println(); 
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
    mpu.evaluateGyro();
    mpu.evaluateAccel();
    
    baro.measureBaroSum(); // Baro is being sampled every 10ms (because measuring pressure is slow) 
    
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
        Serial.print(TX_baro);
        Serial.write(','); 
        Serial.print(TX_cam);
        Serial.write(','); 
        Serial.print(TX_last);
        Serial.write(',');
        
        // PPM error
        Serial.print(PPM_error);
        
        Serial.println();     
    #endif    
}

void process50HzTask() {
    processPilotCommands();
    baro.evaluateBaroAltitude();
    baro.getBaroAltitude();
}

void process10HzTask() {
    // Trigger RX failsafe function every 100ms
    RX_failSafe();
    
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