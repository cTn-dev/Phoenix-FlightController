// Signed square root implementation
double s_sqrt(double val) {
    if (val < 0.00) {
        // negative input = negative sqrt
        return -sqrt(abs(val));
    }
    return sqrt(val);
}

// Fast inverse square root implementation
// @see: http://en.wikipedia.org/wiki/Fast_inverse_square_root
float invSqrt(float number) {
    long i;
    float x, y;
    const float f = 1.5F;

    x = number * 0.5F;
    y = number;
    i = * ( long * ) &y;
    i = 0x5f375a86 - ( i >> 1 );
    y = * ( float * ) &i;
    y = y * ( f - ( x * y * y ) );
    return y;
}