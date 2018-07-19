#pragma once

#include "ofMain.h"
#include "ofxFaceTracker2.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
        void draw();
        void keyPressed(int key);
    
    
        ofVideoGrabber grabber;
        ofxFaceTracker2 tracker;
    
        bool transformB;
        int lineLength;
        float globalEyeDistance;
        float originY, originX;

};
