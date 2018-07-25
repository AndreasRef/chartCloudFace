#include "ofApp.h"

using namespace ofxDLib;

const float dyingTime = 1;

void FaceAugmented::setup(const Face & track) {
    color.setHsb(ofRandom(0, 255), 255, 255);
    cur = track.rect.getCenter();
    smooth = cur;
    roi = track.rect;
    face = track;
}

void FaceAugmented::update(const Face & track) {
    cur = track.rect.getCenter();
    roi = track.rect;
    smooth.interpolate(cur, .5);
    all.addVertex(smooth);
    face = track;
}

void FaceAugmented::setImage(const ofPixels & pixels) {
    pixels.cropTo(image.getPixels(), roi.getX(), roi.getY(), roi.getWidth(), roi.getHeight());
    image.update();
}

void FaceAugmented::kill() {
    float curTime = ofGetElapsedTimef();
    if(startedDying == 0) {
        startedDying = curTime;
    } else if(curTime - startedDying > dyingTime) {
        dead = true;
    }
}

void FaceAugmented::draw() {
    // draw face rect
    ofNoFill();
    ofSetColor(255, 0, 0);
    ofDrawRectangle(face.rect);
    
    // draw face features
    face.leftEye.draw();
    face.rightEye.draw();
    face.innerMouth.draw();
    face.outerMouth.draw();
    face.leftEyebrow.draw();
    face.rightEyebrow.draw();
    face.jaw.draw();
    face.noseBridge.draw();
    face.noseTip.draw();
    
    //Testing how to acces the individual points
//    for (int i=0; i<face.jaw.size();i++) {
//        ofPoint p = face.jaw.getPointAtIndexInterpolated(i);
//        ofFill();
//        ofDrawCircle(p.x, p.y, 2);
//    }
    
    ofDrawCircle(face.leftEyeCenter, 7);
    ofDrawCircle(face.rightEyeCenter, 7);;
    
    // draw label
    ofSetColor(255, 255, 255);
    ofDrawBitmapString(ofToString(label), cur);
    
    //ofPopStyle();
}


//--------------------------------------------------------------
void FaceAugmented::drawNumbers() {
    ofSetColor(255);
    
    int featureNumber = 0;
    
    for (int i=0; i<face.leftEye.size();i++) {
        ofPoint p = face.leftEye.getPointAtIndexInterpolated(i);
        ofFill();
        ofDrawBitmapString(ofToString(featureNumber), p.x, p.y);
        ofDrawCircle(p.x, p.y, 2);
        featureNumber++;
    }
    
    for (int i=0; i<face.rightEye.size();i++) {
        ofPoint p = face.rightEye.getPointAtIndexInterpolated(i);
        ofFill();
        ofDrawBitmapString(ofToString(featureNumber), p.x, p.y);
        ofDrawCircle(p.x, p.y, 2);
        featureNumber++;
    }
    
    for (int i=0; i<face.innerMouth.size();i++) {
        ofPoint p = face.innerMouth.getPointAtIndexInterpolated(i);
        ofFill();
        ofDrawBitmapString(ofToString(featureNumber), p.x, p.y);
        ofDrawCircle(p.x, p.y, 2);
        featureNumber++;
    }
    
    for (int i=0; i<face.outerMouth.size();i++) {
        ofPoint p = face.outerMouth.getPointAtIndexInterpolated(i);
        ofFill();
        ofDrawBitmapString(ofToString(featureNumber), p.x, p.y);
        ofDrawCircle(p.x, p.y, 2);
        featureNumber++;
    }
    
    for (int i=0; i<face.leftEyebrow.size();i++) {
        ofPoint p = face.leftEyebrow.getPointAtIndexInterpolated(i);
        ofFill();
        ofDrawBitmapString(ofToString(featureNumber), p.x, p.y);
        ofDrawCircle(p.x, p.y, 2);
        featureNumber++;
    }
    
    for (int i=0; i<face.rightEyebrow.size();i++) {
        ofPoint p = face.rightEyebrow.getPointAtIndexInterpolated(i);
        ofFill();
        ofDrawBitmapString(ofToString(featureNumber), p.x, p.y);
        ofDrawCircle(p.x, p.y, 2);
        featureNumber++;
    }
    
    for (int i=0; i<face.jaw.size();i++) {
        ofPoint p = face.jaw.getPointAtIndexInterpolated(i);
        ofFill();
        ofDrawBitmapString(ofToString(featureNumber), p.x, p.y);
        ofDrawCircle(p.x, p.y, 2);
        featureNumber++;
    }
    
    for (int i=0; i<face.noseBridge.size();i++) {
        ofPoint p = face.noseBridge.getPointAtIndexInterpolated(i);
        ofFill();
        ofDrawBitmapString(ofToString(featureNumber), p.x, p.y);
        ofDrawCircle(p.x, p.y, 2);
        featureNumber++;
    }
    
    for (int i=0; i<face.noseTip.size();i++) {
        ofPoint p = face.noseTip.getPointAtIndexInterpolated(i);
        ofFill();
        ofDrawBitmapString(ofToString(featureNumber), p.x, p.y);
        ofDrawCircle(p.x, p.y, 2);
    }
}


//--------------------------------------------------------------
void ofApp::setup(){
    tracker.setup("shape_predictor_68_face_landmarks.dat");
    tracker.setSmoothingRate(0.5);
    tracker.setDrawStyle(ofxDLib::lines);
    tracker.getTracker().setPersistence(5);
    tracker.getTracker().setMaximumDistance(80);
    
    video.setDeviceID(1);
    video.setup(640, 480);
}

//--------------------------------------------------------------
void ofApp::update(){
    video.update();
    if(video.isFrameNew()){
        tracker.findFaces(video.getPixels());
        vector<FaceAugmented>& facesAugmented = tracker.getFollowers();
        for (auto & face : facesAugmented) {
            face.setImage(video.getPixels());
        }
        // amount of movement regulates smoothing rate
        for(int i=0; i < tracker.size(); i++) {
            int label = tracker.getLabel(i);
            int length = tracker.getVelocity(i).length();
            length = length < 10 ? length : 10; // arbitrarily choosen
            float smoothingRate = ofMap(length, 0, 10, 0.35, 1);
            tracker.setSmoothingRate(label, smoothingRate);
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofSetColor(255,255,255);
    video.draw(0, 0);
    vector<FaceAugmented>& facesAugmented = tracker.getFollowers();
    for (auto & face : facesAugmented) {
        face.draw();
        //face.drawNumbers();
    }
    tracker.draw();
    
    ofDrawBitmapStringHighlight("nFaces: " + ofToString(tracker.size()), 10, 20);
}
