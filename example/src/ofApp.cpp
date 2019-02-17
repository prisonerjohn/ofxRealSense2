#include "ofApp.h"

void ofApp::setup()
{
    this->eventListeners.push(this->context.deviceAddedEvent.newListener([&](std::string serialNumber)
    {
        ofLogNotice(__FUNCTION__) << "Starting device " << serialNumber;
        auto device = this->context.getDevice(serialNumber);
        device->startPipeline();
        device->enableDepth();
        device->enableColor();
    }));
 
    this->context.setup();
}

void ofApp::exit()
{
    this->context.clear();
}

void ofApp::update() 
{
    this->context.update();
}

void ofApp::draw()
{
    ofBackground(0);

    int i = 0;
    auto it = this->context.getDevices().begin();
    while (it != this->context.getDevices().end())
    {
        it->second->getDepthTex().draw(640 * i, 0);
        it->second->getColorTex().draw(640 * i, 360);

        ++it;
        ++i;
    }
    
    ofDrawBitmapString(ofToString(ofGetFrameRate()), 10, 10);
}

void ofApp::keyPressed(int key)
{
    
}
