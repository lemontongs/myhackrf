
#include "HackRFDevice.h"
#include "Socket.h"

#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <string.h>
#include <vector>

// Device driver
HackRFDevice hackrf;

uint64_t fc_hz      = 5981e6; // center freq
double   fs_hz      = 20e6;   // sample rate
uint32_t lna_gain   = 40;     // 0-40 in steps of 8
uint8_t  amp_enable = 0;
uint32_t txvga_gain = 0;


bool transfer_ok = true;


void signal_handler(int s)
{
    printf("Caught signal %d\n",s);
    hackrf.cleanup();
    exit(0);
}


//
// Callback function for rx samples
//
int sample_block_cb_fn(hackrf_transfer* transfer)
{
    // send the packet out over the network
    Socket* client = (Socket*)transfer->rx_ctx;
    
    if ( ! client->is_valid() )
    {
        printf("Invalid socket\n" );
        transfer_ok = false;
        return 0;
    }
    
    
    for (int ii = 0; ii < 10; ii+=2)
    {
        std::cout << int(transfer->buffer[ii]);
    }
    std::cout << std::endl << std::flush;
    
    if ( ! client->send( transfer->buffer, transfer->valid_length ) )
    {
        int e = errno;
        if ( e == EAGAIN )
        {
            std::cout << "U" << std::flush;
        }
        else
        {
            std::cout << "Could not send to client: (" << e << ") " << strerror(e) << std::endl;
            transfer_ok = false;
        }
        return 0;
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

      
    if ( hackrf.initialize( "22be1" /*"23ec7"*/ ) )
    {
        hackrf.tune( 5981e6 );
        hackrf.set_sample_rate( fs_hz );
        hackrf.set_lna_gain( lna_gain );
        hackrf.set_amp_enable( amp_enable );
        hackrf.set_txvga_gain( txvga_gain );
        
        printf("Starting server...\n");
        // Data stream interface
        Socket server;
        
        if ( ! server.create() )
        {
            int e = errno;
            std::cout << "Could not create server socket: error: (" << e << ") " << strerror(e);
            hackrf.cleanup();
            return 1;
        }

        if ( ! server.bind( 30000 ) )
        {
            int e = errno;
            std::cout << "Could not bind to port: error: (" << e << ") " << strerror(e);
            hackrf.cleanup();
            return 1;
        }

        if ( ! server.listen() )
        {
            int e = errno;
            std::cout << "Could not listen to socket: error: (" << e << ") " << strerror(e);
            hackrf.cleanup();
            return 1;
        }
        
        
        while (1)
        {
            printf("Waiting for client...\n");
            Socket client;
            
            if ( ! server.accept( client ) )
            {
                int e = errno;
                std::cout << "Could not accept client: error: (" << e << ") " << strerror(e);
                continue;
            }
            printf("Got client!\n");
            
            client.set_non_blocking( true );
            
            transfer_ok = true;
            
            // Start receiving data
            if ( ! hackrf.start_Rx( sample_block_cb_fn, (void*)(&client) ) )
            {
                std::cout << "HackRF could not enter Rx mode" << std::endl;
                hackrf.stop_Rx();
                continue;
            }
            
            // Wait for ctrl+c or client disconnect
            printf("Waiting for disconnect...\n");
            while ( transfer_ok )
                sleep(0.1);
            
            hackrf.stop_Rx();
        }
    }
    
    return 0;
}





