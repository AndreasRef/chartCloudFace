#include "ofApp.h"

using namespace ofxCv;
using namespace cv;

//--------------------------------------------------------------
void ofApp::setup(){
    learned_functions = vector<pfunct_type>(4);
    // Load SVM data model
    
    dlib::deserialize(ofToDataPath("data_ecstatic_smile.func")) >> learned_functions[0];
    dlib::deserialize(ofToDataPath("data_small_smile.func")) >> learned_functions[1];
    
    //Static image + video
//    img.load("images/faces.jpg");
//    img.resize(ofGetWidth(),ofGetHeight());
//
//    video.load("videos/trump_short.mp4");
//    video.setLoopState(OF_LOOP_NORMAL);
//    video.setVolume(0);
//    video.play();
    
    // All examples share data files from example-data, so setting data path to this folder
    // This is only relevant for the example apps
    ofSetDataPathRoot(ofFile(__BASE_FILE__).getEnclosingDirectory()+"../../model/");
    
    // Setup grabber
    grabber.setup(1280,720);
    
    // Setup tracker
    tracker.setup();
    
    //CLAHE
    outputImage.allocate(1280, 720, OF_IMAGE_COLOR);
    
    //Have up to 100 faces at a time
    bigSmileValues.resize(100);
    smallSmileValues.resize(100);
    
    for (int i = 0; i<smallSmileValues.size();i++) {
        smallSmileValues[i].setFc(0.04);
        bigSmileValues[i].setFc(0.04);
    }
}

//--------------------------------------------------------------
void ofApp::update(){
    grabber.update();
    //video.update();
    //tracker.update(video);
    
    if(grabber.isFrameNew()){
        
        //CLAHE
        cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
        clahe->setClipLimit(clipLimit);
        
        //Grayscale
        ofxCv::copyGray(grabber, greyImg);
        
        // apply the CLAHE algorithm
        clahe->apply(greyImg,claheImg);
        
        // convert to ofImage
        ofxCv::toOf(claheImg, outputImage);
        outputImage.update();
        
        
        
        //tracker.update(grabber);
        tracker.update(outputImage);
        
        
        vector<ofxFaceTracker2Instance> instances = tracker.getInstances();
        if (instances.size() == 0) {
            return;
        }
        
        for (int i = 0; i< tracker.size(); i++) {
            
            //FACEPOSE CALCULATIONS START
            // initialize variables for pose decomposition
            ofVec3f scale, transition;
            ofQuaternion rotation, orientation;
            
            // get pose matrix
            ofMatrix4x4 p = instances[i].getPoseMatrix();
            
            // decompose the modelview
            ofMatrix4x4(p).decompose(transition, rotation, scale, orientation);
            
            ofPushView();
            ofPushMatrix();
            
            // obtain pitch, roll, yaw
            double pitch =
            atan2(2*(rotation.y()*rotation.z()+rotation.w()*rotation.x()),rotation.w()*rotation.w()-rotation.x()*rotation.x()-rotation.y()*rotation.y()+rotation.z()*rotation.z());
            double roll =
            atan2(2*(rotation.x()*rotation.y()+rotation.w()*rotation.z()),rotation.w()*rotation.w()+rotation.x()*rotation.x()-rotation.y()*rotation.y()-rotation.z()*rotation.z());
            double yaw =
            asin(-2*(rotation.x()*rotation.z()-rotation.w()*rotation.y()));
            
//            ofLog()<<"pitch "<<pitch; //pitch is a good indicator of facing up/down
//            ofLog()<<"roll "<<roll;
//            ofLog()<<"yaw "<<yaw; //yaw is a good indicator of facing sideways
            
            ofPopMatrix();
            ofPopView();
            //FACEPOSE CALCULATIONS END
            
            bigSmileValues[i].update(ofClamp(learned_functions[0](makeSampleID(i))*1.2-abs(yaw)+MIN(0,pitch)*1,0, 1));
            smallSmileValues[i].update(ofClamp(learned_functions[1](makeSampleID(i))*1.2-abs(yaw)+MIN(0,pitch)*1,0, 1));
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    //grabber.draw(0, 0);
    outputImage.draw(0,0);
    //video.draw(0,0);
    tracker.drawDebug();
    
#ifndef __OPTIMIZE__
    ofSetColor(ofColor::Red);
    ofDrawBitmapString("Warning! Run this app in release mode to get proper performance!",10,60);
    ofSetColor(ofColor::White);
#endif
    
    
    //New vectorized test
    for (int i = 0; i < tracker.size(); i++) {
        
        ofRectangle bb = tracker.getInstances()[i].getBoundingBox();
        
        float val = smallSmileValues[i].value() + bigSmileValues[i].value();
        //ofDrawRectangle(20, 20 + 100*i, 300*val, 30);
        
        ofDrawBitmapStringHighlight("smile", bb.x, bb.y -10 );
        ofDrawRectangle(bb.x + 50, bb.y - 25, 100*val, 20);
        
        ofNoFill();
        ofDrawRectangle(bb.x + 50, bb.y - 25, 100, 20);
        ofFill();
        
    }
    
    string info = "Clip Limit " + ofToString(clipLimit) + "\n";
    ofDrawBitmapStringHighlight(info, ofGetWidth()/2-30, 50);
    
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

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == OF_KEY_UP) {
        clipLimit ++;
    }
    if (key == OF_KEY_DOWN) {
        clipLimit --;
    }
}



