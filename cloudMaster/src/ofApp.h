#pragma once

#include "ofMain.h"
#include "ofxOsc.h"

#define ABLETONPORT 9000
#define MADMAPPERPORT 8000

#define WEBCAM1PORT 12000
#define WEBCAM2PORT 12001

#define WEBCAM1MESSAGE "/webcam1"
#define WEBCAM2MESSAGE "/webcam2"


class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
        void recieveOsc();
        void sendOsc();
    
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
    
    
};
