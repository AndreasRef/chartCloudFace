#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    filteredEyeBrows.setFc(0.04);
    
    
    // All examples share data files from example-data, so setting data path to this folder
    // This is only relevant for the example apps
    ofSetDataPathRoot(ofFile(__BASE_FILE__).getEnclosingDirectory()+"../../model/");
    
    vector<ofVideoDevice> devices = grabber.listDevices();
    
    for(int i = 0; i < devices.size(); i++){
        if(devices[i].bAvailable){
            ofLogNotice() << devices[i].id << ": " << devices[i].deviceName;
        }else{
            ofLogNotice() << devices[i].id << ": " << devices[i].deviceName << " - unavailable ";
        }
    }
    
    grabber.setDeviceID(0);
    
    // Setup grabber
    grabber.setup(1280,720);
    
    // Setup tracker
    tracker.setup();
    
    // open an outgoing connection to HOST:PORT
    sender.setup(HOST, PORT);
    
}

//--------------------------------------------------------------
void ofApp::update(){
    grabber.update();
    if(grabber.isFrameNew()){
        tracker.update(grabber);
        
        vector<ofxFaceTracker2Instance> instances = tracker.getInstances();
        if (instances.size() == 0) {
            return;
        }
        
        //Old calculations Sloppy eyebrows, should be redone with orientation or perhaps inspired by the stuff that happens in (makeSample)?
        faceDist = tracker.getInstances()[0].getPoseMatrix().getRowAsVec3f(3)[2];//Okay measure for distance between -2000, -10000
        faceDistMapped = ofMap(faceDist, -10000,-2000,0,5);
        
        float eyeBrowInput = ((getGesture(RIGHT_EYEBROW_HEIGHT) + getGesture(LEFT_EYEBROW_HEIGHT) + getGesture(RIGHT_EYE_OPENNESS) + getGesture(LEFT_EYE_OPENNESS)) -0.8);
        filteredEyeBrows.update(eyeBrowInput); //Also add eyes
        
        cout << "rightEye: ";
        cout << ofToString(getGesture(RIGHT_EYE_OPENNESS)) << endl;
        
        cout << "leftEye: ";
        cout << ofToString(getGesture(LEFT_EYE_OPENNESS)) << endl;
        
        
        cout << "rightEyebrow: ";
        cout << ofToString(getGesture(RIGHT_EYEBROW_HEIGHT)) << endl;
        
        cout << "leftEyebrow: ";
        cout << ofToString(getGesture(LEFT_EYEBROW_HEIGHT)) << endl;
        
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    grabber.draw(0, 0);
    
    tracker.drawDebug();
    
    ofSetColor(255);
#ifndef __OPTIMIZE__
    ofSetColor(ofColor::Red);
    ofDrawBitmapString("Warning! Run this app in release mode to get proper performance!",10,60);
    ofSetColor(ofColor::White);
#endif
    
    float val = filteredEyeBrows.value();
    string str = "EYEBROW ADDED: " + ofToString(val);
    
    ofFill();
    ofDrawBitmapStringHighlight(str, 20, 110);
    ofDrawRectangle(20, 120, 300*val, 30);
    
    ofNoFill();
    ofDrawRectangle(20, 120, 300, 30);
}


//--------------------------------------------------------------
float ofApp:: getGesture (Gesture gesture){
    
    if(tracker.size()<1) {
        return 0;
    }
    int start = 0, end = 0;
    
    float compareFloat = 1.0;
    
    switch(gesture) {
            // left to right of mouth
        case MOUTH_WIDTH: start = 48; end = 54; break;
            // top to bottom of inner mouth
        case MOUTH_HEIGHT: start = 51; end = 57; break;
            // center of the eye to middle of eyebrow
        case LEFT_EYEBROW_HEIGHT: start = 38; end = 20;
            
            compareFloat = tracker.getInstances()[0].getLandmarks().getImagePoint(33).y - tracker.getInstances()[0].getLandmarks().getImagePoint(27).y;
            
            break;
            // center of the eye to middle of eyebrow
        case RIGHT_EYEBROW_HEIGHT: start = 43; end = 23;
            
            compareFloat = tracker.getInstances()[0].getLandmarks().getImagePoint(33).y - tracker.getInstances()[0].getLandmarks().getImagePoint(27).y;
            
            break;
            // upper inner eye to lower outer eye
        case LEFT_EYE_OPENNESS: start = 38; end = 40;
            compareFloat = ofDist(
                                  tracker.getInstances()[0].getLandmarks().getImagePoint(36).x,
                                  tracker.getInstances()[0].getLandmarks().getImagePoint(36).y,
                                  tracker.getInstances()[0].getLandmarks().getImagePoint(39).x,
                                  tracker.getInstances()[0].getLandmarks().getImagePoint(39).y
                                  );
            break;
            // upper inner eye to lower outer eye
        case RIGHT_EYE_OPENNESS: start = 43; end = 47;
            
            compareFloat = ofDist(
                                  tracker.getInstances()[0].getLandmarks().getImagePoint(42).x,
                                  tracker.getInstances()[0].getLandmarks().getImagePoint(42).y,
                                  tracker.getInstances()[0].getLandmarks().getImagePoint(45).x,
                                  tracker.getInstances()[0].getLandmarks().getImagePoint(45).y
                                  );
            break;
            // nose center to chin center
        case JAW_OPENNESS: start = 33; end = 8; break;
            // left side of nose to right side of nose
        case NOSTRIL_FLARE: start = 31; end = 35; break;
    }
    
    float gestureFloat = abs(tracker.getInstances()[0].getLandmarks().getImagePoint(start).y - tracker.getInstances()[0].getLandmarks().getImagePoint(end).y);
    return (gestureFloat/compareFloat);
}


