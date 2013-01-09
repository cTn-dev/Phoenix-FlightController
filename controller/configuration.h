/*  This file defines sensor and future setup that will bundled inside the flight controller
    
    Phoenix flight controller was initially build to support only one array of sensors but
    with minor sensor changes it seemed like a waste to just "drop" previous sensor support
    with this idea in mind i will try to make the flight controller feature set flexible as
    possible to accommodate as much hw as possible.

    Defines below only enable/disable "pre-defined" hw setups, you can always define your own
    setup in controller.ino file in the == Hardware setup == section.
    
    -== Feature set ==-

    == Gyroscopes ==
    mpu6050
    
    == Accelerometers ==
    mpu6050
    
    == Magnetometers ==
    none
    
    == Barometers ==
    BMP085
    
    == Sonars / Ultrasonics = 
    SRF04
    
    == InfraRed ==    
    none
    
    == Kinematics ==
    #include "kinematics_CMP.h" // created by cTn with kha optimizations
    #include "kinematics_ARG.h" // aeroquads ARG 
    #include "kinematics_DCM.h" // FreeImu

*/

#define Maggie
