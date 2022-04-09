
#include "fft.h"


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
    void calcFrequencyBins(FFT_Packet* packet, uint32_t N, double fs, uint64_t fc)
    {
        double fs_over_N = double(fs)/double(N);
        packet->clear_freq_bins_hz();
        for (int ii = (-1.0 * double(N)/2.0); ii < (double(N)/2.0); ii++)
            packet->add_freq_bins_hz( int64_t( double(ii) * fs_over_N ) + fc );
    }

    inline double hamming_window( uint32_t N, double n )
    {
        return ( 0.54 - ( 0.46 * cos( 2.0 * M_PI * ( n / N ) ) ) );
    }

    //
    // fft
    //
    void fft( FFT_Packet* result, SampleChunk &buffer, uint32_t num_fft_bins, double fs, uint64_t fc )
    {
        uint32_t num_samples( buffer.size() );
        calcFrequencyBins( result, num_fft_bins, fs, fc );
        
        fftw_complex *in, *out;
        in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*num_samples);
        out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*num_fft_bins);

        for (std::size_t ii = 0; ii < buffer.size(); ii++)
        {
            in[ii][0] = buffer[ii].real() * hamming_window( num_samples, ii );
            in[ii][1] = buffer[ii].imag() * hamming_window( num_samples, ii );
        }
        
        fftw_plan my_plan;
        my_plan = fftw_plan_dft_1d(int(num_fft_bins), in, out, FFTW_FORWARD, FFTW_ESTIMATE);
        fftw_execute(my_plan);
        
        result->clear_fft();
        
        for (uint32_t ii = 0; ii < num_fft_bins; ii++)
        {
            //
            // fftshift
            //
            int idx = ii+(num_fft_bins/2);
            if (ii >= (num_fft_bins/2))
                idx = ii - (num_fft_bins/2);
            
            //
            // 20*log10(val)
            //
            double val = fabs(out[idx][0]);
            if ( 0.0 != val )
                val = 20.0 * log10(val);
            
            // save the bin
            result->add_fft( val );
            
            //
            // Find the average value across the FFT
            // This should give us the center of the noise
            // (assuming the noise dominates the bandwidth)
            //
            // result.set_mean_db( result.mean_db() + val );
            
            // if ( val > result.peak_db() )
            // {
            //     result.set_peak_bin_index( ii );
            //     result.set_peak_db( val );
            // }
            
            // if ( val < result.low_db() )
            //     result.set_low_db( val );
        }
        
        // noise mean for this FFT
        // result.set_mean_db( result.mean_db() / num_fft_bins );
        
        fftw_destroy_plan(my_plan);
        fftw_free(in);
        fftw_free(out);
    }
}

