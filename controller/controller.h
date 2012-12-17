// main loop variables
uint8_t itterations = 0;
unsigned long currentTime = 0;
unsigned long previousTime = 0;
unsigned long sensorPreviousTime = 0;
uint8_t frameCounter = 0;
bool all_ready = false;
bool armed = false;
bool flightMode = false;

// Blinking LED to indicate activity
#define LED_PIN 13
bool blinkState = false;

// Modulo definitions (integer remainder)
#define TASK_50HZ 2
#define TASK_10HZ 10
#define TASK_1HZ 100

// Kinematics variable defnitions
double kinematicsAngleX = 0.0;
double kinematicsAngleY = 0.0;
double kinematicsAngleZ = 0.0;

// Custom definitions
//#define RX_GRAPH
//#define KINEMATICS_GRAPH
//#define SENSOR_DATA
//#define DISPLAY_ITTERATIONS