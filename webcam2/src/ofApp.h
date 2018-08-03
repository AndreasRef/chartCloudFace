#pragma once

#include "ofMain.h"
#include "ofxFaceTracker2.h"
#include "ofxBiquadFilter.h"
#include "ofxGui.h"
#include "ofxOsc.h"

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
    void exit();
    
    void trackingResolutionChanged(bool & trackingResolution);
    void eChangeCamera();
    
    ofxFaceTracker2 tracker;
    ofVideoGrabber grabber;
    
    //CLAHE local constrast algorithm
    cv::Mat greyImg, labImg, claheImg, tmpImg;
    ofImage outputImage;
    
    //SmileSVM
    sample_type makeSampleID(int id);
    vector<pfunct_type> learned_functions;
    vector<ofxBiquadFilter1f> smallSmileValues;
    vector<ofxBiquadFilter1f> bigSmileValues;
    
    //ofxGui
    ofxToggle claheFilter;
    ofxIntSlider clipLimit;
    ofxToggle debugView;
    ofxPanel gui;
    
    ofParameter<string>gDeviceId;
    
    ofxButton bCameraSettings;
    
    ofxToggle trackingResolution;
    
    //Moods
    vector<float> moods;
    float avgMood;
    float varMood;
    
    // Osc
    ofxOscSender sender;
    string oscDestination, oscAddress;
    int oscPort;
    
    // Misc
    bool startUpText;
};
