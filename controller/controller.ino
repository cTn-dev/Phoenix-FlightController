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
#include "SerialCommunication.h"

// == Hardware setup/s == 
#define Maggie

#ifdef Maggie
    // Features requested
    #define Accelerometer
    #define Magnetometer
    #define AltitudeHoldBaro
    #define AltitudeHoldSonar
    #define BatteryMonitorCurrent
    #define GPS

    // Critical sensors on board (gyro/accel)
    #include <mpu6050.h>
    
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
    #include <ESC_teensy3_HW.h>        
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
#include "PilotCommandProcessor.h"  
  
void setup() {
    // PIN settings
    pinMode(LED_PIN, OUTPUT); // build in status LED
    pinMode(LED_ORIENTATION, OUTPUT); // orientation lights
    
    // Initialize serial communication
    Serial.begin(115200); // Virtual USB Serial on teensy 3.0 is always 12 Mbit/sec (can be initialized with baud rate 0)

    #ifdef GPS
        Serial3.begin(38400);
    #endif
 
    // Join I2C bus as master
    Wire.begin();

    // I2C bus hardware specific settings
    #if defined(__MK20DX128__)
        I2C0_F = 0x00; // 2.4 MHz (prescaler 20)
        I2C0_FLT = 4;
    #endif
    
    #if defined(__AVR__)
        TWBR = 12; // 400 KHz (maximum supported frequency)
    #endif
    
    // Read data from EEPROM to CONFIG union
    readEEPROM();
    
    // Initialize PID objects with data from EEPROM
    yaw_command_pid = PID(&kinematicsAngleZ, &YawCommandPIDSpeed, &commandYaw, &CONFIG.data.PID_YAW_c[P], &CONFIG.data.PID_YAW_c[I], &CONFIG.data.PID_YAW_c[D], &CONFIG.data.PID_YAW_c[WG]);
    pitch_command_pid = PID(&kinematicsAngleY, &PitchCommandPIDSpeed, &commandPitch, &CONFIG.data.PID_PITCH_c[P], &CONFIG.data.PID_PITCH_c[I], &CONFIG.data.PID_PITCH_c[D], &CONFIG.data.PID_PITCH_c[WG]);
    roll_command_pid = PID(&kinematicsAngleX, &RollCommandPIDSpeed, &commandRoll, &CONFIG.data.PID_ROLL_c[P], &CONFIG.data.PID_ROLL_c[I], &CONFIG.data.PID_ROLL_c[D], &CONFIG.data.PID_ROLL_c[WG]);
  
    yaw_motor_pid = PID(&gyro[ZAXIS], &YawMotorSpeed, &YawCommandPIDSpeed, &CONFIG.data.PID_YAW_m[P], &CONFIG.data.PID_YAW_m[I], &CONFIG.data.PID_YAW_m[D], &CONFIG.data.PID_YAW_m[WG]);
    pitch_motor_pid = PID(&gyro[YAXIS], &PitchMotorSpeed, &PitchCommandPIDSpeed, &CONFIG.data.PID_PITCH_m[P], &CONFIG.data.PID_PITCH_m[I], &CONFIG.data.PID_PITCH_m[D], &CONFIG.data.PID_PITCH_m[WG]);
    roll_motor_pid = PID(&gyro[XAXIS], &RollMotorSpeed, &RollCommandPIDSpeed, &CONFIG.data.PID_ROLL_m[P], &CONFIG.data.PID_ROLL_m[I], &CONFIG.data.PID_ROLL_m[D], &CONFIG.data.PID_ROLL_m[WG]);  
    
    #ifdef AltitudeHoldBaro
        altitude_hold_baro_pid = PID(&baroAltitudeToHoldTarget, &AltitudeHoldMotorSpeed, &baroAltitudeRunning, &CONFIG.data.PID_BARO[P], &CONFIG.data.PID_BARO[I], &CONFIG.data.PID_BARO[D], &CONFIG.data.PID_BARO[WG]);
    #endif
    
    #ifdef AltitudeHoldSonar
        altitude_hold_sonar_pid = PID(&sonarAltitudeToHoldTarget, &AltitudeHoldMotorSpeed, &sonarAltitude, &CONFIG.data.PID_SONAR[P], &CONFIG.data.PID_SONAR[I], &CONFIG.data.PID_SONAR[D], &CONFIG.data.PID_SONAR[WG]);
    #endif    
    
    // Initialize motors/receivers/sensors
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
    kinematics_update(&gyro[XAXIS], &gyro[YAXIS], &gyro[ZAXIS], &accel[XAXIS], &accel[YAXIS], &accel[ZAXIS]);
    
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
    
    // This section is place where the actual "force" gets applied
    if (armed) {
        updateMotorsMix(); // Frame specific motor mix
        updateMotors(); // Update ESCs
    } else {
        MotorOut[0] = 1000;
        MotorOut[1] = 1000;
        MotorOut[2] = 1000;
        MotorOut[3] = 1000;
        
        updateMotors();
    } 
}

void process50HzTask() {
    processPilotCommands();
    
    #ifdef AltitudeHoldBaro
        sensors.evaluateBaroAltitude();
    #endif   

    #ifdef DATA_VISUALIZATION
        // Gyro data
        Serial.print(gyro[XAXIS]);
        Serial.write(',');
        Serial.print(gyro[YAXIS]);
        Serial.write(',');
        Serial.print(gyro[ZAXIS]);
        Serial.write(',');        
        
        // Accel data
        Serial.print(accel[XAXIS]);
        Serial.write(',');
        Serial.print(accel[YAXIS]);
        Serial.write(','); 
        Serial.print(accel[ZAXIS]);
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
    
    // Listens/read Serial commands on Serial1 interface 
    // (used to pass configuration data from configurator)
    readSerial();
    
    // Blink LED to indicated activity
    Alive_LED_state = !Alive_LED_state;
    digitalWrite(LED_PIN, Alive_LED_state);    
    
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