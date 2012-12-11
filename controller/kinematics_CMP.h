/*
    Kinematics implementation using first order complementary filter
    by cTn
*/

// Complementary Filter global variables
double kinematicsAngleX = 0.0;
double kinematicsAngleY = 0.0;
double kinematicsAngleZ = 0.0;

unsigned long complementary_timer;

// Function where all the magic happen
void kinematics_update(double* KaccelX, double* KaccelY, double* KaccelZ, double* KgyroX, double* KgyroY, double* KgyroZ) {

    // Calculate accelerometer angles (roll/pitch)
    double KaccelXangle = atan2(*KaccelY, *KaccelZ);
    double KaccelYangle = atan2(*KaccelX, *KaccelZ);
    
    // Calculate Angles using complementary filter
    unsigned long now = micros();
    
    kinematicsAngleX = (0.99 * (kinematicsAngleX + (*KgyroX * (double)(now - complementary_timer) / 1000000))) + (0.01 * KaccelXangle);
    kinematicsAngleY = (0.99 * (kinematicsAngleY + (*KgyroY * (double)(now - complementary_timer) / 1000000))) + (0.01 * KaccelYangle);
    kinematicsAngleZ = kinematicsAngleZ + (*KgyroZ * (double)(now - complementary_timer) / 1000000);
    
    // Saves time for next comparison
    complementary_timer = micros();    
}