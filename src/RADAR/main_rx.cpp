
#include "HackRFDevice.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <iostream>
#include <vector>

#define PI 3.141592653589793

HackRFDevice hackrf;

uint64_t fc_hz      = 2500e6; // center freq
double   fs_hz      = 20e6;   // sample rate
uint32_t lna_gain   = 32;     // 0-40 in steps of 8
uint8_t  amp_enable = 0;
uint32_t txvga_gain = 0;


void signal_handler(int s)
{
    printf("Caught signal %d\n",s);
    hackrf.cleanup();
    exit(0);
}


//
// Callback function for tx samples
//
//262144
double t = 0.0;
double dt = 1/fs_hz;

//int sample_block_cb_fn(hackrf_transfer* transfer)
int device_sample_block_cb(SampleChunk* samples, void* args)
{
    std::cout << "P " << std::flush;
    
    double max_amp = -9999999.0;
    std::size_t ii = 0;
    double i,q;
    
    while (ii < samples->size())
    {
        i = (*samples)[ii].real();
        q = (*samples)[ii].imag();
        ii += 1;
        
        double amp = 20.0 * log10( sqrt( pow(i,2) + pow(q,2) ) );
        
        if ( amp > max_amp )
            max_amp = amp;
    }
    
    std::cout << i << "  " << max_amp << std::endl << std::flush;
    
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

    
    if ( ! hackrf.initialize( "22be1" ) )
    {
        std::cout << "Error initializing hackrf device!" << std::endl;
        return 1;
    }
    
    hackrf.set_center_freq( fc_hz );
    hackrf.set_sample_rate( fs_hz );
    hackrf.set_lna_gain( lna_gain );
    hackrf.set_amp_enable( amp_enable );
    hackrf.set_txvga_gain( txvga_gain );
    hackrf.start_Rx( device_sample_block_cb, (void*)(NULL) );

    //  Wait a while
    sleep(300);
    
    return 0;
}





