#include "ofApp.h"

void ofApp::setup()
{
    ofDisableArbTex();

    this->guiPanel.setup("settings.xml");

    this->eventListeners.push(this->context.deviceAddedEvent.newListener([&](std::string serialNumber)
    {
        ofLogNotice(__FUNCTION__) << "Starting device " << serialNumber;
        auto device = this->context.getDevice(serialNumber);
        device->enableDepth();
        device->enableColor();
        device->enablePoints();
        device->startPipeline();
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
        int x = 640 * i;
        it->second->getDepthTex().draw(x, 0);
        it->second->getColorTex().draw(x, 360);

        if (ofInRange(ofGetMouseX(), x, x + 640) &&
            ofInRange(ofGetMouseY(), 0, 360))
        {
            float distance = it->second->getDistance(ofMap(ofGetMouseX(), x, x + 640, 0, 640), ofGetMouseY());
            ofDrawBitmapStringHighlight(ofToString(distance, 3) + " m", ofGetMouseX(), ofGetMouseY());
        }

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
