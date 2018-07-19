#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    learned_functions = vector<pfunct_type>(4);
    learned_functions2 = vector<pfunct_type>(4);
    
    // Load SVM data model
    
    dlib::deserialize(ofToDataPath("data_small_smile.func")) >> learned_functions[0];
    dlib::deserialize(ofToDataPath("data_small_smile.func")) >> learned_functions2[0];
    //    dlib::deserialize(ofToDataPath("data_ecstatic_smile.func")) >> learned_functions[0];
    //    dlib::deserialize(ofToDataPath("data_small_smile.func")) >> learned_functions[1];
    //    dlib::deserialize(ofToDataPath("data_o.func")) >> learned_functions[2];
    //    dlib::deserialize(ofToDataPath("data_neutral.func")) >> learned_functions[3];
    //
    //    dlib::deserialize(ofToDataPath("data_ecstatic_smile.func")) >> learned_functions2[0];
    //    dlib::deserialize(ofToDataPath("data_small_smile.func")) >> learned_functions2[1];
    //    dlib::deserialize(ofToDataPath("data_o.func")) >> learned_functions2[2];
    //    dlib::deserialize(ofToDataPath("data_neutral.func")) >> learned_functions2[3];
    
    // Setup value filters for the classifer
    smallSmileValue.setFc(0.04);
    smallSmileValue2.setFc(0.04);
    
    // All examples share data files from example-data, so setting data path to this folder
    // This is only relevant for the example apps
    ofSetDataPathRoot(ofFile(__BASE_FILE__).getEnclosingDirectory()+"../../model/");
    
    // Setup grabber
    grabber.setup(1280,720);
    
    // Setup tracker
    tracker.setup();
}

//--------------------------------------------------------------
void ofApp::update(){
    grabber.update();
    if(grabber.isFrameNew()){
        tracker.update(grabber);
        
        if(tracker.size() > 1){
            // Run the classifiers and update the filters
            
            smallSmileValue.update(learned_functions[0](makeSample()));
            smallSmileValue2.update(learned_functions2[0](makeSample2()));
            
            //Debug
            ofPoint centerFacePos = tracker.getInstances()[0].getLandmarks().getImagePoint(27);
            //cout << centerFacePos << endl;
            
            
            //Debug
            ofPoint centerFacePos2 = tracker.getInstances()[1].getLandmarks().getImagePoint(27);
            //cout << centerFacePos2 << endl;
            
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    grabber.draw(0, 0);
    tracker.drawDebug();
    
#ifndef __OPTIMIZE__
    ofSetColor(ofColor::Red);
    ofDrawBitmapString("Warning! Run this app in release mode to get proper performance!",10,60);
    ofSetColor(ofColor::White);
#endif
    
    
    ofPushMatrix();
    ofTranslate(0, 100);
    for (int i = 0; i < 1; i++) {
        ofSetColor(255);
        
        string str;
        float val;
        switch (i) {
            case 0:
                str = "SMALL SMILE 0";
                val = smallSmileValue.value();
                break;
        }
        
        ofDrawBitmapStringHighlight(str, 20, 0);
        ofDrawRectangle(20, 20, 300*val, 30);
        
        ofNoFill();
        ofDrawRectangle(20, 20, 300, 30);
        ofFill();
    }
    ofPopMatrix();
    
    
    ofPushMatrix();
    ofTranslate(0, 200);
    for (int i = 0; i < 1; i++) {
        ofSetColor(255);
        
        string str;
        float val;
        switch (i) {
            case 0:
                str = "SMALL SMILE 1";
                val = smallSmileValue2.value();
                break;
        }
        
        ofDrawBitmapStringHighlight(str, 20, 0);
        ofDrawRectangle(20, 20, 300*val, 30);
        
        ofNoFill();
        ofDrawRectangle(20, 20, 300, 30);
        ofFill();
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


sample_type ofApp::makeSample2(){
    auto outer = tracker.getInstances()[1].getLandmarks().getImageFeature(ofxFaceTracker2Landmarks::OUTER_MOUTH);
    auto inner = tracker.getInstances()[1].getLandmarks().getImageFeature(ofxFaceTracker2Landmarks::INNER_MOUTH);
    
    auto lEye = tracker.getInstances()[1].getLandmarks().getImageFeature(ofxFaceTracker2Landmarks::LEFT_EYE);
    auto rEye = tracker.getInstances()[1].getLandmarks().getImageFeature(ofxFaceTracker2Landmarks::RIGHT_EYE);
    
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

