#!/bin/bash
set -e

echo "======================================"
echo " Rockstar Linux Audio Installer"
echo "======================================"

# Determine if we are on Debian/Ubuntu/PopOS by checking apt
if ! command -v apt >/dev/null 2>&1; then
    echo "This installation script only supports Debian/Ubuntu/Pop!_OS derivatives."
    echo "Please build manually via CMake if you are on another distribution."
    exit 1
fi

echo ">> Installing required dependencies..."
sudo apt update
sudo apt install -y build-essential cmake pkg-config \
    libgtk-3-dev libgtkmm-3.0-dev \
    libjack-dev \
    libfftw3-dev \
    libsndfile1-dev \
    git

echo ">> Initializing DSP core submodules..."
git submodule update --init --recursive

echo ">> Configuring CMake Project..."
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release

echo ">> Building Rockstar..."
make -j$(nproc)

echo ">> Installing Rockstar System-Wide..."
sudo make install

echo ">> Creating standard config folders..."
mkdir -p ~/.config/rockstar/skins/

echo ">> Generating .desktop Launcher..."
cat << EOF | sudo tee /usr/share/applications/rockstar.desktop > /dev/null
[Desktop Entry]
Name=Rockstar
Comment=Real-Time Guitar DSP Application
Exec=rockstar
Icon=audio-card
Terminal=false
Type=Application
Categories=AudioVideo;Audio;
EOF

echo "======================================"
echo " Installation Complete!"
echo " Run 'rockstar' from terminal or your app launcher."
echo " Drop custom amp face designs (background.png) in ~/.config/rockstar/skins/"
echo "======================================"
