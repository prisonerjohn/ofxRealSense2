# https://github.com/acrobotic/Ai_Demos_RPi/wiki/Raspberry-Pi-4-and-Intel-RealSense-D435

sudo apt-get install automake libtool vim cmake libusb-1.0-0-dev libx11-dev xorg-dev libglu1-mesa-dev

mkdir setup
cd setup
git clone https://github.com/IntelRealSense/librealsense
cd librealsense
sudo cp config/99-realsense-libusb.rules /etc/udev/rules.d/

sudo udevadm control --reload-rules
sudo udevadm trigger

mkdir build
cd build
cmake .. -DBUILD_EXAMPLES=true -DCMAKE_BUILD_TYPE=Release -DFORCE_LIBUVC=true
make
sudo make install

sudo cp librealsense2.so /usr/lib/librealsense2.so


