#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    learned_functions = vector<pfunct_type>(4);
    // Load SVM data model
    
    dlib::deserialize(ofToDataPath("data_small_smile.func")) >> learned_functions[0];
    //dlib::deserialize(ofToDataPath("data_small_smile.func")) >> learned_functions2[0];
    //    dlib::deserialize(ofToDataPath("data_ecstatic_smile.func")) >> learned_functions[0];
    //    dlib::deserialize(ofToDataPath("data_small_smile.func")) >> learned_functions[1];
    //    dlib::deserialize(ofToDataPath("data_o.func")) >> learned_functions[2];
    //    dlib::deserialize(ofToDataPath("data_neutral.func")) >> learned_functions[3];
    //
    //    dlib::deserialize(ofToDataPath("data_ecstatic_smile.func")) >> learned_functions2[0];
    //    dlib::deserialize(ofToDataPath("data_small_smile.func")) >> learned_functions2[1];
    //    dlib::deserialize(ofToDataPath("data_o.func")) >> learned_functions2[2];
    //    dlib::deserialize(ofToDataPath("data_neutral.func")) >> learned_functions2[3];
    
    // All examples share data files from example-data, so setting data path to this folder
    // This is only relevant for the example apps
    ofSetDataPathRoot(ofFile(__BASE_FILE__).getEnclosingDirectory()+"../../model/");
    
    // Setup grabber
    grabber.setup(1280,720);
    
    // Setup tracker
    tracker.setup();
    
    
    smallSmileValues.resize(2);
    
    for (int i = 0; i<smallSmileValues.size();i++) {
        smallSmileValues[i].setFc(0.04);
    }
    
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
        
        
        for (int i = 0; i< tracker.size(); i++) {
            smallSmileValues[i].update(learned_functions[0](makeSampleID(i)));
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
    
    
    //New vectorized test
    for (int i = 0; i < tracker.size(); i++) {
        float val = smallSmileValues[i].value();
        ofDrawRectangle(20, 20 + 100*i, 300*val, 30);
    }
    
}

//--------------------------------------------------------------
// Function that creates a sample for the classifier containing the mouth and eyes
sample_type ofApp::makeSampleID(int id){
    auto outer = tracker.getInstances()[id].getLandmarks().getImageFeature(ofxFaceTracker2Landmarks::OUTER_MOUTH);
    auto inner = tracker.getInstances()[id].getLandmarks().getImageFeature(ofxFaceTracker2Landmarks::INNER_MOUTH);
    
    auto lEye = tracker.getInstances()[id].getLandmarks().getImageFeature(ofxFaceTracker2Landmarks::LEFT_EYE);
    auto rEye = tracker.getInstances()[id].getLandmarks().getImageFeature(ofxFaceTracker2Landmarks::RIGHT_EYE);
    
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


