
#ifndef WAVEFORMS_H
#define WAVEFORMS_H

#include <deque>
#include <complex>

#define PI 3.141592653589793

//#define SAVE_FILE

void createTone(std::deque<std::complex<double>> &out_sig, double fs, double freq_offset, double pw_s, double mag)
{
    uint32_t num_samples = uint32_t(fs * pw_s);
    out_sig.resize(num_samples);
    
    std::cout << "---------------" << std::endl;
    std::cout << "Making waveform" << std::endl;
    std::cout << "fs:            " << uint32_t(fs) << std::endl;
    std::cout << "freq_offset:   " << freq_offset << std::endl;
    std::cout << "pw (s):        " << pw_s << std::endl;
    std::cout << "pw (samps):    " << num_samples << std::endl;
    std::cout << "mag:           " << mag << std::endl;
    
    double dt = 1.0/fs;
    for ( std::size_t ii = 0; ii < num_samples; ii++)
    {
        double i = mag * cos( 2.0 * PI * freq_offset * (ii*dt) );  // I
        double q = mag * sin( 2.0 * PI * freq_offset * (ii*dt) );  // Q

        out_sig[ii] = std::complex<double>(i,q);
    }

    std::cout << "---------------" << std::endl;
}


void createChirp(std::deque<std::complex<double>> &out_sig, double fs, double start_freq_hz, double stop_freq_hz, double pw_s, double mag)
{
    uint32_t num_samples = uint32_t(fs * pw_s);
    out_sig.resize(num_samples);

    std::cout << "---------------" << std::endl;
    std::cout << "Making waveform" << std::endl;
    std::cout << "fs:            " << uint32_t(fs) << std::endl;
    std::cout << "start_freq_hz: " << start_freq_hz << std::endl;
    std::cout << "stop_freq_hz:  " << stop_freq_hz << std::endl;
    std::cout << "pw (s):        " << pw_s << std::endl;
    std::cout << "pw (samps):    " << num_samples << std::endl;
    std::cout << "mag:           " << mag << std::endl;
    
    double start_phi = 2.0 * PI * start_freq_hz;
    double stop_phi = 2.0 * PI * stop_freq_hz;
    double delta_phi = stop_phi - start_phi;
    double dt = 1.0/fs;

#ifdef SAVE_FILE
    std::ofstream ofile("tx.csv", std::ofstream::out);
    ofile << "t,i,q" << std::endl;
#endif

    for ( std::size_t ii = 0; ii < num_samples; ii++)
    {
        double t = ii*dt;
        double phi = start_phi * t + delta_phi * t * t / (2.0*pw_s);
        double i = mag * cos( phi );  // I
        double q = mag * sin( phi );  // Q

        out_sig[ii] = std::complex<double>(i,q);
        
#ifdef SAVE_FILE
        ofile << t << "," << i << "," << q << std::endl;
#endif

    }
    
    std::cout << "---------------" << std::endl;

#ifdef SAVE_FILE
    ofile.close();
#endif

}



#endif
