#pragma once

#include "librealsense2/rs.hpp"

#include "ofParameter.h"
#include "ofTexture.h"
#include "ofThread.h"
#include "ofVbo.h"

namespace ofxRealSense2
{
    class Device
        : ofThread
    {
    public:
        Device(rs2::device device);
        ~Device();

        void startPipeline();
        void stopPipeline();
        bool isRunning() const;

        void setupParams();
        void clearParams();

        void enableDepth(int width = 640, int height = 360, int fps = 30);
        void disableDepth();

        void enableInfrared(int width = 640, int height = 360, int fps = 30);
        void disableInfrared();

        void enableColor(int width = 640, int height = 360, int fps = 30);
        void disableColor();

        void enablePoints();
        void disablePoints();

        void threadedFunction() override;
        void update();

        const ofTexture & getDepthTex() const;
        const ofTexture & getRawDepthTex() const;
        const ofTexture & getInfraredTex() const;
        const ofTexture & getColorTex() const;

        const ofVbo & getPointsVbo() const;
        const size_t getNumPoints() const;

        const rs2::device & getNativeDevice() const;
        const rs2::pipeline & getNativePipeline() const;
        const rs2::pipeline_profile & getNativeProfile() const;

    public:
        ofParameterGroup params;
        ofParameter<bool> autoExposure;
        ofParameter<bool> enableEmitter;
        ofParameter<int> irExposure;
        ofParameter<float> depthMin;
        ofParameter<float> depthMax;

    private:
        rs2::device device;
        rs2::config config;
        rs2::pipeline pipeline;
        rs2::pipeline_profile profile;
        rs2::colorizer colorizer;

        bool running;

        int depthWidth;
        int depthHeight;
        bool depthEnabled;
        rs2::frame_queue depthQueue;
        ofTexture depthTex;
        ofTexture rawDepthTex;
   
        int infraredWidth;
        int infraredHeight;
        bool infraredEnabled;
        rs2::frame_queue infraredQueue;
        ofTexture infraredTex;

        int colorWidth;
        int colorHeight;
        bool colorEnabled;
        rs2::frame_queue colorQueue;
        ofTexture colorTex;

        rs2::pointcloud pointCloud;
        rs2::points points;
        bool pointsEnabled;
        ofVbo pointsVbo;

        ofEventListeners eventListeners;
    };
}