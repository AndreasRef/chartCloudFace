#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    learned_functions = vector<pfunct_type>(4);

    // Load SVM data model
    dlib::deserialize(ofToDataPath("data_ecstatic_smile.func")) >> learned_functions[0];
    dlib::deserialize(ofToDataPath("data_small_smile.func")) >> learned_functions[1];
    dlib::deserialize(ofToDataPath("data_o.func")) >> learned_functions[2];
    dlib::deserialize(ofToDataPath("data_neutral.func")) >> learned_functions[3];
    
    // Setup value filters for the classifer
    neutralValue.setFc(0.04);
    bigSmileValue.setFc(0.04);
    smallSmileValue.setFc(0.04);
    oValue.setFc(0.04);
    filteredEyeBrows.setFc(0.04);
    

    // All examples share data files from example-data, so setting data path to this folder
    // This is only relevant for the example apps
    //ofSetDataPathRoot(ofFile(__BASE_FILE__).getEnclosingDirectory()+"../../model/");
    
    
    
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
    
    
    //Sound
    happy.load("sounds/Happy.wav");
    neutral.load("sounds/Neutral.wav");
    sad.load("sounds/Worried.wav");
    happy.setVolume(0.75f);
    neutral.setVolume(0.75f);
    sad.setVolume(0.5f);
    
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
        
//        ofxFaceTracker2Landmarks landmarks = instances[0].getLandmarks();
//        vector<ofVec2f> points = landmarks.getImagePoints();
//        ofRectangle bb = instances[0].getBoundingBox();
        
            
            // Run the classifiers and update the filters
            bigSmileValue.update(learned_functions[0](makeSample()));
            smallSmileValue.update(learned_functions[1](makeSample()));
            oValue.update(learned_functions[2](makeSample()));
            neutralValue.update(learned_functions[3](makeSample()));
            
            ofxOscMessage m;
            m.setAddress("/smileValues");
            m.addFloatArg(smallSmileValue.value());
            m.addFloatArg(bigSmileValue.value());
            sender.sendMessage(m, false);
        
            
            //Sloppy eyebrows, should be redone with orientation or perhaps inspired by the stuff that happens in (makeSample)?
            
            //Gesture
            faceDist = tracker.getInstances()[0].getPoseMatrix().getRowAsVec3f(3)[2];//Okay measure for distance between -2000, -10000
            faceDistMapped = ofMap(faceDist, -10000,-2000,0,5);
        
        float eyeBrowInput = (getGesture(RIGHT_EYEBROW_HEIGHT) + getGesture(LEFT_EYEBROW_HEIGHT) + getGesture(RIGHT_EYE_OPENNESS) + getGesture(LEFT_EYE_OPENNESS)) / 4;
            
            filteredEyeBrows.update(eyeBrowInput); //Also add eyes
            //filteredEyeBrows.update( getGesture(RIGHT_EYEBROW_HEIGHT) / (faceDistMapped) + getGesture(LEFT_EYEBROW_HEIGHT) / (faceDistMapped) );
        //filteredEyeBrows.update( getGesture(RIGHT_EYEBROW_HEIGHT) + getGesture(LEFT_EYEBROW_HEIGHT));
        
    }
    
    ofSoundUpdate();
}

//--------------------------------------------------------------
void ofApp::draw(){
    grabber.draw(0, 0);
    
    
    
    
    
    //Quick & Dirty emotion prototype
    
    
    if (neutralValue.value() > 0.3 )  {
        ofSetColor(0,0,255); //neutral: blue
        emotion = "neutral";
        if (filteredEyeBrows.value() < 0.4) {
            ofSetColor(255,0,0); //sad: red
            emotion = "sad";
        }
        
    } else {
        ofSetColor(0,255,0); //happy: green
        emotion = "happy";
    }
    
    ofDrawBitmapStringHighlight(emotion, ofGetWidth()/2, 100);
    
    tracker.drawDebug();
    
    ofSetColor(255);
    
#ifndef __OPTIMIZE__
    ofSetColor(ofColor::Red);
    ofDrawBitmapString("Warning! Run this app in release mode to get proper performance!",10,60);
    ofSetColor(ofColor::White);
#endif
    
    
    ofPushMatrix();
    ofTranslate(0, 100);
    for (int i = 0; i < 5; i++) {
        ofSetColor(255);
        
        string str;
        float val;
        switch (i) {
            case 0:
                str = "BIG SMILE";
                val = bigSmileValue.value();
                break;
            case 1:
                str = "SMALL SMILE";
                val = smallSmileValue.value();
                break;
            case 2:
                str = "OOO MOUTH";
                val = oValue.value();
                break;
            case 3:
                str = "NEUTRAL MOUTH";
                val = neutralValue.value();
                break;
            case 4:
                str = "EYEBROW ADDED";
                val = filteredEyeBrows.value();
                break;
        }
        
        ofDrawBitmapStringHighlight(str, 20, 0);
        ofDrawRectangle(20, 20, 300*val, 30);
        
        ofNoFill();
        ofDrawRectangle(20, 20, 300, 30);
        ofFill();       
        
        ofTranslate(0, 70);
    }
    
    ofPopMatrix();

}


// Function that creates a sample for the classifier containing the mouth and eyes
sample_type ofApp::makeSample(){
    auto outer = tracker.getInstances()[0].getLandmarks().getImageFeature(ofxFaceTracker2Landmarks::OUTER_MOUTH);
    auto inner = tracker.getInstances()[0].getLandmarks().getImageFeature(ofxFaceTracker2Landmarks::INNER_MOUTH);
    
    auto lEye = tracker.getInstances()[0].getLandmarks().getImageFeature(ofxFaceTracker2Landmarks::LEFT_EYE);
    auto rEye = tracker.getInstances()[0].getLandmarks().getImageFeature(ofxFaceTracker2Landmarks::RIGHT_EYE);
    
    ofVec2f vec = rEye.getCentroid2D() - lEye.getCentroid2D();
    float rot = vec.angle(ofVec2f(1,0));
    
    vector<ofVec2f> relativeMouthPoints;
    
    ofVec2f centroid = outer.getCentroid2D();
    for(ofVec2f p : outer.getVertices()){
        p -= centroid;
        p.rotate(rot);
        p /= vec.length();
        
        relativeMouthPoints.push_back(p);
    }
    
    for(ofVec2f p : inner.getVertices()){
        p -= centroid;
        p.rotate(rot);
        p /= vec.length();
        
        relativeMouthPoints.push_back(p);
    }
    
    sample_type s;
    for(int i=0;i<20;i++){
        s(i*2+0) = relativeMouthPoints[i].x;
        s(i*2+1) = relativeMouthPoints[i].y;
    }
    return s;
}



//--------------------------------------------------------------
float ofApp:: getGesture (Gesture gesture){
    
    //Current issues: How to make it scale accordingly
    
    
    if(tracker.size()<1) {
        return 0;
    }
    int start = 0, end = 0;
    int gestureMultiplier = 10;
    
    
    switch(gesture) {
            // left to right of mouth
        case MOUTH_WIDTH: start = 48; end = 54; break;
            // top to bottom of inner mouth
        case MOUTH_HEIGHT: start = 51; end = 57; gestureMultiplier = 10; break;
            // center of the eye to middle of eyebrow
        case LEFT_EYEBROW_HEIGHT: start = 38; end = 20; gestureMultiplier = 10; break;
            // center of the eye to middle of eyebrow
        case RIGHT_EYEBROW_HEIGHT: start = 43; end = 23; gestureMultiplier = 10; break;
            // upper inner eye to lower outer eye
        case LEFT_EYE_OPENNESS: start = 38; end = 40; gestureMultiplier = 25; break;
            // upper inner eye to lower outer eye
        case RIGHT_EYE_OPENNESS: start = 43; end = 47; gestureMultiplier = 25; break;
            // nose center to chin center
        case JAW_OPENNESS: start = 33; end = 8; break;
            // left side of nose to right side of nose
        case NOSTRIL_FLARE: start = 31; end = 35; break;
    }
    
    //    return (1000*abs(tracker.getInstances()[0].getLandmarks().getImagePoint(start).getNormalized().x - tracker.getInstances()[0].getLandmarks().getImagePoint(end).getNormalized().x) + abs(tracker.getInstances()[0].getLandmarks().getImagePoint(start).getNormalized().y - tracker.getInstances()[0].getLandmarks().getImagePoint(end).getNormalized().y));
    
    
    //Attempting to only calculate based on either x or y
    
    //Normalized
    
    return (gestureMultiplier*abs(abs(tracker.getInstances()[0].getLandmarks().getImagePoint(start).getNormalized().y - tracker.getInstances()[0].getLandmarks().getImagePoint(end).getNormalized().y)));
    
    //Not normalized
    //            return 0.01 * abs(tracker.getInstances()[0].getLandmarks().getImagePoint(start).y - tracker.getInstances()[0].getLandmarks().getImagePoint(end).y);
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
    if (emotion == "sad") {
        sad.play();
    } else if (emotion == "happy") {
        happy.play();
    } else if (emotion == "neutral") {
        neutral.play();
    }
}


