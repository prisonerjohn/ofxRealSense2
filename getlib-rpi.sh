URL=https://fox-gieg.com/patches/github/n1ckfg/ofxRealSense2/libs/librealsense2/lib/rpi

sudo curl $URL/librealsense2.so --output /usr/lib/librealsense2.so

sudo cp libs/librealsense2/lib/rpi/99-realsense-libusb.rules /etc/udev/rules.d/99-realsense-libusb.rules

sudo udevadm control --reload-rules
sudo udevadm trigger
