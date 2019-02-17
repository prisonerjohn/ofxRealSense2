#pragma once

#include "librealsense2/rs.hpp"

#include "ofTexture.h"
#include "ofThread.h"

namespace ofxRealSense2
{
    class Device
        : ofThread
    {
    public:
        Device(rs2::device device);
        ~Device();

        void enableDepth(int width = 640, int height = 360, int fps = 30);
        void disableDepth();

        void enableInfrared(int width = 640, int height = 360, int fps = 30);
        void disableInfrared();

        void enableColor(int width = 640, int height = 360, int fps = 30);
        void disableColor();

        void startPipeline();
        void stopPipeline();
        bool isRunning() const;

        void threadedFunction() override;
        void update();

        const ofTexture & getDepthTex() const;
        const ofTexture & getRawDepthTex() const;
        const ofTexture & getInfraredTex() const;
        const ofTexture & getColorTex() const;

        const rs2::device & getNativeDevice() const;
        const rs2::pipeline_profile & getProfile() const;

    private:
        rs2::device device;
        rs2::config config;
        rs2::pipeline pipe;
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
    };
}