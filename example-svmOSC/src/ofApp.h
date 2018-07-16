#pragma once

#include "ofMain.h"
#include "ofxFaceTracker2.h"
#include "ofxBiquadFilter.h" 

#include "ofxOsc.h"

#define HOST "localhost"
#define PORT 8000

typedef dlib::matrix<double,40,1> sample_type;
typedef dlib::radial_basis_kernel<sample_type> kernel_type;

typedef dlib::decision_function<kernel_type> dec_funct_type;
typedef dlib::normalized_function<dec_funct_type> funct_type;

typedef dlib::probabilistic_decision_function<kernel_type> probabilistic_funct_type;
typedef dlib::normalized_function<probabilistic_funct_type> pfunct_type;


class ofApp : public ofBaseApp {
public:
    void setup();
    void update();
    void draw();
    
    sample_type makeSample();
    
    ofxFaceTracker2 tracker;
    ofVideoGrabber grabber;
    
    ofxBiquadFilter1f neutralValue;
    ofxBiquadFilter1f smallSmileValue;
    ofxBiquadFilter1f bigSmileValue;
    ofxBiquadFilter1f oValue;
    ofxBiquadFilter1f filteredEyeBrows;
    
    vector<pfunct_type> learned_functions;
    
    ofxOscSender sender;
    
    
    //Gestures
    enum Gesture {
        MOUTH_WIDTH, MOUTH_HEIGHT,
        LEFT_EYEBROW_HEIGHT, RIGHT_EYEBROW_HEIGHT,
        LEFT_EYE_OPENNESS, RIGHT_EYE_OPENNESS,
        JAW_OPENNESS,
        NOSTRIL_FLARE
    };
    
    float getGesture (Gesture gesture);
    
    float faceDist;
    float faceDistMapped;
};
