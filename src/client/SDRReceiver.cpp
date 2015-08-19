
#include "SDRReceiver.h"

#include <iostream>

SDRReceiver::SDRReceiver()
{
    m_isRunning = false;
    m_isInitialized = false;
    
    m_data_port = "5555";
    m_comm_port = "5556";
    
    m_data_target = "tcp://localhost:" + m_data_port;
    m_comm_target = "tcp://localhost:" + m_comm_port;

    // Setup the comm bits
    m_comm_context = new zmq::context_t(1);
    m_comm_socket = new zmq::socket_t(*m_comm_context, ZMQ_REQ);
}

SDRReceiver::~SDRReceiver()
{
    delete m_comm_socket;
    delete m_comm_context;
}

void SDRReceiver::setIP(std::string ipAddress)
{
    m_data_target = "tcp://" + ipAddress + ":" + m_data_port;
    m_comm_target = "tcp://" + ipAddress + ":" + m_comm_port;
}

void* thread_func(void* args)
{
    std::cout << "Rx thread started" << std::endl;
    
    SDRReceiver* sdr_obj = (SDRReceiver*)args;
    
    sdr_obj->receivePackets();
}


bool SDRReceiver::initialize(sdr_receive_callback_t callback, void* callback_args)
{
    if(!m_isInitialized)
    {
        try
        {
            m_comm_socket->connect( m_comm_target.c_str() );
        }
        catch (zmq::error_t &e)
        {
            std::cout << e.what() << std::endl;
            m_isInitialized = false;
            return false;
        }
        
        m_callback = callback;
        m_callback_args = callback_args;
        
        pthread_create( &m_thread_context, NULL, thread_func, (void*)this );
        
        m_isInitialized = true;
        return true;
    }
    return false;
}

void SDRReceiver::stop()
{
    m_isRunning = false;

    while(m_isInitialized)
        sleep(1);

    m_comm_socket->close();
}

void SDRReceiver::receivePackets()
{
    // Setup the data receiver
    zmq::context_t recv_context(1);
    zmq::socket_t socket(recv_context, ZMQ_SUB);
    socket.connect( m_data_target.c_str() );
    socket.setsockopt(ZMQ_SUBSCRIBE, "", 0);
    int timeout = 100; // ms
    socket.setsockopt(ZMQ_RCVTIMEO, &timeout, sizeof(timeout));

    m_isRunning = true;
    while (m_isRunning)
    {
        zmq::message_t msg;
        if ( socket.recv(&msg) )
        {
            char* data_p = static_cast<char*>(msg.data());
            std::string data( data_p, msg.size() );
            
            Packet p;
            p.ParseFromString(data);
            
            m_callback( p, m_callback_args );
        }
    }

    m_isInitialized = false;
}


void SDRReceiver::tune(uint64_t fc_hz)
{
    std::stringstream ss;
    ss << "set-fc " << fc_hz;
    s_send(*m_comm_socket, ss.str());
    std::string response = s_recv(*m_comm_socket);
}

void SDRReceiver::setSampleRate(double fs_hz)
{
    std::stringstream ss;
    ss << "set-fs " << fs_hz;
    s_send(*m_comm_socket, ss.str());
    std::string response = s_recv(*m_comm_socket);
}

double SDRReceiver::getSampleRate()
{
    s_send(*m_comm_socket, std::string("get-fs"));
    std::stringstream ss(s_recv(*m_comm_socket));
    double fs_hz;
    ss >> fs_hz;
    return fs_hz;
}

uint64_t SDRReceiver::getCenterFrequency()
{
    s_send(*m_comm_socket, std::string("get-fc"));
    std::stringstream ss(s_recv(*m_comm_socket));
    uint64_t fc_hz;
    ss >> fc_hz;
    return fc_hz;
}


