#ifndef ROCKSTAR_DSP_ENGINE_H
#define ROCKSTAR_DSP_ENGINE_H

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "cabsim.h"
#include "eq.h"

// Forward declaration for NAM core to avoid including full eigen headers here
// if possible
namespace nam {
class DSP;
}

namespace rockstar {
namespace dsp {

class DspEngine {
public:
  DspEngine(float sample_rate, int max_buffer_size);
  ~DspEngine();

  void process(float **in_buffers, float **out_buffers, uint32_t nframes);

  // Provide parameters routing
  void load_nam_model(const std::string &model_path);
  FourBandEQ &get_eq() { return eq_; }
  MultiCabSimMixer &get_cab_mixer() { return cab_mixer_; }

  void set_input_gain(float gain_db);
  void set_output_gain(float gain_db);

private:
  float sample_rate_;
  int max_buffer_size_;

  float input_gain_ = 1.0f;
  float output_gain_ = 1.0f;

  // Blocks
  std::unique_ptr<nam::DSP> nam_core_;
  FourBandEQ eq_;
  MultiCabSimMixer cab_mixer_;

  // Intermediate block buffer
  std::vector<float *> tmp_nam_out_;

  std::mutex nam_mutex_;
};

} // namespace dsp
} // namespace rockstar

#endif // ROCKSTAR_DSP_ENGINE_H
