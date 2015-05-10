
#include "SDRReceiver.h"
#include "packet.pb.h"

#include <iostream>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>


// Interrupt handler. Sends quit message
void signal_handler(int s)
{
    std::cout << "Caught signal " << s << std::endl;
    exit(1);
}

void callback( Packet &p, void* args )
{
    std::cout << "Got a packet! (" << p.signal_size() << ")" << std::endl;
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

    // Setup the data receiver
    SDRReceiver rcv;
    rcv.initialize( callback, NULL );
    rcv.tune( 101100000 );
    
    sleep(5);
}


