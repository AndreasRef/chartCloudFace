#include "ofApp.h"

using namespace ofxCv;
using namespace cv;

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetWindowTitle("webcam2");
    
    learned_functions = vector<pfunct_type>(2);
    // Load SVM data model
    dlib::deserialize(ofToDataPath("data_ecstatic_smile.func")) >> learned_functions[0];
    dlib::deserialize(ofToDataPath("data_small_smile.func")) >> learned_functions[1];
    
    // Set model path
    //ofSetDataPathRoot(ofFile(__BASE_FILE__).getEnclosingDirectory()+"../../model/");
    
    //Print out a list of devices
    vector<ofVideoDevice> devices = grabber.listDevices();
    
    for(int i = 0; i < devices.size(); i++){
        if(devices[i].bAvailable){
            ofLogNotice() << devices[i].id << ": " << devices[i].deviceName;
        }else{
            ofLogNotice() << devices[i].id << ": " << devices[i].deviceName << " - unavailable ";
        }
    }
    
    //Setup Grabber //Delete afterwards
//    grabber.setDeviceID(1);
//    grabber.setup(1280,720);
    
    // Setup tracker
    tracker.faceDetectorImageSize = 640*360;
    //tracker.faceDetectorImageSize = 1280*720;
    tracker.setup();
    
    //CLAHE
    outputImage.allocate(1280, 720, OF_IMAGE_COLOR);
    
    //Have up to 20 faces at a time
    bigSmileValues.resize(20);
    smallSmileValues.resize(20);
    moods.resize(20);
    
    for (int i = 0; i<smallSmileValues.size();i++) {
        smallSmileValues[i].setFc(0.04);
        bigSmileValues[i].setFc(0.4);
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
    gui.add(debugView.setup("debugView", true));
    
    //OSC
    sender.setup("localhost", 12001);
    
    //Startup text
    startUpText = true;
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
        grabber.setDesiredFrameRate(5);
        grabber.initGrabber(1280, 720);
        gDeviceId.set(ofToString(newDeviceId));
    }
    startUpText = false;
}



//--------------------------------------------------------------
void ofApp::update(){
    grabber.update();
    
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
            ofxOscMessage msg;
            msg.setAddress("/webcam");
            msg.addIntArg(0);
            msg.addFloatArg(0.0);
            msg.addFloatArg(0.0);
            sender.sendMessage(msg, false);
            return;
        }
        
        
        //Calculate mood values
        float minMood = 1.0;
        float maxMood = 0.0;
        
        
        for (int i = 0; i< tracker.size(); i++) {
            
            //FACEPOSE CALCULATIONS (needed to compensate for faces facing sideways or too much up/down)
            
            // initialize variables for pose decomposition
            ofVec3f scale, transition;
            ofQuaternion rotation, orientation;
            
            // get pose matrix
            ofMatrix4x4 p = instances[i].getPoseMatrix();
            
            // decompose the modelview
            ofMatrix4x4(p).decompose(transition, rotation, scale, orientation);
            
            // obtain pitch for facing up/down and yaw for facing sideways
            double pitch =
            atan2(2*(rotation.y()*rotation.z()+rotation.w()*rotation.x()),rotation.w()*rotation.w()-rotation.x()*rotation.x()-rotation.y()*rotation.y()+rotation.z()*rotation.z());
            double yaw =
            asin(-2*(rotation.x()*rotation.z()-rotation.w()*rotation.y()));
            
            //SMILES
            bigSmileValues[i].update(ofClamp(learned_functions[0](makeSampleID(i))*1.2-abs(yaw)+MIN(0,pitch)*1,0, 1));
            smallSmileValues[i].update(ofClamp(learned_functions[1](makeSampleID(i))*1.2-abs(yaw)+MIN(0,pitch)*1,0, 1));
            
            float currentSmile = smallSmileValues[i].value() + bigSmileValues[i].value();
            
            //CALCULATE MOODS + MAX, MIN, VAR & AVG
            moods[i] = currentSmile;
            
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
        
        
        //Rounding to fewer decimals
        varMood =  ofClamp(roundf(varMood * 100) / 100, 0, 1); //Make sure it never goes negative or above 1
        avgMood =  ofClamp(roundf(avgMood * 100) / 100, 0.01, 1); //Make sure it never goes negative or above 1
        
        
        ofxOscMessage msg;
        msg.setAddress("/webcam");
        msg.addIntArg(tracker.size());
        msg.addFloatArg(avgMood);
        msg.addFloatArg(varMood);
        sender.sendMessage(msg, false);
        
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    if (debugView) {
    
    if (claheFilter) {
        outputImage.draw(0,0);
    } else {
        grabber.draw(0, 0);
    }
    tracker.drawDebug();
    }
        
    for (int i = 0; i < tracker.size(); i++) {
        
        ofRectangle bb = tracker.getInstances()[i].getBoundingBox();
        
        //Overall mood
        ofFill();
        ofDrawBitmapStringHighlight("smile", bb.x-15, bb.y -10 );
        ofDrawRectangle(bb.x + 50, bb.y - 25, 100*moods[i], 20);
        
        ofNoFill();
        ofDrawRectangle(bb.x + 50, bb.y - 25, 100, 20);
        ofFill();
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
    
    gui.draw();
    
    if (startUpText) ofDrawBitmapStringHighlight("select a camera in the gui", ofGetWidth()/2, ofGetHeight()/2);
    
#ifndef __OPTIMIZE__
    ofSetColor(ofColor::red);
    ofDrawBitmapString("Warning! Run this app in release mode to get proper performance!",10,60);
    ofSetColor(ofColor::white);
#endif
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
void ofApp::exit(){
    
    ofxOscMessage msg;
    msg.setAddress("/webcam");
    msg.addIntArg(0);
    msg.addFloatArg(0.0);
    msg.addFloatArg(0.0);
    sender.sendMessage(msg, false);
    
}
