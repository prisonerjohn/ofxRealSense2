#pragma once

#include "librealsense2/rs.hpp"

#include "ofParameter.h"
#include "ofPixels.h"
#include "ofTexture.h"
#include "ofThread.h"
#include "ofVboMesh.h"

namespace ofxRealSense2
{
    class Device
        : ofThread
    {
    public:
        enum Align
        {
            None,
            Depth,
            Color
        };
    public:
        Device(rs2::context& context, const rs2::device& device);
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

        const ofPixels& getDepthPix() const;
        const ofShortPixels& getRawDepthPix() const;
        const ofPixels& getInfraredPix() const;
        const ofPixels& getColorPix() const;

        const ofTexture& getDepthTex() const;
        const ofTexture& getRawDepthTex() const;
        const ofTexture& getInfraredTex() const;
        const ofTexture& getColorTex() const;

        const ofVboMesh& getPointsMesh() const;
        const size_t getNumPoints() const;

        float getDistance(int x, int y) const;

        const rs2::device& getNativeDevice() const;
        const rs2::pipeline& getNativePipeline() const;
        const rs2::pipeline_profile& getNativeProfile() const;

    public:
        ofParameterGroup params;

        ofParameter<int> alignMode;

        ofParameter<bool> autoExposure;
        ofParameter<bool> emitterEnabled;
        ofParameter<int> irExposure;

        ofParameter<float> depthMin;
        ofParameter<float> depthMax;

        ofParameter<bool> decimateEnabled;
        ofParameter<int> decimateMagnitude;

        ofParameter<bool> disparityTransformEnabled;

        ofParameter<bool> spatialFilterEnabled;
        ofParameter<int> spatialFilterMagnitude;
        ofParameter<float> spatialFilterSmoothAlpha;
        ofParameter<int> spatialFilterSmoothDelta;
        ofParameter<int> spatialFilterHoleFillingMode;

        ofParameter<bool> temporalFilterEnabled;
        ofParameter<float> temporalFilterSmoothAlpha;
        ofParameter<int> temporalFilterSmoothDelta;
        ofParameter<int> temporalFilterPersistencyMode;

        ofParameter<bool> holeFillingEnabled;
        ofParameter<int> holeFillingMode;

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
        std::shared_ptr<rs2::depth_frame> depthFrameRef;
        ofPixels depthPix;
        ofShortPixels rawDepthPix;
        ofTexture depthTex;
        ofTexture rawDepthTex;
   
        int infraredWidth;
        int infraredHeight;
        bool infraredEnabled;
        rs2::frame_queue infraredQueue;
        ofPixels infraredPix;
        ofTexture infraredTex;

        int colorWidth;
        int colorHeight;
        bool colorEnabled;
        rs2::frame_queue colorQueue;
        ofPixels colorPix;
        ofTexture colorTex;

        rs2::align alignToDepth;
        rs2::align alignToColor;

        rs2::pointcloud pointCloud;
        rs2::points points;
        bool pointsEnabled;
        ofVboMesh pointsMesh;

        rs2::decimation_filter decimationFilter;
        rs2::disparity_transform disparityTransform;
        rs2::disparity_transform depthTransform;
        rs2::spatial_filter spatialFilter;
        rs2::temporal_filter temporalFilter;
        rs2::hole_filling_filter holeFillingFilter;

        ofEventListeners eventListeners;
    };
}