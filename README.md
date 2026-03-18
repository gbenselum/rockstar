# Rockstar: Real-Time Linux Audio Application

[![Rockstar CI Build](https://github.com/OWNER/REPO/actions/workflows/build.yml/badge.svg)](https://github.com/OWNER/REPO/actions/workflows/build.yml)

Rockstar is a standalone C++ audio application optimized for the JACK Audio Connection Kit on Linux (Debian/Ubuntu/Pop!_OS).

### Features
* **Amp Emulation:** Powered by [Neural Amp Modeler Core](https://github.com/sdatkinson/NeuralAmpModelerCore).
* **Tone Shaping:** Includes a post-amp 4-band Parametric/Shelving EQ.
* **Cabinet Simulation:** Parallel multi-IR loader powered by FFTW convolver (derived from `mod-cabsim-IR-loader`).
* **Skins (Amp Faces):** GTK3/Guitarix GUI widgets that overlay dynamic backgrounds placed in `~/.config/rockstar/skins/background.png`.

## Installation

### Method 1: Provided Installer Script
Run the installer script to automatically download dependencies and compile source:
```bash
./install.sh
```

### Method 2: DEB Package
Download the latest `.deb` package from the [Releases](https://github.com/OWNER/REPO/releases) page or CI Artifacts, then install via:
```bash
sudo dpkg -i rockstar-0.1.0-Linux.deb
sudo apt-get install -f # to resolve dependencies
```

### Method 3: Manual CMake
```bash
sudo apt install build-essential cmake pkg-config libgtk-3-dev libgtkmm-3.0-dev libjack-jackd2-dev libfftw3-dev libsndfile1-dev
git clone --recursive https://github.com/OWNER/REPO.git rockstar
cd rockstar
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
sudo make install
```

## Continuous Integration (GitHub Actions)

This repository includes a fully configured **GitHub Actions** workflow (`.github/workflows/build.yml`) that automatically ensures the codebase is healthy.

### What the CI Pipeline Does:
1. **Triggers:** Automatically runs on every `push` or `pull_request` to the `main` or `master` branches.
2. **Build Environment:** Provisions an Ubuntu 22.04 runner and installs all necessary audio and GTK+ dependencies.
3. **Compilation:** Compiles the entire C++ project and NAM submodules using `CMake` in `Release` mode.
4. **Testing:** Automatically executes the `Catch2` unit testing suite (`ctest`) to verify the mathematical correctness of our DSP blocks (like the 4-Band Biquad EQ).
5. **Packaging:** If compilation and tests pass, it uses `CPack` to generate a `.deb` installer.
6. **Artifact Generation:** The resulting `.deb` package is securely uploaded as a GitHub Artifact attached to the run, allowing users to easily download and install the latest compiled version without building locally.

## License

**Personal & Non-Commercial Use Only.** 
This project integrates multiple open-source technologies including the Neural Amp Modeler Core (MIT) and FFTW3 (GPL). Please refer to the `LICENSE` file in the repository root for explicit terms regarding use and third-party restrictions. This software may not be used commercially or distributed for profit.
