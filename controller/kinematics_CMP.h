/*
    Kinematics implementation using first order complementary filter
    by cTn
*/

// Complementary Filter global variables
double kinematicsAngleX = 0.0;
double kinematicsAngleY = 0.0;
double kinematicsAngleZ = 0.0;

unsigned long kinematics_timer;

// Function where all the magic happen
// Accel units doesn't matter because they are normalized, but gyro units have to be properly scaled
// before they are fed into kinematics.
void kinematics_update(double* accelX, double* accelY, double* accelZ, double* gyroX, double* gyroY, double* gyroZ) {

    // Calculate accelerometer angles (roll/pitch)
    double accelXangle = atan2(*accelY, *accelZ);
    double accelYangle = atan2(*accelX, *accelZ);
    
    // Calculate Angles using complementary filter
    unsigned long now = micros();
    
    kinematicsAngleX = (0.99 * (kinematicsAngleX + (*gyroX * (double)(now - kinematics_timer) / 1000000))) + (0.01 * accelXangle);
    kinematicsAngleY = (0.99 * (kinematicsAngleY + (*gyroY * (double)(now - kinematics_timer) / 1000000))) + (0.01 * accelYangle);
    kinematicsAngleZ = kinematicsAngleZ + (*gyroZ * (double)(now - kinematics_timer) / 1000000);
    
    // Saves time for next comparison
    kinematics_timer = micros();    
}