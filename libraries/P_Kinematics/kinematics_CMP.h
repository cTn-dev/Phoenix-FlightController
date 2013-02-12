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
    float accelWeight = 0.0025; // normal operation
    if (norm > 13.0 || norm < 7.0) accelWeight = 0.00; // gyro only
    
    // Save current time into variable for better computation time
    unsigned long now = micros();    
    
    // Fuse in gyroscope
    kinematicsAngle[XAXIS] = kinematicsAngle[XAXIS] + (gyroX * (float)(now - kinematics_timer) / 1000000);
    kinematicsAngle[YAXIS] = kinematicsAngle[YAXIS] + (gyroY * (float)(now - kinematics_timer) / 1000000);
    kinematicsAngle[ZAXIS] = kinematicsAngle[ZAXIS] + (gyroZ * (float)(now - kinematics_timer) / 1000000);  
    
    // Normalize gyro kinematics (+ - PI)
    if (kinematicsAngle[XAXIS] > PI) kinematicsAngle[XAXIS] -= TWO_PI;
    else if (kinematicsAngle[XAXIS] < -PI) kinematicsAngle[XAXIS] += TWO_PI;    
    
    if (kinematicsAngle[YAXIS] > PI) kinematicsAngle[YAXIS] -= TWO_PI;
    else if (kinematicsAngle[YAXIS] < -PI) kinematicsAngle[YAXIS] += TWO_PI;
    
    // While normalizing Z angle, attitudeYaw (angle desired by user) is also normalized
    // attitudeYaw variable was added to handle clean normalization of angles while still
    // allowing for a smooth rate/attitude mode switching.
    if (kinematicsAngle[ZAXIS] > PI) {
        kinematicsAngle[ZAXIS] -= TWO_PI;
        commandYawAttitude -= TWO_PI;
    } else if (kinematicsAngle[ZAXIS] < -PI) {
        kinematicsAngle[ZAXIS] += TWO_PI; 
        commandYawAttitude += TWO_PI;
    }
    
    // Fuse in accel (handling accel flip)
    // This is second order accelerometer cut off, which restricts accel data fusion in only
    // "up-side UP" angle estimation and restricts it further to avoid incorrect accelerometer
    // data correction.
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