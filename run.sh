sudo apt install pkg-config libwayland-dev libxrandr-dev libxkbcommon-dev libxinerama-dev libxcursor-dev libxi-dev

git submodule init
git submodule update

cd deps/paho.mqtt.c
cmake -B build -DPAHO_BUILD_SHARED=OFF -DPAHO_BUILD_STATIC=ON
cd build
sudo make install

cd ../../glfw
cmake -B build 
cd build
sudo make install

cd ../../../
cmake -B build
cd build
sudo make