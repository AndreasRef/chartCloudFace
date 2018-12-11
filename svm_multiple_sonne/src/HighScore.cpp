#include "HighScore.h"
HighScore::HighScore(){
}

void HighScore::setup(ofImage _img, float _score){
//    x = ofRandom(0, ofGetWidth());      // give some random positioning
//    y = ofRandom(0, ofGetHeight());
    
    highScoreImage = _img;
    score = roundf(_score * 100) / 100;
    
}

void HighScore::update(){
 
}

void HighScore::draw(int _y){
    highScoreImage.draw(ofGetWidth()-200,_y);
    ofDrawBitmapStringHighlight("Score: " + ofToString(score), ofGetWidth()-200, _y+120);
}
