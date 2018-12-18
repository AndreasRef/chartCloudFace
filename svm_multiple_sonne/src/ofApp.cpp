#include "ofApp.h"
#include "HighScore.h"

using namespace ofxCv;
using namespace cv;

//--------------------------------------------------------------
void ofApp::setup(){
    learned_functions = vector<pfunct_type>(2);
    // Load SVM data model
    dlib::deserialize(ofToDataPath("data_ecstatic_smile.func")) >> learned_functions[0];
    dlib::deserialize(ofToDataPath("data_small_smile.func")) >> learned_functions[1];
    
    //Static image + video
   //img.load("images/exp6.jpg");
   //img.resize(ofGetWidth(),ofGetHeight());
//
    video.load("videos/femaleFacialExpressions.mp4");
    video.setLoopState(OF_LOOP_NORMAL);
    video.setVolume(0);
    //video.play();
    
    // Set model path
    ofSetDataPathRoot(ofFile(__BASE_FILE__).getEnclosingDirectory()+"../../model/");
    
    //Print out a list of devices
    vector<ofVideoDevice> devices = grabber.listDevices();
    
    for(int i = 0; i < devices.size(); i++){
        if(devices[i].bAvailable){
            ofLogNotice() << devices[i].id << ": " << devices[i].deviceName;
        }else{
            ofLogNotice() << devices[i].id << ": " << devices[i].deviceName << " - unavailable ";
        }
    }
    
    // Setup grabber
    grabber.setDeviceID(0);
    grabber.setDesiredFrameRate(25);
    grabber.setup(1280,720);
    
    // Setup tracker
    //tracker.faceDetectorImageSize = 640*360;
    tracker.faceDetectorImageSize = 1280*720;
    tracker.setup();
    
    //CLAHE
    outputImage.allocate(1280, 720, OF_IMAGE_COLOR);
    
    //Have up to 100 faces at a time
    bigSmileValues.resize(100);
    smallSmileValues.resize(100);
    eyeBrows.resize(100);
    moods.resize(100);
    
    for (int i = 0; i<smallSmileValues.size();i++) {
        smallSmileValues[i].setFc(0.1);
        bigSmileValues[i].setFc(0.1);
        eyeBrows[i].setFc(0.1);
    }
    
    //GUI
    trackingResolution.addListener(this, &ofApp::trackingResolutionChanged);
    bCameraSettings.addListener(this, &ofApp::eChangeCamera);
    
    gui.setup();
    gui.add(claheFilter.setup("CLAHE contrast enhancer", false));
    gui.add(clipLimit.setup("CLAHE clipLimit", 10, 0, 30));
    gui.add(trackingResolution.setup("hi-res tracking", false));
    gui.add(gDeviceId.set("deviceId", ofToString(0)));
    gui.add(bCameraSettings.setup("change Camera settings"));
    
    //OSC
    sender.setup("localhost", 12000);
    

    faceImgs.resize(100);
    
    storedFaces.resize(6);
    ofColor c;
    c.set(255,0,0);
    for (int i = 0; i<storedFaces.size();i++ ) {
        storedFaces[i].resize(100, 100);
        storedFaces[i].setColor(c);
    }
    
    //OOP
    //myHighScore.setup();
    
}


//--------------------------------------------------------------
void ofApp::trackingResolutionChanged(bool &hiRes){
    if (hiRes == false) {
       tracker.faceDetectorImageSize = 640*360;
    } else if (hiRes == true) {
        tracker.faceDetectorImageSize = 960*540;
    }
}

//--------------------------------------------------------------
void ofApp::eChangeCamera() {
    string msg = "Select camera:";
    int idx = 0;
    for (auto d : grabber.listDevices()) {
        msg += "\n "+ofToString(idx++)+": "+d.deviceName;
    }
    string selection = ofSystemTextBoxDialog(msg);
    if (selection != "") {
        int newDeviceId = ofToInt(selection);
        grabber.setDeviceID(newDeviceId);
        grabber.initGrabber(1280, 720);
        gDeviceId.set(ofToString(newDeviceId));
    }
}



//--------------------------------------------------------------
void ofApp::update(){
    grabber.update();
    video.update();
    
    //myHighScore.update();
    
    
    //tracker.update(video);
    //tracker.update(img);
    
    if(grabber.isFrameNew()){
        
        if (claheFilter) { //GUI claheFilter
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
        tracker.update(outputImage);
        } else {
        tracker.update(grabber);
        }
        
        varMood = 0;
        avgMood = 0;
        
        vector<ofxFaceTracker2Instance> instances = tracker.getInstances();
        if (instances.size() == 0) {
            return;
        }
        
        
        //Calculate stuff for music demo: nPersons - average mood - variation between moods
        float minMood = 1.0;
        float maxMood = 0.0;
        
        
        for (int i = 0; i< tracker.size(); i++) {
            
            
            //FACEPOSE CALCULATIONS (needed to compensate for faces facing sideways or too much up/down
            // initialize variables for pose decomposition
            ofVec3f scale, transition;
            ofQuaternion rotation, orientation;
            
            // get pose matrix
            ofMatrix4x4 p = instances[i].getPoseMatrix();
            
            // decompose the modelview
            ofMatrix4x4(p).decompose(transition, rotation, scale, orientation);
            
            
            // obtain pitch, roll, yaw: //pitch is a good indicator of facing up/down // //yaw is a good indicator of facing sideways
            double pitch =
            atan2(2*(rotation.y()*rotation.z()+rotation.w()*rotation.x()),rotation.w()*rotation.w()-rotation.x()*rotation.x()-rotation.y()*rotation.y()+rotation.z()*rotation.z());
            double roll =
            atan2(2*(rotation.x()*rotation.y()+rotation.w()*rotation.z()),rotation.w()*rotation.w()+rotation.x()*rotation.x()-rotation.y()*rotation.y()-rotation.z()*rotation.z());
            double yaw =
            asin(-2*(rotation.x()*rotation.z()-rotation.w()*rotation.y()));
            
            
            //SMILES
            bigSmileValues[i].update(ofClamp(learned_functions[0](makeSampleID(i))*1.2-abs(yaw)+MIN(0,pitch)*1,0, 1));
            smallSmileValues[i].update(ofClamp(learned_functions[1](makeSampleID(i))*1.2-abs(yaw)+MIN(0,pitch)*1,0, 1));
            
            
            //EYEBROWS + EYES
            float eyeBrowInput = ((getGesture(RIGHT_EYEBROW_HEIGHT, i) + getGesture(LEFT_EYEBROW_HEIGHT, i) + getGesture(RIGHT_EYE_OPENNESS, i) + getGesture(LEFT_EYE_OPENNESS, i)) -0.8 - pitch*1.5);
            eyeBrowInput = ofClamp(eyeBrowInput, 0.0, 1.0);
            eyeBrows[i].update(eyeBrowInput);
            
            float currentSmile = (smallSmileValues[i].value() + bigSmileValues[i].value())/2;
            float currentAngry =  ofClamp(0.5-eyeBrows[i].value()-currentSmile,0,0.5);
            
            
            //CALCULATE MOODS + MAX, MIN, VAR & AVG
            moods[i] = 0.501 + currentSmile - currentAngry;
            
            avgMood+= moods[i];
            
            if (moods[i] >= maxMood) {
                maxMood = moods[i];
            }
            
            if (moods[i] <= minMood ) {
                minMood = moods[i];
            }
            
        }
        
        varMood = maxMood - minMood;
        avgMood = avgMood/tracker.size();
        
        //Quick rounding
        varMood =  ofClamp(roundf(varMood * 100) / 100, 0, 1);
        avgMood =  ofClamp(roundf(avgMood * 100) / 100, 0, 1);
        
        
        
        //OSC
        ofxOscMessage msg;
        msg.setAddress("/wek/outputs");
        msg.addFloatArg(tracker.size()/5.0);
        msg.addFloatArg(avgMood);
        msg.addFloatArg(varMood);
        sender.sendMessage(msg, false);
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    
    if (claheFilter) {
        outputImage.draw(0,0);
    } else {
        grabber.draw(0, 0);
    }
    
    //img.draw(0,0);
    //video.draw(0,0);
    tracker.drawDebug();
    
#ifndef __OPTIMIZE__
    ofSetColor(ofColor::red);
    ofDrawBitmapString("Warning! Run this app in release mode to get proper performance!",10,60);
    ofSetColor(ofColor::white);
#endif
    
    for (int i = 0; i < tracker.size(); i++) {
        
        faceImgs[i].clear();
        
        ofRectangle bb = tracker.getInstances()[i].getBoundingBox();
        
        //Overall mood
        ofFill();
        ofDrawBitmapStringHighlight("mood", bb.x-15, bb.y -10 );
        ofDrawRectangle(bb.x + 50, bb.y - 25, 100*moods[i], 20);
        
        ofNoFill();
        ofDrawRectangle(bb.x + 50, bb.y - 25, 100, 20);
        ofFill();
        
//        myImg.setFromPixels(grabber.getPixels());
//        myImg.crop(bb.x, bb.y, bb.width, bb.height);
//        myImg.resize(100, 100);
        
        faceImgs[i].setFromPixels(grabber.getPixels());
        faceImgs[i].crop(bb.x, bb.y, bb.width, bb.height);
        faceImgs[i].resize(100, 100);
        
        faceImgs[i].draw(i*120 +100, 600);
        
    }

    // Draw framerate
    ofDrawBitmapStringHighlight("Framerate : "+ofToString(ofGetFrameRate()), 10, 170);
    ofDrawBitmapStringHighlight("Tracker thread framerate : "+ofToString(tracker.getThreadFps()), 10, 190);
    
    // Draw tracker resolution
    if (trackingResolution == false) {
        ofDrawBitmapStringHighlight("Tracker resolution: 640x360", 10, 210);
    } else if (trackingResolution == true) {
        ofDrawBitmapStringHighlight("Tracker resolution: 960x540", 10, 210);
    }
    
    // Draw tracking info
    ofDrawBitmapStringHighlight("nPersons: " + ofToString(tracker.size()), 10, 250);
    ofDrawBitmapStringHighlight("avgMood: " + ofToString(avgMood), 10, 270);
    ofDrawBitmapStringHighlight("varMood: " + ofToString(varMood), 10, 290);
    
    for (int i = 0; i<storedFaces.size();i++ ) {
        storedFaces[i].draw(ofGetWidth()-150, i*120);
    }
    
    //gui.draw();
    //myHighScore.draw();
    
    for (int i = 0 ; i<highScores.size(); i++) {
        highScores[i].draw(i*140);
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


//--------------------------------------------------------------
float ofApp:: getGesture (Gesture gesture, int id){
    
    if(tracker.size()<1) {
        return 0;
    }
    int start = 0, end = 0;
    float compareFloat = 1.0;
    
    switch(gesture) {
        case LEFT_EYEBROW_HEIGHT: start = 38; end = 20; // center of the eye to middle of eyebrow
            
            compareFloat = tracker.getInstances()[id].getLandmarks().getImagePoint(33).y - tracker.getInstances()[id].getLandmarks().getImagePoint(27).y;
            break;
            
            
        case RIGHT_EYEBROW_HEIGHT: start = 43; end = 23; // center of the eye to middle of eyebrow
            
            compareFloat = tracker.getInstances()[id].getLandmarks().getImagePoint(33).y - tracker.getInstances()[id].getLandmarks().getImagePoint(27).y;
            
            break;
            
        case LEFT_EYE_OPENNESS: start = 38; end = 40; // upper inner eye to lower outer eye
            compareFloat = ofDist(
                                  tracker.getInstances()[id].getLandmarks().getImagePoint(36).x,
                                  tracker.getInstances()[id].getLandmarks().getImagePoint(36).y,
                                  tracker.getInstances()[id].getLandmarks().getImagePoint(39).x,
                                  tracker.getInstances()[id].getLandmarks().getImagePoint(39).y
                                  );
            break;
            
        case RIGHT_EYE_OPENNESS: start = 43; end = 47;// upper inner eye to lower outer eye
            
            compareFloat = ofDist(
                                  tracker.getInstances()[id].getLandmarks().getImagePoint(42).x,
                                  tracker.getInstances()[id].getLandmarks().getImagePoint(42).y,
                                  tracker.getInstances()[id].getLandmarks().getImagePoint(45).x,
                                  tracker.getInstances()[id].getLandmarks().getImagePoint(45).y
                                  );
            break;
    }
    
    float gestureFloat = abs(tracker.getInstances()[id].getLandmarks().getImagePoint(start).y - tracker.getInstances()[id].getLandmarks().getImagePoint(end).y);
    return (gestureFloat/compareFloat);
}


//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
    for (int i = 0; i < tracker.size(); i++) {
        HighScore tempHighScore;
        tempHighScore.setup(faceImgs[i], moods[i]);
        highScores.push_back(tempHighScore);
    }
}



