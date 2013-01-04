void drawAxisX() {
  /* draw first x-axis */
  noFill();
  stroke(255,0,0); // red
  // redraw everything
  beginShape();
  for(int i = 0; i<firstX.length;i++)
    vertex(i,firstX[i]);
  endShape();
  // put all data one array back
  for(int i = 1; i<firstX.length;i++)
    firstX[i-1] = firstX[i];   
   
  /* draw second x-axis */
  noFill();
  stroke(0,0,255); // blue
  // redraw everything
  beginShape();
  for(int i = 0; i<secondX.length;i++)
    vertex(i,secondX[i]);  
  endShape();
  // put all data one array back
  for(int i = 1; i<secondX.length;i++)
    secondX[i-1] = secondX[i];   
   
  /* draw third filter x-axis */
  noFill();
  stroke(0,255,0); // green
  // redraw everything
  beginShape();
  for(int i = 0; i<thirdX.length;i++)
    vertex(i,thirdX[i]);
  endShape();
  // put all data one array back
  for(int i = 1; i<thirdX.length;i++)
    thirdX[i-1] = thirdX[i];  
   
  /* draw fourth filter x-axis */
  noFill();
  stroke(255,180,0); // orange
  // redraw everything
  beginShape();
  for(int i = 0; i<fourthX.length;i++)
    vertex(i,fourthX[i]);  
  endShape();
  // put all data one array back
  for(int i = 1; i<fourthX.length;i++)
    fourthX[i-1] = fourthX[i];
}

void drawAxisY() {
  /* draw first y-axis */
  noFill();
  stroke(255,0,0); // red
  // redraw everything
  beginShape();
  for(int i = 0; i<firstY.length;i++)
    vertex(i,firstY[i]);
  endShape();
  // put all data one array back
  for(int i = 1; i<firstY.length;i++)
   firstY[i-1] = firstY[i];
   
  /* draw second y-axis */
  noFill();
  stroke(0,0,255); // blue
  // redraw everything
  beginShape();
  for(int i = 0; i<secondY.length;i++)
    vertex(i,secondY[i]);
  endShape();
  // put all data one array back
  for(int i = 1; i<secondY.length;i++)
    secondY[i-1] = secondY[i];
   
  /* draw third filter y-axis */
  noFill();
  stroke(0,255,0); // green
  // redraw everything
  beginShape();
  for(int i = 0; i<thirdY.length;i++)
    vertex(i,thirdY[i]);
  endShape();
  // put all data one array back
  for(int i = 1; i<thirdY.length;i++)
    thirdY[i-1] = thirdY[i];
  
  /* draw fourth filter y-axis */
  noFill();
  stroke(255,180,0); // orange
  // redraw everything
  beginShape();
  for(int i = 0; i<fourthY.length;i++)
    vertex(i,fourthY[i]);
  endShape();
  //put all data one array back
  for(int i = 1; i<fourthY.length;i++)
    fourthY[i-1] = fourthY[i];
}    

