#pragma once

#include "ofMain.h"
#include "ofxCv.h"

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
    void keyPressed(int key);

	//ofVideoGrabber cam;
	//ofImage gray, edge, sobel;
    
    int clipLimit = 1;
    
    cv::Mat greyImg, labImg, claheImg, tmpImg;
    
    ofVideoGrabber input;
    ofImage outputImage;
    
    ofImage gray;
    ofImage grayOutputImage;
};
