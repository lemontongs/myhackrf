
#include "SDRReceiver.h"

#include <stdint.h>
#include <algorithm>
#include <ctime>
#include <fftw3.h>
#include <iostream>
#include <iomanip>
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

double max_mean     = -9999;
int    num_fft_bins = 4096/2;
std::vector<double> history;
std::vector< std::pair<uint64_t,uint64_t> > monitor_ranges;

//
// fft
//
void fft( uint8_t * buffer, int buffer_size, zmq::socket_t* blink_interface, SDRReceiver* receiver )
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
    double peak = -9999.0;
    int num_mean = 0;
    std::vector<double> spectra;
    
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
        
        // save the bin
        spectra.push_back(val);
        
        //
        // Find the average value across the FFT (this should give us the noise floor)
        //
        mean += val;
        
        //
        // Find the peak in the target frequency range
        //
        for (int mm = 0; mm < monitor_ranges.size(); mm++)
        {
            if ( freq_bins_mhz[ii] > monitor_ranges[mm].first && freq_bins_mhz[ii] < monitor_ranges[mm].second )
            {
                if ( val > peak )
                    peak = val;
            }
        }
    }
    
    // noise floor for this FFT
    mean = mean / double(num_fft_bins);
    
    // Add to the noise floor history
    if (history.size() < 10)
        history.push_back(mean);
    else
    {
        history.erase( history.begin() );
        history.push_back( mean );
    }
    
    // noise floor for the previous 10 FFTs
    double history_average = 0.0;
    for ( int ii = 0; ii < history.size(); ii++ )
    {
        history_average += history[ii];
    }
    history_average = history_average / double( history.size() );
    
    // threshold is some constant over the history
    double signal_to_noise = peak - history_average;
    
    std::cout << std::left << std::setw(8) << get_duration();
    for (int mm = 0; mm < monitor_ranges.size(); mm++)
    {
        std::cout << "  ";
        for (int ii = 0; ii < spectra.size(); ii++)
        {
            if ( freq_bins_mhz[ii] > monitor_ranges[mm].first && freq_bins_mhz[ii] < monitor_ranges[mm].second )
            {
                std::cout << "\e[48;5;" << std::max(232, std::min(int(232 + int(spectra[ii]-history_average)),255)) << "m \e[0m";
            }
        }
    }
    std::cout << " " << signal_to_noise << std::endl;
    
    if ( signal_to_noise > 150.0 )
        blink_on( *blink_interface );
    else
        blink_off( *blink_interface );
    
    
    if (mean > max_mean)
        max_mean = mean;
    
    fftw_destroy_plan(my_plan);
    fftw_free(in);
    fftw_free(out);
}

//
// Parse arguments
//
void parse_args(int argc, char* argv[])
{
    for (int aa = 0; aa < argc; aa++)
    {
        std::string arg( argv[aa] );
        
        if ( arg == "-h" || arg == "--help")
        {
            std::cout << "Usage: " << argv[0] << " [ -f <filename> ] [ -r <low frequency> <high frequency> ]" << std::endl;
            std::cout << "    -f <filename>               filename containing comma separated list of high and low frequencies" << std::endl;
            std::cout << "    -r <low freq> <high freq>   low and high are frequency (Hz) ranges to monitor (can be many -r specifications" << std::endl << std::endl;
        }
        else if ( arg == "-f" )
        {
            if (aa + 1 >= argc)
            {
                std::cout << "Missing filename!" << std::endl;
                exit(1);
            }
            
            aa += 1;
            
            std::cout << "Not implemented yet :(" << std::endl;
            exit(1);
            
            //TODO: parse file
        }
        else if ( arg == "-r" )
        {
            if (aa + 2 >= argc)
            {
                std::cout << "Missing frequency argument!" << std::endl;
                exit(1);
            }
            
            // low argument
            std::istringstream ss_lo(argv[aa+1]);
            uint64_t freq_lo;
            if (!(ss_lo >> freq_lo))
            {
                std::cout << "ERROR: Invalid frequency argument! '" << argv[aa+1] << "'" << std::endl;
                exit(1);
            }
            
            // high argument
            std::istringstream ss_hi(argv[aa+2]);
            uint64_t freq_hi;
            if (!(ss_hi >> freq_hi))
            {
                std::cout << "ERROR: Invalid frequency argument! '" << argv[aa+2] << "'" << std::endl;
                exit(1);
            }
            
            aa += 2;
            
            monitor_ranges.push_back( std::make_pair( freq_lo, freq_hi ) );
        }
    }
    
    if ( monitor_ranges.size() == 0 )
    {
        std::cout << "ERROR: no ranges specified!" << std::endl;
        exit(2);
    }
    
    std::cout << monitor_ranges.size() << " ranges specified!" << std::endl;
}

//
// receiver callback
//
void receive_callback( uint8_t* buf, uint32_t len, void* args )
{
    std::pair< zmq::socket_t*, SDRReceiver*>* p_args = (std::pair< zmq::socket_t*, SDRReceiver*>*)args;
    
    fft( buf, len, p_args->first, p_args->second );
}

//
// MAIN
//
int main(int argc, char* argv[])
{
    parse_args(argc, argv);
    
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
    SDRReceiver rcv;
    std::pair< zmq::socket_t*, SDRReceiver*> args = std::make_pair( &blink_interface, &rcv );
    rcv.initialize( receive_callback, (void*)&args );
    rcv.tune( monitor_ranges[0].first ); //TODO: do something interesting with the center freq
    
    calculateFrequencyBins( num_fft_bins, 
                            rcv.getSampleRate(), 
                            rcv.getCenterFrequency() );
    
    while(1)
        sleep(1);
    
    return 0;
}





