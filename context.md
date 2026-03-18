# Rockstar Context & Handoff Document

**Date:** March 2026
**Target OS:** Debian / Ubuntu / Pop!_OS
**Frameworks:** C++17, CMake, JACK Audio Connection Kit, GTKmm 3.0, FFTW3, SndFile, Catch2.

## Project Overview
"Rockstar" is a standalone, real-time Linux guitar processing application. It is NOT a plugin; it connects directly to the JACK audio server. The user requested an app architected similarly to Guitarix, but utilizing the state-of-the-art Neural Amp Modeler (NAM) for the core amp tone.

### Core DSP Signal Chain (Order)
1. **Input Gain**
2. **NAM Core**: Runs `.nam` neural network models. Integrated via the `NeuralAmpModelerCore` Git submodule.
3. **4-Band Post-EQ**: Integrated via a custom `Biquad` implementation (`test_eq.cpp` confirms the math).
   - Low Shelf
   - Low-Mid Peaking (Parametric)
   - High-Mid Peaking (Parametric)
   - High Shelf
4. **Multi-Cabsim Mixer**: Allows loading multiple `.wav` Impulse Responses (IRs) in parallel. It uses `fftw3` for zero-latency blocked convolution, based on logic from `mod-cabsim-IR-loader`.
5. **Output Gain / Panning**

### The GUI & "Skins"
The GUI is built using GTKmm 3.0. The user requested interchangeable "amp faces". The current setup spawns a GTK Window and attempts to load a custom background image from:
`~/.config/rockstar/skins/background.png`
(It falls back to `FACE.png` in the local directory for testing).

The idea is that interactive GTK widgets (like knobs from `libgxw`) will be overlayed on top of this single flattened background image.

### Current State (What is Done)
- **Scaffolding Complete**: The directory structure, CMake configuration (with `CPack` for building `.deb` files), and Git submodules are all initialized.
- **DSP Engine Written**: `src/dsp/dsp_engine.cpp` connects the NAM, EQ, and CabSim instances perfectly into a processing loop.
- **JACK Host Written**: `src/host/jack_host.cpp` is written and passes audio frames to the `DspEngine`.
- **Installer**: `install.sh` exists to automatically grab `apt` dependencies and run the CMake build.
- **CI/CD**: `.github/workflows/build.yml` is configured to run tests and package `.deb` artifacts on push.
- **Documentation**: README, LICENSE (Non-Commercial), and GitFlow branching strategies are documented.

### Next Immediate Steps (The "Test Bench" Phase)
This handoff is occurring because the original work was done on a Mac. The user is now moving to a native Linux machine (the Test Bench) to compile and test.

1. **Compile the App**: Run `./install.sh` (or CMake manually) on the Linux host. Ensure GTKmm3 and JACK headers are found.
2. **Run the App**: Launch `rockstar`. Verify JACK connects and the GUI Window appears with the mockup face.
3. **Testing Audio**: Pass guitar audio through JACK. Connect a `.nam` file to the `load_nam_model` function in the codebase (currently missing GUI file pickers for this).
4. **GUI Wiring**: The DSP parameters (EQ frequencies, gains, CabSim IR paths, volume, pan) now need to be wired to physical GTK widgets built over the skin image. This is the biggest remaining developmental task.
