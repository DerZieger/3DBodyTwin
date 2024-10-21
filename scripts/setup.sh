#!/bin/bash

sudo apt-get install -y build-essential libopengl-dev xorg-dev libx11-dev lsb-release freeglut3-dev make git libglu1-mesa-dev g++ gcc libwayland-dev libxkbcommon-dev wayland-protocols extra-cmake-modules

cd scripts

chmod +x *

source ~/.bashrc

conda activate twin

wget https://raw.githubusercontent.com/opensim-org/opensim-core/17baa22f089642e18baf82b404add0a27fbacc54/scripts/build/opensim-core-linux-build-script.sh

chmod +x opensim-core-linux-build-script.sh

sed -i '209s@^@#@' opensim-core-linux-build-script.sh #comment out ctest because of python init error

sed -i '2s@^@source ~/.bashrc \nconda activate twin@' opensim-core-linux-build-script.sh

./opensim-core-linux-build-script.sh -j 8 -d Release -n

source ~/.bashrc

rm ./opensim-core-linux-build-script.sh

cd ..
