#ifndef _HIGHSCORE // if this class hasn't been defined, the program can define it
#define _HIGHSCORE // by using this if statement you prevent the class to be called more than once which would confuse the compiler
#include "ofMain.h" // we need to include this to have a reference to the openFrameworks framework
class HighScore {
    
public: // place public functions or variables declarations here
    
    // methods, equivalent to specific functions of your class objects
    void setup(ofImage _img, float _score);    // setup method, use this to setup your object's initial state
    void update();  // update method, used to refresh your objects properties
    void draw();    // draw method, this where you'll do the object's drawing
    
    // variables
//    float x;        // position
//    float y;
    
    float score = -1;
    
    int rank;
    
    ofImage highScoreImage;
    
    HighScore();
        
    // constructor - used to initialize an object, if no properties are passed the program sets them to the default value
private: // place private functions or variables declarations here
}; // don't forget the semicolon!
#endif
