
#include <signal.h>
#include <iostream> // cout
#include <stdlib.h> // exit
#include <math.h>   // sin cos M_PI
#include <unistd.h> // sleep

#include "HackRFDevice.h"

uint64_t fc_hz = 462610000; // center freq
double   fs_hz = 1000000;   // sample rate

void signal_handler(int s)
{
    exit(1);
}

//
// Callback function for Tx samples
//
//262144
double t = 0.0;
double dt = 1/1000000.0;
bool print_signal = true;
double df = 1000;
int tx_cb_fn(hackrf_transfer* transfer)
{
    for (int ii = 0; ii < transfer->valid_length; ii+=2)
    {
        double i = 128.0 * cos( 2.0 * M_PI * df * t );  // I
        double q = 128.0 * sin( 2.0 * M_PI * df * t );  // Q
        
        uint8_t i8 = uint8_t(i);
        uint8_t q8 = uint8_t(q);
        
        transfer->buffer[ii+0] = i8;
        transfer->buffer[ii+1] = q8;
        
        t = t+dt;
        if ( t >= 1.0 )
        {
            t = 0.0;
        }
    }
    
    df = -df;
    
    return 0;
}

//
// Callback function for Rx samples
//
int rx_cb_fn(SampleChunk* samples, void* args)
{
    std::cout << "got a packet with " << samples->size() << " samples!" << std::endl;
    return 0;
}

//
// MAIN
//
int main()
{
    // Setup the interrupt signal handler
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = signal_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);
    
    HackRFDevice hackrf;
    
    bool rv = hackrf.initialize();
    rv = hackrf.start_Rx( rx_cb_fn, NULL );
    
    sleep(1);
    
    return 0;
}





