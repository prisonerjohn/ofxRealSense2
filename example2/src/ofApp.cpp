#include "ofApp.h"

void ofApp::setup() {
    ofDisableArbTex();

    this->eventListeners.push(this->context.deviceAddedEvent.newListener([&](std::string serialNumber) {
        ofLogNotice(__FUNCTION__) << "Starting device " << serialNumber;
        auto device = this->context.getDevice(serialNumber);
		device->enableDepth(width, height, fps);
		device->enableColor(width, height, fps);
		//device->enablePoints();
		device->startPipeline();

		//device->holeFillingEnabled = true;
		//device->temporalFilterEnabled = true;
		device->alignMode = 2; // 0 none, 1 color to depth, 2 depth to color
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
