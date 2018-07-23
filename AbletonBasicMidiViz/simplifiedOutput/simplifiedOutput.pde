import oscP5.*;
import netP5.*;

OscP5 oscP5;

int pitch;
int velocity;
float x, y;

color fillColor = color(255);

void setup()
{
  size(800, 600);
  noStroke();
  colorMode(HSB);

  oscP5 = new OscP5(this, 12000);
}

void draw()
{
  fill(0,5);
  rect(0,0,width, height);
  
  fill(fillColor);
  ellipse(x, y, velocity, velocity);
}


void oscEvent(OscMessage theOscMessage) {
  if (theOscMessage.checkAddrPattern("note")==true) {
    if (theOscMessage.checkTypetag("ii")) {
      pitch = theOscMessage.get(0).intValue();  
      velocity = theOscMessage.get(1).intValue();  

      println("pitch : " + pitch);
      println("velocity : " + velocity);
      println("__________");
      println();

      if (pitch > 0 && velocity > 0) {       
         fillColor = color(map(pitch, 20, 100, 0, 255), 255, 255);
         x=random(width);
         y=random(height);
        
      }
    }
  }
}