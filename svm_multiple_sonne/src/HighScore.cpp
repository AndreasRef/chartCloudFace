#include "HighScore.h"
HighScore::HighScore(){
}

void HighScore::setup(ofImage _img, float _score){
//    x = ofRandom(0, ofGetWidth());      // give some random positioning
//    y = ofRandom(0, ofGetHeight());
    
    highScoreImage = _img;
    score = _score;

    
}

void HighScore::update(){
 
}

void HighScore::draw(){
    highScoreImage.draw(ofGetWidth()-200,100);
    ofDrawBitmapStringHighlight("Score: " + ofToString(score), ofGetWidth()-200,120);
}
