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
    
    
    //Gui
    longTermMinutes.addListener(this, &ofApp::longTermMinutesChanged);
    
    gui.setup();
    gui.add(nFaceFactor.setup("nFaceFactor", 10, 1, 32));
    gui.add(longTermAvgMoodThreshold.setup("moodThreshold", 0.3, 0.0, 1.0));
    gui.add(longTermMinutes.setup("longTermMinutes", 1, 1, 6*60));
    
    //longTermAvgMoodTotal vector
    storedValues.resize(longTermMinutes*60);
}

//--------------------------------------------------------------
void ofApp::update(){
    
    recieveOsc();
    
    //Perform calculations
    nFacesTotal = nFacesWebcam1 + nFacesWebcam2;
    
    if (nFacesWebcam1==0) { //If one webcam does not see anybody, set the total values to follow the other camera
        avgMoodTotal = avgMoodWebcam2;
        varMoodTotal = varMoodWebcam2;
    } else if (nFacesWebcam2==0) { //If one webcam does not see anybody, set the total values to follow the other camera
        avgMoodTotal = avgMoodWebcam1;
        varMoodTotal = varMoodWebcam1;
    } else { //If both cameras detect faces, perform calculations
        avgMoodTotal = nFacesWebcam1*avgMoodWebcam1/nFacesTotal + nFacesWebcam2*avgMoodWebcam2/nFacesTotal; //weighted avg
        
        //avgMoodTotal = (avgMoodWebcam1+avgMoodWebcam2)/2;
        
        varMoodTotal = max(max(varMoodWebcam1,varMoodWebcam2),abs(avgMoodWebcam1-avgMoodWebcam2)); //lazy varMoodTotal: the proper thing to do would be to compare max+min from both cameras, but this sounds nicer :-)
    }
    
    
    
    
    //longTermAvgMoodTotal calculations
    timeElapsed = round(ofGetElapsedTimef());
    if (timeElapsed > pTimeElapsed) {
        
        float previousLongTermMood = longTermAvgMoodTotal; //Variable that contains the previous value of longTermAvgMoodTotal
        
        AddNewValue(avgMoodTotal);
        
        if(count > 0)
        {
            longTermAvgMoodTotal = sum / count;
        }
        
        
        if (longTermAvgMoodTotal > longTermAvgMoodThreshold && previousLongTermMood < longTermAvgMoodThreshold) { //If longTermAvgMood goes above longTermAvgMoodThreshold, trigg clips in Ableton
            cout << "bang happy" << endl;
            
            ofxOscMessage abletonMessage;
            abletonMessage.setAddress("/live/play/scene");
            abletonMessage.addIntArg(1); //launch scene number
            senderToAbleton.sendMessage(abletonMessage, false);
            
            //Send a signal to MadMapper
            
            
        } else if (longTermAvgMoodTotal < longTermAvgMoodThreshold && previousLongTermMood > longTermAvgMoodThreshold) { //If longTermAvgMood goes belov 0.5, trigg clips in Ableton
            cout << "bang sad" << endl;
            
            ofxOscMessage abletonMessage;
            abletonMessage.setAddress("/live/play/scene");
            abletonMessage.addIntArg(0); //launch scene number
            senderToAbleton.sendMessage(abletonMessage, false);
            
            //Send a signal to MadMapper
            
        }
        
        
        
    }
    
    pTimeElapsed = timeElapsed;
    
    
    
    
    if (ofGetFrameNum() % 30 == 0) { //Stuff we don't need to do all the time
        
        sendOsc();
    }
    
    
    //Timer for messages not recieved - will set values to zero if there is no messages for 200 frames from each webcam sketch
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
    
    ofDrawLine(0, 200, ofGetWidth(), 200);
    
    ofDrawLine(0, ofGetHeight()/2 + 100, ofGetWidth(), ofGetHeight()/2+100);
    
    // DRAW INFO
    
    //GUI
    ofDrawBitmapString("GUI", ofGetWidth()/3 + 50, 30);
    
    gui.draw();
    
    
    //SUMMED INPUTS
    int x = 10;
    int y = 230;
    
    int yInc = 30;
    
    ofDrawBitmapString("SUMMED INPUTS", x, y);
    y+=yInc;
    
    ofDrawBitmapString("total faces: " + ofToString(nFacesTotal), x, y);
    y+=yInc;

    ofDrawBitmapString("average mood: " + ofToString(avgMoodTotal), x, y);
    y+=yInc;
    
    ofDrawBitmapString("mood variation: " + ofToString(varMoodTotal), x, y);
    y+=yInc;
    
    ofDrawBitmapString("long term avg mood: " + ofToString(longTermAvgMoodTotal, 2), x, y);
    
    
    //WEBCAM1 INPUT
    x = ofGetWidth()/3 + 50;
    y = 230;
    
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
    
    ofDrawBitmapString("adress: " + ofToString(WEBCAMMESSAGE), x, y);
    
    
    //WEBCAM2 INPUT
    x = 2*ofGetWidth()/3 + 50;
    y = 230;
    
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
    
    ofDrawBitmapString("adress: " + ofToString(WEBCAMMESSAGE), x, y);
    

    //ABLETON OUTPUT
    x = 10;
    y = 140 + ofGetHeight()/2;
    
    ofDrawBitmapString("ABLETON OUTPUT", x, y);
    y+=yInc;
    
    ofDrawBitmapString("Port: " + ofToString(ABLETONPORT), x, y);
    y+=yInc;
    
    ofDrawBitmapString("adress: /live/device (0) (0) (1-3)", x, y);
    y+=yInc;
    
    ofDrawBitmapString("values: " + ofToString(min(nFacesTotal*nFaceFactor,127)) + " " + ofToString(avgMoodTotal*127) + " " + ofToString(varMoodTotal*127), x, y);
    y+=yInc;
    
    if (longTermAvgMoodTotal > longTermAvgMoodThreshold) {
        ofDrawBitmapString("/live/play/scene (1)", x, y);
    } else if (longTermAvgMoodTotal < longTermAvgMoodThreshold) {
        ofDrawBitmapString("/live/play/scene (0)", x, y);
    }
    
    
    //MADMAPPER OUTPUT
    x = ofGetWidth()/2 + 30;
    y = 140 + ofGetHeight()/2;
    
    ofDrawBitmapString("MADMAPPER OUTPUT", x, y);
    y+=yInc;
    
    ofDrawBitmapString("Port: " + ofToString(MADMAPPERPORT), x, y);
    y+=yInc;
    
    ofDrawBitmapString("adress: "+ ofToString(MADMAPPERMESSAGE), x, y);
    y+=yInc;
    
    ofDrawBitmapString("values: " + ofToString(nFacesTotal*nFaceFactor) + " " + ofToString(avgMoodTotal) + " " + ofToString(varMoodTotal)  + " " + ofToString(longTermAvgMoodTotal, 2), x, y);
    
    
}

//--------------------------------------------------------------
void ofApp::longTermMinutesChanged(int &longTermMinutes){
    storedValues.resize(longTermMinutes*60);
    
    count = 0;
    sum = 0;
}


//--------------------------------------------------------------
void ofApp::AddNewValue(float val){
    if(count < storedValues.size())
    {
        //array is not full yet
        storedValues[count++] = val;
        
        sum += val;
    }
    else
    {
        sum += val;
        sum -= storedValues[0];
        
        //shift all of the values, drop the first one (oldest)
        for(int i = 0; i < storedValues.size()-1; i++)
        {
            storedValues[i] = storedValues[i+1] ;
        }
        //the add the new one
        storedValues[storedValues.size()-1] = val;
    }
}



//--------------------------------------------------------------
void ofApp::recieveOsc(){
    
    //Recieve from webcam1
    // check for waiting messages
    while(receiverWebcam1.hasWaitingMessages()){
        
        // get the next message
        ofxOscMessage m;
        receiverWebcam1.getNextMessage(m);
        
        
        // check adress
        if(m.getAddress() == WEBCAMMESSAGE){
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
        if(m.getAddress() == WEBCAMMESSAGE){
            nFacesWebcam2 = m.getArgAsInt(0);
            avgMoodWebcam2 = m.getArgAsFloat(1);
            varMoodWebcam2 = m.getArgAsFloat(2);
        }
        
        webcam2Timer=0;
    }
}

//--------------------------------------------------------------
void ofApp::sendOsc(){
    
    //Send OSC to Ableton
    //Generic Ableton Output (control first three values in an instrument rack on first track)
    
    for (int i =0; i < 3; i++) { //Send three messages to control the three first dials on the first device on first track
        ofxOscMessage abletonMessage;
        abletonMessage.setAddress("/live/device");
        abletonMessage.addIntArg(0); //track
        abletonMessage.addIntArg(0); //device
        abletonMessage.addIntArg(i+1); //parameter - 0 is on/off, so start from 1
        if (i == 0) {// nFacesTotal maps to X: Probability
            abletonMessage.addIntArg(min(nFacesTotal*nFaceFactor,127));
        } else if (i == 1) {// avgMoodTotal maps to X: CENTER
            abletonMessage.addIntArg(int(avgMoodTotal*127)); //value - from 0-127
        } else if (i == 2) {// varMoodTotal maps to Y: RANGE
            abletonMessage.addIntArg(int(varMoodTotal*127)); //value - from 0-127
        }
        senderToAbleton.sendMessage(abletonMessage, false);
    }
    
    
    
    
    //Send OSC to MadMapper
    ofxOscMessage madMapperMessage;
    madMapperMessage.setAddress(MADMAPPERMESSAGE);
    madMapperMessage.addFloatArg(nFacesTotal);
    madMapperMessage.addFloatArg(avgMoodTotal);
    madMapperMessage.addFloatArg(varMoodTotal);
    madMapperMessage.addFloatArg(longTermAvgMoodTotal);
    senderToMadmapper.sendMessage(madMapperMessage, false);
}
