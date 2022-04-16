
#include "fft.h"
#include "pulse_detector.h"
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
Packet_Header_PacketType rx_mode = Packet_Header_PacketType_PDW;

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
    static uint64_t sample_count = 0;
    
    Packet packet;
    Packet_Header* header = packet.mutable_header();
    bool send = true;

    header->set_type(rx_mode);
    header->set_fc(uint32_t(rf_device->get_center_freq()));
    header->set_fs(uint32_t(rf_device->get_sample_rate()));

    switch (rx_mode)
    {
        case Packet_Header_PacketType_FFT:
        {
            FFT_Packet* fft_packet = packet.mutable_fft_packet();
            utilities::fft( fft_packet,
                            *samples,
                            2048,
                            rf_device->get_sample_rate(),
                            rf_device->get_center_freq() );
            
            break;
        }
        
        case Packet_Header_PacketType_IQ:
        {
            IQ_Packet* iq_packet = packet.mutable_iq_packet();
            for (std::size_t ii = 0; ii < samples->size(); ii++)
            {
                iq_packet->add_signal((*samples).at(ii).real());
                iq_packet->add_signal((*samples).at(ii).imag());
            }
            break;
        }
        
        case Packet_Header_PacketType_PDW:
        {
            PDW_Packet* pdw_packet = packet.mutable_pdw_packet();
            utilities::detectPulses(pdw_packet, 
                                    samples, 
                                    sample_count, 
                                    rf_device->get_sample_rate(),
                                    5.0);
            
            // If no PDWs then don't send
            if (pdw_packet->pdws_size() < 1)
                send = false;
            break;
        }

        default:
            return 0;
    }
    
    sample_count += samples->size();

    if (send)
    {
        //std::cout << "Sending: " << header->type() << std::endl;
        std::string data;
        packet.SerializeToString(&data);
        s_send( *publisher, data );
    }

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
                if (new_rx_mode.compare("iq") == 0)
                    rx_mode = Packet_Header_PacketType_IQ;
                else if (new_rx_mode.compare("pdw") == 0)
                    rx_mode = Packet_Header_PacketType_PDW;
                else if (new_rx_mode.compare("fft") == 0)
                    rx_mode = Packet_Header_PacketType_FFT;
                else
                {
                    std::cout << "sdr_server: invalid rx mode! '" << new_rx_mode << "'  current mode: " << rx_mode << std::endl;
                    s_send( *comm_sock, std::string("ERROR: invalid rx mode argument") );
                    continue;
                }
                
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
        
        // Start receiving data
        rf_device->start_Rx( sample_block_cb_fn, (void*)(&publisher) );
        
        // Listen for messages, this blocks until a "quit" is received
        process_messages( &receiver );

        rf_device->stop_Rx();
    }

    rf_device->cleanup();
    delete rf_device;
    
    return 0;
}





