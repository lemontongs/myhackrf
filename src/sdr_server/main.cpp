
#include "fft.h"
#include "pulse_detector.h"
#include "HackRFDevice.h"
#include "packet.pb.h"
#include "zhelpers.hpp"
#include "waveforms.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <deque>
#include <complex>
#include <mutex>

// Device driver
RFDevice* rf_device;
std::string sdr_type;
std::string sdr_tr_mode;
Packet_Header_PacketType rx_mode = Packet_Header_PacketType_PDW;

bool g_shutdown_requested = false;

void signal_handler(int s)
{
    printf("Caught signal %d\n",s);
    g_shutdown_requested = true;
}


struct Tx_Context
{
    std::mutex waveform_mutex;
    std::deque<std::complex<double>> waveform;
    int64_t waveform_index;
};
Tx_Context g_tx_context;

//
// Callback function for tx samples
//
int sample_block_tx_cb_fn(SampleChunk* samples, void* args)
{
    Tx_Context* tx_ctx_ptr = (Tx_Context*)args;
    
    // Exit early if nothing to transmit
    if (tx_ctx_ptr->waveform_index == -1)
    {
        //(*samples).resize(0);
        std::cout << "." << std::flush;
        return 0;
    }
    
    // Wait for the waveform buffer to be released
    //tx_ctx_ptr->waveform_mutex.lock();

    //std::cout << "Waveform size: " << tx_ctx_ptr->waveform.size() << std::endl;

    // Fill the sample buffer
    for (std::size_t ii = 0; ii < samples->size(); ii++)
    {
        if (tx_ctx_ptr->waveform_index >= tx_ctx_ptr->waveform.size())
        {
            tx_ctx_ptr->waveform_index = -1;
            break;
        }
        else
        {
            (*samples)[ii] = tx_ctx_ptr->waveform[tx_ctx_ptr->waveform_index];
            tx_ctx_ptr->waveform_index++;
        }
    }
    
    std::cout << tx_ctx_ptr->waveform_index << std::endl;

    //tx_ctx_ptr->waveform_mutex.unlock();

    return 0;
}

//
// Callback function for rx samples
//
int sample_block_rx_cb_fn(SampleChunk* samples, void* args)
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
        std::string message;
        try
        {
            message = s_recv( *comm_sock );
        }
        catch(const zmq::error_t& e)
        {
            std::cerr << e.what() << '\n';
            continue;
        }
        
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

        if ( fields[0] == "set-tx-gain" && fields.size() >= 2 )
        {
            std::istringstream ss(fields[1]);
            uint64_t new_tx_gain;
            if (!(ss >> new_tx_gain))
            {
                std::cout << "sdr_server: set-tx-gain failed: '" << fields[1] << "'" << std::endl;
                s_send( *comm_sock, std::string("ERROR: invalid tx gain argument") );
            }
            else if ( rf_device->set_tx_gain( new_tx_gain ) )
            {
                std::cout << "sdr_server: tx gain set to: " << new_tx_gain << std::endl;
                s_send( *comm_sock, std::string("OK") );
            }
            else
                s_send( *comm_sock, std::string("ERROR: set-tx-gain failed") );
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

        // set-waveform tone <pulse width s>
        if ( fields[0] == "set-waveform" && fields.size() >= 2 )
        {
            if ( fields[1] == "tone" && fields.size() == 5 )
            {
                double pulse_width_s;
                {
                    std::istringstream ss(fields[2]);
                    if (!(ss >> pulse_width_s))
                    {
                        std::cout << "sdr_server: set-waveform failed: '" << fields[2] << "'" << std::endl;
                        s_send( *comm_sock, std::string("ERROR: invalid set-waveform tone argument") );
                        continue;
                    }
                }

                double mag;
                {
                    std::istringstream ss(fields[3]);
                    if (!(ss >> mag))
                    {
                        std::cout << "sdr_server: set-waveform failed: '" << fields[3] << "'" << std::endl;
                        s_send( *comm_sock, std::string("ERROR: invalid set-waveform chirp argument") );
                        continue;
                    }
                }

                int32_t tone_offset_hz;
                {
                    std::istringstream ss(fields[4]);
                    if (!(ss >> tone_offset_hz))
                    {
                        std::cout << "sdr_server: set-waveform failed: '" << fields[4] << "'" << std::endl;
                        s_send( *comm_sock, std::string("ERROR: invalid set-waveform tone argument") );
                        continue;
                    }
                }
                
                // Wait for the waveform buffer to be released
                g_tx_context.waveform_mutex.lock();
                createTone(g_tx_context.waveform, rf_device->get_sample_rate(), tone_offset_hz, pulse_width_s, mag);
                g_tx_context.waveform_index = 0;
                g_tx_context.waveform_mutex.unlock();
                s_send( *comm_sock, std::string("OK") );
                continue;
            }
            else
            {
                s_send( *comm_sock, std::string("ERROR: invalid set-waveform tone arguments") );
                continue;
            }
            
            if ( fields[1] == "chirp" && fields.size() == 6 )
            {
                double pulse_width_s;
                {
                    std::istringstream ss(fields[2]);
                    if (!(ss >> pulse_width_s))
                    {
                        std::cout << "sdr_server: set-waveform failed: '" << fields[2] << "'" << std::endl;
                        s_send( *comm_sock, std::string("ERROR: invalid set-waveform chirp argument") );
                        continue;
                    }
                }

                double mag;
                {
                    std::istringstream ss(fields[3]);
                    if (!(ss >> mag))
                    {
                        std::cout << "sdr_server: set-waveform failed: '" << fields[3] << "'" << std::endl;
                        s_send( *comm_sock, std::string("ERROR: invalid set-waveform chirp argument") );
                        continue;
                    }
                }

                int32_t chirp_start_hz;
                {
                    std::istringstream ss(fields[4]);
                    if (!(ss >> chirp_start_hz))
                    {
                        std::cout << "sdr_server: set-waveform failed: '" << fields[4] << "'" << std::endl;
                        s_send( *comm_sock, std::string("ERROR: invalid set-waveform chirp argument") );
                        continue;
                    }
                }
                
                int32_t chirp_width_hz;
                {
                    std::istringstream ss(fields[5]);
                    if (!(ss >> chirp_width_hz))
                    {
                        std::cout << "sdr_server: set-waveform failed: '" << fields[5] << "'" << std::endl;
                        s_send( *comm_sock, std::string("ERROR: invalid set-waveform chirp argument") );
                        continue;
                    }
                }
                
                // Wait for the waveform buffer to be released
                g_tx_context.waveform_mutex.lock();
                createChirp(g_tx_context.waveform, rf_device->get_sample_rate(), chirp_start_hz, chirp_width_hz, pulse_width_s, mag);
                g_tx_context.waveform_index = 0;
                g_tx_context.waveform_mutex.unlock();
                s_send( *comm_sock, std::string("OK") );
                continue;
            }
            else
            {
                s_send( *comm_sock, std::string("ERROR: invalid set-waveform chirp arguments") );
                continue;
            }
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
            std::cout << "    [-t <SDR type>]    [hackrf] or rtlsdr" << std::endl;
            std::cout << "    [-m <SDR mode>]    [rx] or tx" << std::endl;
            exit(0);
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
        else if ( arg == "-m" )
        {
            if (aa + 1 >= argc)
            {
                std::cout << "Missing mode!" << std::endl;
                exit(1);
            }
            
            // Type argument
            std::istringstream ss(argv[aa+1]);
            if (!(ss >> sdr_tr_mode))
            {
                std::cout << "ERROR: Invalid mode argument! '" << argv[aa+1] << "'" << std::endl;
                exit(1);
            }
            
            aa += 1;
        }
    }
    
    // Defaults
    if ( sdr_type != "hackrf" && sdr_type != "rtlsdr" )
    {
        sdr_type = "hackrf";
        std::cout << "INFO: no type specified, defaulting to " << sdr_type << std::endl;
    }
    if ( sdr_tr_mode != "rx" && sdr_tr_mode != "tx" )
    {
        sdr_tr_mode = "rx";
        std::cout << "INFO: no mode specified, defaulting to " << sdr_tr_mode << std::endl;
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
        rf_device->set_tx_gain( 10 );

        if ( sdr_tr_mode == "rx" )
        {
            // Start receiving data
            rf_device->start_Rx( sample_block_rx_cb_fn, (void*)(&publisher) );
        }
        else
        {
            // Start transmitting data
            rf_device->start_Tx( sample_block_tx_cb_fn, (void*)(&g_tx_context) );
        }

        // Listen for messages, this blocks until a "quit" is received
        process_messages( &receiver );

        rf_device->stop_Rx();
        rf_device->stop_Tx();
    }

    rf_device->cleanup();
    delete rf_device;
    
    return 0;
}





