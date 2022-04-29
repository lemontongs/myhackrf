
#include "pulse_detector.h"

#include <iostream>
#include <iomanip> 
#include <random>


void add_pulse(SampleChunk &samples, double toa, double pw, double amp, double fs)
{
    uint64_t start_sample = uint64_t( toa * fs );
    uint64_t stop_sample = uint64_t( (toa + pw) * fs );

    if ( samples.size() < stop_sample )
        samples.resize(stop_sample);

    for (uint64_t x = start_sample; x < stop_sample; x++)
        samples[x] = std::complex<double>(0,amp);
}


void make_noise(SampleChunk &samples, double duration, double noise_std_dev, double fs)
{
    std::default_random_engine generator;
    std::normal_distribution<double> distribution(0.0, noise_std_dev);
    
    uint64_t num_samples = uint64_t( duration * fs );

    samples.resize(num_samples);
    for (uint64_t x = 0; x < num_samples; x++)
        samples[x] = std::complex<double>(distribution(generator), distribution(generator));
}


int main()
{
    PDW_Packet result;
    SampleChunk samples;
    const double fs = 1e6;

    make_noise(samples, 0.003, 0.01, fs);
    
    add_pulse(samples, 0.001, 0.000100, 0.5, fs); // 100us pulse
    
    add_pulse(samples, 0.0012, 0.000050, 0.2, fs); // 10us pulse
    add_pulse(samples, 0.0013, 0.000050, 0.2, fs); // 10us pulse
    add_pulse(samples, 0.0014, 0.000050, 0.2, fs); // 10us pulse
    add_pulse(samples, 0.0015, 0.000050, 0.2, fs); // 10us pulse
    
    std::cout << "Waveform size: " << samples.size() << " samples" << std::endl;

    utilities::detectPulses(&result, &samples, 0, fs, 5.0);

    std::cout << std::setw(10) << "TOA" 
              << std::setw(10) << "PW" 
              << std::setw(10) << "MEAN" 
              << std::setw(10) << "PEAK" 
              << std::setw(10) << "NOISE"
              << std::setw(15) << "FREQ"
              << std::endl;

    for (int x = 0; x < result.pdws_size(); x++)
    {
        std::cout 
          << std::setw(10) << result.pdws(x).toa_s() 
          << std::setw(10) << result.pdws(x).pw_s() 
          << std::setw(10) << result.pdws(x).mean_amp_db() 
          << std::setw(10) << result.pdws(x).peak_amp_db() 
          << std::setw(10) << result.pdws(x).noise_amp_db() 
          << std::setw(15) << result.pdws(x).freq_offset_hz() 
          << std::endl;
    }
    
    return 0;
}

