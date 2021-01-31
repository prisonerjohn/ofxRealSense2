#include "ofApp.h"

void ofApp::setup() {
    settings.loadFile("settings.xml");
    width = settings.getValue("settings:width", 640);
    height = settings.getValue("settings:height", 480);
    fps = settings.getValue("settings:fps", 30);
    vsyncEnabled = (bool) settings.getValue("settings:vsync_enabled", 1);
    overUnder = (bool) settings.getValue("settings:over_under", 0); // 0 sbs, 1 ou
    alignment = settings.getValue("settings:alignment", 2); // 0 none, 1 depth, 2 color
    infraredEnabled = (bool) settings.getValue("settings:infrared_enabled", 0);
    pointsEnabled = (bool) settings.getValue("settings:points_enabled", 0);
    emitterEnabled = (bool) settings.getValue("settings:emitter_enabled", 1);
    holeFilling = (bool) settings.getValue("settings:hole_filling", 0);
    spatialNoiseReduction = (bool) settings.getValue("settings:spatial_noise_reduction", 0);
    temporalNoiseReduction = (bool) settings.getValue("settings:temporal_noise_reduction", 0);

    ofDisableArbTex();
    ofAddListener(rsContext.deviceAddedEvent, this, &ofApp::deviceAdded);
    ofSetVerticalSync(vsyncEnabled);
    
    if (overUnder) {
        ofSetWindowShape(width, height*2);
        x1 = 0;
        y1 = 0;
        x2 = 0;
        y2 = height;
    } else {
        ofSetWindowShape(width*2, height);
        x1 = width;
        y1 = 0;
        x2 = 0;
        y2 = 0;
    }
    
    try {
        rsContext.setup(false);
    } catch (std::exception& e) {
        ofLogFatalError(__FUNCTION__) << e.what();
    }
}

void ofApp::deviceAdded(std::string& serialNumber) {
    ofLogNotice(__FUNCTION__) << "Starting device " << serialNumber;
    
    rsDevice = rsContext.getDevice(serialNumber);
    
    // device methods go before pipeline
    rsDevice->enableDepth(width, height, fps);
    if (infraredEnabled) {
        rsDevice->enableInfrared(width, height, fps);
    } else {
        rsDevice->enableColor(width, height, fps);
    }
    if (pointsEnabled) rsDevice->enablePoints();

    rsDevice->startPipeline();

    // device switches go after pipeline
    rsDevice->emitterEnabled = emitterEnabled;
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
        if (infraredEnabled) {
            rsDevice->getInfraredTex().draw(x1, y1, width, height);
        } else {
            rsDevice->getColorTex().draw(x1, y1, width, height);
        }
        rsDevice->getDepthTex().draw(x2, y2, width, height);
    }
}

void ofApp::keyPressed(int key) {
    //
}
