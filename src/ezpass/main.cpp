
#include "HackRFDevice.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <iostream>
#include <vector>

#define PI 3.14159265

// Device driver
HackRFDevice hackrf;

uint64_t fc_hz      = 915000000; // center freq
double   fs_hz      = 10000000;   // sample rate
uint32_t lna_gain   = 0;
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
double df = 700000;

bool printit = false;

int sample_block_cb_fn(hackrf_transfer* transfer)
{    
    for (int ii = 0; ii < transfer->valid_length; ii+=2)
    {
        int sample = ii / 2;
        
        if ( sample % 1100 < 300 )
        {
            double i = 128.0 * cos( 2.0 * PI * df * t );  // I
            double q = 128.0 * sin( 2.0 * PI * df * t );  // Q
            
            uint8_t i8 = uint8_t(i);
            uint8_t q8 = uint8_t(q);
            
            transfer->buffer[ii+0] = i8;
            transfer->buffer[ii+1] = q8;
            
            if (printit)
                std::cout << int(i8) << std::endl;
        }
        else
        {
            transfer->buffer[ii+0] = 0;
            transfer->buffer[ii+1] = 0;
            if (printit)
                std::cout << "0" << std::endl;
        }
        
        t = t+dt;
    }
    printit = false;
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

    if ( hackrf.initialize() )
    {
        hackrf.force_sample_rate( fs_hz );
        hackrf.tune( fc_hz );
        hackrf.set_txvga_gain( 0 );
        
        // Start receiving data
        hackrf.start_Tx( sample_block_cb_fn, NULL );
        
        sleep(10);
    }
    
    hackrf.cleanup();
    
    return 0;
}





