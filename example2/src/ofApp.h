#pragma once

#include "ofMain.h"
#include "ofxRealSense2.h"

class ofApp : public ofBaseApp {

public:
    void setup();
    void exit();

    void update();
    void draw();

    void keyPressed(int key);

    ofxRealSense2::Context context;
    ofEventListeners eventListeners;

};
