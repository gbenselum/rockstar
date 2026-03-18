#include <chrono>
#include <cstdlib>
#include <gtkmm.h>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "host/jack_host.h"

#include "dsp/dsp_engine.h"

// Global engine reference for the DSP audio callback
std::unique_ptr<rockstar::dsp::DspEngine> g_engine;

// Real DSP callback for routing audio through NAM, EQ, and CabSim Mixer
void dsp_process(float **in_buffers, float **out_buffers, uint32_t nframes) {
  if (g_engine) {
    g_engine->process(in_buffers, out_buffers, nframes);
  }
}

int main(int argc, char *argv[]) {
  std::cout << "Starting Rockstar Audio Application..." << std::endl;

  // 1. Initialize Audio Host Thread
  auto jack = std::make_unique<rockstar::host::JackHost>("rockstar");

  // 1b. Initialize DSP Engine
  g_engine =
      std::make_unique<rockstar::dsp::DspEngine>(jack->get_sample_rate(), 2048);

  jack->set_process_callback(dsp_process);

  if (!jack->start()) {
    std::cerr << "Failed to start audio engine. Exiting." << std::endl;
    return 1;
  }

  // 2. Initialize GTK GUI
  auto app = Gtk::Application::create(argc, argv, "org.open_source.rockstar");

  Gtk::Window window;
  window.set_default_size(800, 600);
  window.set_title("Rockstar - Real Time DSP");

  // Display the mockup skin as a test (if it exists in current dir)
  std::string home_dir = getenv("HOME") ? getenv("HOME") : "";
  std::string skin_path = home_dir + "/.config/rockstar/skins/background.png";

  Gtk::Image mockup_image;
  try {
    mockup_image.set(skin_path);
  } catch (...) {
    std::cout << "Skin not found at " << skin_path << ", trying local FACE.png"
              << std::endl;
    mockup_image.set("FACE.png");
  }
  window.add(mockup_image);
  mockup_image.show();

  std::cout << "Running GUI Loop..." << std::endl;
  int status = app->run(window);

  // 3. Cleanup
  std::cout << "Shutting down..." << std::endl;
  jack->stop();

  return status;
}
