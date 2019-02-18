#include "Device.h"

#include "ofLog.h"

namespace ofxRealSense2
{
    Device::Device(rs2::device device)
        : device(device)
        , depthWidth(640), depthHeight(360)
        , depthEnabled(true)
        , infraredWidth(640), infraredHeight(360)
        , infraredEnabled(false)
        , colorWidth(640), colorHeight(360)
        , colorEnabled(false)
        , running(false)
    {

    }

    Device::~Device()
    {
        this->stopPipeline();
        this->waitForThread();
    }

    void Device::enableDepth(int width, int height, int fps)
    {
        this->depthWidth = width;
        this->depthHeight = height;
        this->config.enable_stream(RS2_STREAM_DEPTH, this->depthWidth, this->depthHeight, RS2_FORMAT_Z16, fps);
        this->depthTex.allocate(this->depthWidth, this->depthHeight, GL_RGB);
        this->rawDepthTex.allocate(this->depthWidth, this->depthHeight, GL_LUMINANCE16);
        this->colorizer.set_option(RS2_OPTION_COLOR_SCHEME, 2);
        this->depthEnabled = true;
    }

    void Device::disableDepth()
    {
        this->config.disable_stream(RS2_STREAM_DEPTH);
        this->depthTex.clear();
        this->rawDepthTex.clear();
        this->depthEnabled = false;
    }

    void Device::enableInfrared(int width, int height, int fps)
    {
        this->infraredWidth = width;
        this->infraredHeight = height;
        this->config.enable_stream(RS2_STREAM_INFRARED, this->infraredWidth, this->infraredHeight, RS2_FORMAT_Y8, fps);
        this->infraredTex.allocate(this->infraredWidth, this->infraredHeight, GL_LUMINANCE);
        this->infraredEnabled = true;
    }

    void Device::disableInfrared()
    {
        this->config.disable_stream(RS2_STREAM_INFRARED);
        this->infraredTex.clear();
        this->infraredEnabled = false;
    }

    void Device::enableColor(int width, int height, int fps)
    {
        this->colorWidth = width;
        this->colorHeight = height;
        this->config.enable_stream(RS2_STREAM_COLOR, this->colorWidth, this->colorHeight, RS2_FORMAT_RGB8, fps);
        this->colorTex.allocate(this->colorWidth, this->colorHeight, GL_RGB);
        this->colorEnabled = true;
    }

    void Device::disableColor()
    {
        this->config.disable_stream(RS2_STREAM_COLOR);
        this->colorTex.clear();
        this->colorEnabled = false;
    }

    void Device::enablePoints()
    {
        if (!this->depthEnabled)
        {
            this->enableDepth();
        }
        this->pointsEnabled = true;
    }

    void Device::disablePoints()
    {
        this->pointsVbo.clear();
        this->pointsEnabled = false;
    }

    void Device::startPipeline()
    {
        if (this->running)
        {
            this->stopPipeline();
        }

        auto serialNumber = std::string(this->device.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER));
        this->config.enable_device(serialNumber);
        this->profile = this->pipe.start(this->config);
        this->startThread();
        this->running = true;
    }

    void Device::stopPipeline()
    {
        if (!this->running) return;

        this->stopThread();
        this->pipe.stop();
        this->running = false;
    }

    bool Device::isRunning() const
    {
        return this->running;
    }

    void Device::threadedFunction()
    {
        while (isThreadRunning())
        {
            rs2::frameset frameset = this->pipe.wait_for_frames();
            if (this->depthEnabled)
            {
                auto depthFrame = frameset.get_depth_frame();

                if (this->pointsEnabled)
                {
                    // Generate the pointcloud and texture mappings.
                    this->points = this->pointCloud.calculate(depthFrame);
                }

                this->depthQueue.enqueue(depthFrame);
            }
            if (this->colorEnabled)
            {
                auto colorFrame = frameset.get_color_frame();

                if (this->pointsEnabled)
                {
                    // Map point cloud to color frame.
                    this->pointCloud.map_to(colorFrame);
                }

                this->colorQueue.enqueue(colorFrame);
            }
            if (this->infraredEnabled)
            {
                auto infraredFrame = frameset.get_infrared_frame();

                if (this->pointsEnabled && !this->colorEnabled)
                {
                    // Map point cloud to infrared frame.
                    this->pointCloud.map_to(infraredFrame);
                }

                this->infraredQueue.enqueue(infraredFrame);
            }
        }
    }

    void Device::update()
    {
        if (this->colorEnabled)
        {
            rs2::frame frame;
            if (this->colorQueue.poll_for_frame(&frame))
            {
                auto videoFrame = rs2::video_frame(frame);
                auto colorData = (uint8_t *)videoFrame.get_data();
                this->colorWidth = videoFrame.get_width();
                this->colorHeight = videoFrame.get_height();
                this->colorTex.loadData(colorData, this->colorWidth, this->colorHeight, GL_RGB);
            }
        }
        if (this->infraredEnabled)
        {
            rs2::frame frame;
            if (this->infraredQueue.poll_for_frame(&frame))
            {
                auto videoFrame = rs2::video_frame(frame);
                auto infraredData = (uint8_t *)videoFrame.get_data();
                this->infraredWidth = videoFrame.get_width();
                this->infraredHeight = videoFrame.get_height();
                this->infraredTex.loadData(infraredData, this->infraredWidth, this->infraredHeight, GL_LUMINANCE);
            }
        }
        if (this->depthEnabled)
        {
            rs2::frame frame;
            if (this->depthQueue.poll_for_frame(&frame))
            {
                auto depthFrame = rs2::depth_frame(frame);
                auto rawDepthData = (uint16_t *)depthFrame.get_data();
                this->depthWidth = depthFrame.get_width();
                this->depthHeight = depthFrame.get_height();
                this->rawDepthTex.loadData(rawDepthData, this->depthWidth, this->depthHeight, GL_LUMINANCE);

                auto normalizedDepthFrame = this->colorizer.process(depthFrame);
                auto normalizedDepthData = (uint8_t *)normalizedDepthFrame.get_data();
                this->depthTex.loadData(normalizedDepthData, this->depthWidth, this->depthHeight, GL_RGB);

                if (this->pointsEnabled)
                {
                    // Upload point data to the vbo.
                    auto vertices = this->points.get_vertices();
                    auto texCoords = this->points.get_texture_coordinates();
                    ofLogVerbose(__FUNCTION__) << "Uploading " << this->points.size() << " points";
                    if (this->pointsVbo.getNumVertices() < this->points.size())
                    {
                        this->pointsVbo.setVertexData((const float *)vertices, 3, this->points.size(), GL_STREAM_DRAW);
                        this->pointsVbo.setTexCoordData((const float *)texCoords, this->points.size(), GL_STREAM_DRAW);
                    }
                    else
                    {
                        this->pointsVbo.updateVertexData((const float *)vertices, this->points.size());
                        this->pointsVbo.updateTexCoordData((const float *)texCoords, this->points.size());
                    }
                }
            }
        }
    }

    const ofTexture & Device::getDepthTex() const
    {
        return this->depthTex;
    }

    const ofTexture & Device::getRawDepthTex() const
    {
        return this->rawDepthTex;
    }

    const ofTexture & Device::getInfraredTex() const
    {
        return this->infraredTex;
    }

    const ofTexture & Device::getColorTex() const
    {
        return this->colorTex;
    }

    const ofVbo & Device::getPointsVbo() const
    {
        return this->pointsVbo;
    }

    const size_t Device::getNumPoints() const
    {
        return this->points.size();
    }

    const rs2::device & Device::getNativeDevice() const
    {
        return this->device;
    }

    const rs2::pipeline & Device::getNativePipeline() const
    {
        return this->pipe;
    }

    const rs2::pipeline_profile & Device::getNativeProfile() const
    {
        return this->profile;
    }

}