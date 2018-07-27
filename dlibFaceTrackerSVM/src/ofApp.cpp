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
    
    //How to get all the points? face.landmarks!
    ofDrawCircle(face.leftEyeCenter, 7);
    ofDrawCircle(face.rightEyeCenter, 7);
    
    // draw label
    ofSetColor(255, 255, 255);
    ofDrawBitmapString(ofToString(label), cur);
    
}


//--------------------------------------------------------------
void FaceAugmented::drawNumbers() {
    ofSetColor(255);
    ofFill();
    
    for (int i=0; i<face.landmarks.size();i++) {
        ofDrawBitmapString(ofToString(i), face.landmarks[i].x, face.landmarks[i].y);
    }
}

//--------------------------------------------------------------
vector<ofVec2f> FaceAugmented::returnLandmarks() {
    vector<ofVec2f> lMarks;
    for (int i=0; i<face.landmarks.size();i++) {
        ofVec2f p = face.landmarks[i];
        lMarks.push_back(p);
    }
    return lMarks;
}


//--------------------------------------------------------------
ofPolyline FaceAugmented::returnInnerMouth() {
    ofPolyline myLine = face.innerMouth;
    return myLine;
}


//--------------------------------------------------------------
ofPolyline FaceAugmented::returnOuterMouth() {
    ofPolyline myLine = face.outerMouth;
    return myLine;
}

//--------------------------------------------------------------
ofPolyline FaceAugmented::returnRightEye() {
    ofPolyline myLine = face.rightEye;
    return myLine;
}

//--------------------------------------------------------------
ofPolyline FaceAugmented::returnLeftEye() {
    ofPolyline myLine = face.leftEye;
    return myLine;
}



//--------------------------------------------------------------
void ofApp::setup(){
    tracker.setup(ofFile(__BASE_FILE__).getEnclosingDirectory()+"../../model/shape_predictor_68_face_landmarks.dat");
    tracker.setSmoothingRate(0.5);
    tracker.setDrawStyle(ofxDLib::lines);
    tracker.getTracker().setPersistence(5);
    tracker.getTracker().setMaximumDistance(80);
    
    video.setDeviceID(0);
    video.setup(640, 480);
    
    learned_functions = vector<pfunct_type>(4);
    
    // Load SVM data model
    dlib::deserialize(ofToDataPath("data_ecstatic_smile.func")) >> learned_functions[0];
    
    // Setup value filters for the classifer
    bigSmileValue.setFc(0.04);
    
}

//--------------------------------------------------------------
void ofApp::update(){
    video.update();
    if(video.isFrameNew()){
        tracker.findFaces(video.getPixels());
        vector<FaceAugmented>& facesAugmented = tracker.getFollowers();
        for (auto & face : facesAugmented) {
            face.setImage(video.getPixels());
            
            if (tracker.size() == 1 ) {
                
                inner = face.returnInnerMouth();
                outer = face.returnOuterMouth();
                lEye = face.returnLeftEye();
                rEye = face.returnRightEye();

                bigSmileValue.update(learned_functions[0](makeSample()));
                //bigSmileValueNoFilter = learned_functions[0](makeSample());
                cout << bigSmileValue.value() << endl;
            }
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
        face.drawNumbers();
    }
    tracker.draw();
    
    ofDrawBitmapStringHighlight("nFaces: " + ofToString(tracker.size()), 10, 20);
    
}

//--------------------------------------------------------------
// Function that creates a sample for the classifier containing the mouth and eyes
sample_type ofApp::makeSample(){
    
    
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
        //cout << p.x << endl;
        //cout << p.y << endl;
    }
    
    //cout << relativeMouthPoints[0].x << endl;
    
    
    sample_type s;
    for(int i=0;i<20;i++){
        s(i*2+0) = relativeMouthPoints[i].x;
        s(i*2+1) = relativeMouthPoints[i].y;
    }
    return s;
    
}

