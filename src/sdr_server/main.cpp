
#include "fft.h"
#include "HackRFDevice.h"
#include "packet.pb.h"
#include "zhelpers.hpp"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <vector>

// Device driver
RFDevice* rf_device;
std::string sdr_type;
Packet_PacketType rx_mode = Packet_PacketType_FFT;

bool g_shutdown_requested = false;

void signal_handler(int s)
{
    printf("Caught signal %d\n",s);
    g_shutdown_requested = true;
}


//
// Callback function for rx samples
//
int sample_block_cb_fn(SampleChunk* samples, void* args)
{
    // send the packet out over the network
    zmq::socket_t * publisher = (zmq::socket_t *)args;
    
    Packet packet;
    if (rx_mode == Packet_PacketType_FFT)
    {
        packet = \
            utilities::fft( *samples,
                            2048,
                            rf_device->get_sample_rate(),
                            rf_device->get_center_freq() );
        
        packet.set_type(Packet_PacketType_FFT);
    }
    else
    {
        packet.set_type(Packet_PacketType_RAW);
        packet.set_num_samples( samples->size() );
        packet.set_num_bins( 0 );
        packet.set_mean_db( 0.0 );
        packet.set_peak_db( -9999.0 );
        packet.set_low_db( 9999.0 );
        packet.set_peak_bin_index( 0 );
        packet.clear_signal();
        for (int ii = 0; ii < samples->size(); ii++)
        {
            packet.add_signal((*samples).at(ii).real());
            packet.add_signal((*samples).at(ii).imag());
        }
    }

    packet.set_fc(rf_device->get_center_freq());
    packet.set_fs(rf_device->get_sample_rate());
    
    std::string data;
    packet.SerializeToString(&data);
    s_send( *publisher, data );
    
    return 0;
}

//
// Handle received message. Return false to exit main loop.
//
void process_messages( zmq::socket_t* comm_sock )
{
    while (!g_shutdown_requested)
    {
        std::string message = s_recv( *comm_sock );
        
        std::cout << "sdr_server: Received: '" << message << "'" << std::endl;

        std::vector<std::string> fields;
        std::string temp;
        std::stringstream s( message );
        while( s >> temp )
            fields.push_back( temp );

        if ( fields.size() < 1 )
        {
            s_send( *comm_sock, std::string("sdr_server: ERROR: invalid request") );
            continue;
        }
        
        // GET
        if ( fields[0] == "get-fc" )
        {
            std::stringstream ss;
            ss << rf_device->get_center_freq();
            s_send( *comm_sock, ss.str() );
            continue;
        }
        
        if ( fields[0] == "get-fs" )
        {
            std::stringstream ss;
            ss << rf_device->get_sample_rate();
            s_send( *comm_sock, ss.str() );
            continue;
        }
        if ( fields[0] == "get-rx-gain" )
        {
            std::stringstream ss;
            ss << rf_device->get_rx_gain();
            s_send( *comm_sock, ss.str() );
            continue;
        }
        
        
        // ACTIONS
        if ( fields[0] == "quit" )
        {
            s_send( *comm_sock, std::string("OK") );
            return;
        }
        
        if ( fields[0] == "set-fc" && fields.size() >= 2 )
        {
            std::istringstream ss(fields[1]);
            uint64_t new_fc_hz;
            if (!(ss >> new_fc_hz))
            {
                std::cout << "sdr_server: tune failed: '" << fields[1] << "'" << std::endl;
                s_send( *comm_sock, std::string("ERROR: invalid tune argument") );
            }
            else if ( rf_device->set_center_freq( new_fc_hz ) )
            {
                std::cout << "sdr_server: tuned to: " << new_fc_hz << std::endl;
                s_send( *comm_sock, std::string("OK") );
            }
            else
                s_send( *comm_sock, std::string("ERROR: tune failed") );
            continue;
        }
        
        if ( fields[0] == "set-fs" && fields.size() >= 2 )
        {
            std::istringstream ss(fields[1]);
            double new_fs_hz;
            if (!(ss >> new_fs_hz))
            {
                std::cout << "sdr_server: set-fs failed: '" << fields[1] << "'" << std::endl;
                s_send( *comm_sock, std::string("ERROR: invalid sample rate argument") );
            }
            else if ( rf_device->set_sample_rate( new_fs_hz ) )
            {
                std::cout << "sdr_server: sample rate set to: " << new_fs_hz << std::endl;
                s_send( *comm_sock, std::string("OK") );
            }
            else
                s_send( *comm_sock, std::string("ERROR: set-fs failed") );
            continue;
        }
        
        if ( fields[0] == "set-rx-gain" && fields.size() >= 2 )
        {
            std::istringstream ss(fields[1]);
            uint64_t new_rx_gain;
            if (!(ss >> new_rx_gain))
            {
                std::cout << "sdr_server: set-rx-gain failed: '" << fields[1] << "'" << std::endl;
                s_send( *comm_sock, std::string("ERROR: invalid rx gain argument") );
            }
            else if ( rf_device->set_rx_gain( new_rx_gain ) )
            {
                std::cout << "sdr_server: rx gain set to: " << new_rx_gain << std::endl;
                s_send( *comm_sock, std::string("OK") );
            }
            else
                s_send( *comm_sock, std::string("ERROR: set-rx-gain failed") );
            continue;
        }

        if ( fields[0] == "set-rx-mode" && fields.size() >= 2 )
        {
            std::istringstream ss(fields[1]);
            std::string new_rx_mode;
            if (!(ss >> new_rx_mode))
            {
                std::cout << "sdr_server: set-rx-mode failed: '" << fields[1] << "'" << std::endl;
                s_send( *comm_sock, std::string("ERROR: invalid rx mode argument") );
            }
            else
            {
                rx_mode = Packet_PacketType_FFT;
                if (new_rx_mode == "iq")
                    rx_mode = Packet_PacketType_RAW;

                std::cout << "sdr_server: rx mode set to: " << new_rx_mode << "  " << rx_mode << std::endl;
                s_send( *comm_sock, std::string("OK") );
            }
            continue;
        }
    }
}

//
// Parse arguments
//
void parse_args(int argc, char* argv[])
{
    for (int aa = 0; aa < argc; aa++)
    {
        std::string arg( argv[aa] );

        if ( arg == "-h" || arg == "--help")
        {
            std::cout << "Usage: " << argv[0] << " -t <sdr type>" << std::endl;
            std::cout << "    -t <SDR type>    hackrf or rtlsdr" << std::endl;
        }
        else if ( arg == "-t" )
        {
            if (aa + 1 >= argc)
            {
                std::cout << "Missing type!" << std::endl;
                exit(1);
            }
            
            // Type argument
            std::istringstream ss(argv[aa+1]);
            if (!(ss >> sdr_type))
            {
                std::cout << "ERROR: Invalid type argument! '" << argv[aa+1] << "'" << std::endl;
                exit(1);
            }
            
            aa += 1;
        }
    }
    
    if ( sdr_type != "hackrf" && sdr_type != "rtlsdr" )
    {
        std::cout << "ERROR: no type specified!" << std::endl;
        exit(2);
    }
}

//
// MAIN
//
int main(int argc, char* argv[])
{
    parse_args(argc, argv);
    
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
    
    if ( sdr_type == "hackrf" )
    {
        rf_device = reinterpret_cast<RFDevice*>(new HackRFDevice);
    }
    if ( sdr_type == "rtlsdr" )
    {
//        rf_device = reinterpret_cast<RFDevice*>(new RTLSDRDevice);
    }

    if ( rf_device->initialize() )
    {
        rf_device->set_sample_rate( 8000000 );
        rf_device->set_center_freq( 433900000 );
        rf_device->set_rx_gain( 40 );
        
        // Start receiving data
        rf_device->start_Rx( sample_block_cb_fn, (void*)(&publisher) );
        
        // Listen for messages, this blocks until a "quit" is received
        process_messages( &receiver );
    }

    rf_device->cleanup();
    delete rf_device;
    
    return 0;
}





