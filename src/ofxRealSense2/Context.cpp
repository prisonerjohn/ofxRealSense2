#include "Context.h"

#include "ofLog.h"

namespace ofxRealSense2
{
    Context::Context()
    {

    }

    Context::~Context()
    {
        clear();
    }

    void Context::setup(bool autoStart)
    {
        this->autoStart = autoStart;

        this->context = std::make_shared<rs2::context>();

        // Register callback for tracking which devices are currently connected.
        this->context->set_devices_changed_callback([&](rs2::event_information & info)
        {
            this->removeDevices(info);
            for (auto && dev : info.get_new_devices())
            {
                this->addDevice(dev);
            }
        });

        // Populate initial device list.
        for (auto && dev : this->context->query_devices())
        {
            this->addDevice(dev);
        }
    }

    void Context::clear()
    {
        auto it = this->devices.begin();
        while (it != this->devices.end())
        {
            it->second->stopPipeline();
            ++it;
        }
        this->devices.clear();

        this->context.reset();
    }

    void Context::update()
    {
        std::lock_guard<std::mutex> lock(this->mutex);
        for (auto it : this->devices)
        {
            if (it.second->isRunning())
            {
                //it.second->pollForFrames();
                //it.second->updateData();
                it.second->update();
            }
        }
    }

    void Context::addDevice(rs2::device device)
    {
        auto serialNumber = std::string(device.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER));
        std::lock_guard<std::mutex> lock(this->mutex);

        if (this->devices.find(serialNumber) != this->devices.end())
        {
            // Already added, ignore.
            return;
        }

        static const std::string platformCameraName = "Platform Camera";
        if (platformCameraName == device.get_info(RS2_CAMERA_INFO_NAME))
        {
            // Platform camera (webcam, etc), ignore.
            return;
        }

        // Add the device.
        ofLogNotice(__FUNCTION__) << "Add device " << serialNumber;
        this->devices.emplace(serialNumber, std::make_shared<Device>(*this->context, device));
        this->deviceAddedEvent.notify(serialNumber);

        if (this->autoStart)
        {
            // Start the device.
            ofLogNotice(__FUNCTION__) << "Start device " << serialNumber;
            auto device = this->devices.at(serialNumber);
            device->startPipeline();
            device->enableDepth();
            device->enableColor();
        }
    }

    void Context::removeDevices(const rs2::event_information & info)
    {
        std::lock_guard<std::mutex> lock(this->mutex);
        // Go over the list of devices and check if it was disconnected.
        auto it = this->devices.begin();
        while (it != this->devices.end())
        {
            if (info.was_removed(it->second->getNativeDevice()))
            {
                auto serialNumber = it->first;
                ofLogNotice(__FUNCTION__) << "Remove device " << serialNumber;
                this->deviceRemovedEvent.notify(serialNumber);
                it = this->devices.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    const std::map<std::string, std::shared_ptr<Device>> & Context::getDevices() const
    {
        return this->devices;
    }

    std::shared_ptr<Device> Context::getDevice(const std::string & serialNumber) const
    {
        return this->devices.at(serialNumber);
    }

    const std::shared_ptr<rs2::context> Context::getNativeContext() const
    {
        return this->context;
    }
}