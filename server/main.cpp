
#include "HackRFDevice.h"
#include "zhelpers.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <vector>

// Device driver
HackRFDevice hackrf;

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
    zmq::socket_t * publisher = (zmq::socket_t *)transfer->rx_ctx;
    zmq::message_t message((void*)transfer->buffer, transfer->valid_length, NULL);
    publisher->send(message);

    return 0;
}

//
// Handle received message. Return false to exit main loop.
//
bool process_messages( zmq::socket_t & comm_sock )
{
    while (1)
    {
        std::string message = s_recv( comm_sock );
        
        std::cout << "Received: '" << message << "'" << std::endl;

        std::vector<std::string> fields;
        std::string temp;
        std::stringstream s( message );
        while( s >> temp )
            fields.push_back( temp );

        if ( fields.size() < 1 )
        {
            s_send( comm_sock, std::string("ERROR: invalid request") );
            continue;
        }
        
        // GET
        if ( fields[0] == "get-fc" )
        {
            std::stringstream ss;
            ss << hackrf.get_center_frequency();
            s_send( comm_sock, ss.str() );
            continue;
        }
        
        if ( fields[0] == "get-fs" )
        {
            std::stringstream ss;
            ss << hackrf.get_sample_rate();
            s_send( comm_sock, ss.str() );
            continue;
        }
        if ( fields[0] == "get-lna" )
        {
            std::stringstream ss;
            ss << hackrf.get_lna_gain();
            s_send( comm_sock, ss.str() );
            continue;
        }
        
        
        // ACTIONS
        if ( fields[0] == "quit" )
        {
            s_send( comm_sock, std::string("OK") );
            return false;
        }
        
        if ( fields[0] == "set-fc" && fields.size() >= 2 )
        {
            std::istringstream ss(fields[1]);
            uint64_t new_fc_hz;
            if (!(ss >> new_fc_hz))
            {
                std::cout << "tune failed: '" << fields[1] << "'" << std::endl;
                s_send( comm_sock, std::string("ERROR: invalid tune argument") );
            }
            else if ( hackrf.tune( new_fc_hz ) )
            {
                std::cout << "tuned to: " << new_fc_hz << std::endl;
                s_send( comm_sock, std::string("OK") );
            }
            else
                s_send( comm_sock, std::string("ERROR: tune failed") );
            continue;
        }
        
        if ( fields[0] == "set-fs" && fields.size() >= 2 )
        {
            std::istringstream ss(fields[1]);
            uint64_t new_fs_hz;
            if (!(ss >> new_fs_hz))
            {
                std::cout << "set-fs failed: '" << fields[1] << "'" << std::endl;
                s_send( comm_sock, std::string("ERROR: invalid sample rate argument") );
            }
            else if ( hackrf.tune( new_fs_hz ) )
            {
                std::cout << "sample rate set to: " << new_fs_hz << std::endl;
                s_send( comm_sock, std::string("OK") );
            }
            else
                s_send( comm_sock, std::string("ERROR: set-fs failed") );
            continue;
        }
        
        if ( fields[0] == "set-lna" && fields.size() >= 2 )
        {
            std::istringstream ss(fields[1]);
            uint64_t new_lna_gain;
            if (!(ss >> new_lna_gain))
            {
                std::cout << "set-lna failed: '" << fields[1] << "'" << std::endl;
                s_send( comm_sock, std::string("ERROR: invalid lna gain argument") );
            }
            else if ( hackrf.tune( new_lna_gain ) )
            {
                std::cout << "lna gain set to: " << new_lna_gain << std::endl;
                s_send( comm_sock, std::string("OK") );
            }
            else
                s_send( comm_sock, std::string("ERROR: set-lna failed") );
            continue;
        }
    }
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

    // Data stream interface
    zmq::context_t pub_context(1);
    zmq::socket_t publisher(pub_context, ZMQ_PUB);
    publisher.bind("tcp://*:5555");

    // Command interface
    zmq::context_t recv_context(1);
    zmq::socket_t receiver(recv_context, ZMQ_REP);
    receiver.bind("tcp://*:5556");
    
    if ( hackrf.initialize() )
    {
        hackrf.force_sample_rate( 1000000 );
        hackrf.tune( 433900000 );
        hackrf.set_lna_gain( 40 );
        
        // Start receiving data
        hackrf.start_Rx( sample_block_cb_fn, (void*)(&publisher) );
        
        // Listen for messages, this blocks until a "quit" is received
        process_messages( receiver );
    }
    
    return 0;
}





