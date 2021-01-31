#pragma once

#include "ofMain.h"
#include "ofxRealSense2.h"
#include "ofxXmlSettings.h"

class ofApp : public ofBaseApp {

	public:
		void setup();
		void exit();

		void update();
		void draw();

		void keyPressed(int key);

		void deviceAdded(std::string& serialNumber);

		ofxRealSense2::Context rsContext;
		std::shared_ptr<ofxRealSense2::Device> rsDevice;

        ofxXmlSettings settings;
		int width, height, fps, alignment;
        bool overUnder;
        int x1, y1, x2, y2;
        bool pointsEnabled, emitterEnabled, infraredEnabled;
        bool holeFilling, spatialNoiseReduction, temporalNoiseReduction;
		
};
