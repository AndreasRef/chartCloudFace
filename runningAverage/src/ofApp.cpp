#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    storedValues.resize(100);
}

//--------------------------------------------------------------
void ofApp::update(){
    
    timeEllapsed = round(ofGetElapsedTimef());
    
    //cout << ofToString(timeEllapsed) << endl;
    
    if (timeEllapsed > pTimeEllapsed) {
    
    float newValueFromStream = ofMap(ofGetMouseX(),0,ofGetWidth(),0,1);
    
    AddNewValue(newValueFromStream);
    
    
    
    if(count > 0)
    {
        average = sum / count;
    }
        cout << "tick" << endl;
        
            ofLog(OF_LOG_NOTICE, "count: " + ofToString(count) + " new value: " + ofToString(newValueFromStream) + " proper average: " + ofToString(average));
        
    }
    
    pTimeEllapsed = timeEllapsed;
    

}

//--------------------------------------------------------------
void ofApp::draw(){

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
void ofApp::keyPressed(int key){
    storedValues.resize(1000);
    count = 0;
    sum = 0;
//    average = 0;
    
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    storedValues.resize(50);
    count = 0;
    sum = 0;
//    average = 0;
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
