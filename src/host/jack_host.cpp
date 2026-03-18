#include "jack_host.h"
#include <iostream>

namespace rockstar {
namespace host {

JackHost::JackHost(const std::string &client_name) : client_name_(client_name) {
  jack_status_t status;
  client_ = jack_client_open(client_name_.c_str(), JackNullOption, &status);
  if (!client_) {
    std::cerr << "Failed to open JACK client. Is the JACK server running?"
              << std::endl;
    return;
  }

  sample_rate_ = jack_get_sample_rate(client_);
  buffer_size_ = jack_get_buffer_size(client_);

  // Set callbacks
  jack_set_process_callback(client_, jack_process_callback, this);
  jack_on_shutdown(client_, jack_shutdown_callback, this);
  jack_set_sample_rate_callback(client_, jack_sample_rate_callback, this);
  jack_set_buffer_size_callback(client_, jack_buffer_size_callback, this);

  // Register simple 1 In, 2 Out setup for now
  input_ports_.push_back(jack_port_register(
      client_, "in_1", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0));
  output_ports_.push_back(jack_port_register(
      client_, "out_l", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0));
  output_ports_.push_back(jack_port_register(
      client_, "out_r", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0));
}

JackHost::~JackHost() {
  stop();
  if (client_) {
    jack_client_close(client_);
  }
}

bool JackHost::start() {
  if (!client_)
    return false;

  if (jack_activate(client_)) {
    std::cerr << "Failed to activate JACK client." << std::endl;
    return false;
  }
  std::cout << "JACK host started successfully." << std::endl;
  return true;
}

void JackHost::stop() {
  if (client_) {
    jack_deactivate(client_);
  }
}

int JackHost::jack_process_callback(jack_nframes_t nframes, void *arg) {
  return static_cast<JackHost *>(arg)->process(nframes);
}

void JackHost::jack_shutdown_callback(void *arg) {
  std::cerr << "JACK server has been shutdown." << std::endl;
}

int JackHost::jack_sample_rate_callback(jack_nframes_t nframes, void *arg) {
  auto host = static_cast<JackHost *>(arg);
  host->sample_rate_ = nframes;
  std::cout << "JACK sample rate changed to: " << nframes << std::endl;
  return 0;
}

int JackHost::jack_buffer_size_callback(jack_nframes_t nframes, void *arg) {
  auto host = static_cast<JackHost *>(arg);
  host->buffer_size_ = nframes;
  std::cout << "JACK buffer size changed to: " << nframes << std::endl;
  return 0;
}

int JackHost::process(jack_nframes_t nframes) {
  if (process_cb_) {
    // Collect buffer pointers
    std::vector<float *> in_buffers(input_ports_.size());
    for (size_t i = 0; i < input_ports_.size(); ++i) {
      in_buffers[i] =
          static_cast<float *>(jack_port_get_buffer(input_ports_[i], nframes));
    }

    std::vector<float *> out_buffers(output_ports_.size());
    for (size_t i = 0; i < output_ports_.size(); ++i) {
      out_buffers[i] =
          static_cast<float *>(jack_port_get_buffer(output_ports_[i], nframes));
    }

    // Call the user provided DSP process
    process_cb_(in_buffers.data(), out_buffers.data(), nframes);
  } else {
    // Clear outputs if no logic is attached
    for (auto port : output_ports_) {
      float *out = static_cast<float *>(jack_port_get_buffer(port, nframes));
      for (uint32_t i = 0; i < nframes; ++i)
        out[i] = 0.0f;
    }
  }
  return 0;
}

} // namespace host
} // namespace rockstar
