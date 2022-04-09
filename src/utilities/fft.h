
#ifndef FFT_H
#define FFT_H

#include "RFDevice.h"
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
    void calcFrequencyBins(FFT_Packet* packet, uint32_t N, double fs, uint64_t fc);
    
    //
    // fft
    //
    void fft( FFT_Packet* result, SampleChunk &buffer, uint32_t num_fft_bins, double fs, uint64_t fc );
}

#endif /* FFT_H */

