#ifndef SDRRECEIVER_H
#define SDRRECEIVER_H

#include "zhelpers.h"
#include "packet.pb.h"

#include <pthread.h>
#include <stdint.h>
#include <string>

typedef void(*sdr_receive_callback_t)( Packet &p, void* args );

class SDRReceiver
{
public:
    SDRReceiver();
    ~SDRReceiver();
    bool initialize(sdr_receive_callback_t callback, void* callback_args = NULL);
    void stop();
    void setIP(std::string ipAddress);
    void tune(uint64_t fc_hz);
    void setSampleRate(double fs_hz);
    double   getSampleRate();
    uint64_t getCenterFrequency();

private:
    void receivePackets();

    pthread_t m_thread_context;
    sdr_receive_callback_t m_callback;
    void*                  m_callback_args;
    
    bool m_isRunning;
    bool m_isInitialized;
    std::string m_data_port;
    std::string m_comm_port;
    std::string m_data_target;
    std::string m_comm_target;

    zmq::context_t * m_comm_context;
    zmq::socket_t  * m_comm_socket;
    
    friend void* thread_func(void* args);
};

#endif // SDRRECEIVER_H
