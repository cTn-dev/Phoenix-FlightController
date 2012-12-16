// Arduino standard library imports
#include <Arduino.h>
#include <Wire.h>

// Custom imports
#include "controller.h"
#include "receiver.h"
#include "PilotCommandProcessor.h"
#include "PID.h"
#include "esc.h"

//#include "kinematics_ARG.h"
#include "kinematics_CMP.h"
#include "mpu6050.h"

// MPU definitions
MPU6050 mpu;

// PID definitions
double YawCommandPIDSpeed, PitchCommandPIDSpeed, RollCommandPIDSpeed;
double YawMotorSpeed, PitchMotorSpeed, RollMotorSpeed;

PID yaw_command_pid(&kinematicsAngleZ, &YawCommandPIDSpeed, &commandYaw, 4.0, 0.0, 0.0, 25.0);
PID pitch_command_pid(&kinematicsAngleY, &PitchCommandPIDSpeed, &commandPitch, 4.0, 0.0, 0.0, 25.0);
PID roll_command_pid(&kinematicsAngleX, &RollCommandPIDSpeed, &commandRoll, 4.0, 0.0, 0.0, 25.0);

PID yaw_motor_pid(&gyroZsumRate, &YawMotorSpeed, &YawCommandPIDSpeed, 200.0, 5.0, 0.0, 1000.0);
PID pitch_motor_pid(&gyroYsumRate, &PitchMotorSpeed, &PitchCommandPIDSpeed, 80.0, 0.0, -300.0, 1000.0);
PID roll_motor_pid(&gyroXsumRate, &RollMotorSpeed, &RollCommandPIDSpeed, 80.0, 0.0, -300.0, 1000.0);
  
void setup() {    
    // Initialize serial communication
    Serial.begin(38400);   
 
    // Join i2c bus as master
    Wire.begin();

    // ESC timer and PIN setup
    setupFTM0();
    
    // RX timer and PIN setup
    setupFTM1();    
 
    // Define the integrated LED pin (usually pin 13) as output
    pinMode(LED_PIN, OUTPUT);
    
    // Initialize sensors
    mpu.initialize();
    mpu.calibrate_gyro();
    
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
    
    // Read data (not faster then every 1ms)
    if (currentTime - sensorPreviousTime >= 1000) {
        mpu.readGyroCalibrated();
        mpu.readAccelCalibrated();        
        
        gyroXsum += gyroX;
        gyroYsum += gyroY;
        gyroZsum += gyroZ;
        
        accelXsum += accelX;
        accelYsum += accelY;
        accelZsum += accelZ;
        
        SensorSamples++;
        
        sensorPreviousTime = currentTime;
    }    
    
    // 100 Hz task loop (10 ms)
    if (currentTime - previousTime > 10000) {
        frameCounter++;
        
        // 100 Hz task (10ms)
        process100HzTask();
        
        // 10 Hz task (100 ms)
        if (frameCounter % TASK_10HZ == 0) {
            process10HzTask();
        }  
        
        previousTime = currentTime;
    }
    
    if (frameCounter >= 100) {
        frameCounter = 0;
    }
}

void process100HzTask() {
    processPilotCommands();  
    
    // Calculate accel & gyro average (during previous 10ms)
    // Gyro is also properly scaled and converrted from dps to rps
    
    gyroXsumRate = (gyroXsum / SensorSamples) * gyroScaleFactor;
    gyroYsumRate = -((gyroYsum / SensorSamples) * gyroScaleFactor);
    gyroZsumRate = (gyroZsum / SensorSamples) * gyroScaleFactor;
    
    accelXsumAvr = accelXsum / SensorSamples;
    accelYsumAvr = accelYsum / SensorSamples;
    accelZsumAvr = accelZsum / SensorSamples;

    // Reset SUM variables and Sample counter
    gyroXsum = 0;
    gyroYsum = 0;
    gyroZsum = 0;
    
    accelXsum = 0;
    accelYsum = 0;
    accelZsum = 0;
    
    SensorSamples = 0;
    
    // Update kinematics with latest data
    kinematics_update(&accelXsumAvr, &accelYsumAvr, &accelZsumAvr, &gyroXsumRate, &gyroYsumRate, &gyroZsumRate);
    
    if (flightMode) {
        // ATTITUDE mode
        
        // Compute command PIDs (with kinematics correction)
        yaw_command_pid.Compute();
        pitch_command_pid.Compute();
        roll_command_pid.Compute();
        
        // Compute motor PIDs (rate)    
        yaw_motor_pid.Compute();
        pitch_motor_pid.Compute();
        roll_motor_pid.Compute();   
    } else {
        // RATE mode
        
        // * 4.0 is the rotation speed factor
        YawCommandPIDSpeed = commandYaw * 4.0;
        PitchCommandPIDSpeed = commandPitch * 4.0;
        RollCommandPIDSpeed = commandRoll * 4.0;
        
        // Compute motor PIDs (rate)    
        yaw_motor_pid.Compute();
        pitch_motor_pid.Compute();
        roll_motor_pid.Compute();         
    }
    
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
    
    #ifdef KINEMATICS_GRAPH
        Serial.print(kinematicsAngleX * RAD_TO_DEG + 180.0);
        Serial.write('\t');      
        Serial.print(kinematicsAngleY * RAD_TO_DEG + 180.0);
        Serial.write('\t');      
        Serial.print(kinematicsAngleZ * RAD_TO_DEG + 180.0);
        Serial.write('\t');              
        Serial.println();    
    #endif    
    
    if (armed) {               
        MotorOut[0] = constrain(TX_throttle + PitchMotorSpeed + RollMotorSpeed + YawMotorSpeed, 1000, 2000);
        MotorOut[1] = constrain(TX_throttle + PitchMotorSpeed - RollMotorSpeed - YawMotorSpeed, 1000, 2000);
        MotorOut[2] = constrain(TX_throttle - PitchMotorSpeed - RollMotorSpeed + YawMotorSpeed, 1000, 2000);
        MotorOut[3] = constrain(TX_throttle - PitchMotorSpeed + RollMotorSpeed - YawMotorSpeed, 1000, 2000);

        updateMotors();
    } else {
        MotorOut[0] = 1000;
        MotorOut[1] = 1000;
        MotorOut[2] = 1000;
        MotorOut[3] = 1000;
        
        updateMotors();
    } 
}

void process10HzTask() {
    // Trigger RX failsafe function every 100ms
    RX_failSafe();

    // Blink LED to indicated activity
    blinkState = !blinkState;
    digitalWrite(LED_PIN, blinkState);
    
    #ifdef DISPLAY_ITTERATIONS
        // Print itterations per 100ms
        Serial.println(itterations);
    #endif
    
    // Reset Itterations
    itterations = 0;    
}
