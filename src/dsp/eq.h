#ifndef ROCKSTAR_DSP_EQ_H
#define ROCKSTAR_DSP_EQ_H

#include <cmath>
#include <vector>

namespace rockstar {
namespace dsp {

// Standard Biquad Filter implementation
class Biquad {
public:
  enum Type { LOW_SHELF, PEAKING, HIGH_SHELF };

  Biquad(Type type, float sample_rate)
      : type_(type), sample_rate_(sample_rate) {
    reset();
  }

  void reset() { x1 = x2 = y1 = y2 = 0.0f; }

  void set_params(float freq, float q, float gain_db) {
    float A = pow(10.0f, gain_db / 40.0f);
    float w0 = 2.0f * M_PI * freq / sample_rate_;
    float alpha = sin(w0) / (2.0f * q);
    float cos_w0 = cos(w0);

    float a0 = 1.0f;

    switch (type_) {
    case PEAKING:
      b0 = 1.0f + alpha * A;
      b1 = -2.0f * cos_w0;
      b2 = 1.0f - alpha * A;
      a0 = 1.0f + alpha / A;
      a1 = -2.0f * cos_w0;
      a2 = 1.0f - alpha / A;
      break;
    case LOW_SHELF:
      b0 = A * ((A + 1.0f) - (A - 1.0f) * cos_w0 + 2.0f * sqrt(A) * alpha);
      b1 = 2.0f * A * ((A - 1.0f) - (A + 1.0f) * cos_w0);
      b2 = A * ((A + 1.0f) - (A - 1.0f) * cos_w0 - 2.0f * sqrt(A) * alpha);
      a0 = (A + 1.0f) + (A - 1.0f) * cos_w0 + 2.0f * sqrt(A) * alpha;
      a1 = -2.0f * ((A - 1.0f) + (A + 1.0f) * cos_w0);
      a2 = (A + 1.0f) + (A - 1.0f) * cos_w0 - 2.0f * sqrt(A) * alpha;
      break;
    case HIGH_SHELF:
      b0 = A * ((A + 1.0f) + (A - 1.0f) * cos_w0 + 2.0f * sqrt(A) * alpha);
      b1 = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * cos_w0);
      b2 = A * ((A + 1.0f) + (A - 1.0f) * cos_w0 - 2.0f * sqrt(A) * alpha);
      a0 = (A + 1.0f) - (A - 1.0f) * cos_w0 + 2.0f * sqrt(A) * alpha;
      a1 = 2.0f * ((A - 1.0f) - (A + 1.0f) * cos_w0);
      a2 = (A + 1.0f) - (A - 1.0f) * cos_w0 - 2.0f * sqrt(A) * alpha;
      break;
    }

    // Normalize
    b0 /= a0;
    b1 /= a0;
    b2 /= a0;
    a1 /= a0;
    a2 /= a0;
  }

  float process(float in) {
    float out = b0 * in + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
    x2 = x1;
    x1 = in;
    y2 = y1;
    y1 = out;
    return out;
  }

private:
  Type type_;
  float sample_rate_;
  float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
  float x1 = 0, x2 = 0, y1 = 0, y2 = 0;
};

// 4-Band parametric EQ
class FourBandEQ {
public:
  FourBandEQ()
      : low(Biquad::LOW_SHELF, 48000), low_mid(Biquad::PEAKING, 48000),
        high_mid(Biquad::PEAKING, 48000), high(Biquad::HIGH_SHELF, 48000) {
    update_params();
  }

  void set_sample_rate(float sample_rate) {
    low = Biquad(Biquad::LOW_SHELF, sample_rate);
    low_mid = Biquad(Biquad::PEAKING, sample_rate);
    high_mid = Biquad(Biquad::PEAKING, sample_rate);
    high = Biquad(Biquad::HIGH_SHELF, sample_rate);
    update_params();
  }

  // Parameters
  float low_gain = 0.0f; // dB

  float low_mid_freq = 500.0f; // Hz
  float low_mid_gain = 0.0f;   // dB
  float low_mid_q = 0.707f;

  float high_mid_freq = 2000.0f; // Hz
  float high_mid_gain = 0.0f;    // dB
  float high_mid_q = 0.707f;

  float high_gain = 0.0f; // dB

  void update_params() {
    low.set_params(100.0f, 0.707f, low_gain);
    low_mid.set_params(low_mid_freq, low_mid_q, low_mid_gain);
    high_mid.set_params(high_mid_freq, high_mid_q, high_mid_gain);
    high.set_params(5000.0f, 0.707f, high_gain);
  }

  void process(float *buffer, int num_frames) {
    for (int i = 0; i < num_frames; ++i) {
      float s = buffer[i];
      s = low.process(s);
      s = low_mid.process(s);
      s = high_mid.process(s);
      s = high.process(s);
      buffer[i] = s;
    }
  }

private:
  Biquad low, low_mid, high_mid, high;
};

} // namespace dsp
} // namespace rockstar

#endif // ROCKSTAR_DSP_EQ_H
