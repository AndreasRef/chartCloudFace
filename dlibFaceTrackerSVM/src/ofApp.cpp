#include "ofApp.h"

using namespace ofxDLib;

const float dyingTime = 1;

void FaceAugmented::setup(const Face & track) {
    color.setHsb(ofRandom(0, 255), 255, 255);
    cur = track.rect.getCenter();
    smooth = cur;
    roi = track.rect;
    face = track;
    
    faceLearned_functions = vector<pfunct_type>(4);
}

void FaceAugmented::update(const Face & track) {
    cur = track.rect.getCenter();
    roi = track.rect;
    smooth.interpolate(cur, .5);
    all.addVertex(smooth);
    face = track;
    
    faceSmileVal = faceLearned_functions[0](faceMakeSample());
//    cout << faceSmileVal << endl;
    
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
    
    //ofPopStyle();
    
    
    //DRAW SVM values
    
    ofDrawBitmapStringHighlight(ofToString(faceSmileVal), cur.x, cur.y);
    
    //        string str = "BIG SMILE";
    //        //float val =  bigSmileValue.value();
    //        float val =  bigSmileValueNoFilter;
    //
    //        ofDrawBitmapStringHighlight(str, 20, 100);
    //        ofDrawRectangle(20, 120, 300*val, 30);
    //
    //        ofNoFill();
    //        ofDrawRectangle(20, 120, 300, 30);
    //        ofFill();
    
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
sample_type FaceAugmented::faceMakeSample(){
    
    sample_type s;
    for(int i=0;i<20;i++){
        //s(i*2+0) = ofRandom(-1,1);
        //s(i*2+1) = ofRandom(-1,1);
        //s(i*2+0) = 0.0;
        //s(i*2+1) = 0.0;
        //s(i*2+0) = relativeMouthPoints[i].x;
        //s(i*2+1) = relativeMouthPoints[i].y;
    }
    //cout << s << endl;
    
    
    s(0) = -0.376373;
    s(1) = 0.0109848;
    
    s(2) = -0.288482;
    s(3) = -0.0857038;
    
    s(4) = -0.156153;
    s(5) = -0.112567;
    
    s(6) = -0.011125;
    s(7) = -0.0886462;
    
    s(8) = 0.11095;
    s(9) = -0.11502;
    
    s(10) = 0.256956;
    s(11) = -0.070591;
    
    s(12) = 0.416148;
    s(13) = 0.0348758;
    
    s(14) = 0.254033;
    s(15) = 0.0837137;
    
    s(16) = 0.11096;
    s(17) = 0.100811;
    
    s(18) = -0.0326015;
    s(19) = 0.107654;
    
    s(20) = -0.166886;
    s(21) = 0.0934989;
    
    s(22) = -0.291405;
    s(23) = 0.068601;
    
    s(24) = -0.325101;
    s(25) = 0.0085409;
    
    s(26) = -0.152243;
    s(27) = -0.0305314;
    
    s(28) = -0.017958;
    s(29) = -0.0163766;
    
    s(30) = 0.11486;
    s(31) = -0.032985;
    
    s(32) = 0.343879;
    s(33) = 0.0280428;
    
    s(34) = 0.105584;
    s(35) = -0.0119875;
    
    s(36) = -0.0272348;
    s(37) = 0.00462095;
    
    s(38) = -0.16152;
    s(39) = -0.00953383;
    
    
    
    return s;
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
            float whatever = face.returnLandmarks()[0].x;
            cout << whatever << endl;
            //bigSmileValue.update(learned_functions[0](makeSample(face.landmarks[0].x)));
        }
        // amount of movement regulates smoothing rate
        for(int i=0; i < tracker.size(); i++) {
            int label = tracker.getLabel(i);
            int length = tracker.getVelocity(i).length();
            length = length < 10 ? length : 10; // arbitrarily choosen
            float smoothingRate = ofMap(length, 0, 10, 0.35, 1);
            tracker.setSmoothingRate(label, smoothingRate);
        }
        
        
        //SVM test
//        for(int i=0; i < tracker.size(); i++) {
//            if (i==0) bigSmileValue.update(learned_functions[0](makeSample()));
//
//            //without the filter
//            if (i==0) bigSmileValueNoFilter = learned_functions[0](makeSample());
//        }
        
        bigSmileValueNoFilter = learned_functions[0](makeSample());
        
        //cout << bigSmileValueNoFilter << endl;
        
        //How do I access the points of the face from here?
        //make a function that returns all face landmark points?
        
        
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
    
//        string str = "BIG SMILE";
//        //float val =  bigSmileValue.value();
//        float val =  bigSmileValueNoFilter;
//
//        ofDrawBitmapStringHighlight(str, 20, 100);
//        ofDrawRectangle(20, 120, 300*val, 30);
//
//        ofNoFill();
//        ofDrawRectangle(20, 120, 300, 30);
//        ofFill();
    
}

//--------------------------------------------------------------
// Function that creates a sample for the classifier containing the mouth and eyes
sample_type ofApp::makeSample(){
    
    sample_type s;
    for(int i=0;i<20;i++){
        //s(i*2+0) = 0.0;
        //s(i*2+1) = 0.0;
        
        s(0) = -0.376373;
        s(1) = 0.0109848;
        
        s(2) = -0.288482;
        s(3) = -0.0857038;
        
        s(4) = -0.156153;
        s(5) = -0.112567;
        
        s(6) = -0.011125;
        s(7) = -0.0886462;
        
        s(8) = 0.11095;
        s(9) = -0.11502;
        
        s(10) = 0.256956;
        s(11) = -0.070591;
        
        s(12) = 0.416148;
        s(13) = 0.0348758;
        
        s(14) = 0.254033;
        s(15) = 0.0837137;
        
        s(16) = 0.11096;
        s(17) = 0.100811;
        
        s(18) = -0.0326015;
        s(19) = 0.107654;
        
        s(20) = -0.166886;
        s(21) = 0.0934989;
        
        s(22) = -0.291405;
        s(23) = 0.068601;
        
        s(24) = -0.325101;
        s(25) = 0.0085409;
        
        s(26) = -0.152243;
        s(27) = -0.0305314;
        
        s(28) = -0.017958;
        s(29) = -0.0163766;
        
        s(30) = 0.11486;
        s(31) = -0.032985;
        
        s(32) = 0.343879;
        s(33) = 0.0280428;
        
        s(34) = 0.105584;
        s(35) = -0.0119875;
        
        s(36) = -0.0272348;
        s(37) = 0.00462095;
        
        s(38) = -0.16152;
        s(39) = -0.00953383;
        
        
        
        
        //s(i*2+0) = relativeMouthPoints[i].x;
        //s(i*2+1) = relativeMouthPoints[i].y;
    }
    return s;
}

