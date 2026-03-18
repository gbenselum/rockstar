#ifndef ROCKSTAR_HOST_JACK_HOST_H
#define ROCKSTAR_HOST_JACK_HOST_H

#include <jack/jack.h>
#include <string>
#include <vector>
#include <functional>

namespace rockstar {
namespace host {

class JackHost {
public:
    JackHost(const std::string& client_name = "rockstar");
    ~JackHost();

    // Start processing audio
    bool start();
    
    // Stop processing
    void stop();

    // Set a callback for DSP processing block
    // Signature: void process(float** in_buffers, float** out_buffers, int num_frames)
    using ProcessCallback = std::function<void(float**, float**, uint32_t)>;
    void set_process_callback(ProcessCallback cb) { process_cb_ = cb; }

    int get_sample_rate() const { return sample_rate_; }
    int get_buffer_size() const { return buffer_size_; }

private:
    static int jack_process_callback(jack_nframes_t nframes, void* arg);
    static void jack_shutdown_callback(void* arg);
    static int jack_sample_rate_callback(jack_nframes_t nframes, void* arg);
    static int jack_buffer_size_callback(jack_nframes_t nframes, void* arg);

    int process(jack_nframes_t nframes);

    std::string client_name_;
    jack_client_t* client_ = nullptr;
    
    std::vector<jack_port_t*> input_ports_;
    std::vector<jack_port_t*> output_ports_;
    
    int sample_rate_ = 48000;
    int buffer_size_ = 256;
    
    ProcessCallback process_cb_ = nullptr;
};

} // namespace host
} // namespace rockstar

#endif // ROCKSTAR_HOST_JACK_HOST_H
