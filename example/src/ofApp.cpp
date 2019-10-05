#include "ofApp.h"

void ofApp::setup()
{
    ofDisableArbTex();

    this->guiPanel.setup("settings.xml");

    this->eventListeners.push(this->context.deviceAddedEvent.newListener([&](std::string serialNumber)
    {
        ofLogNotice(__FUNCTION__) << "Starting device " << serialNumber;
        auto device = this->context.getDevice(serialNumber);
        device->startPipeline();
        device->enableDepth();
        device->enableColor();
        device->enablePoints();
        this->guiPanel.add(device->params);
    }));

    try
    {
        this->context.setup(false);
    }
    catch (std::exception& e)
    {
        ofLogFatalError(__FUNCTION__) << e.what();
    }
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

    this->cam.begin();
    ofPushMatrix();
    ofScale(10);
    {
        auto it = this->context.getDevices().begin();
        while (it != this->context.getDevices().end())
        {
            it->second->getColorTex().bind();
            it->second->getPointsVbo().draw(GL_POINTS, 0, it->second->getNumPoints());
            it->second->getColorTex().unbind();

            ++it;
        }
    }
    ofPopMatrix();
    this->cam.end();
    
    ofDrawBitmapString(ofToString(ofGetFrameRate()), 10, 10);

    this->guiPanel.draw();
}

void ofApp::keyPressed(int key)
{
    
}
