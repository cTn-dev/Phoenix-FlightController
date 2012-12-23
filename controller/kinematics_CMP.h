/*
    Kinematics implementation using first order complementary filter 
    with accelerometer data normalization and accelerometer cut-off
    by cTn
    
    Kinematics input is averaged (from multiple samples) and scaled (gyro is in radians/s) and accel is in m/s^2
    accel measurement is normalized before any angles are computed.
*/

unsigned long kinematics_timer;

void kinematics_update(double* accelX, double* accelY, double* accelZ, double* gyroX, double* gyroY, double* gyroZ) {

    // Normalize accel values
    double norm = sqrt(*accelX * *accelX + *accelY * *accelY + *accelZ * *accelZ);
    *accelX /= norm;
    *accelY /= norm;
    *accelZ /= norm;
    
    // Calculate accelerometer angles (roll/pitch)
    double accelXangle = atan2(*accelY, *accelZ);
    double accelYangle = atan2(*accelX, *accelZ);
    
    // Save current time into variable for better computation time
    unsigned long now = micros();
    
    // Accelerometer cut-off
    double accelWeight = 0.01; // normal operation
    if (norm > 12.0) {
        accelWeight = 0.00; // gyro only
    }
    
    kinematicsAngleX = ((1.00 - accelWeight) * (kinematicsAngleX + (*gyroX * (double)(now - kinematics_timer) / 1000000))) + (accelWeight * accelXangle);
    kinematicsAngleY = ((1.00 - accelWeight) * (kinematicsAngleY + (*gyroY * (double)(now - kinematics_timer) / 1000000))) + (accelWeight * accelYangle);
    kinematicsAngleZ = kinematicsAngleZ + (*gyroZ * (double)(now - kinematics_timer) / 1000000);
    
    // Saves time for next comparison
    kinematics_timer = now;

    // Used for debugging
    #ifdef KINEMATICS_GRAPH
        Serial.print(kinematicsAngleX * RAD_TO_DEG + 180.0);
        Serial.write('\t');      
        Serial.print(kinematicsAngleY * RAD_TO_DEG + 180.0);
        Serial.write('\t');      
        Serial.print(kinematicsAngleZ * RAD_TO_DEG + 180.0);
        Serial.write('\t');              
        Serial.println();    
    #endif     
}