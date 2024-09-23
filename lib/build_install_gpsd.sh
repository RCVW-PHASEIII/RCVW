#!/bin/bash

# Check if an argument is provided
if [ $# -eq 0 ]; then
    echo "Usage: $0 <gpsd_release_path>"
    exit 1
fi

# Change directory to the provided path
cd "$1" || exit

# Run the necessary commands
sudo scons prefix=/usr/local &&
sudo scons check &&
sudo scons udev-install &&
sudo scons install

echo "Build completed successfully."
