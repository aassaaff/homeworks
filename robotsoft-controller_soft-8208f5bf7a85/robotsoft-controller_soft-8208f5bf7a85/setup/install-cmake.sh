#!/bin/bash
wget https://cmake.org/files/v3.18/cmake-3.18.5.tar.gz
tar xvfz cmake-3.18.5.tar.gz
cd cmake-3.18.5
./configure
sudo make install
cmake --version
