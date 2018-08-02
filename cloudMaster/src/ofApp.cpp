#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetWindowTitle("master");
    
    
    receiverWebcam1.setup(WEBCAM1PORT);
    receiverWebcam2.setup(WEBCAM2PORT);
    
    senderToAbleton.setup("localhost", ABLETONPORT);
    senderToMadmapper.setup("localhost", MADMAPPERPORT);
    
    //Summed variables
    nFacesTotal = 0;
    avgMoodTotal = 0;
    varMoodTotal = 0;
    longTermAvgMoodTotal = 0.5;
    
    //Webcam1 variables
    nFacesWebcam1 = 0;
    avgMoodWebcam1 = 0;
    varMoodWebcam1 = 0;
    
    //Webcam2 variables
    nFacesWebcam2 = 0;
    avgMoodWebcam2 = 0;
    varMoodWebcam2 = 0;
    
    //Start Ableton transport on setup
    ofxOscMessage abletonMessage;
    abletonMessage.setAddress("/live/play");
    senderToAbleton.sendMessage(abletonMessage, false);
}

//--------------------------------------------------------------
void ofApp::update(){
    
    recieveOsc();
    
    //Perform calculations
    nFacesTotal = nFacesWebcam1 + nFacesWebcam2;
    avgMoodTotal = (avgMoodWebcam1+avgMoodWebcam2)/2; //not valid if there are no faces in one of the cams - perhaps total mood summed? Otherwise do logic statements where you check for nFaces == 0
    varMoodTotal = (varMoodWebcam1+varMoodWebcam2)/2;
    
    //To do: longTermAvgMoodTotal
    
    //Quick longTermAvgMoodTotal: Lerping
    
    
    
    
    if (ofGetFrameNum() % 30 == 0) { //Stuff we don't need to do all the time
        longTermAvgMoodTotal = ofLerp(longTermAvgMoodTotal, avgMoodTotal, 0.01);
        sendOsc();
    }
    
    
    //Timer for messages not recieved
    if (webcam1Timer > 200) {
        nFacesWebcam1 = 0;
        avgMoodWebcam1 = 0;
        varMoodWebcam1 = 0;
    }
    
    if (webcam2Timer > 200) {
        nFacesWebcam2 = 0;
        avgMoodWebcam2 = 0;
        varMoodWebcam2 = 0;
    }
    
    webcam1Timer++;
    webcam2Timer++;
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(0);
    
    ofSetColor(255);
    ofDrawLine(0, ofGetHeight()/2, ofGetWidth(), ofGetHeight()/2);
    
    //Draw info
    
    
    //SUMMED INPUTS
    int x = 10;
    int y = 30;
    
    int yInc = 30;
    
    ofDrawBitmapString("SUMMED INPUTS", x, y);
    y+=yInc;
    
    ofDrawBitmapString("total faces: " + ofToString(nFacesTotal), x, y);
    y+=yInc;

    ofDrawBitmapString("average mood: " + ofToString(avgMoodTotal), x, y);
    y+=yInc;
    
    ofDrawBitmapString("mood variation: " + ofToString(varMoodTotal), x, y);
    y+=yInc;
    
    ofDrawBitmapString("long term avg mood: " + ofToString(longTermAvgMoodTotal), x, y);
    
    
    //WEBCAM1 INPUT
    x = ofGetWidth()/3 + 50;
    y = 30;
    
    ofDrawBitmapString("WEBCAM1 INPUT", x, y);
    y+=yInc;
    
    ofDrawBitmapString("faces: " + ofToString(nFacesWebcam1), x, y);
    y+=yInc;
    
    ofDrawBitmapString("mood: " + ofToString(avgMoodWebcam1), x, y);
    y+=yInc;
    
    ofDrawBitmapString("mood var: " + ofToString(varMoodWebcam1), x, y);
    y+=yInc;
    
    ofDrawBitmapString("port: " + ofToString(WEBCAM1PORT), x, y);
    y+=yInc;
    
    ofDrawBitmapString("adress: " + ofToString(WEBCAM1MESSAGE), x, y);
    
    
    //WEBCAM2 INPUT
    x = 2*ofGetWidth()/3 + 50;
    y = 30;
    
    ofDrawBitmapString("WEBCAM2 INPUT", x, y);
    y+=yInc;
    
    ofDrawBitmapString("faces: " + ofToString(nFacesWebcam2), x, y);
    y+=yInc;
    
    ofDrawBitmapString("mood: " + ofToString(avgMoodWebcam2), x, y);
    y+=yInc;
    
    ofDrawBitmapString("mood var: " + ofToString(varMoodWebcam2), x, y);
    y+=yInc;
    
    ofDrawBitmapString("port: " + ofToString(WEBCAM2PORT), x, y);
    y+=yInc;
    
    ofDrawBitmapString("adress: " + ofToString(WEBCAM2MESSAGE), x, y);
    

    //ABLETON OUTPUT
    x = 10;
    y = 40 + ofGetHeight()/2;
    
    ofDrawBitmapString("ABLETON OUTPUT", x, y);
    y+=yInc;
    
    ofDrawBitmapString("Port: " + ofToString(ABLETONPORT), x, y);
    y+=yInc;
    
    ofDrawBitmapString("adress: /live/device (0) (0) (1-4)", x, y);
    y+=yInc;
    
    ofDrawBitmapString("values: " + ofToString(nFacesTotal) + " " + ofToString(avgMoodTotal) + " " + ofToString(varMoodTotal)  + " " + ofToString(longTermAvgMoodTotal), x, y);

    
    //MADMAPPER OUTPUT
    x = ofGetWidth()/2 + 30;
    y = 40 + ofGetHeight()/2;
    
    ofDrawBitmapString("MADMAPPER OUTPUT", x, y);
    y+=yInc;
    
    ofDrawBitmapString("Port: " + ofToString(MADMAPPERPORT), x, y);
    y+=yInc;
    
    ofDrawBitmapString("adress: /whatever/whatever", x, y);
    y+=yInc;
    
    ofDrawBitmapString("values: " + ofToString(nFacesTotal) + " " + ofToString(avgMoodTotal) + " " + ofToString(varMoodTotal)  + " " + ofToString(longTermAvgMoodTotal), x, y);
    
    
    
    
    
}


//--------------------------------------------------------------
void ofApp::recieveOsc(){
    
    //Recieve from webcam1
    // check for waiting messages
    while(receiverWebcam1.hasWaitingMessages()){
        
        // get the next message
        ofxOscMessage m;
        receiverWebcam1.getNextMessage(m);
        
        //cout << m << endl;
        
        
        // check adress
        if(m.getAddress() == WEBCAM1MESSAGE){
            nFacesWebcam1 = m.getArgAsInt(0);
            avgMoodWebcam1 = m.getArgAsFloat(1);
            varMoodWebcam1 = m.getArgAsFloat(2);
        }
        
        webcam1Timer=0;
        
        
    }
    
    //Recieve from webcam2
    // check for waiting messages
    while(receiverWebcam2.hasWaitingMessages()){
        
        // get the next message
        ofxOscMessage m;
        receiverWebcam2.getNextMessage(m);
        
        // check adress
        if(m.getAddress() == WEBCAM2MESSAGE){
            nFacesWebcam2 = m.getArgAsInt(0);
            avgMoodWebcam2 = m.getArgAsFloat(1);
            varMoodWebcam2 = m.getArgAsFloat(2);
            
        }
        
        webcam2Timer=0;
    }
}

//--------------------------------------------------------------
void ofApp::sendOsc(){
    
    //Send to Ableton
    
//    for (int i =0; i < 8; i++) { //Send three messages to control the three first dials on the first device on first track
//    ofxOscMessage abletonMessage;
//    abletonMessage.setAddress("/live/device");
//    abletonMessage.addIntArg(0); //track
//    abletonMessage.addIntArg(0); //device
//    abletonMessage.addIntArg(i+1); //parameter - 0 is on/off so start at 1
//    abletonMessage.addIntArg(sin(ofGetFrameNum()/100.0)*127/2 + 127/2); //value - from 0-127
//    senderToAbleton.sendMessage(abletonMessage, false);
//
//    }
    
    //SEV SPECIFIC ABLETON OUTPUTS
    
    for (int i =0; i < 3; i++) { //Send three messages to control the three first dials on the first device on first track
        ofxOscMessage abletonMessage;
        abletonMessage.setAddress("/live/device");
        abletonMessage.addIntArg(0); //track
        abletonMessage.addIntArg(0); //device
        
        if (i == 0) {// nFacesTotal maps to X: Probability
            abletonMessage.addIntArg(4); //parameter
            abletonMessage.addIntArg(nFacesTotal*20); //value - from 0-127
        } else if (i == 1) {// avgMoodTotal maps to X: CENTER
            abletonMessage.addIntArg(2); //parameter
            abletonMessage.addIntArg(int(avgMoodTotal*127)); //value - from 0-127
        }
        senderToAbleton.sendMessage(abletonMessage, false);
    }
    
    
    
    
    //Send to MadMapper
    ofxOscMessage madMapperMessage;
    madMapperMessage.setAddress("/whatever");
    madMapperMessage.addFloatArg(1.0); //not sure?
    madMapperMessage.addFloatArg(1.0); //not sure?
    madMapperMessage.addFloatArg(1.0); //not sure?
    senderToMadmapper.sendMessage(madMapperMessage, false);
}
