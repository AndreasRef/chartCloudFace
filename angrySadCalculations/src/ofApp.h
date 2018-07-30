#pragma once

#include "ofMain.h"
#include "ofxFaceTracker2.h"
#include "ofxBiquadFilter.h" 

#include "ofxOsc.h"

#define HOST "localhost"
#define PORT 8000



class ofApp : public ofBaseApp {
public:
    void setup();
    void update();
    void draw();
    
    ofxFaceTracker2 tracker;
    ofVideoGrabber grabber;
    
    ofxBiquadFilter1f filteredEyeBrows;
    
    ofxOscSender sender;
    
    //Gestures
    enum Gesture {
        MOUTH_WIDTH, MOUTH_HEIGHT,
        LEFT_EYEBROW_HEIGHT, RIGHT_EYEBROW_HEIGHT,
        LEFT_EYE_OPENNESS, RIGHT_EYE_OPENNESS,
        JAW_OPENNESS,
        NOSTRIL_FLARE
    };
    
    float getGesture (Gesture gesture);
    
    float faceDist;
    float faceDistMapped;
    
    string emotion;
    
};
