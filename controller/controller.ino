// Arduino standard library imports
#include <arduino.h>
#include <Wire.h>

// Custom imports
#include "PID.h"
//#include "kinematics_ARG.h"
#include "kinematics_CMP.h"
#include "mpu6050.h"
#include "ppm_esc.h"

// Custom definitions
//#define RX_GRAPH
//#define SENSOR_GRAPH
//#define DISPLAY_ITTERATIONS

// main loop variables
uint8_t itterations = 0;
unsigned long currentTime = 0;
unsigned long previousTime = 0;
unsigned long sensorPreviousTime = 0;
uint8_t frameCounter = 0;
bool all_ready = false;
bool armed = false;
bool flightMode = false;

// Blinking LED to indicate activity
#define LED_PIN 13
bool blinkState = false;

// Modulo definitions (integer remainder)
#define TASK_10HZ 10

// MPU definitions
MPU6050 mpu;

double gyroXsum, gyroYsum, gyroZsum;
double accelXsum, accelYsum, accelZsum;
double gyroXsumRate, gyroYsumRate, gyroZsumRate;
unsigned long gyroRateTimer;
uint8_t SensorSamples;

// PID definitions
double YawCommandPIDSpeed, PitchCommandPIDSpeed, RollCommandPIDSpeed;
double YawMotorSpeed, PitchMotorSpeed, RollMotorSpeed;
double commandYaw, commandPitch, commandRoll;

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
    
    // set ESC pins to output
    for (int i = 0; i < MOTORS; i++) {
        pinMode(motorPins[i], OUTPUT);
    }  

    // Attach external interrupt to pin 8 and setup timer1
    //setupTimer1();    
 
    // Define the integrated LED pin as output
    pinMode(LED_PIN, OUTPUT);
    
    // Initialize IMU unit
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
        mpu.readAccelRaw();        
        
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
    cli(); // disable interrupts
    
    // read data into variables
    int TX_roll = PPM[0];     // CH-1 AIL
    int TX_pitch = PPM[1];    // CH-2 ELE
    int TX_throttle = PPM[2]; // CH-3 THR
    int TX_yaw = PPM[3];      // CH-4 RUD
    int TX_mode = PPM[4];     // CH-5 FULL ELE switch (off = rate, on = attitude)
    int TX_heading = PPM[5];  // CH-6 FULL THROTTLE switch (off = gyro heading, on = gyro + mag heading)
    
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
        Serial.print(TX_heading);
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
    
    // Ignore TX_yaw while throttle is below 1100
    if (TX_throttle < 1100) TX_yaw = 1500;
    
    // Channel center = 1500
    TX_roll = TX_roll - 1500;
    TX_pitch = TX_pitch - 1500;
    TX_yaw = TX_yaw - 1500;
    
    // Reverse YAW
    TX_yaw = -TX_yaw;
    
    // PWM2RAD = 0.002, ATTITUDE_SCALING = 0.75 * PWM2RAD = 0.0015
    if (flightMode) {
        // ATTITUDE mode = YAW angle build up over time
        commandYaw += (TX_yaw * 0.0015) / 100;
    }    
    else {
        // RATE mode = raw stick input
        commandYaw = (TX_yaw * 0.0015);
    }    
    commandRoll = TX_roll * 0.0015;
    commandPitch = TX_pitch * 0.0015;    
    
    // Calculate accel & gyro average (during previous 10ms)
    // Gyro is also properly scaled and converrted from dps to rps
    
    gyroXsumRate = (gyroXsum / SensorSamples) * gyroScaleFactor;
    gyroYsumRate = -((gyroYsum / SensorSamples) * gyroScaleFactor);
    gyroZsumRate = (gyroZsum / SensorSamples) * gyroScaleFactor;
    
    double accelXsumAvr = accelXsum / SensorSamples;
    double accelYsumAvr = accelYsum / SensorSamples;
    double accelZsumAvr = accelZsum / SensorSamples;

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
    
    #ifdef SENSOR_GRAPH
        Serial.print(kinematicsAngleX * RAD_TO_DEG + 180.0);
        Serial.write('\t');      
        Serial.print(kinematicsAngleY * RAD_TO_DEG + 180.0);
        Serial.write('\t');      
        Serial.print(kinematicsAngleZ * RAD_TO_DEG + 180.0);
        Serial.write('\t');              
        Serial.println();    
    #endif    
    
    if (armed) {        
        cli(); // disable interrupts

        /*
        MotorOut[0] = constrain(TX_throttle - PitchMotorSpeed - RollMotorSpeed, 1000, 2000);
        MotorOut[1] = constrain(TX_throttle - PitchMotorSpeed + RollMotorSpeed, 1000, 2000);
        MotorOut[2] = constrain(TX_throttle + PitchMotorSpeed + RollMotorSpeed, 1000, 2000);
        MotorOut[3] = constrain(TX_throttle + PitchMotorSpeed - RollMotorSpeed, 1000, 2000);         
        */
        
        MotorOut[0] = constrain(TX_throttle + PitchMotorSpeed + RollMotorSpeed + YawMotorSpeed, 1000, 2000);
        MotorOut[1] = constrain(TX_throttle + PitchMotorSpeed - RollMotorSpeed - YawMotorSpeed, 1000, 2000);
        MotorOut[2] = constrain(TX_throttle - PitchMotorSpeed - RollMotorSpeed + YawMotorSpeed, 1000, 2000);
        MotorOut[3] = constrain(TX_throttle - PitchMotorSpeed + RollMotorSpeed - YawMotorSpeed, 1000, 2000);         
        
        sei(); // enable interrupts
    } else {
        cli(); // disable interrupts
        
        MotorOut[0] = 1000;
        MotorOut[1] = 1000;
        MotorOut[2] = 1000;
        MotorOut[3] = 1000;
        
        sei(); // enable interrupts
    } 
}

void process10HzTask() {
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
