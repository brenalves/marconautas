if libdecor error: dont run it in vscode

cmake -S . -B build
cd build
make

probably for first non-glfw lib user will occur error of missing dependencies, just install every one
e.g: libxcursor-dev, libxi-dev, libxinerama-dev, libxrandr-dev, etc...