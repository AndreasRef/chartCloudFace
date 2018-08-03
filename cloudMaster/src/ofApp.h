#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "ofxGui.h"

#define ABLETONPORT 9000
#define MADMAPPERPORT 10001

#define WEBCAM1PORT 12000
#define WEBCAM2PORT 12001

#define WEBCAMMESSAGE "/webcam"

#define MADMAPPERMESSAGE "/faceinfo"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
        void AddNewValue(float val);
        void recieveOsc();
        void sendOsc();
    
    void longTermMinutesChanged(int & longTermMinutes);
    
    ofxOscReceiver receiverWebcam1;
    ofxOscReceiver receiverWebcam2;
    
    ofxOscSender senderToAbleton;
    ofxOscSender senderToMadmapper;
    
    //Summed variables
    int nFacesTotal;
    float avgMoodTotal;
    float varMoodTotal;
    float longTermAvgMoodTotal;
    
    
    //Webcam1 variables
    int nFacesWebcam1;
    float avgMoodWebcam1;
    float varMoodWebcam1;
    long webcam1Timer;
    
    //Webcam2 variables
    int nFacesWebcam2;
    float avgMoodWebcam2;
    float varMoodWebcam2;
    long webcam2Timer;
    
    
    //GUI
    ofxPanel gui;
    ofxIntSlider nFaceFactor;
    ofxFloatSlider longTermAvgMoodThreshold;
    ofxIntSlider longTermMinutes; 
    
    //longTermAvgMoodTotal variables
    float sum = 0;
    vector<float>storedValues; //rename!
    int count = 0;
    
    //Time
    unsigned int timeElapsed;
    unsigned int pTimeElapsed;
    
};
