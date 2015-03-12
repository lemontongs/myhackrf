#include "receiver.h"

#include <QByteArray>
#include <QThread>

Receiver::Receiver()
{
    isRunning = false;
    data_target = "tcp://192.168.1.174:5555";
    comm_target = "tcp://192.168.1.174:5556";

    // Setup the data receiver
    comm_context = new zmq::context_t(1);
    comm_socket = new zmq::socket_t(*comm_context, ZMQ_REQ);
    comm_socket->connect(comm_target.toLocal8Bit().data());
}

void Receiver::stop()
{
    isRunning = false;
}

void Receiver::receivePackets()
{
    // Setup the data receiver
    zmq::context_t recv_context(1);
    zmq::socket_t socket(recv_context, ZMQ_SUB);
    socket.connect(data_target.toLocal8Bit().data());
    socket.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    isRunning = true;
    while (isRunning)
    {
        zmq::message_t msg;
        socket.recv(&msg);   //todo: add timeout so the thread can die cleanly when there is no messages
        char* data = static_cast<char*>(msg.data());
        QByteArray ba(data, msg.size());

        emit newPacket(ba);
    }

    // Send quit message

}


void Receiver::tune(int hertz)
{
    QString command = QString("tune ") + QString::number(hertz);
    s_send(*comm_socket, command.toStdString());
}
