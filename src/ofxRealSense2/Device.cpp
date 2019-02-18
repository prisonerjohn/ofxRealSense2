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

    void Device::startPipeline()
    {
        if (this->running)
        {
            this->stopPipeline();
        }

        auto serialNumber = std::string(this->device.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER));
        this->config.enable_device(serialNumber);
        this->profile = this->pipeline.start(this->config);
        this->setupParams();
        this->startThread();
        this->running = true;
    }

    void Device::stopPipeline()
    {
        if (!this->running) return;

        this->stopThread();
        this->pipeline.stop();
        this->clearParams();
        this->running = false;
    }

    bool Device::isRunning() const
    {
        return this->running;
    }

    void Device::setupParams()
    {
        rs2::sensor sensor = this->device.query_sensors()[0];
        rs2::option_range orExposure = sensor.get_option_range(rs2_option::RS2_OPTION_EXPOSURE);
        rs2::option_range orGain = sensor.get_option_range(rs2_option::RS2_OPTION_GAIN);
        rs2::option_range orMinDist = this->colorizer.get_option_range(rs2_option::RS2_OPTION_MIN_DISTANCE);
        rs2::option_range orMaxDist = this->colorizer.get_option_range(rs2_option::RS2_OPTION_MAX_DISTANCE);

        this->autoExposure.set("Auto-exposure", true);
        this->enableEmitter.set("Emitter", true);
        this->irExposure.set("IR Exposure", orExposure.def, orExposure.min, orExposure.max);
        this->depthMin.set("Min Depth", orMinDist.def, orMinDist.min, orMinDist.max);
        this->depthMax.set("Max Depth", orMaxDist.def, orMaxDist.min, orMaxDist.max);

        this->eventListeners.push(autoExposure.newListener([this](bool &)
        {
            if (!this->running) return;

            auto sensor = this->pipeline.get_active_profile().get_device().first<rs2::depth_sensor>();
            if (sensor.supports(RS2_OPTION_ENABLE_AUTO_EXPOSURE))
            {
                sensor.set_option(RS2_OPTION_ENABLE_AUTO_EXPOSURE, this->autoExposure ? 1.0f : 0.0f);
            }
        }));

        this->eventListeners.push(enableEmitter.newListener([this](bool &)
        {
            if (!this->running) return;

            auto sensor = this->pipeline.get_active_profile().get_device().first<rs2::depth_sensor>();
            if (sensor.supports(RS2_OPTION_EMITTER_ENABLED))
            {
                sensor.set_option(RS2_OPTION_EMITTER_ENABLED, this->enableEmitter ? 1.0f : 0.0f);
            }
        }));

        this->eventListeners.push(irExposure.newListener([this](int &)
        {
            if (!this->running) return;

            auto sensor = this->pipeline.get_active_profile().get_device().first<rs2::depth_sensor>();
            if (sensor.supports(rs2_option::RS2_OPTION_EXPOSURE))
            {
                sensor.set_option(rs2_option::RS2_OPTION_EXPOSURE, (float)this->irExposure);
            }
        }));

        this->eventListeners.push(depthMin.newListener([this](float &)
        {
            if (!this->running) return;

            this->colorizer.set_option(rs2_option::RS2_OPTION_HISTOGRAM_EQUALIZATION_ENABLED, 0);
            if (this->colorizer.supports(rs2_option::RS2_OPTION_MIN_DISTANCE))
            {
                this->colorizer.set_option(rs2_option::RS2_OPTION_MIN_DISTANCE, this->depthMin);
            }
        }));

        this->eventListeners.push(depthMax.newListener([this](float &)
        {
            if (!this->running) return;

            this->colorizer.set_option(rs2_option::RS2_OPTION_HISTOGRAM_EQUALIZATION_ENABLED, 0);
            if (this->colorizer.supports(rs2_option::RS2_OPTION_MAX_DISTANCE))
            {
                this->colorizer.set_option(rs2_option::RS2_OPTION_MAX_DISTANCE, this->depthMax);
            }
        }));

        auto name = std::string(this->device.get_info(RS2_CAMERA_INFO_NAME));
        auto serialNumber = std::string(this->device.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER));

        this->params.setName(name + " " + serialNumber);
        this->params.add(this->autoExposure, this->enableEmitter, this->irExposure, this->depthMin, this->depthMax);
    }

    void Device::clearParams()
    {
        this->eventListeners.unsubscribeAll();
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

    void Device::threadedFunction()
    {
        while (isThreadRunning())
        {
            rs2::frameset frameset = this->pipeline.wait_for_frames();
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
        return this->pipeline;
    }

    const rs2::pipeline_profile & Device::getNativeProfile() const
    {
        return this->profile;
    }

}