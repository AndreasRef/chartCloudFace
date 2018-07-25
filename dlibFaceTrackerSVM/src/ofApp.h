#pragma once

#include "ofMain.h"
#include "FaceTracker.h"
#include "ofxBiquadFilter.h"


//SMILE SVM
typedef dlib::matrix<double,40,1> sample_type;
typedef dlib::radial_basis_kernel<sample_type> kernel_type;

typedef dlib::decision_function<kernel_type> dec_funct_type;
typedef dlib::normalized_function<dec_funct_type> funct_type;

typedef dlib::probabilistic_decision_function<kernel_type> probabilistic_funct_type;
typedef dlib::normalized_function<probabilistic_funct_type> pfunct_type;


class FaceAugmented : public ofxDLib::Follower<ofxDLib::Face> {
protected:
    ofImage image;
    ofColor color;
    ofRectangle roi;
    ofVec2f cur, smooth;
    ofPolyline all;
    ofxDLib::Face face;
    float startedDying;
public:
    FaceAugmented(){
    };
    void setup(const ofxDLib::Face & track);
    void update(const ofxDLib::Face & track);
    void setImage(const ofPixels & pixels);
    void kill();
    void draw();
    void drawNumbers();
    
    sample_type makeSample();
//    
    ofxBiquadFilter1f smallSmileValue;
    ofxBiquadFilter1f bigSmileValue;
    
};

class ofApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    void draw();
    
    ofVideoGrabber video;
    ofxDLib::FaceTrackerFollower<FaceAugmented> tracker;
    
    vector<pfunct_type> learned_functions;

    
    //Calculate stuff for music demo
    float avgMood;
    float varMood;
    
};