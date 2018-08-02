#pragma once

//#define DEFAULT_OSC_DESTINATION "localhost"
//#define DEFAULT_OSC_ADDRESS "/wek/outputs"
//#define DEFAULT_OSC_PORT 12001
//#define DEFAULT_DEVICE_ID 0


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
    
    
    
//    void setupOSC();
//    void sendOSC();
    
    ofxFaceTracker2 tracker;
    ofVideoGrabber grabber;
    
    //CLAHE local constrast algorithm
    //int clipLimit = 1;
    cv::Mat greyImg, labImg, claheImg, tmpImg;
    ofImage outputImage;
    
    
    //Static image + video
    ofImage img;
    ofVideoPlayer video;
    
    
    //SmileSVM
    sample_type makeSampleID(int id);
    vector<pfunct_type> learned_functions;
    vector<ofxBiquadFilter1f> smallSmileValues;
    vector<ofxBiquadFilter1f> bigSmileValues;
    vector<ofxBiquadFilter1f> eyeBrows;
    
    
    //Gestures for eyes + eyeBrows
    enum Gesture {
        LEFT_EYEBROW_HEIGHT, RIGHT_EYEBROW_HEIGHT,
        LEFT_EYE_OPENNESS, RIGHT_EYE_OPENNESS
    };
    float getGesture (Gesture gesture, int id);
    
    
    //ofxGui
    ofxToggle claheFilter;
    ofxIntSlider clipLimit;
    ofxPanel gui;
    
    ofParameter<string>gDeviceId;
    
    ofxButton bCameraSettings;
    
    ofxToggle trackingResolution;
    
    
    //Moods
    vector<float> moods;
    
    //Calculate stuff for music demo
    float avgMood;
    float varMood;
    
    
    // osc
    ofxOscSender sender;
    string oscDestination, oscAddress;
    int oscPort;
    
    
    // Misc
    bool startUpText;
};
