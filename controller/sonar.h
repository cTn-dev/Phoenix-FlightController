#define SONAR_RANGE 300 // 300cm
#define SONAR_PIN_TRIGGER 15
#define SONAR_PIN_ECHO 16

unsigned long sonar_start = 0;
unsigned long sonar_end = 0;
bool sonar_ready = true;

double sonar_raw = 0.0; // This shouldn't be used directly / ergo needs a smoothing filter
double sonarAltitude = 0.0;
double sonarAltitudeToHoldTarget = 0.0;
int16_t sonarAltitudeHoldThrottle = 1000;

void readSonar() {
    // SRF04 Requires at least 10us long TTL pulse to trigger
    // This pin will go low in next 1000 <= us
    digitalWrite(SONAR_PIN_TRIGGER, HIGH);
}

void readSonarFinish() {
    digitalWrite(SONAR_PIN_TRIGGER, LOW);
}

void sonarEcho() {
    if (sonar_ready) {
        sonar_start = micros();
        sonar_ready = false;
    } else {
        // The speed of sound is 340 m/s or 29 microseconds per centimeter.
        // The ping travels out and back, so to find the distance of the
        // object we take half of the distance travelled.       
        sonar_end = micros();
        
        sonar_raw = sonar_end - sonar_start;
        sonar_raw = sonar_raw / 29.0 / 2.0;
        
        if (sonar_raw > SONAR_RANGE) {
            sonar_raw = 0;
        } else {
            // Compute
            sonarAltitude = filterSmooth(sonar_raw, sonarAltitude, 0.02);
        }
        
        sonar_ready = true;
    }
}