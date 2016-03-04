
#include "HackRFDevice.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <iostream>
#include <vector>

#define PI 3.14159265
#define MESSAGE "Hello World! "

char msg[] = MESSAGE;
bool bits[sizeof(msg)*8];

HackRFDevice hackrf;

uint64_t fc_hz      = 910000000; // center freq
double   fs_hz      = 8000000;   // sample rate
uint32_t lna_gain   = 0;
uint8_t  amp_enable = 0;
uint32_t txvga_gain = 27;


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
bool print_signal = true;
double df = 1000;
int bit_index = 0;

int sample_block_cb_fn(hackrf_transfer* transfer)
{
    bool bit = bits[bit_index++];
    
    if (bit_index >= sizeof(bits))
        bit_index = 0;
    
    for (int ii = 0; ii < transfer->valid_length; ii+=2)
    {
        // Manchester encode
        bool first_half = ( ii < ( transfer->valid_length / 2 ) );
        
        // High bit means transition low to high
        if ( bit )
            if ( first_half )
                df = -1000;
            else
                df = 1000;
        // Low bit means transition high to low
        else
            if ( first_half )
                df = 1000;
            else
                df = -1000;
        
        double i = 128.0 * cos( 2.0 * PI * df * t );  // I
        double q = 128.0 * sin( 2.0 * PI * df * t );  // Q
        
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

    // Convert message into bits
    printf("String:  %s\nHex:     ", msg);
    for (int ii = 0; ii < sizeof(msg); ii++ )
    {
        printf("%d ", msg[ii]);
    }
    printf("\nBinary:  ");
    
    for (int ii = 0; ii < sizeof(msg); ii++ )
    {
        for ( int jj = 0; jj < 8; jj++ )
        {
            bits[(ii*8) + jj] = (msg[ii] & (0x80 >> jj)) > 0;
            printf("%d ", bits[(ii*8) + jj]?1:0);
        }
    }
    printf("\n");
    
    if ( ! hackrf.initialize() )
    {
        std::cout << "Error initializing hackrf device!" << std::endl;
        return 1;
    }
    
    hackrf.tune( fc_hz );
    hackrf.set_sample_rate( fs_hz );
    hackrf.set_lna_gain( lna_gain );
    hackrf.set_amp_enable( amp_enable );
    hackrf.set_txvga_gain( txvga_gain );
    hackrf.start_Tx( sample_block_cb_fn, (void*)(NULL) );

    //  Wait a while
    sleep(300);
    
    return 0;
}





