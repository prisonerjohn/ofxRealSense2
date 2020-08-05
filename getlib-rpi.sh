URL=https://fox-gieg.com/patches/github/n1ckfg/ofxRealSense2/libs/librealsense2/lib/rpi

cd /usr/lib/
sudo curl $URL/librealsense2.so --output librealsense2.so

cd /etc/udev/rules.d/
sudo curl $URL/99-realsense-libusb.rules --output 99-realsense-libusb.rules 

sudo udevadm control --reload-rules
sudo udevadm trigger
