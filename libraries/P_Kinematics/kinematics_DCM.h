/*
    Quaternion implementation of the 'DCM filter' [Mayhony et al].  Incorporates the magnetic distortion
    compensation algorithms from Sebastian Madgwick's filter which eliminates the need for a reference
    direction of flux (bx bz) to be predefined and limits the effect of magnetic distortions to yaw axis only.

    This is the EXACT AHRS algorythm used in Free IMU, developed by Fabio Varesano (deceased) =(
*/

#define twoKpDef  (2.0f * 0.5f) // 2 * proportional gain
#define twoKiDef  (2.0f * 0.1f) // 2 * integral gain

// initialize quaternion
float q0 = 1.0f;
float q1 = 0.0f;
float q2 = 0.0f;
float q3 = 0.0f;
float exInt = 0.0;
float eyInt = 0.0;
float ezInt = 0.0;
float twoKp = twoKpDef;
float twoKi = twoKiDef;
float integralFBx = 0.0f,  integralFBy = 0.0f, integralFBz = 0.0f;
float sampleFreq;
unsigned long kinematics_timer = 0;

void AHRSupdate(float gx, float gy, float gz, float ax, float ay, float az) {
    float recipNorm;
    float q0q0, q0q1, q0q2, q0q3, q1q1, q1q2, q1q3, q2q2, q2q3, q3q3;
    float halfex = 0.0f, halfey = 0.0f, halfez = 0.0f;
    float qa, qb, qc;

    // Auxiliary variables to avoid repeated arithmetic
    q0q0 = q0 * q0;
    q0q1 = q0 * q1;
    q0q2 = q0 * q2;
    q0q3 = q0 * q3;
    q1q1 = q1 * q1;
    q1q2 = q1 * q2;
    q1q3 = q1 * q3;
    q2q2 = q2 * q2;
    q2q3 = q2 * q3;
    q3q3 = q3 * q3;
    
    // Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
    if((ax != 0.0f) && (ay != 0.0f) && (az != 0.0f)) {
        float halfvx, halfvy, halfvz;

        // Normalise accelerometer measurement
        recipNorm = invSqrt(ax * ax + ay * ay + az * az);
        ax *= recipNorm;
        ay *= recipNorm;
        az *= recipNorm;

        // Estimated direction of gravity
        halfvx = q1q3 - q0q2;
        halfvy = q0q1 + q2q3;
        halfvz = q0q0 - 0.5f + q3q3;

        // Error is sum of cross product between estimated direction and measured direction of field vectors
        halfex += (ay * halfvz - az * halfvy);
        halfey += (az * halfvx - ax * halfvz);
        halfez += (ax * halfvy - ay * halfvx);
    } 

    // Apply feedback only when valid data has been gathered from the accelerometer or magnetometer
    if(halfex != 0.0f && halfey != 0.0f && halfez != 0.0f) {
        // Compute and apply integral feedback if enabled
        if(twoKi > 0.0f) {
            integralFBx += twoKi * halfex * (1.0f / sampleFreq);  // integral error scaled by Ki
            integralFBy += twoKi * halfey * (1.0f / sampleFreq);
            integralFBz += twoKi * halfez * (1.0f / sampleFreq);
            gx += integralFBx;  // apply integral feedback
            gy += integralFBy;
            gz += integralFBz;
        }
        else {
            integralFBx = 0.0f; // prevent integral windup
            integralFBy = 0.0f;
            integralFBz = 0.0f;
        }

        // Apply proportional feedback
        gx += twoKp * halfex;
        gy += twoKp * halfey;
        gz += twoKp * halfez;
    }

    // Integrate rate of change of quaternion 
    gx *= (0.5f * (1.0f / sampleFreq));   // pre-multiply common factors
    gy *= (0.5f * (1.0f / sampleFreq));
    gz *= (0.5f * (1.0f / sampleFreq));
    qa = q0;
    qb = q1;
    qc = q2;
    q0 += (-qb * gx - qc * gy - q3 * gz);
    q1 += (qa * gx + qc * gz - q3 * gy);
    q2 += (qa * gy - qb * gz + q3 * gx);
    q3 += (qa * gz + qb * gy - qc * gx);

    // Normalise quaternion
    recipNorm = invSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    q0 *= recipNorm;
    q1 *= recipNorm;
    q2 *= recipNorm;
    q3 *= recipNorm;
}


void kinematics_update(double* gyroX, double* gyroY, double* gyroZ, double* accelX, double* accelY, double* accelZ) {
    unsigned long now = micros();
    sampleFreq = 1.0 / ((now - kinematics_timer) / 1000000.0);
    kinematics_timer = now;
    
    // Some sensors require different orientation, do the inverse here.
    // Also store sensor data in different variables as they are also used elsewhere
    float gyX = *gyroX;
    float gyY = -*gyroY;
    float gyZ = *gyroZ;
    float acX = *accelX;
    float acY = *accelY;
    float acZ = *accelZ;
    
    AHRSupdate(gyX, gyY, gyZ, acX, acY, acZ);
    
    float gx = 2 * (q1*q3 - q0*q2);
    float gy = 2 * (q0*q1 + q2*q3);
    float gz = q0*q0 - q1*q1 - q2*q2 + q3*q3;
    
    kinematicsAngle[XAXIS] = atan(gy / sqrt(gx*gx + gz*gz));
    kinematicsAngle[YAXIS] = atan(gx / sqrt(gy*gy + gz*gz));
    kinematicsAngle[ZAXIS] = atan2(2 * q1 * q2 - 2 * q0 * q3, 2 * q0*q0 + 2 * q1 * q1 - 1);
    
    // invert
    kinematicsAngle[ZAXIS] = -kinematicsAngle[ZAXIS];
}