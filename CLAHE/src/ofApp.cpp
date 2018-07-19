#include "ofApp.h"

using namespace ofxCv;
using namespace cv;

//--------------------------------------------------------------
void ofApp::setup() {
    input.initGrabber(640, 480);
    outputImage.allocate(640, 480, OF_IMAGE_COLOR);
}
//--------------------------------------------------------------
void ofApp::update() {
	 input.update();
}

//--------------------------------------------------------------
void ofApp::draw() {
    ofSetColor(255);
    
    int w = input.getWidth();
    int h = input.getHeight();
    input.draw(0, 0, w/2, h/2);
    outputImage.draw(w/2, 0, w/2, h/2);
    
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
    clahe->setClipLimit(clipLimit);
    
    // convert the color image to lab color space
    cv::cvtColor(ofxCv::toCv(input), labImg, CV_BGR2Lab);
    
    // Extract the L channel
    vector<cv::Mat> lab_planes(3);
    cv::split(labImg, lab_planes);  // now we have the L image in lab_planes[0]
    
    // apply the CLAHE algorithm to the L channel
    clahe->apply(lab_planes[0], tmpImg);
    
    // Merge the the color planes back into an Lab img
    tmpImg.copyTo(lab_planes[0]);
    cv::merge(lab_planes, labImg);
    
    // convert back to RGB
    cv::cvtColor(labImg, claheImg, CV_Lab2BGR);
    
    // convert to ofImage
    ofxCv::toOf(claheImg, outputImage);
    outputImage.update();
    
    ofSetColor(0);
    string info = "Clip Limit " + ofToString(clipLimit) + "\n";
    ofDrawBitmapString(info, 20, h/2 + 100);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == OF_KEY_UP) {
        clipLimit ++;
    }
    if (key == OF_KEY_DOWN) {
        clipLimit --;
    }
    
}
