//convert all axis
int minAngle = 0;
int maxAngle = 360;

void convert() {   
  /* convert the gyro x-axis */
  if (stringFirstX != null) {
    // trim off any whitespace:
    stringFirstX = trim(stringFirstX);
    // convert to an float and map to the screen height, then save in buffer:    
    firstX[firstX.length-1] = map(float(stringFirstX), minAngle, maxAngle, 0, height);
  }
  
  /* convert the gyro y-axis */
  if (stringFirstY != null) {    
    // trim off any whitespace:
    stringFirstY = trim(stringFirstY);
    // convert to an float and map to the screen height, then save in buffer:   
    firstY[firstY.length-1] = map(float(stringFirstY), minAngle, maxAngle, 0, height);
  }
  
  /* convert the accelerometer x-axis */
  if (stringSecondX != null) {
    // trim off any whitespace:
    stringSecondX = trim(stringSecondX);
    // convert to an float and map to the screen height, then save in buffer:    
    secondX[secondX.length-1] = map(float(stringSecondX), minAngle, maxAngle, 0, height);
  }
  
  /* convert the accelerometer y-axis */
  if (stringSecondY != null) {
    // trim off any whitespace:
    stringSecondY = trim(stringSecondY);
    // convert to an float and map to the screen height, then save in buffer:        
    secondY[secondY.length-1] = map(float(stringSecondY), minAngle, maxAngle, 0, height);
  }

  /* convert the complementary filter x-axis */
  if (stringThirdX != null) {
    // trim off any whitespace:
    stringThirdX = trim(stringThirdX);
    // convert to an float and map to the screen height, then save in buffer:    
    thirdX[thirdX.length-1] = map(float(stringThirdX), minAngle, maxAngle, 0, height);
  }
  
  /* convert the complementary filter x-axis */
  if (stringThirdY != null) {
    // trim off any whitespace:
    stringThirdY = trim(stringThirdY);
    // convert to an float and map to the screen height, then save in buffer:    
    thirdY[thirdY.length-1] = map(float(stringThirdY), minAngle, maxAngle, 0, height);
  }
  
  /* convert the kalman filter x-axis */
  if (stringFourthX != null) {
    // trim off any whitespace:
    stringFourthX = trim(stringFourthX);
    // convert to an float and map to the screen height, then save in buffer:    
    fourthX[fourthX.length-1] = map(float(stringFourthX), minAngle, maxAngle, 0, height);
  }
  
  /* convert the kalman filter y-axis */
  if (stringFourthY != null) {
    // trim off any whitespace:
    stringFourthY = trim(stringFourthY);
    // convert to an float and map to the screen height, then save in buffer:    
    fourthY[fourthY.length-1] = map(float(stringFourthY), minAngle, maxAngle, 0, height);
  }
}
