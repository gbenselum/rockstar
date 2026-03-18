#include "dsp_engine.h"
#include <NAM/get_dsp.h>
#include <cmath>
#include <iostream>

namespace rockstar {
namespace dsp {

DspEngine::DspEngine(float sample_rate, int max_buffer_size)
    : sample_rate_(sample_rate), max_buffer_size_(max_buffer_size) {

  // Initialize temporary buffer for NAM output
  tmp_nam_out_.push_back(new float[max_buffer_size]());

  // Initialize empty NAM core (pass-through)
  nam_core_ = nullptr;

  // Configure Blocks
  eq_.set_sample_rate(sample_rate_);
}

DspEngine::~DspEngine() {
  for (auto buf : tmp_nam_out_) {
    delete[] buf;
  }
}

void DspEngine::set_input_gain(float gain_db) {
  input_gain_ = powf(10.0f, gain_db * 0.05f);
}

void DspEngine::set_output_gain(float gain_db) {
  output_gain_ = powf(10.0f, gain_db * 0.05f);
}

void DspEngine::load_nam_model(const std::string &model_path) {
  std::lock_guard<std::mutex> lock(nam_mutex_);
  try {
    nam_core_ = nam::get_dsp(model_path);
  } catch (std::exception &e) {
    std::cerr << "Failed to load NAM model: " << e.what() << std::endl;
    nam_core_ = nullptr;
  }
}

void DspEngine::process(float **in_buffers, float **out_buffers,
                        uint32_t nframes) {
  if (nframes > max_buffer_size_) {
    // Drop audio if buffer exceeds max (should not happen usually)
    return;
  }

  float *in_l = in_buffers[0]; // Mono input
  float *out_l = out_buffers[0];
  float *out_r = out_buffers[1];

  // 1. Input Stage & Gain
  for (uint32_t i = 0; i < nframes; ++i) {
    tmp_nam_out_[0][i] = in_l[i] * input_gain_;
  }

  // 2. NAM Stage
  {
    std::lock_guard<std::mutex> lock(nam_mutex_);
    if (nam_core_) {
      // Process NAM. Note: process takes input, output, and num frames
      // Assuming the common nam::DSP API
      nam_core_->process(tmp_nam_out_[0], tmp_nam_out_[0], nframes);
    }
  }

  // 3. EQ Stage (Post-Amp as requested)
  eq_.process(tmp_nam_out_[0], nframes);

  // 4. Multi-IR Stage & Output Mixing
  cab_mixer_.process(tmp_nam_out_[0], out_l, out_r, nframes);

  // 5. Output Stage & Gain
  for (uint32_t i = 0; i < nframes; ++i) {
    out_l[i] *= output_gain_;
    out_r[i] *= output_gain_;
  }
}

} // namespace dsp
} // namespace rockstar
