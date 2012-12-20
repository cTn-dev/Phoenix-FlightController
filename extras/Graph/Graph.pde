import processing.serial.*; 
Serial arduino; 

String stringFirstX;
String stringFirstY;

String stringSecondX;
String stringSecondY;

String stringThirdX;
String stringThirdY;

String stringFourthX;
String stringFourthY;

int width = 1200;
int height = 800;

float[] firstX = new float[width];
float[] firstY = new float[width];

float[] secondX = new float[width];
float[] secondY = new float[width];

float[] thirdX = new float[width];
float[] thirdY = new float[width];

float[] fourthX = new float[width];
float[] fourthY = new float[width];

void setup() {  
  size(width, height);
  //println(arduino.list()); // Use this to print connected serial devices
  arduino = new Serial(this, "COM3", 38400);
  arduino.bufferUntil('\n'); // Buffer until line feed

  for (int i=0;i<width;i++) { // center all variables
    firstX[i] = height/2;
    firstY[i] = height/2;
    secondX[i] = height/2;
    secondY[i] = height/2;
    thirdX[i] = height/2; 
    thirdY[i] = height/2; 
    fourthX[i] = height/2; 
    fourthY[i] = height/2;
  }
}

void draw()
{ 
  // Draw graphPaper
  background(255); // white
  for (int i = 0 ;i<=width/10;i++) {      
    stroke(200); // gray
    line((-frameCount%10)+i*10, 0, (-frameCount%10)+i*10, height);
    line(0, i*10, width, i*10);
  }

  stroke(0); // black
  for (int i = 1; i <= 3; i++)
    line(0, height/4*i, width, height/4*i); // Draw line, indicating 90 deg, 180 deg, and 270 deg

  convert();
  drawAxisX();
  drawAxisY();
}

void serialEvent (Serial arduino) {
  // get the ASCII strings:  
  stringFirstX = arduino.readStringUntil('\t');
  stringFirstY = arduino.readStringUntil('\t');
  
  stringSecondX = arduino.readStringUntil('\t');
  stringSecondY = arduino.readStringUntil('\t');

  stringThirdX = arduino.readStringUntil('\t');
  stringThirdY = arduino.readStringUntil('\t'); 

  stringFourthX = arduino.readStringUntil('\t');
  stringFourthY = arduino.readStringUntil('\t');
  
  arduino.clear(); // Clear buffer

  //printAxis(); // slows down the process and can result in error readings - use for debugging
}

void printAxis() {
  print(stringFirstX);
  print(stringFirstY);

  print(stringSecondX);
  print(stringSecondY);

  print(stringThirdX);
  print(stringThirdY);

  print(stringFourthX);
  print(stringFourthY);
}
