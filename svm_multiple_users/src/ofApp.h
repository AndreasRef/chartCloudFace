#pragma once

#include "ofMain.h"
#include "ofxFaceTracker2.h"
#include "ofxBiquadFilter.h"

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
    sample_type makeSample2();
    
    ofxFaceTracker2 tracker;
    ofVideoGrabber grabber;
    
    ofxBiquadFilter1f smallSmileValue;
    ofxBiquadFilter1f smallSmileValue2;
    
    vector<pfunct_type> learned_functions;
    vector<pfunct_type> learned_functions2;
    
    
    //As vectors
    vector<sample_type> makeSamples;
    vector<ofxBiquadFilter1f> smallSmileValues;
    vector<vector<pfunct_type>> learned_functions_vector;
    
};
