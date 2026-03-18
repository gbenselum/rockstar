#include "cabsim.h"
#include <math.h>
#include <string.h>

namespace rockstar {
namespace dsp {

CabSim::CabSim(float target_sample_rate)
    : target_sample_rate_(target_sample_rate) {
  outComplex =
      (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * MAX_FFT_SIZE);
  IRout = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * MAX_FFT_SIZE);
  convolved =
      (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * MAX_FFT_SIZE);
  outbuf = new float[MAX_FFT_SIZE]();
  inbuf = new float[MAX_FFT_SIZE]();
  IR = new float[MAX_FFT_SIZE]();
  tmp_mono_out_ = new float[MAX_FFT_SIZE]();

  fft = fftwf_plan_dft_r2c_1d(MAX_FFT_SIZE, inbuf, outComplex, FFTW_ESTIMATE);
  IRfft = fftwf_plan_dft_r2c_1d(MAX_FFT_SIZE, IR, IRout, FFTW_ESTIMATE);
  ifft = fftwf_plan_dft_c2r_1d(MAX_FFT_SIZE, convolved, outbuf, FFTW_ESTIMATE);
}

CabSim::~CabSim() {
  fftwf_destroy_plan(fft);
  fftwf_destroy_plan(IRfft);
  fftwf_destroy_plan(ifft);
  fftwf_free(outComplex);
  fftwf_free(IRout);
  fftwf_free(convolved);
  delete[] outbuf;
  delete[] inbuf;
  delete[] IR;
  delete[] tmp_mono_out_;
  if (ir_data_)
    delete[] ir_data_;
}

float CabSim::resample_linear(const float *in, float *out, int in_rate,
                              int out_rate, int in_frames) {
  if (!in || !out)
    return 0;
  double stepDist = ((double)in_rate / (double)out_rate);
  const uint64_t fixedFraction = (1LL << 32);
  const double normFixed = (1.0 / (1LL << 32));
  uint64_t step = ((uint64_t)(stepDist * fixedFraction + 0.5));
  uint64_t curOffset = 0;

  uint64_t out_frames = in_frames * out_rate / in_rate;
  const float *in_ptr = in;
  float *out_ptr = out;

  for (uint32_t i = 0; i < out_frames; i++) {
    *out_ptr++ = (float)(in_ptr[0] +
                         (in_ptr[1] - in_ptr[0]) *
                             ((double)(curOffset >> 32) +
                              ((curOffset & (fixedFraction - 1)) * normFixed)));
    curOffset += step;
    in_ptr += (curOffset >> 32);
    curOffset &= (fixedFraction - 1);
  }
  return out_frames;
}

bool CabSim::load_ir(const std::string &filepath) {
  std::lock_guard<std::mutex> lock(process_mutex_);

  SF_INFO info;
  info.format = 0;
  SNDFILE *sndfile = sf_open(filepath.c_str(), SFM_READ, &info);
  if (!sndfile || info.frames == 0) {
    std::cerr << "Failed to load IR: " << filepath << std::endl;
    return false;
  }

  float *data = new float[info.frames * info.channels];
  sf_readf_float(sndfile, data, info.frames);
  sf_close(sndfile);

  // Convert to Mono
  if (info.channels > 1) {
    float *mono = new float[info.frames];
    for (int i = 0; i < info.frames; ++i) {
      mono[i] = data[i * info.channels]; // just take first channel
    }
    delete[] data;
    data = mono;
  }

  // Resample if needed
  if (info.samplerate != (int)target_sample_rate_) {
    int out_frames = info.frames * target_sample_rate_ / info.samplerate;
    float *resampled = new float[out_frames];
    resample_linear(data, resampled, info.samplerate, target_sample_rate_,
                    info.frames);
    delete[] data;
    data = resampled;
    info.frames = out_frames;
  }

  if (ir_data_)
    delete[] ir_data_;
  ir_data_ = data;
  ir_frames_ = info.frames;

  // Prepare IR fft
  int valid_len = (MAX_FFT_SIZE < ir_frames_) ? MAX_FFT_SIZE : ir_frames_;
  memcpy(IR, ir_data_, valid_len * sizeof(float));
  memset(IR + valid_len, 0, (MAX_FFT_SIZE - valid_len) * sizeof(float));

  fftwf_execute(IRfft);
  ir_loaded_ = true;

  // reset overlap
  overlap_buffer_.clear();
  memset(outbuf, 0, MAX_FFT_SIZE * sizeof(float));
  memset(inbuf, 0, MAX_FFT_SIZE * sizeof(float));

  std::cout << "Loaded IR: " << filepath << " (" << valid_len
            << " samples context)" << std::endl;
  return true;
}

void CabSim::process(const float *in, float *out, int num_frames) {
  std::lock_guard<std::mutex> lock(process_mutex_);

  if (!ir_loaded_ || num_frames > MAX_FFT_SIZE) {
    memset(out, 0, num_frames * sizeof(float));
    return;
  }

  if (num_frames != prev_buffer_size_) {
    overlap_add_buffers_ = MAX_FFT_SIZE / num_frames;
    if (overlap_add_buffers_ < 2)
      overlap_add_buffers_ = 2;
    overlap_buffer_.init(overlap_add_buffers_ * MAX_FFT_SIZE + num_frames);
    prev_buffer_size_ = num_frames;
  }

  for (int i = 0; i < num_frames; i++) {
    inbuf[i] = in[i]; // apply no attenuation here, do it at mixer
  }
  memset(inbuf + num_frames, 0, (MAX_FFT_SIZE - num_frames) * sizeof(float));

  fftwf_execute(fft);

  for (int m = 0; m < MAX_FFT_SIZE; m++) {
    convolved[m][0] =
        outComplex[m][0] * IRout[m][0] - outComplex[m][1] * IRout[m][1];
    convolved[m][1] =
        outComplex[m][0] * IRout[m][1] + outComplex[m][1] * IRout[m][0];
  }

  fftwf_execute(ifft);

  for (int j = 0; j < MAX_FFT_SIZE; j++) {
    overlap_buffer_.push(outbuf[j] / MAX_FFT_SIZE);
  }

  for (int j = 0; j < num_frames; j++) {
    float overlap_value = 0.0f;
    for (int Oa = 0; Oa < overlap_add_buffers_ - 1; Oa++) {
      overlap_value += overlap_buffer_.get_relative(
          (Oa * MAX_FFT_SIZE) + ((overlap_add_buffers_ - Oa - 1) * num_frames) +
          j + 1);
    }
    out[j] = ((outbuf[j] / MAX_FFT_SIZE) + overlap_value);
  }
}

void CabSim::get_panned_output(float *out_l, float *out_r, int num_frames) {
  process(tmp_mono_out_, tmp_mono_out_,
          num_frames); // this assumes tmp_mono_out_ has been filled by caller

  // Constant power panning
  // pan is -1 (left) to 1 (right). x = (pan + 1) / 2 => 0 to 1
  float x = (pan_ + 1.0f) * 0.5f;
  float gain_l = cosf(x * M_PI * 0.5f) * volume_;
  float gain_r = sinf(x * M_PI * 0.5f) * volume_;

  for (int i = 0; i < num_frames; ++i) {
    out_l[i] += tmp_mono_out_[i] * gain_l;
    out_r[i] += tmp_mono_out_[i] * gain_r;
  }
}

// ========================

MultiCabSimMixer::MultiCabSimMixer(int num_cabs, float sample_rate) {
  for (int i = 0; i < num_cabs; ++i) {
    cabs_.push_back(new CabSim(sample_rate));
  }
}

MultiCabSimMixer::~MultiCabSimMixer() {
  for (auto c : cabs_)
    delete c;
}

void MultiCabSimMixer::process(const float *in, float *out_l, float *out_r,
                               int num_frames) {
  // Clear outputs
  for (int i = 0; i < num_frames; ++i) {
    out_l[i] = 0.0f;
    out_r[i] = 0.0f;
  }

  for (auto c : cabs_) {
    // We use a hacky way to feed the same input to each CabSim and get panned
    // output mixed Wait, CabSim::get_panned_output processes `tmp_mono_out_`
    // but it needs `in`. Let's refactor this slightly.
    float *tmp = new float[num_frames];
    c->process(in, tmp, num_frames);

    float x = (c->get_pan() + 1.0f) * 0.5f;
    float vol = c->get_volume();
    float gain_l = cosf(x * M_PI * 0.5f) * vol;
    float gain_r = sinf(x * M_PI * 0.5f) * vol;

    for (int i = 0; i < num_frames; ++i) {
      out_l[i] += tmp[i] * gain_l;
      out_r[i] += tmp[i] * gain_r;
    }
    delete[] tmp;
  }
}

} // namespace dsp
} // namespace rockstar
