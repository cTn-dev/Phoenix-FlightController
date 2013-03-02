/*
    Kinematics implementation using first order complementary filter by cTn
    
    Extra:
    1. accelerometer data normalization
    2. accelerometer data cut-off
    3. kinematics-gyroscope angle overflow handling

    Kinematics input is averaged (from multiple samples) and scaled (gyro is in radians/s) and accel is in m/s^2
    accel measurement is normalized before any angles are computed.
*/

unsigned long kinematics_timer;

void kinematics_update(float gyroX, float gyroY, float gyroZ, float accelX, float accelY, float accelZ) {

    // Normalize accel values
    float norm = sqrt(accelX * accelX + accelY * accelY + accelZ * accelZ);
    accelX /= norm;
    accelY /= norm;
    accelZ /= norm;
    
    // Determinate Sensor orientation
    bool orientation = true; // up-side UP
    if (accelZ < 0.00) orientation = false; // up-side DOWN    
    
    float accelXangle = atan2(accelY, accelZ);
    float accelYangle = atan2(accelX, accelZ); 
    //float accelYangle = atan2(*accelX, sqrt(*accelY * *accelY + *accelZ * *accelZ));   
    
    // Accelerometer cut-off
    float accelWeight = 0.0050; // normal operation
    if (norm > 13.0 || norm < 7.0) accelWeight = 0.00; // gyro only
    
    // Save current time into variable for better computation time
    unsigned long now = micros();    
    
    // Fuse in gyroscope
    kinematicsAngle[XAXIS] += (gyroX * (float)(now - kinematics_timer) / 1000000);
    kinematicsAngle[YAXIS] += (gyroY * (float)(now - kinematics_timer) / 1000000);
    kinematicsAngle[ZAXIS] += (gyroZ * (float)(now - kinematics_timer) / 1000000);  
    
    // Normalize gyro kinematics (+- PI)
    NORMALIZE(kinematicsAngle[XAXIS]);
    NORMALIZE(kinematicsAngle[YAXIS]);
    NORMALIZE(kinematicsAngle[ZAXIS]);

    // Fuse in accel
    // This is second order accelerometer cut off, which restricts accel data fusion in only
    // "up-side UP" orientation and restricts it further to avoid incorrect accelerometer data fusion.
    if (accelZ > 0.75) {
        if ((kinematicsAngle[XAXIS] - accelXangle) > PI) {
            kinematicsAngle[XAXIS] = (1.00 - accelWeight) * kinematicsAngle[XAXIS] + accelWeight * (accelXangle + TWO_PI);
        } else if ((kinematicsAngle[XAXIS] - accelXangle) < -PI) {
            kinematicsAngle[XAXIS] = (1.00 - accelWeight) * kinematicsAngle[XAXIS] + accelWeight * (accelXangle - TWO_PI);
        } else {
            kinematicsAngle[XAXIS] = (1.00 - accelWeight) * kinematicsAngle[XAXIS] + accelWeight * accelXangle;
        }
    }
    
    if (accelZ > 0.60) {
        if ((kinematicsAngle[YAXIS] - accelYangle) > PI) {
            kinematicsAngle[YAXIS] = (1.00 - accelWeight) * kinematicsAngle[YAXIS] + accelWeight * (accelYangle + TWO_PI);
        } else if ((kinematicsAngle[YAXIS] - accelYangle) < -PI) {
            kinematicsAngle[YAXIS] = (1.00 - accelWeight) * kinematicsAngle[YAXIS] + accelWeight * (accelYangle - TWO_PI);
        } else {
            kinematicsAngle[YAXIS] = (1.00 - accelWeight) * kinematicsAngle[YAXIS] + accelWeight * accelYangle;
        } 
    }
    // Saves time for next comparison
    kinematics_timer = now;
}