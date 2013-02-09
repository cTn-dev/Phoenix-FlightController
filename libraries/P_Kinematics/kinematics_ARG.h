// Quaternion implementation of the 'DCM filter' [Mayhony et al].
//
// This is the EXACT ARG kinematics taken from aeroquad flight controll software
// @see: https://github.com/AeroQuad/AeroQuad
//
// User must define 'halfT' as the (sample period / 2), and the filter gains 'Kp' and 'Ki'.
//
// Global variables 'q0', 'q1', 'q2', 'q3' are the quaternion elements representing the estimated
// orientation.
//
// User must call 'IMUupdate()' every sample period and parse calibrated gyroscope ('gx', 'gy', 'gz')
// and accelerometer ('ax', 'ay', 'ay') data.  Gyroscope units are radians/second, accelerometer 
// units are irrelevant as the vector is normalised.

float q0 = 1.0;
float q1 = 0.0;
float q2 = 0.0;
float q3 = 0.0;
float exInt = 0.0;
float eyInt = 0.0;
float ezInt = 0.0; 

float previousEx = 0;
float previousEy = 0;
float previousEz = 0;
  
float Kp = 0.2;
float Ki = 0.0005;

unsigned long kinematics_timer;

boolean isSwitched(float previousError, float currentError) {
    if ( (previousError > 0 &&  currentError < 0) || (previousError < 0 &&  currentError > 0)) {
        return true;
    }
    return false;
}

void argUpdate(float gx, float gy, float gz, float ax, float ay, float az, float G_Dt) {

    float norm;
    float vx, vy, vz;
    float q0i, q1i, q2i, q3i;
    float ex, ey, ez;

    float halfT = G_Dt / 2;

    // normalise the measurements
    norm = sqrt(ax * ax + ay * ay + az * az);       
    ax = ax / norm;
    ay = ay / norm;
    az = az / norm;

    // estimated direction of gravity and flux (v and w)
    vx = 2 * (q1 * q3 - q0 * q2);
    vy = 2 * (q0 * q1 + q2 * q3);
    vz = q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3;

    // error is sum of cross product between reference direction of fields and direction measured by sensors
    ex = (vy * az - vz * ay);
    ey = (vz * ax - vx * az);
    ez = (vx * ay - vy * ax);

    // integral error scaled integral gain
    exInt = exInt + ex * Ki;
    if (isSwitched(previousEx,ex)) {
        exInt = 0.0;
    }
    previousEx = ex;

    eyInt = eyInt + ey * Ki;
    if (isSwitched(previousEy,ey)) {
        eyInt = 0.0;
    }
    previousEy = ey;

    ezInt = ezInt + ez * Ki;
    if (isSwitched(previousEz,ez)) {
        ezInt = 0.0;
    }
    previousEz = ez;

    // adjusted gyroscope measurements
    gx = gx + Kp * ex + exInt;
    gy = gy + Kp * ey + eyInt;
    gz = gz + Kp * ez + ezInt;

    // integrate quaternion rate and normalise
    q0i = (-q1 * gx - q2 * gy - q3 * gz) * halfT;
    q1i = ( q0 * gx + q2 * gz - q3 * gy) * halfT;
    q2i = ( q0 * gy - q1 * gz + q3 * gx) * halfT;
    q3i = ( q0 * gz + q1 * gy - q2 * gx) * halfT;
    q0 += q0i;
    q1 += q1i;
    q2 += q2i;
    q3 += q3i;

    // normalise quaternion
    norm = sqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    q0 = q0 / norm;
    q1 = q1 / norm;
    q2 = q2 / norm;
    q3 = q3 / norm;
}

void kinematics_update(double* gyroX, double* gyroY, double* gyroZ, double* accelX, double* accelY, double* accelZ) {    
    // Change signs on some of the variables
    float accelXfloat = *accelX;
    float accelYfloat = -*accelY;
    float accelZfloat = -*accelZ;
    
    // Store current time
    float now = micros();
    
    argUpdate(*gyroX, *gyroY, *gyroZ, accelXfloat, accelYfloat, accelZfloat, (now - kinematics_timer) / 1000000.0);
    
    // Save kinematics time for next comparison
    kinematics_timer = now;
    
    kinematicsAngle[XAXIS] = atan2(2 * (q0 * q1 + q2 * q3), 1 - 2 * (q1 * q1 + q2 * q2));
    kinematicsAngle[YAXIS] = asin(2 * (q0 * q2 - q1 * q3));
    kinematicsAngle[ZAXIS] = atan2(2 * (q0 * q3 + q1 * q2), 1 - 2 * (q2 * q2 + q3 * q3));    
}