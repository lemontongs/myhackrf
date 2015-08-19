
#ifndef FFT_H
#define FFT_H

#include "packet.pb.h"

#include <fftw3.h>
#include <math.h>
#include <stdint.h>
#include <vector>
#include <iostream>

namespace utilities
{
    //
    // Calculate Frequency Bins
    //
    // f = -(N/2):(N/2)-1; // matlab code
    // f = f*fs/N;
    // f = f + fc;
    // f = f / 1e6;
    //
    void calcFrequencyBins(Packet &packet, int N, double fs, uint64_t fc);
    
    //
    // fft
    //
    Packet fft( uint8_t * buffer, int buffer_size, int num_fft_bins, double fs, uint64_t fc );
}

#endif /* FFT_H */

