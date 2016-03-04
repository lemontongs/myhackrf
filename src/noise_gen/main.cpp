
#include "HackRFDevice.h"
#include "firdes.h"
#include "fir_filter.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

// Device driver
HackRFDevice hackrf;

using namespace gr::filter;

void signal_handler(int s)
{
    printf("Caught signal %d\n",s);
    hackrf.cleanup();
    exit(0);
}


void write_sig_to_file(const char * filename, const char * varname, std::vector<float> sig)
{
    std::ofstream ofs( filename, std::ofstream::out );
    ofs << varname << std::endl;
    for (int ii = 0; ii < sig.size(); ii++)
    {
        ofs << sig[ii] << std::endl;
    }
    ofs.close();
} 

bool print_once = true;

//
// Callback function for tx samples
//
int sample_block_cb_fn(hackrf_transfer* transfer)
{
    std::vector<float> rand_sig;
    
    for (int ii = 0; ii < transfer->valid_length/2; ii++)
    {
        float val = (float(rand() % 256)-128.0)/128.0;
        
        rand_sig.push_back( val );
    }

    std::vector< float > taps = 
        firdes::band_pass(     1.0, // gain
                           1000000, // fs
                            200000, // low cutoff
                            300000, // high cutoff
                             30000);// transition width
    
    
    kernel::fir_filter_fff filter_obj(0,taps);



    filter_obj.filter( rand_sig.data() );
    
    //std::cout << "filter: " << taps.size() << std::endl;
    std::vector< float > sig_8;
    for (int ii = 0; ii < transfer->valid_length; ii+=2)
    {
        uint8_t i8 = uint8_t(rand_sig[ii/2]);
        uint8_t q8 = uint8_t(rand_sig[ii/2]);
        
        sig_8.push_back(i8);
        
        transfer->buffer[ii+0] = i8;
        transfer->buffer[ii+1] = q8;
    }
    if (print_once)
    {
        write_sig_to_file("sig8.txt","sig8",sig_8);
        write_sig_to_file("sig.txt","sig",rand_sig);
        write_sig_to_file("filter.txt","filter",taps);
        std::cout << "Done" << std::endl;
        print_once=false;
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

    if ( hackrf.initialize() )
    {
        hackrf.set_sample_rate( 1e6 );
        hackrf.tune( 2460e6 );
        hackrf.set_txvga_gain( 47 );  // range 0-47 step 1db 
        
        // Start receiving data
        hackrf.start_Tx( sample_block_cb_fn, NULL );
        
        while (true)
        {
            for ( uint64_t fc = 2402e6; fc < 2480e6; fc += 20e6)
            {
            //    hackrf.tune( fc );
            //    std::cout << "Tuning to: " << fc << std::endl;
                sleep(0.1);
            }
        }
    }
    
    return 0;
}





