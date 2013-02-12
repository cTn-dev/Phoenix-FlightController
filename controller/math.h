// Fast inverse square root implementation
// @see: http://en.wikipedia.org/wiki/Fast_inverse_square_root
float invSqrt(float number) {
    /* temporary disabled
    long i;
    float x, y;
    const float f = 1.5F;

    x = number * 0.5F;
    y = number;
    i = * ( long * ) &y;
    i = 0x5f375a86 - ( i >> 1 ); // older version with less accuracy = 0x5f3759df
    y = * ( float * ) &i;
    y = y * ( f - ( x * y * y ) );
    
    return y;
    */
    
    return sqrt(number);
}

// Signed square root implementation
float s_invSqrt(float val) {
    if (val < 0.00) {
        // negative input = negative sqrt
        return -invSqrt(abs(val));
    }
    return invSqrt(val);
}

// Smooth Filter
float filterSmooth(float currentData, float previousData, float smoothFactor) {
    return (previousData * (1.0 - smoothFactor) + (currentData * smoothFactor)); 
}