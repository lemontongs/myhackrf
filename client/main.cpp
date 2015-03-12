
#include <string>
#include <iostream>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>

#include "zhelpers.h"

// Interrupt handler. Sends quit message
void signal_handler(int s)
{
    std::cout << "Caught signal " << s << std::endl;

    zmq::context_t send_context(1);
    zmq::socket_t socket(send_context, ZMQ_REQ);
    socket.connect("tcp://192.168.1.174:5556");

    s_send(socket, std::string("quit"));
    //zmq::message_t message((void*)&string, sizeof(string), NULL);
    //socket.send(message);

    std::cout << "Quit message sent" << std::endl << std::flush;

    sleep(1);

    exit(1);
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
    zmq::context_t recv_context(1);
    zmq::socket_t socket(recv_context, ZMQ_SUB);
    socket.connect("tcp://192.168.1.174:5555");
    socket.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    // Receive data until ctrl-c is handled
    while (1)
    {
        zmq::message_t msg;
        socket.recv(&msg);
        uint8_t* data = static_cast<uint8_t*>(msg.data());
        std::cout << int(data[0]) << " " << int(data[1]) << " " << int(data[2]) << std::endl;
    }
}


