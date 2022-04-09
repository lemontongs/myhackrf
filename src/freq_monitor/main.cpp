
#include "packet.pb.h"
#include "SDRReceiver.h"

#include <unistd.h>
#include <stdint.h>
#include <algorithm>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <math.h>
#include <time.h>
#include <vector>

struct timespec program_start;

bool isRunning = true;
bool led_is_on = false;
uint64_t fc = 0;
double fs = 0.0;
double thresh = 0.0;
std::string host = "";
uint32_t print_count = 0;

//
// Returns time (seconds) since program start
//
double get_duration()
{
    struct timespec now;
    clock_gettime( CLOCK_REALTIME, &now );

    return double(now.tv_sec - program_start.tv_sec) + ( double(now.tv_nsec - program_start.tv_nsec) / 1e9 );
}

//
// LED ON
//
void blink_on( zmq::socket_t* socket )
{
    if (!led_is_on)
    {
        led_is_on = true;
        s_send( *socket, std::string("blink on") );
    }
}

//
// LED OFF
//
void blink_off( zmq::socket_t* socket )
{
    if (led_is_on)
    {
        led_is_on = false;
        s_send( *socket, std::string("blink off") );
    }
}

std::vector< std::pair<uint64_t,uint64_t> > monitor_ranges;

//
// fft
//
void process_data( const FFT_Packet &p, zmq::socket_t* blink_interface, SDRReceiver* receiver )
{
    double peak = -9999.0;
    double fft_sum = 0.0;
    //
    // Find the peak in the target frequency range
    //
    for (std::size_t mm = 0; mm < monitor_ranges.size(); mm++)
    {
        for (int ii = 0; ii < p.fft_size(); ii++)
        {
            if ( ( p.freq_bins_hz(ii) > monitor_ranges[mm].first ) && \
                 ( p.freq_bins_hz(ii) < monitor_ranges[mm].second ) && \
                 ( p.fft(ii) > peak ) )
                    peak = p.fft(ii);
            
            fft_sum += p.fft(ii);
        }
    }

    // threshold is some constant over the mean
    double fft_mean_db = (fft_sum / p.fft_size());
    double threshold = fft_mean_db + thresh;
    double signal_to_noise = peak - threshold;

    // Blink the LED (and print sone info) if we have detected a signal
    if ( signal_to_noise > 0.0 )
        blink_on( blink_interface );
    else
        blink_off( blink_interface );

    if ( signal_to_noise > 0.0 )
    {
        if ( print_count == 0 )
            std::cout << " Time                     Mean  Thresh  Peak  SNR" << std::endl;
        std::cout << std::left << std::setw(8) << get_duration();
        for (std::size_t mm = 0; mm < monitor_ranges.size(); mm++)
        {
            std::cout << "  ";
            for (int ii = 0; ii < p.fft_size(); ii++)
            {
                if ( p.freq_bins_hz(ii) > monitor_ranges[mm].first && p.freq_bins_hz(ii) < monitor_ranges[mm].second )
                {
                    std::cout << "\e[48;5;" << std::max(232, std::min(int(232 + int(p.fft(ii)-threshold)),255)) << "m \e[0m";
                }
            }
        }
        std::cout << " " << std::left << std::setw(8) << std::setprecision(4) << fft_mean_db
                  << " " << std::left << std::setw(8) << std::setprecision(4) << threshold
                  << " " << std::left << std::setw(8) << std::setprecision(4) << peak
                  << " " << std::left << std::setw(8) << std::setprecision(4) << signal_to_noise
                  << std::endl;
        print_count = ( print_count + 1 ) % 20;
    }
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
            std::cout << "Usage: " << argv[0] << " -c <center frequency> -s <sample rate> -r <low frequency> <high frequency>" << std::endl;
            std::cout << "    -i <host>                   hostname of the SDR server" << std::endl;
            std::cout << "    -t <threshold>              amplitude threshold for detection reporting" << std::endl;
            std::cout << "    -c <center frequency hz>    center frequencie to tune to" << std::endl;
            std::cout << "    -s <sample rate hz>         device sample rate to use" << std::endl;
            std::cout << "    -r <low freq> <high freq>   low and high are frequency (Hz) ranges to monitor (can be many -r specifications" << std::endl << std::endl;
        }
        else if ( arg == "-i" )
        {
            if (aa + 1 >= argc)
            {
                std::cout << "Missing host!" << std::endl;
                exit(1);
            }
            
            // host argument
            std::istringstream ss(argv[aa+1]);
            if (!(ss >> host))
            {
                std::cout << "ERROR: Invalid host argument! '" << argv[aa+1] << "'" << std::endl;
                exit(1);
            }
            
            aa += 1;
        }
        else if ( arg == "-t" )
        {
            if (aa + 1 >= argc)
            {
                std::cout << "Missing threshold!" << std::endl;
                exit(1);
            }
            
            // threshold argument
            std::istringstream ss(argv[aa+1]);
            if (!(ss >> thresh))
            {
                std::cout << "ERROR: Invalid threshold argument! '" << argv[aa+1] << "'" << std::endl;
                exit(1);
            }
            
            aa += 1;
        }
        else if ( arg == "-c" )
        {
            if (aa + 1 >= argc)
            {
                std::cout << "Missing center frequency!" << std::endl;
                exit(1);
            }
            
            // fc argument
            std::istringstream ss(argv[aa+1]);
            if (!(ss >> fc))
            {
                std::cout << "ERROR: Invalid frequency argument! '" << argv[aa+1] << "'" << std::endl;
                exit(1);
            }
            
            aa += 1;
        }
        else if ( arg == "-s" )
        {
            if (aa + 1 >= argc)
            {
                std::cout << "Missing sample rate!" << std::endl;
                exit(1);
            }
            
            // fc argument
            std::istringstream ss(argv[aa+1]);
            if (!(ss >> fs))
            {
                std::cout << "ERROR: Invalid sample rate argument! '" << argv[aa+1] << "'" << std::endl;
                exit(1);
            }
            
            aa += 1;
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
    
    if ( fc == 0 )
    {
        std::cout << "ERROR: no center frequency specified!" << std::endl;
        exit(2);
    }
    
    if ( fs == 0.0 )
    {
        std::cout << "ERROR: no sample rate specified!" << std::endl;
        exit(2);
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
void receive_callback( Packet &p, void* args )
{
    std::pair< zmq::socket_t*, SDRReceiver*>* p_args = (std::pair< zmq::socket_t*, SDRReceiver*>*)args;
    
    if ( p.has_fft_packet() )
    {
        process_data( p.fft_packet(), p_args->first, p_args->second );
    }
    else
    {
        std::cout << "WARNING! Packet has no FFT data." << std::endl;
    }
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
    
    blink_on( &blink_interface );
    sleep(1);
    blink_off( &blink_interface );
    
    // Setup the data receiver
    SDRReceiver rcv;
    rcv.setIP(host);
    std::pair< zmq::socket_t*, SDRReceiver*> args = std::make_pair( &blink_interface, &rcv );
    rcv.initialize( receive_callback, (void*)&args );
    rcv.tune( fc );
    rcv.setSampleRate( fs );
    
    while(1)
        sleep(1);
    
    return 0;
}





