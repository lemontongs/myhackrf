
#include "HackRFDevice.h"
#include "zhelpers.h"

#include <ctime>
#include <fftw3.h>
#include <iostream>
#include <math.h>
#include <time.h>
#include <vector>

struct timespec program_start;

bool isRunning = true;
bool led_is_on = false;

double get_duration()
{
    struct timespec now;
    clock_gettime( CLOCK_REALTIME, &now );
    
    return double(now.tv_sec - program_start.tv_sec) + ( double(now.tv_nsec - program_start.tv_nsec) / 1e9 );
}

//
// LED ON
//
void blink_on( zmq::socket_t & socket )
{
    if (!led_is_on)
    {
        led_is_on = true;
        s_send( socket, std::string("blink on") );
        std::cout << get_duration() << " blink" << std::endl;
    }
}

//
// LED OFF
//
void blink_off( zmq::socket_t & socket )
{
    if (led_is_on)
    {
        led_is_on = false;
        s_send( socket, std::string("blink off") );
    }
}

std::vector<uint64_t> freq_bins_mhz;
void calculateFrequencyBins(int N, double fs, uint64_t fc)
{
    // Frequency Bins
    // f = -(N/2):(N/2)-1;
    // f = f*fs/N;
    // f = f + fc;
    // f = f / 1e6;
    double fs_over_N = double(fs)/double(N);
    freq_bins_mhz.clear();
    for (int ii = (-N/2); ii < (N/2); ii++)
        freq_bins_mhz.push_back( uint64_t( double(ii) * fs_over_N ) + fc );
}

double   max_mean     = -9999;
int      num_fft_bins = 4096;
uint64_t target_lo_hz = 433920000;
uint64_t target_hi_hz = 433940000;

//
// fft
//
void fft( uint8_t * buffer, int buffer_size, zmq::socket_t & blink_interface )
{
    int num_samples = buffer_size/2;
    
    fftw_complex *in, *out;
    in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*num_samples);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*num_fft_bins);

    for (int ii = 0; ii < buffer_size; ii+=2)
    {
        in[ii/2][0] = ( double( buffer[ii+0] + uint8_t(128) ) - double(128) ) / double(128);
        in[ii/2][1] = ( double( buffer[ii+1] + uint8_t(128) ) - double(128) ) / double(128);
    }

    fftw_plan my_plan;
    my_plan = fftw_plan_dft_1d(num_fft_bins, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(my_plan);
    
    double mean = 0.0;
    int num_mean = 0;
    calculateFrequencyBins( num_fft_bins, 1000000, 433900000 );
    for (int ii = 0; ii < num_fft_bins; ii++)
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
        double val = abs(out[idx][0]);
        if (0 != val)
            val = 20*log10(val);

        if ( freq_bins_mhz[ii] > target_lo_hz && freq_bins_mhz[ii] < target_hi_hz )
        {
            mean += out[idx][0];
            num_mean++;
            //std::cout << ii << " " << freq_bins_mhz[ii] << " " << out[idx][0] << " " << mean << std::endl;
        }
    }
    
    mean = mean / double(num_mean);
    
    if (mean > 15.0)
        blink_on( blink_interface );
    else
        blink_off( blink_interface );
    
    
    if (mean > max_mean)
        max_mean = mean;
    
    fftw_destroy_plan(my_plan);
    fftw_free(in);
    fftw_free(out);
}


//
// MAIN
//
int main()
{
    clock_gettime(CLOCK_REALTIME, &program_start);
    
    // Start the blink server interface
    zmq::context_t blink_context(1);
    zmq::socket_t blink_interface(blink_context, ZMQ_PUB);
    blink_interface.connect("tcp://localhost:5558");
    
    blink_on( blink_interface );
    sleep(1);
    blink_off( blink_interface );
    sleep(1);
    
    // Setup the data receiver
    zmq::context_t recv_context(1);
    zmq::socket_t data_interface(recv_context, ZMQ_SUB);
    data_interface.connect("tcp://localhost:5555");
    data_interface.setsockopt(ZMQ_SUBSCRIBE, "", 0);
    int timeout = 100; // ms
    data_interface.setsockopt(ZMQ_RCVTIMEO, &timeout, sizeof(timeout));

    while (isRunning)
    {
        zmq::message_t msg;
        if ( data_interface.recv(&msg) )
        {
            uint8_t* data = static_cast<uint8_t*>( msg.data() );
            fft( data, msg.size(), blink_interface );
        }
    }

    return 0;
}





