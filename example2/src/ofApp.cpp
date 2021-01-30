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

    this->eventListeners.push(this->context.deviceAddedEvent.newListener([&](std::string serialNumber) {
        ofLogNotice(__FUNCTION__) << "Starting device " << serialNumber;
        auto device = this->context.getDevice(serialNumber);
		device->enableDepth(width, height, fps);
		device->enableColor(width, height, fps);
		if (pointsEnabled) device->enablePoints();
		device->startPipeline();

		device->holeFillingEnabled = holeFilling;
        device->temporalFilterEnabled = temporalNoiseReduction;
        device->spatialFilterEnabled = spatialNoiseReduction;
        device->alignMode = alignment;
	}));

    try {
        this->context.setup(false);
    } catch (std::exception& e) {
        ofLogFatalError(__FUNCTION__) << e.what();
    }
}

void ofApp::exit() {
    this->context.clear();
}

void ofApp::update() {
    this->context.update();
}

void ofApp::draw() {
    ofBackground(0);

    int i = 0;
    auto it = this->context.getDevices().begin();
    while (it != this->context.getDevices().end()) {
        int x = width * i;
        it->second->getColorTex().draw(x, 0);
		it->second->getDepthTex().draw(x, height);

        ++it;
        ++i;
    }
}

void ofApp::keyPressed(int key) {
    //
}
