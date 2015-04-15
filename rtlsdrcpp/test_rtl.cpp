
#include <signal.h>
#include <iostream> // cout
#include <stdlib.h> // exit
#include <math.h>   // sin cos M_PI
#include <unistd.h> // sleep

#include "RTLSDRDevice.h"

uint64_t fc_hz = 462610000; // center freq
double   fs_hz = 1000000;   // sample rate

void signal_handler(int s)
{
    exit(1);
}


//
// Callback function for Rx samples
//
void rx_cb_fn(unsigned char *buf, uint32_t len, void *ctx)
{
    std::cout << "got a packet! (" << len << ")" << std::endl;
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
    
    RTLSDRDevice rtlsdr;
    
    bool rv = rtlsdr.initialize();
    rv = rtlsdr.start_Rx( rx_cb_fn, NULL );
    
    sleep(1);
    
    std::cout << "got here" << std::endl;
    
    return 0;
}





