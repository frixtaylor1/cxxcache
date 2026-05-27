#!/bin/bash

#
# Install g++ compiler.
#
sudo apt install g++

#
# Install Bun runtime/compiler.
#
curl -fsSL https://bun.com/install | bash

mkdir -p ./build