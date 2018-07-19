#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup(){
    // Initialize variables
    transformB = true;
    lineLength = 100;
    globalEyeDistance = 50;
    originY = 300;
    originX = ofGetWidth() / 2;
    
    // Setup grabber
    grabber.setup(1280, 720);
    
    // Setup tracker
    ofSetDataPathRoot(ofFile(__BASE_FILE__).getEnclosingDirectory()+"../../model/");
    tracker.setup();
    //tracker.setup(ofToDataPath("shape_predictor_68_face_landmarks.dat"));
}

//--------------------------------------------------------------
void ofApp::update(){
    grabber.update();
    
    // Update tracker when there are new frames
    if(grabber.isFrameNew()){
        tracker.update(grabber);
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    ofSetColor(255);
    
    ofPushMatrix();
    ofScale(0.25, 0.25);
    grabber.setAnchorPoint(0, 0);
    grabber.draw(0,0);
    tracker.drawDebug(0, 0);
    ofPopMatrix();
    
    for (auto instance : tracker.getInstances()){
        // initialize variables for pose decomposition
        ofVec3f scale, transition;
        ofQuaternion rotation, orientation;
        
        // get pose matrix
        ofMatrix4x4 p = instance.getPoseMatrix();
        
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
        
        
        ofLog()<<"pitch "<<pitch;
        ofLog()<<"roll "<<roll;
        ofLog()<<"yaw "<<yaw; //yaw is a good indicator of facing sideways
        
        // get point of sellion - place our pose marker here
        ofPoint sellion = instance.getLandmarks().getImagePoint(27);
        
        
        if (transformB) {
            // mirror image
            ofTranslate(grabber.getWidth(), 0);
            ofScale(-1, 1, 1);
            
            // use sellion as image center point
            grabber.setAnchorPoint(sellion.x, sellion.y);
            
            // transpose to new origin;
            ofTranslate(originX, originY);
            
            // calculate the distance between booth eyes
            ofPoint centerLeftEye  = instance.getLandmarks().getImageFeature(ofxFaceTracker2Landmarks::LEFT_EYE).getCentroid2D();
            ofPoint centerRightEye = instance.getLandmarks().getImageFeature(ofxFaceTracker2Landmarks::RIGHT_EYE).getCentroid2D();
            float localEyeDistance = centerLeftEye.distance(centerRightEye);
            
            // scale in relation to the global eye distance
            float scaleFactor = globalEyeDistance / localEyeDistance;
            ofScale(scaleFactor, scaleFactor);
            
            // draw our transformed image
            grabber.draw(0,0);
        } else {
            // draw our transformed image
            grabber.draw(0,0);
            // demonstrate that there are slight differences in the pose markers obtained by the lib and calculated by me
            tracker.drawDebugPose();
            ofTranslate(sellion.x, sellion.y);
        }
        
        // apply rotation from pose matrix (need to flip the z-axis)
        ofScale(1, 1, -1);
        ofRotateX(ofRadToDeg(-pitch));
        ofRotateY(ofRadToDeg(yaw));
        ofRotateZ(ofRadToDeg(-roll));
        
        // draw a rectangle to indicate when coordinate system flips (why?)
        ofSetColor(255, 255, 0, 50);
        ofDrawRectangle(0, 0, lineLength, lineLength);
   
        // draw our pose marker
        ofSetLineWidth(1);
        ofSetColor(255);
        ofSetColor(255,0,0);
        ofDrawLine(-lineLength,0,0, lineLength,0,0);
        ofSetColor(0,255,0);
        ofDrawLine(0,-lineLength,0, 0,lineLength,0);
        ofSetColor(0,0,255);
        ofDrawLine(0,0,0, 0,0,-lineLength);
        ofSetLineWidth(1);
        ofPopMatrix();
        ofPopView();
    }
    
}

void ofApp::keyPressed(int key){
    if(key == 't') transformB = !transformB;
}

