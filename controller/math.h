// Signed square root
double s_sqrt(double val) {
    if (val < 0.00) {
        // negative input = negative sqrt
        return -sqrt(abs(val));
    }
    return sqrt(val);
}