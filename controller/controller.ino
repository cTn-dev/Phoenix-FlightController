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
#include <EEPROM.h>

// Custom imports
#include "controller.h"
#include "sensors.h"
#include "math.h"
#include "PID.h"
#include "dataStorage.h"

// == Hardware setup/s == 
#define PHOENIX_SHIELD_V_01
//#define AQ_SHIELD_V_20
//#define AQ_SHIELD_V_21

#ifdef PHOENIX_SHIELD_V_01
    // Features requested
    #define Magnetometer
    #define AltitudeHoldBaro
    #define AltitudeHoldSonar
    #define BatteryMonitorCurrent
    #define GPS
    
    // Critical sensors on board (gyro/accel)
    #include <mpu6050_10DOF_stick_px01.h>
    
    // Magnetometer
    #include <Magnetometer_HMC5883L.h>
    
    // Barometer
    #include <Barometer_ms5611.h>
    
    // Sonar
    #include <Sonar_srf04.h>
    
    // GPS (ublox neo 6m)
    #include <GPS_ublox.h>
    
    // Current sensor
    #include <BatteryMonitor_current.h>
    
    // Kinematics used
    #include <kinematics_CMP.h>
    
    // Receiver
    #include <Receiver_teensy3_HW_PPM.h>
    
    // Frame type definition
    #include <FrameType_QuadX.h> 

    // Motor / ESC setup
    #define ESC_400HZ
    #include <ESC_teensy3_HW.h>     
#endif

#ifdef AQ_SHIELD_V_20
    // Features requested
    #define Magnetometer
    
    // Critical sensors on board (gyro/accel)
    #include <ITG3200_AQ_v20.h>
    #include <BMA180_AQ_v20.h>
    
    // Magnetometer
    #include <Magnetometer_HMC5883L.h>
    
    // Kinematics used
    #include <kinematics_CMP.h>
    
    // Receiver
    #include <Receiver_328p_HW_PPM.h> // this is just temporary, this shield requires proper mega 1280 2560 support
    
    // Frame type definition
    #include <FrameType_QuadX.h> 

    // Motor / ESC setup
    #include <ESC_328p_HW.h> // this is just temporary, this shield requires proper mega 1280 2560 support      
#endif

#ifdef AQ_SHIELD_V_21
    // Features requested
    #define Magnetometer
    
    // Critical sensors on board (gyro/accel)
    #include <ITG3200_AQ_v21.h>
    #include <ADXL345_AQ_v21.h>
    
    // Magnetometer
    #include <Magnetometer_HMC5883L.h>
    
    // Kinematics used
    #include <kinematics_CMP.h>
    
    // Receiver
    #include <Receiver_328p_HW_PPM.h> // this is just temporary, this shield requires proper mega 1280 2560 support
    
    // Frame type definition
    #include <FrameType_QuadX.h> 

    // Motor / ESC setup
    #include <ESC_328p_HW.h> // this is just temporary, this shield requires proper mega 1280 2560 support     
#endif
// == END of Hardware setup ==

// Global PID object definitions
PID yaw_command_pid;
PID pitch_command_pid;
PID roll_command_pid;

PID yaw_motor_pid;
PID pitch_motor_pid;
PID roll_motor_pid;

#ifdef AltitudeHoldBaro
    PID altitude_hold_baro_pid;
#endif

#ifdef AltitudeHoldSonar    
    PID altitude_hold_sonar_pid;
#endif

#ifdef GPS
    PID yaw_position_hold_pid;
    PID pitch_position_hold_pid;
    PID roll_position_hold_pid;
#endif  

// Function to reset I terms inside PID objects
void reset_PID_integrals() {
    yaw_command_pid.IntegralReset();
    pitch_command_pid.IntegralReset();
    roll_command_pid.IntegralReset();
    
    yaw_motor_pid.IntegralReset();
    pitch_motor_pid.IntegralReset();
    roll_motor_pid.IntegralReset();
    
    #ifdef AltitudeHoldBaro
        altitude_hold_baro_pid.IntegralReset();
    #endif
    
    #ifdef AltitudeHoldSonar
        altitude_hold_sonar_pid.IntegralReset();
    #endif      
}
  
// Include this last as it contains objects from previous declarations
#include "GPS.h"
#include "PilotCommandProcessor.h"
#include "SerialCommunication.h"  
  
void setup() {
    // PIN settings
    pinMode(LED_PIN, OUTPUT); // build in status LED
    pinMode(LED_ORIENTATION, OUTPUT); // orientation lights

    // Initialize serial communication
    Serial.begin(38400); // Virtual USB Serial on teensy 3.0 is always 12 Mbit/sec (can be initialized with baud rate 0)

    #ifdef GPS
        Serial3.begin(38400);
    #endif
 
    // Join I2C bus as master
    Wire.begin();

    // I2C bus hardware specific settings
    #if defined(__MK20DX128__)
        // I2C bus settings
        I2C0_F = 0x00; // 2.4 MHz (prescaler 20)
        I2C0_FLT = 4;
    #endif
    
    #if defined(__AVR__)
        // I2C bus settings
        TWBR = 12; // 400 KHz (maximum supported frequency)
    #endif
    
    // Read data from EEPROM to CONFIG union
    readEEPROM();
    
    // Initialize PID objects with data from EEPROM
    yaw_command_pid = PID(&headingError, &YawCommandPIDSpeed, &headingSetpoint, 
        &CONFIG.data.PID_YAW_c[P], &CONFIG.data.PID_YAW_c[I], &CONFIG.data.PID_YAW_c[D], &CONFIG.data.PID_YAW_c[WG]);
        
    pitch_command_pid = PID(&kinematicsAngle[YAXIS], &PitchCommandPIDSpeed, &commandPitch, 
        &CONFIG.data.PID_PITCH_c[P], &CONFIG.data.PID_PITCH_c[I], &CONFIG.data.PID_PITCH_c[D], &CONFIG.data.PID_PITCH_c[WG]);
        
    roll_command_pid = PID(&kinematicsAngle[XAXIS], &RollCommandPIDSpeed, &commandRoll, 
        &CONFIG.data.PID_ROLL_c[P], &CONFIG.data.PID_ROLL_c[I], &CONFIG.data.PID_ROLL_c[D], &CONFIG.data.PID_ROLL_c[WG]);
  
    yaw_motor_pid = PID(&gyro[ZAXIS], &YawMotorSpeed, &YawCommandPIDSpeed,
        &CONFIG.data.PID_YAW_m[P], &CONFIG.data.PID_YAW_m[I], &CONFIG.data.PID_YAW_m[D], &CONFIG.data.PID_YAW_m[WG]);
        
    pitch_motor_pid = PID(&gyro[YAXIS], &PitchMotorSpeed, &PitchCommandPIDSpeed, 
        &CONFIG.data.PID_PITCH_m[P], &CONFIG.data.PID_PITCH_m[I], &CONFIG.data.PID_PITCH_m[D], &CONFIG.data.PID_PITCH_m[WG]);
        
    roll_motor_pid = PID(&gyro[XAXIS], &RollMotorSpeed, &RollCommandPIDSpeed, 
        &CONFIG.data.PID_ROLL_m[P], &CONFIG.data.PID_ROLL_m[I], &CONFIG.data.PID_ROLL_m[D], &CONFIG.data.PID_ROLL_m[WG]);  
    
    #ifdef AltitudeHoldBaro
        altitude_hold_baro_pid = PID(&baroAltitudeToHoldTarget, &AltitudeHoldMotorSpeed, &baroAltitudeRunning, 
            &CONFIG.data.PID_BARO[P], &CONFIG.data.PID_BARO[I], &CONFIG.data.PID_BARO[D], &CONFIG.data.PID_BARO[WG]);
    #endif
    
    #ifdef AltitudeHoldSonar
        altitude_hold_sonar_pid = PID(&sonarAltitudeToHoldTarget, &AltitudeHoldMotorSpeed, &sonarAltitude, 
            &CONFIG.data.PID_SONAR[P], &CONFIG.data.PID_SONAR[I], &CONFIG.data.PID_SONAR[D], &CONFIG.data.PID_SONAR[WG]);
    #endif    
    
    #ifdef GPS
        // yaw_position_hold_pid = PID();
        // pitch_position_hold = PID();
        // roll_position_hold = PID();
    #endif
    
    // Initialize motors/receivers/sensors
    initializeESC();    
    initializeReceiver();
    
    sensors.initializeGyro();
    sensors.initializeAccel();
    
    #ifdef Magnetometer
        sensors.initializeMag();
    #endif
    
    #ifdef AltitudeHoldBaro
        sensors.initializeBaro();    
    #endif
    
    #ifdef AltitudeHoldSonar
        initializeSonar();
    #endif    
    
    #ifdef GPS
        gps.initializeBaseStation();
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
        sensors.readAccelSum();        
        
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

    // Listens/read Serial commands on Serial1 interface (used to pass data from configurator)
    readSerial();
    
    // Update kinematics with latest data
    kinematics_update(gyro[XAXIS], gyro[YAXIS], gyro[ZAXIS], accel[XAXIS], accel[YAXIS], accel[ZAXIS]);
    
    // Update heading
    headingError = kinematicsAngle[ZAXIS] - commandYawAttitude;
    NORMALIZE(headingError); // +- PI
    
    // Update PIDs according the selected mode
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
    
    // This is the place where the actual "force" gets applied
    if (armed) {
        updateMotorsMix(); // Frame specific motor mix
        updateMotors(); // Update ESCs
    } else {
        // Reset all motors to 0 throttle/power
        #if MOTORS == 3
            MotorOut[0] = 1000;
            MotorOut[1] = 1000;
            MotorOut[2] = 1000;
        #elif MOTORS == 4
            MotorOut[0] = 1000;
            MotorOut[1] = 1000;
            MotorOut[2] = 1000;
            MotorOut[3] = 1000;
        #elif MOTORS == 6
            MotorOut[0] = 1000;
            MotorOut[1] = 1000;
            MotorOut[2] = 1000;
            MotorOut[3] = 1000;
            MotorOut[4] = 1000;
            MotorOut[5] = 1000;
        #elif MOTORS == 8
            MotorOut[0] = 1000;
            MotorOut[1] = 1000;
            MotorOut[2] = 1000;
            MotorOut[3] = 1000;
            MotorOut[4] = 1000;
            MotorOut[5] = 1000; 
            MotorOut[6] = 1000;
            MotorOut[7] = 1000;
        #endif
        updateMotors(); // Update ESCs
    } 
}

void process50HzTask() {
    processPilotCommands();
    
    #ifdef AltitudeHoldBaro
        sensors.evaluateBaroAltitude();
    #endif   

    // Blink LED to indicated activity
    if ((Alive_LED_state == 51) || (Alive_LED_state == 59) || (Alive_LED_state == 67)) {
        digitalWrite(LED_PIN, HIGH);
    } else {
        digitalWrite(LED_PIN, LOW);
    }

    if (Alive_LED_state >= 100) {
        Alive_LED_state = 0;
    } else {
        Alive_LED_state++;
    }
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
        sensors.evaluateMag();
    #endif
    
    #ifdef BatteryMonitorCurrent
        readBatteryMonitorCurrent();
    #endif   
    
    // Print itterations per 100ms
    #ifdef DISPLAY_ITTERATIONS
        Serial.println(itterations);
    #endif
    
    // Reset Itterations
    itterations = 0;    
}

void process1HzTask() {   
    // Orientation ligts
    // also displaying armed / dis-armed status
    if (armed) {
        digitalWrite(LED_ORIENTATION, HIGH);
    } else {
        digitalWrite(LED_ORIENTATION, LOW);
    }
}