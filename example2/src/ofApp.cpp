#include "ofApp.h"

void ofApp::setup() {
    settings.loadFile("settings.xml");
    width = settings.getValue("settings:width", 640);
    height = settings.getValue("settings:height", 480);
    fps = settings.getValue("settings:fps", 30);
    alignment = settings.getValue("settings:alignment", 2); // 0 none, 1 depth, 2 color
    pointsEnabled = (bool) settings.getValue("settings:points_enabled", 0);
    holeFilling = (bool) settings.getValue("settings:hole_filling", 0);
    spatialNoiseReduction = (bool) settings.getValue("settings:spatial_noise_reduction", 0);
    temporalNoiseReduction = (bool) settings.getValue("settings:temporal_noise_reduction", 0);

    ofDisableArbTex();
   
    ofAddListener(rsContext.deviceAddedEvent, this, &ofApp::deviceAdded);

    try {
        rsContext.setup(false);
    } catch (std::exception& e) {
        ofLogFatalError(__FUNCTION__) << e.what();
    }
}

void ofApp::deviceAdded(std::string& serialNumber) {
    ofLogNotice(__FUNCTION__) << "Starting device " << serialNumber;
    
    rsDevice = rsContext.getDevice(serialNumber);
    
    rsDevice->enableDepth(width, height, fps);
    rsDevice->enableColor(width, height, fps);
    if (pointsEnabled) rsDevice->enablePoints();
    rsDevice->startPipeline();

    rsDevice->holeFillingEnabled = holeFilling;
    rsDevice->temporalFilterEnabled = temporalNoiseReduction;
    rsDevice->spatialFilterEnabled = spatialNoiseReduction;
    rsDevice->alignMode = alignment;
}

void ofApp::exit() {
    rsContext.clear();
}

void ofApp::update() {
    rsContext.update();
}

void ofApp::draw() {
    ofBackground(0);
    if (rsDevice) {
        rsDevice->getDepthTex().draw(0, height);
        rsDevice->getColorTex().draw(0, 0);
    }
}

void ofApp::keyPressed(int key) {
    //
}
