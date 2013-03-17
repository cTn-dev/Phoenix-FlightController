// Main loop variables
unsigned long currentTime = 0;
unsigned long previousTime = 0;
unsigned long sensorPreviousTime = 0;
uint8_t frameCounter = 0;
uint32_t itterations = 0;

bool all_ready = false;
bool armed = false;
bool flightMode = false;
bool altitudeHoldBaro = false;
bool altitudeHoldSonar = false;
bool positionHoldGPS = false;

// Flight modes
#define RATE_MODE 0
#define ATTITUDE_MODE 1

// Primary channel definitions
#define ROLL        0
#define PITCH       1
#define THROTTLE    2
#define YAW         3
#define FLIGHT_MODE 4

// PID pseudo definitions
#define P  0 // Proportional
#define I  1 // Integral
#define D  2 // Derivative
#define WG 3 // WindupGuard

// Axis definitions
#define XAXIS 0
#define YAXIS 1
#define ZAXIS 2

// Arbitrary ON/OFF definitions
#define OFF 0
#define ON 1

// Blinking LED to indicate activity
bool Arduino_LED_state = 0;
uint8_t Beacon_LED_state = 0;

// Modulo definitions (integer remainder)
#define TASK_50HZ 2
#define TASK_10HZ 10
#define TASK_1HZ 100

// Kinematics variable defnitions
float kinematicsAngle[3];

// FlightController commands definitions
float commandYaw, commandYawAttitude, commandPitch, commandRoll, commandThrottle;

// Heading related variables
float headingError = 0.0;
float headingSetpoint = 0.0;

// PID variables
float YawCommandPIDSpeed, PitchCommandPIDSpeed, RollCommandPIDSpeed;
float YawMotorSpeed, PitchMotorSpeed, RollMotorSpeed, AltitudeHoldMotorSpeed;
int16_t throttle = 1000;

// +- PI normalization macro
#define NORMALIZE(x) do { if ((x) < -PI) (x) += 2 * PI; else if ((x) > PI) (x) -= 2 * PI; } while (0);

// Custom definitions
//#define DISPLAY_ITTERATIONS