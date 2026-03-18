#ifndef ROCKSTAR_DSP_CABSIM_H
#define ROCKSTAR_DSP_CABSIM_H

#include <fftw3.h>
#include <iostream>
#include <mutex>
#include <sndfile.h>
#include <string>
#include <vector>

namespace rockstar {
namespace dsp {

constexpr int MAX_FFT_SIZE = 2048;

class RingBuffer {
public:
  RingBuffer() : buffer(nullptr), size(0), head(0) {}
  ~RingBuffer() {
    if (buffer)
      delete[] buffer;
  }

  void init(int new_size) {
    if (buffer)
      delete[] buffer;
    size = new_size;
    buffer = new float[size];
    clear();
  }

  void clear() {
    if (buffer) {
      for (int i = 0; i < size; ++i)
        buffer[i] = 0.0f;
    }
    head = 0;
  }

  void push(float val) {
    buffer[head] = val;
    head = (head + 1) % size;
  }

  float get_relative(int offset_from_head) {
    // offset 1 means the most recently pushed sample (actually needs to go
    // backwards) Wait, the mod-cabsim ringbuffer gets values relative to the
    // current position. Let's just implement a simple delay line logic.
    int idx = (head - offset_from_head + size) % size;
    return buffer[idx];
  }

private:
  float *buffer;
  int size;
  int head;
};

class CabSim {
public:
  CabSim(float target_sample_rate = 48000.0f);
  ~CabSim();

  bool load_ir(const std::string &filepath);
  void process(const float *in, float *out, int num_frames);

  void set_volume(float db) { volume_ = powf(10.0f, db * 0.05f); }
  void set_pan(float pan) { pan_ = pan; /* -1 to 1 */ }

  float get_volume() const { return volume_; }
  float get_pan() const { return pan_; }

  void get_panned_output(float *out_l, float *out_r, int num_frames);

private:
  float resample_linear(const float *in, float *out, int in_rate, int out_rate,
                        int in_frames);

  float target_sample_rate_;
  float volume_ = 1.0f;
  float pan_ = 0.0f; // -1 to 1

  float *ir_data_ = nullptr;
  int ir_frames_ = 0;
  bool ir_loaded_ = false;

  // Convolution state
  float *inbuf;
  float *outbuf;
  float *IR;
  fftwf_complex *outComplex;
  fftwf_complex *IRout;
  fftwf_complex *convolved;

  fftwf_plan fft;
  fftwf_plan ifft;
  fftwf_plan IRfft;

  int prev_buffer_size_ = 0;
  int overlap_add_buffers_ = 8;
  RingBuffer overlap_buffer_;

  float *tmp_mono_out_ = nullptr;

  std::mutex process_mutex_;
};

class MultiCabSimMixer {
public:
  MultiCabSimMixer(int num_cabs = 2, float sample_rate = 48000);
  ~MultiCabSimMixer();

  CabSim *get_cab(int index) {
    return (index >= 0 && index < static_cast<int>(cabs_.size())) ? cabs_[index]
                                                                  : nullptr;
  }

  void process(const float *in, float *out_l, float *out_r, int num_frames);

private:
  std::vector<CabSim *> cabs_;
};

} // namespace dsp
} // namespace rockstar

#endif // ROCKSTAR_DSP_CABSIM_H
