
#include "fft.h"
#include "packet.pb.h"
#include "RTLSDRDevice.h"
#include "zhelpers.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <vector>

// Device driver
RTLSDRDevice rtlsdr;

void signal_handler(int s)
{
    printf("Caught signal %d\n",s);
    rtlsdr.cleanup();
    exit(0);
}


//
// Callback function for rx samples
//
void sample_block_cb_fn(unsigned char *buf, uint32_t len, void *ctx)
{
    for (int ii = 0; ii < len; ii++)
        buf[ii] += 128;
    
    // send the packet out over the network
    zmq::socket_t * publisher = (zmq::socket_t *)ctx;
    
    Packet packet = \
        utilities::fft( buf, \
                        len, \
                        2048, \
                        rtlsdr.get_sample_rate(), \
                        rtlsdr.get_center_frequency() );
        
    packet.set_type(Packet_PacketType_FFT);
    
    std::string data;
    packet.SerializeToString(&data);
    s_send( *publisher, data );
}


//
// Handle received message. Return false to exit main loop.
//
bool process_messages( zmq::socket_t & comm_sock )
{
    while (1)
    {
        std::string message = s_recv( comm_sock );
        
        std::cout << "rtlsdr_server: Received: '" << message << "'" << std::endl;

        std::vector<std::string> fields;
        std::string temp;
        std::stringstream s( message );
        while( s >> temp )
            fields.push_back( temp );

        if ( fields.size() < 1 )
        {
            s_send( comm_sock, std::string("rtlsdr_server: ERROR: invalid request") );
            continue;
        }
        
        // GET
        if ( fields[0] == "get-fc" )
        {
            std::stringstream ss;
            ss << rtlsdr.get_center_frequency();
            s_send( comm_sock, ss.str() );
            continue;
        }
        
        if ( fields[0] == "get-fs" )
        {
            std::stringstream ss;
            ss << rtlsdr.get_sample_rate();
            s_send( comm_sock, ss.str() );
            continue;
        }
        if ( fields[0] == "get-lna" )
        {
            std::stringstream ss;
            ss << rtlsdr.get_lna_gain();
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
                std::cout << "rtlsdr_server: tune failed: '" << fields[1] << "'" << std::endl;
                s_send( comm_sock, std::string("ERROR: invalid tune argument") );
            }
            else if ( rtlsdr.tune( new_fc_hz ) )
            {
                std::cout << "rtlsdr_server: tuned to: " << new_fc_hz << std::endl;
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
                std::cout << "rtlsdr_server: set-fs failed: '" << fields[1] << "'" << std::endl;
                s_send( comm_sock, std::string("ERROR: invalid sample rate argument") );
            }
            else if ( rtlsdr.tune( new_fs_hz ) )
            {
                std::cout << "rtlsdr_server: sample rate set to: " << new_fs_hz << std::endl;
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
                std::cout << "rtlsdr_server: set-lna failed: '" << fields[1] << "'" << std::endl;
                s_send( comm_sock, std::string("ERROR: invalid lna gain argument") );
            }
            else if ( rtlsdr.tune( new_lna_gain ) )
            {
                std::cout << "rtlsdr_server: lna gain set to: " << new_lna_gain << std::endl;
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
    
    if ( rtlsdr.initialize() )
    {
        rtlsdr.set_sample_rate( 1000000 );
        rtlsdr.tune( 433900000 );
        rtlsdr.set_lna_gain( 480 );
        
        // Start receiving data
        rtlsdr.start_Rx( sample_block_cb_fn, (void*)(&publisher) );
        
        // Listen for messages, this blocks until a "quit" is received
        process_messages( receiver );
    }
    
    return 0;
}





